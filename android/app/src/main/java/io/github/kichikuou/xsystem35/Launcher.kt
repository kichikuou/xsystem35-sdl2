/* Copyright (C) 2020 <KichikuouChrome@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
*/
package io.github.kichikuou.xsystem35

import android.annotation.SuppressLint
import android.os.Build
import android.os.Handler
import android.os.Message
import android.util.Log
import java.io.*
import java.lang.StringBuilder
import java.nio.charset.Charset
import java.util.*
import java.util.zip.ZipEntry
import java.util.zip.ZipInputStream
import java.util.zip.ZipOutputStream

private var gLauncher: Launcher? = null

interface LauncherObserver {
    fun onGameListChange()
    fun onInstallProgress(path: String)
    fun onInstallSuccess(path: File)
    fun onInstallFailure(msgId: Int)
}

private const val SAVE_DIR = "save"
private const val PROGRESS = 0
private const val SUCCESS = 1
private const val FAILURE = 2

class Launcher private constructor(private val rootDir: File) {
    companion object {
        const val TITLE_FILE = "title.txt"
        const val PLAYLIST_FILE = "playlist.txt"

        fun getInstance(rootDir: File): Launcher {
            if (gLauncher == null) {
                gLauncher = Launcher(rootDir)
            }
            return gLauncher!!
        }

        fun updateGameList() {
            gLauncher?.updateGameList()
        }
    }

    data class Entry(val path: File, val title: String, val timestamp: Long)
    val games = arrayListOf<Entry>()
    val titles: List<String>
        get() = games.map(Entry::title)
    var observer: LauncherObserver? = null
    var isInstalling = false
        private set

    init {
        updateGameList()
    }

    fun install(input: InputStream) {
        val dir = createDirForGame()
        @SuppressLint("HandlerLeak")
        val handler = object : Handler() {
            override fun handleMessage(msg: Message) {
                when (msg.what) {
                    PROGRESS -> {
                        observer?.onInstallProgress(msg.obj as String)
                    }
                    SUCCESS -> {
                        isInstalling = false
                        observer?.onInstallSuccess(msg.obj as File)
                    }
                    FAILURE -> {
                        isInstalling = false
                        observer?.onInstallFailure(msg.obj as Int)
                    }
                }
            }
        }
        val t = Thread {
            extractFiles(input, dir, handler)
        }
        t.start()
        isInstalling = true
    }

    fun uninstall(id: Int) {
        games[id].path.deleteRecursively()
        games.removeAt(id)
        observer?.onGameListChange()
    }

    private fun updateGameList() {
        var saveDirFound = false
        games.clear()
        for (path in rootDir.listFiles()) {
            if (!path.isDirectory)
                continue
            if (path.name == SAVE_DIR) {
                saveDirFound = true
                continue
            }
            try {
                val titleFile = File(path, TITLE_FILE)
                val title = titleFile.readText()
                games.add(Entry(path, title, titleFile.lastModified()))
            } catch (e: IOException) {
                // Incomplete game installation. Delete it.
                path.deleteRecursively()
            }
        }
        games.sortByDescending(Entry::timestamp)
        if (!saveDirFound) {
            File(rootDir, SAVE_DIR).mkdir()
        }
    }

    private fun createDirForGame(): File {
        var i = 0
        while (true) {
            val f = File(rootDir, i++.toString())
            if (!f.exists() && f.mkdir()) {
                return f
            }
        }
    }

    // Throws IOException
    fun exportSaveData(output: OutputStream) {
        ZipOutputStream(output.buffered()).use { zip ->
            for (path in File(rootDir, SAVE_DIR).listFiles()) {
                if (path.isDirectory || path.name.endsWith(".asd."))
                    continue
                val pathInZip = "${SAVE_DIR}/${path.name}"
                Log.i("exportSaveData", pathInZip)
                zip.putNextEntry(ZipEntry(pathInZip))
                path.inputStream().buffered().use {
                    it.copyTo(zip)
                }
            }
        }
    }

    fun importSaveData(input: InputStream): Int? {
        try {
            var imported = false
            forEachZipEntry(input) { zipEntry, zip ->
                // Process only files directly under save/
                if (zipEntry.isDirectory || !zipEntry.name.startsWith("save/") ||
                        zipEntry.name.count{it == '/'} != 1)
                    return@forEachZipEntry
                Log.i("importSaveData", zipEntry.name)
                FileOutputStream(File(rootDir, zipEntry.name)).buffered().use {
                    zip.copyTo(it)
                }
                imported = true
            }
            return if (imported) null else R.string.no_data_to_import
        } catch (e: UTFDataFormatException) {
            // Attempted to read Shift_JIS zip in Android < 7
            return R.string.unsupported_zip
        } catch (e: IOException) {
            Log.e("launcher", "Failed to extract ZIP", e)
            return R.string.zip_extraction_error
        }
    }

    private fun extractFiles(input: InputStream, outDir: File, handler: Handler) {
        try {
            val configWriter = GameConfigWriter()
            forEachZipEntry(input) { zipEntry, zip ->
                Log.i("extractFiles", zipEntry.name)
                val path = File(outDir, zipEntry.name)
                if (zipEntry.isDirectory)
                    return@forEachZipEntry
                path.parentFile.mkdirs()
                handler.sendMessage(handler.obtainMessage(PROGRESS, zipEntry.name))
                FileOutputStream(path).buffered().use {
                    zip.copyTo(it)
                }
                configWriter.maybeAdd(zipEntry.name)
            }
            configWriter.write(outDir)
            handler.sendMessage(handler.obtainMessage(SUCCESS, outDir))
        } catch (e: InstallFailureException) {
            handler.sendMessage(handler.obtainMessage(FAILURE, e.msgId))
        } catch (e: UTFDataFormatException) {
            // Attempted to read Shift_JIS zip in Android < 7
            handler.sendMessage(handler.obtainMessage(FAILURE, R.string.unsupported_zip))
        } catch (e: IOException) {
            Log.e("launcher", "Failed to extract ZIP", e)
            handler.sendMessage(handler.obtainMessage(FAILURE, R.string.zip_extraction_error))
        }
    }

    class InstallFailureException(val msgId: Int) : Exception()

    // A helper class which generates xsystem35.gr and playlist.txt in the game root directory.
    private class GameConfigWriter {
        private val grb = StringBuilder()
        private var basename: String? = null
        private val aldRegex = """(.*?)([a-z])([a-z])\.ald""".toRegex(RegexOption.IGNORE_CASE)
        private val resourceType = mapOf(
                "d" to "Data",
                "g" to "Graphics",
                "m" to "Midi",
                "r" to "Resource",
                "s" to "Scenario",
                "w" to "Wave")

        private val audioRegex = """.*?(\d+)\.(wav|mp3|ogg)""".toRegex(RegexOption.IGNORE_CASE)
        private val audioFiles: Array<String?> = arrayOfNulls(100)

        fun maybeAdd(path: String) {
            val name = File(path).name

            if (name.toLowerCase(Locale.US) == "system39.ain") {
                grb.appendln("Ain $path")
                return
            }
            aldRegex.matchEntire(name)?.let {
                val type = resourceType[it.groupValues[2].toLowerCase(Locale.US)]
                val id = it.groupValues[3].toUpperCase(Locale.US)
                if (type != null) {
                    grb.appendln("$type$id $path")
                    basename = it.groupValues[1]
                }
            }
            audioRegex.matchEntire(name)?.let {
                val track = it.groupValues[1].toInt()
                if (track < audioFiles.size)
                    audioFiles[track] = path
            }
        }

        fun write(outDir: File) {
            basename ?: throw InstallFailureException(R.string.cannot_find_ald)
            for (id in 'A' .. 'Z') {
                grb.appendln("Save$id ../save/${basename}s${id.toLowerCase()}.asd")
            }
            val gr = grb.toString()
            Log.i("xsystem35.gr", gr)
            File(outDir, "xsystem35.gr").writeText(gr)

            val playlist = audioFiles.joinToString("\n") { it ?: "" }.trimEnd('\n')
            File(outDir, PLAYLIST_FILE).writeText(playlist)
        }
    }
}

private fun forEachZipEntry(input: InputStream, action: (ZipEntry, ZipInputStream) -> Unit) {
    val zip = if (Build.VERSION.SDK_INT >= 24) {
        ZipInputStream(input.buffered(), Charset.forName("Shift_JIS"))
    } else {
        ZipInputStream(input.buffered())
    }
    zip.use {
        while (true) {
            val zipEntry = zip.nextEntry ?: break
            action(zipEntry, zip)
        }
    }
}
