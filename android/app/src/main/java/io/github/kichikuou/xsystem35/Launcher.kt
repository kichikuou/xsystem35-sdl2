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
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking
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

sealed class InstallState {
    object Idle : InstallState()
    data class Installing(val progress: String?) : InstallState()
    data class Succeeded(val path: File, val archiveName: String?) : InstallState()
    data class Failed(val msgId: Int) : InstallState()
}

data class InstallResult(val path: File, val archiveName: String?)

class Launcher private constructor(private val rootDir: File) {
    companion object {
        const val SAVE_DIR = "save"
        const val TITLE_FILE = "title.txt"
        const val GAMEDIR_FILE = "game_directory.txt"
        const val PLAYLIST_FILE = "playlist.txt"
        const val OLD_PLAYLIST_FILE = "playlist2.txt"

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
    private val scope = CoroutineScope(SupervisorJob() + Dispatchers.Main)
    private var installJob: Job? = null
    var installState: InstallState = InstallState.Idle
        private set

    init {
        updateGameList()
    }

    fun install(input: InputStream, archiveName: String?) {
        if (installJob?.isActive == true) {
            input.close()
            return
        }
        startInstallJob {
            val gameDir = withContext(Dispatchers.IO) {
                input.use {
                    extractZipTransactionally(it) { msg ->
                        withContext(Dispatchers.Main) {
                            setInstallProgress(msg)
                        }
                    }
                }
            }
            InstallResult(gameDir, archiveName)
        }
    }

    private fun startInstallJob(block: suspend () -> InstallResult) {
        if (installJob?.isActive == true) {
            return
        }
        installState = InstallState.Installing(null)
        installJob = scope.launch {
            try {
                val result = block()
                setInstallSucceeded(result)
            } catch (e: InstallFailureException) {
                setInstallFailed(e.msgId)
            } catch (e: Exception) {
                Log.e("launcher", "Failed to extract ZIP", e)
                setInstallFailed(R.string.zip_extraction_error)
            }
        }
    }

    private suspend fun extractZipTransactionally(
        input: InputStream,
        progressCallback: suspend (String) -> Unit
    ): File {
        val dir = createDirForGame()
        var committed = false
        try {
            val gameDir = extractFiles(input, dir, progressCallback)
            committed = true
            return gameDir
        } finally {
            if (!committed && !dir.deleteRecursively()) {
                Log.w("launcher", "Failed to delete incomplete install directory: $dir")
            }
        }
    }

    fun consumeInstallResult() {
        if (installState is InstallState.Succeeded || installState is InstallState.Failed) {
            installState = InstallState.Idle
        }
    }

    private fun setInstallProgress(progress: String) {
        installState = InstallState.Installing(progress)
        observer?.onInstallProgress(progress)
    }

    private fun setInstallSucceeded(result: InstallResult) {
        installState = InstallState.Succeeded(result.path, result.archiveName)
        observer?.onInstallSuccess(result.path, result.archiveName)
    }

    private fun setInstallFailed(msgId: Int) {
        installState = InstallState.Failed(msgId)
        observer?.onInstallFailure(msgId)
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
        observer?.onGameListChange()
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
                val path = resolveOutputPath(rootDir, zipEntry.name).file
                Log.i("importSaveData", zipEntry.name)
                FileOutputStream(path).buffered().use {
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

    private suspend fun extractFiles(
        input: InputStream,
        outDir: File,
        progressCallback: suspend (String) -> Unit
    ): File {
        val configWriter = GameConfigWriter()
        val hadDecodeError = forEachZipEntrySuspending(input) { zipEntry, zip ->
            Log.i("extractFiles", zipEntry.name)
            if (zipEntry.isDirectory)
                return@forEachZipEntrySuspending
            val resolvedPath = resolveOutputPath(outDir, zipEntry.name)
            val path = resolvedPath.file
            path.parentFile?.mkdirs()
            progressCallback(resolvedPath.relativePath)
            FileOutputStream(path).buffered().use {
                zip.copyTo(it)
            }
            configWriter.maybeAdd(resolvedPath.relativePath)
        }
        if (!configWriter.ready) {
            if (hadDecodeError)
                throw InstallFailureException(R.string.unsupported_zip)
            throw InstallFailureException(R.string.cannot_find_ald)
        }
        configWriter.write(outDir)
        return configWriter.gameDir?.let { File(outDir, it) } ?: outDir
    }

    // Xsystem35-sdl2 2.3.0 - 2.11.1 used playlist2.txt. Rename it to playlist.txt.
    private fun migratePlaylist(dir: File) {
        val oldPlaylist = File(dir, OLD_PLAYLIST_FILE)
        if (oldPlaylist.exists()) {
            oldPlaylist.renameTo(File(dir, PLAYLIST_FILE))
        }
    }

    class InstallFailureException(val msgId: Int) : Exception()

    // A helper class which generates GAMEDIR_FILE and PLAYLIST_FILE.
    private class GameConfigWriter {
        var ready = false
            private set
        var gameDir: String? = null
            private set
        private val aldRegex = """.*?s[a-z]\.ald""".toRegex(RegexOption.IGNORE_CASE)
        private val audioRegex = """((\d+).*|.*?(\d+))\.(wav|mp3|ogg)""".toRegex(RegexOption.IGNORE_CASE)
        private val audioFiles: Array<String?> = arrayOfNulls(100)

        fun maybeAdd(path: String) {
            val name = File(path).name
            aldRegex.matchEntire(name)?.let {
                gameDir = File(path).parent
                ready = true
            }
            audioRegex.matchEntire(name)?.let {
                val track = it.groupValues[2].toIntOrNull() ?: it.groupValues[3].toInt()
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

    fun clearSaveData(): Boolean {
        var success = true
        File(rootDir, SAVE_DIR).listFiles()?.forEach { file ->
            if (!file.deleteRecursively()) success = false
        }
        return success
    }
}

/**
 * @property file Safe canonical output path.
 * @property relativePath Path relative to the install root after canonicalization.
 */
private data class ResolvedOutputPath(val file: File, val relativePath: String)

private fun resolveOutputPath(baseDir: File, relativePath: String): ResolvedOutputPath {
    val canonicalBase = baseDir.canonicalFile
    val file = File(canonicalBase, relativePath).canonicalFile
    if (!isFileInsideDirectory(file, canonicalBase)) {
        throw IOException("Output path is outside target directory: $relativePath")
    }
    val basePath = canonicalBase.path + File.separator
    val canonicalRelativePath = file.path.removePrefix(basePath)
    return ResolvedOutputPath(file, canonicalRelativePath)
}

private fun isFileInsideDirectory(file: File, directory: File): Boolean {
    return file.path.startsWith(directory.path + File.separator)
}

private fun forEachZipEntry(input: InputStream, action: (ZipEntry, ZipInputStream) -> Unit): Boolean =
    runBlocking {
        forEachZipEntrySuspending(input) { zipEntry, zip ->
            action(zipEntry, zip)
        }
    }

private suspend fun forEachZipEntrySuspending(
    input: InputStream,
    action: suspend (ZipEntry, ZipInputStream) -> Unit
): Boolean {
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
