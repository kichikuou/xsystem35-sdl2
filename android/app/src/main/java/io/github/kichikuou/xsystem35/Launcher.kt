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
import java.util.zip.ZipInputStream
import kotlin.collections.ArrayList

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
    val games: ArrayList<Entry> = arrayListOf()
    val titles: List<String>
        get() = games.map(Entry::title)
    var observer: LauncherObserver? = null
    var isInstalling = false
        private set

    init {
        for (path in rootDir.listFiles()) {
            if (!path.isDirectory)
                continue
            try {
                val title = File(path, TITLE_FILE).readText()
                games.add(Entry(path, title))
            } catch (e: IOException) {
                // Incomplete game installation. Delete it.
                path.deleteRecursively()
            }
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
            val playlistWriter = PlaylistWriter()
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
                playlistWriter.maybeAdd(path.path)
            }
            zip.close()
            playlistWriter.write(outDir)
            handler.sendMessage(handler.obtainMessage(SUCCESS, outDir))
        } catch (e: UTFDataFormatException) {
            // Attempted to read Shift_JIS zip in Android < 7
            handler.sendMessage(handler.obtainMessage(FAILURE, R.string.unsupported_zip))
        } catch (e: IOException) {
            Log.e("launcher", "Failed to extract ZIP", e)
            handler.sendMessage(handler.obtainMessage(FAILURE, R.string.zip_extraction_error))
        }
    }

    private class PlaylistWriter {
        private val audioRegex = """.*?(\d+)\.(wav|mp3|ogg)""".toRegex(RegexOption.IGNORE_CASE)
        private val audioFiles: Array<String?> = arrayOfNulls(100)

        fun maybeAdd(path: String) {
            audioRegex.matchEntire(path)?.let {
                val track = it.groupValues[1].toInt()
                if (track < audioFiles.size)
                    audioFiles[track] = path
            }
        }

        fun write(outDir: File) {
            val text = audioFiles.joinToString("\n") { it ?: "" }.trimEnd('\n')
            File(outDir, PLAYLIST_FILE).writeText(text)
        }
    }
}
