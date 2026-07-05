/* Copyright (C) 2026 <KichikuouChrome@gmail.com>
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

import android.content.ContentResolver
import android.net.Uri
import android.os.Build
import android.util.Log
import kotlinx.coroutines.runBlocking
import java.io.File
import java.io.FileOutputStream
import java.io.IOException
import java.io.InputStream
import java.io.UTFDataFormatException
import java.nio.charset.Charset
import java.util.zip.ZipEntry
import java.util.zip.ZipInputStream

data class SelectedInstallFile(
    val uri: Uri,
    val displayName: String,
)

class GameInstaller(private val store: GameStore) {
    suspend fun installZip(input: InputStream, progressCallback: suspend (String) -> Unit): File {
        val dir = store.createDirForGame()
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

    suspend fun installCdImage(
        files: List<SelectedInstallFile>,
        contentResolver: ContentResolver,
        tempDir: File,
        progressCallback: suspend (String) -> Unit
    ): File {
        val dir = store.createDirForGame()
        var committed = false
        try {
            val gameDir = extractCdImage(files, contentResolver, tempDir, dir, progressCallback)
            committed = true
            return gameDir
        } finally {
            if (!committed && !dir.deleteRecursively()) {
                Log.w("launcher", "Failed to delete incomplete install directory: $dir")
            }
        }
    }

    internal suspend fun installCdImageForTest(
        cdImage: CdImageReader,
        progressCallback: suspend (String) -> Unit
    ): File {
        val dir = store.createDirForGame()
        var committed = false
        try {
            val gameDir = extractCdImage(cdImage, dir, progressCallback)
            committed = true
            return gameDir
        } finally {
            if (!committed && !dir.deleteRecursively()) {
                Log.w("launcher", "Failed to delete incomplete install directory: $dir")
            }
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

    private suspend fun extractCdImage(
        files: List<SelectedInstallFile>,
        contentResolver: ContentResolver,
        tempDir: File,
        outDir: File,
        progressCallback: suspend (String) -> Unit
    ): File {
        progressCallback("metadata parsing")
        CdImageReader.open(contentResolver, files, tempDir).use { cdImage ->
            return extractCdImage(cdImage, outDir, progressCallback)
        }
    }

    private suspend fun extractCdImage(
        cdImage: CdImageReader,
        outDir: File,
        progressCallback: suspend (String) -> Unit
    ): File {
        val fs = Iso9660FileSystem(cdImage)
        val gameData = fs.findGameDataDirectory()
            ?: throw InstallFailureException(R.string.cannot_find_game_data_directory)
        var foundAld = false
        fs.extractDirectory(gameData, "GAMEDATA") { path, input ->
            val resolvedPath = resolveOutputPath(outDir, path)
            resolvedPath.file.parentFile?.mkdirs()
            progressCallback(resolvedPath.relativePath)
            FileOutputStream(resolvedPath.file).buffered().use {
                input.copyTo(it)
            }
            if (File(path).name.matches(""".*?s[a-z]\.ald""".toRegex(RegexOption.IGNORE_CASE))) {
                foundAld = true
            }
        }
        if (!foundAld) {
            throw InstallFailureException(R.string.cannot_find_ald)
        }
        File(outDir, Launcher.GAMEDIR_FILE).writeText("GAMEDATA")
        extractAudioTracks(cdImage, File(outDir, "GAMEDATA"), progressCallback)
        return File(outDir, "GAMEDATA")
    }

    private suspend fun extractAudioTracks(
        cdImage: CdImageReader,
        gameDir: File,
        progressCallback: suspend (String) -> Unit
    ) {
        val audioTracks = cdImage.audioTracks()
        if (audioTracks.isEmpty()) {
            return
        }
        val cddaDir = File(gameDir, "cdda")
        cddaDir.mkdirs()
        val playlist = arrayOfNulls<String>(audioTracks.maxOrNull() ?: 0)
        for (track in audioTracks) {
            val relativePath = "cdda/track%02d.wav".format(track)
            val outputFile = File(gameDir, relativePath)
            progressCallback(relativePath)
            FileOutputStream(outputFile).buffered().use {
                cdImage.extractAudioTrack(track, it)
            }
            playlist[track - 1] = relativePath
        }
        File(gameDir, Launcher.PLAYLIST_FILE).writeText(playlist.joinToString("\n") { it ?: "" }.trimEnd('\n'))
    }
}

internal class InstallFailureException(val msgId: Int) : Exception()

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
            File(outDir, Launcher.GAMEDIR_FILE).writeText(it)
        }
        // Generate PLAYLIST_FILE
        val absGameDir = gameDir?.let { File(outDir, it) } ?: outDir
        val playlistFile = File(absGameDir, Launcher.PLAYLIST_FILE)
        if (!playlistFile.exists()) {
            val prefixToRemove = gameDir?.let { "$it/" } ?: ""
            val playlist = audioFiles.joinToString("\n") {
                it?.removePrefix(prefixToRemove) ?: ""
            }.trimEnd('\n')
            playlistFile.writeText(playlist)
        }
    }
}

/**
 * @property file Safe canonical output path.
 * @property relativePath Path relative to the install root after canonicalization.
 */
internal data class ResolvedOutputPath(val file: File, val relativePath: String)

internal fun resolveOutputPath(baseDir: File, relativePath: String): ResolvedOutputPath {
    if (File(relativePath).isAbsolute) {
        throw IOException("Output path is absolute: $relativePath")
    }
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

internal fun forEachZipEntry(input: InputStream, action: (ZipEntry, ZipInputStream) -> Unit): Boolean =
    runBlocking {
        forEachZipEntrySuspending(input) { zipEntry, zip ->
            action(zipEntry, zip)
        }
    }

internal suspend fun forEachZipEntrySuspending(
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
