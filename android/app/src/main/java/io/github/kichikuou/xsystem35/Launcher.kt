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
import java.nio.charset.Charset
import java.util.*
import java.util.zip.ZipInputStream

private var gLauncher: Launcher? = null

interface LauncherObserver {
    fun onInstallProgress(path: String)
    fun onInstallSuccess(path: File)
    fun onInstallFailure(msgId: Int)
}

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
    }

    data class Entry(val path: File, val title: String)
    val games = arrayListOf<Entry>()
    val titles: List<String>
        get() = games.map(Entry::title)
    var observer: LauncherObserver? = null
    var isInstalling = false
        private set

    init {
        var saveDirFound = false
        for (path in rootDir.listFiles()) {
            if (!path.isDirectory)
                continue
            if (path.name == "save") {
                saveDirFound = true
                continue
            }
            try {
                val title = File(path, TITLE_FILE).readText()
                games.add(Entry(path, title))
            } catch (e: IOException) {
                // Incomplete game installation. Delete it.
                path.deleteRecursively()
            }
        }
        if (!saveDirFound) {
            File(rootDir, "save").mkdir()
        }
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

    private fun extractFiles(input: InputStream, outDir: File, handler: Handler) {
        try {
            val configWriter = GameConfigWriter()
            val zip = if (Build.VERSION.SDK_INT >= 24) {
                ZipInputStream(input.buffered(), Charset.forName("Shift_JIS"))
            } else {
                ZipInputStream(input.buffered())
            }
            while (true) {
                val zipEntry = zip.nextEntry ?: break
                Log.v("extractFiles", zipEntry.name)
                val path = File(outDir, zipEntry.name)
                if (zipEntry.isDirectory)
                    continue
                path.parentFile.mkdirs()
                handler.sendMessage(handler.obtainMessage(PROGRESS, zipEntry.name))
                val output = FileOutputStream(path).buffered()
                zip.copyTo(output)
                output.close()
                configWriter.maybeAdd(zipEntry.name)
            }
            zip.close()
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
        private val grLines = arrayListOf<String>()
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
                grLines.add("Ain $path")
                return
            }
            aldRegex.matchEntire(name)?.let {
                val type = resourceType[it.groupValues[2]]
                val id = it.groupValues[3].toUpperCase(Locale.US)
                if (type != null) {
                    grLines.add("$type$id $path")
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
                grLines.add("Save$id ../save/${basename}s${id.toLowerCase()}.asd")
            }
            grLines.add("")
            val gr = grLines.joinToString("\n")
            Log.d("xsystem35.gr", gr)
            File(outDir, "xsystem35.gr").writeText(gr)

            val playlist = audioFiles.joinToString("\n") { it ?: "" }.trimEnd('\n')
            File(outDir, PLAYLIST_FILE).writeText(playlist)
        }
    }
}
