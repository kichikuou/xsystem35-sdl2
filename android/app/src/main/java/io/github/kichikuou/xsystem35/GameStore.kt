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

import android.util.Log
import java.io.File
import java.io.FileOutputStream
import java.io.IOException
import java.io.InputStream
import java.io.OutputStream
import java.io.UTFDataFormatException
import java.util.zip.ZipEntry
import java.util.zip.ZipOutputStream

class GameStore(private val rootDir: File) {
    data class Entry(val path: File, val title: String, val timestamp: Long)

    private val gameList = arrayListOf<Entry>()
    val games: List<Entry>
        get() = gameList
    val titles: List<String>
        get() = gameList.map(Entry::title)

    init {
        updateGameList()
    }

    fun uninstall(id: Int) {
        gameList[id].path.deleteRecursively()
        gameList.removeAt(id)
    }

    fun updateGameList() {
        var saveDirFound = false
        gameList.clear()
        for (path in rootDir.listFiles() ?: emptyArray()) {
            if (!path.isDirectory)
                continue
            if (path.name == Launcher.SAVE_DIR) {
                saveDirFound = true
                continue
            }
            try {
                val gameDirFile = File(path, Launcher.GAMEDIR_FILE)
                val gamePath = if (gameDirFile.exists()) File(path, gameDirFile.readText()) else path
                val titleFile = File(gamePath, Launcher.TITLE_FILE)
                val title = titleFile.readText()
                gameList.add(Entry(gamePath, title, titleFile.lastModified()))
                migratePlaylist(path)
            } catch (e: IOException) {
                // Incomplete game installation. Delete it.
                path.deleteRecursively()
            }
        }
        gameList.sortByDescending(Entry::timestamp)
        if (!saveDirFound) {
            File(rootDir, Launcher.SAVE_DIR).mkdir()
        }
    }

    fun createDirForGame(): File {
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
            for (path in File(rootDir, Launcher.SAVE_DIR).listFiles() ?: emptyArray()) {
                if (path.isDirectory || path.name.endsWith(".asd."))
                    continue
                val pathInZip = "${Launcher.SAVE_DIR}/${path.name}"
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

    fun clearSaveData(): Boolean {
        var success = true
        File(rootDir, Launcher.SAVE_DIR).listFiles()?.forEach { file ->
            if (!file.deleteRecursively()) success = false
        }
        return success
    }

    // Xsystem35-sdl2 2.3.0 - 2.11.1 used playlist2.txt. Rename it to playlist.txt.
    private fun migratePlaylist(dir: File) {
        val oldPlaylist = File(dir, Launcher.OLD_PLAYLIST_FILE)
        if (oldPlaylist.exists()) {
            oldPlaylist.renameTo(File(dir, Launcher.PLAYLIST_FILE))
        }
    }
}
