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

import android.os.Build
import android.util.Log
import kotlinx.coroutines.DelicateCoroutinesApi
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.io.*
import java.nio.charset.Charset
import java.util.zip.ZipEntry
import java.util.zip.ZipInputStream
import java.util.zip.ZipOutputStream

private var gLauncher: Launcher? = null

interface LauncherObserver {
    fun onGameListChange()
    fun onInstallProgress(path: String)
    fun onInstallSuccess(path: File, archiveName: String?)
    fun onInstallFailure(msgId: Int)
}

class Launcher private constructor(private val rootDir: File) {
    companion object {
        const val SAVE_DIR = "save"
        const val TITLE_FILE = "title.txt"
        const val GAMEDIR_FILE = "game_directory.txt"
        const val PLAYLIST_FILE = "playlist2.txt"
        const val OLD_PLAYLIST_FILE = "playlist.txt"

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

    @OptIn(DelicateCoroutinesApi::class)
    fun install(input: InputStream, archiveName: String?) {
        val dir = createDirForGame()
        isInstalling = true
        GlobalScope.launch(Dispatchers.Main) {
            try {
                val gameDir = withContext(Dispatchers.IO) {
                    extractFiles(input, dir) { msg ->
                        GlobalScope.launch(Dispatchers.Main) {
                            observer?.onInstallProgress(msg)
                        }
                    }
                }
                observer?.onInstallSuccess(gameDir, archiveName)
            } catch (e: InstallFailureException) {
                observer?.onInstallFailure(e.msgId)
            } catch (e: Exception) {
                Log.e("launcher", "Failed to extract ZIP", e)
                observer?.onInstallFailure(R.string.zip_extraction_error)
            }
            isInstalling = false
        }
    }

    fun uninstall(id: Int) {
        games[id].path.deleteRecursively()
        games.removeAt(id)
        observer?.onGameListChange()
    }

    private fun updateGameList() {
        var saveDirFound = false
        games.clear()
        for (path in rootDir.listFiles() ?: emptyArray()) {
            if (!path.isDirectory)
                continue
            if (path.name == SAVE_DIR) {
                saveDirFound = true
                continue
            }
            try {
                val gameDirFile = File(path, GAMEDIR_FILE)
                val gamePath = if (gameDirFile.exists()) File(path, gameDirFile.readText()) else path
                val titleFile = File(gamePath, TITLE_FILE)
                val title = titleFile.readText()
                games.add(Entry(gamePath, title, titleFile.lastModified()))
                migratePlaylist(path)
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
            for (path in File(rootDir, SAVE_DIR).listFiles() ?: emptyArray()) {
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

    private fun extractFiles(input: InputStream, outDir: File, progressCallback: (String) -> Unit): File {
        val configWriter = GameConfigWriter()
        val hadDecodeError = forEachZipEntry(input) { zipEntry, zip ->
            Log.i("extractFiles", zipEntry.name)
            val path = File(outDir, zipEntry.name)
            if (zipEntry.isDirectory)
                return@forEachZipEntry
            path.parentFile?.mkdirs()
            progressCallback(zipEntry.name)
            FileOutputStream(path).buffered().use {
                zip.copyTo(it)
            }
            configWriter.maybeAdd(zipEntry.name)
        }
        if (!configWriter.ready) {
            if (hadDecodeError)
                throw InstallFailureException(R.string.unsupported_zip)
            throw InstallFailureException(R.string.cannot_find_ald)
        }
        configWriter.write(outDir)
        return configWriter.gameDir?.let { File(outDir, it) } ?: outDir
    }

    // Xsystem35-sdl2 <=2.2.0 had a bug where playlist had an extra empty line at
    // the beginning of the file. This migrates playlist.txt created by an old
    // version of xsystem35-sdl2 to playlist2.txt.
    private fun migratePlaylist(dir: File) {
        val oldPlaylist = File(dir, OLD_PLAYLIST_FILE)
        if (!oldPlaylist.exists())
            return
        var tracks = oldPlaylist.readLines()
        if (tracks.isNotEmpty())
            tracks = tracks.subList(1, tracks.size)
        File(dir, PLAYLIST_FILE).writeText(tracks.joinToString("\n"))
        oldPlaylist.delete()
    }

    class InstallFailureException(val msgId: Int) : Exception()

    // A helper class which generates GAMEDIR_FILE and PLAYLIST_FILE.
    private class GameConfigWriter {
        var ready = false
            private set
        var gameDir: String? = null
            private set
        private val aldRegex = """.*?s[a-z]\.ald""".toRegex(RegexOption.IGNORE_CASE)
        private val audioRegex = """.*?(\d+)\.(wav|mp3|ogg)""".toRegex(RegexOption.IGNORE_CASE)
        private val audioFiles: Array<String?> = arrayOfNulls(100)

        fun maybeAdd(path: String) {
            aldRegex.matchEntire(path)?.let {
                gameDir = File(path).parent
                ready = true
            }
            audioRegex.matchEntire(path)?.let {
                val track = it.groupValues[1].toInt()
                if (0 < track && track <= audioFiles.size)
                    audioFiles[track - 1] = path
            }
        }

        fun write(outDir: File) {
            // Generate GAMEDIR_FILE
            gameDir?.let {
                File(outDir, GAMEDIR_FILE).writeText(it)
            }
            // Generate PLAYLIST_FILE
            val absGameDir = gameDir?.let { File(outDir, it) } ?: outDir
            val playlistFile = File(absGameDir, PLAYLIST_FILE)
            if (!playlistFile.exists()) {
                val prefixToRemove = gameDir?.let { "$it/" } ?: ""
                val playlist = audioFiles.joinToString("\n") {
                    it?.removePrefix(prefixToRemove) ?: ""
                }.trimEnd('\n')
                playlistFile.writeText(playlist)
            }
        }
    }
}

private fun forEachZipEntry(input: InputStream, action: (ZipEntry, ZipInputStream) -> Unit): Boolean {
    val zip = if (Build.VERSION.SDK_INT >= 24) {
        ZipInputStream(input.buffered(), Charset.forName("Shift_JIS"))
    } else {
        ZipInputStream(input.buffered())
    }
    var hadDecodeError = false
    zip.use {
        while (true) {
            try {
                val zipEntry = zip.nextEntry ?: break
                action(zipEntry, zip)
            } catch (e: UTFDataFormatException) {
                // Attempted to read Shift_JIS zip in Android < 7
                Log.w("forEachZipEntry", "UTFDataFormatException: skipping a zip entry")
                zip.closeEntry()
                hadDecodeError = true
            }
        }
    }
    return hadDecodeError
}
