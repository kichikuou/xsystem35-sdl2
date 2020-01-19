/* Copyright (C) 2019 <KichikuouChrome@gmail.com>
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

import android.media.MediaPlayer
import android.os.Bundle
import android.util.Log
import org.libsdl.app.SDLActivity
import java.io.File
import java.io.IOException

// Intent for this activity must have two extras:
// - EXTRA_GAME_ROOT (string): A path to the game installation.
// - EXTRA_TITLE_FILE (string): A file to which the game title will be written.
// - EXTRA_PLAYLIST_FILE (string): A path to the BGM playlist file.
class GameActivity : SDLActivity() {
    companion object {
        const val EXTRA_GAME_ROOT = "GAME_ROOT"
        const val EXTRA_TITLE_FILE = "TITLE_FILE"
        const val EXTRA_PLAYLIST_FILE = "PLAYLIST_FILE"
    }

    private lateinit var player: BGMPlayer

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        player = BGMPlayer(intent.getStringExtra(EXTRA_PLAYLIST_FILE))
    }

    override fun onStop() {
        super.onStop()
        player.onStop()
    }

    override fun onResume() {
        super.onResume()
        player.onResume()
    }

    override fun getLibraries(): Array<String> {
        return arrayOf("hidapi", "SDL2", "xsystem35")
    }

    override fun getArguments(): Array<String> {
        return arrayOf("-gamedir", intent.getStringExtra(EXTRA_GAME_ROOT))
    }

    override fun setTitle(title: CharSequence?) {
        super.setTitle(title)
        val str = title?.toString()?.substringAfter(':', "")
        if (str.isNullOrEmpty())
            return
        intent.getStringExtra(EXTRA_TITLE_FILE)?.let { File(it).writeText(str) }
    }

    // These functions are called in the SDL thread by JNI.
    @Suppress("unused") fun cddaStart(track: Int, loop: Int) = player.cddaStart(track, loop)
    @Suppress("unused") fun cddaStop() = player.cddaStop()
    @Suppress("unused") fun cddaCurrentPosition(): Int = player.cddaCurrentPosition()
}

private class BGMPlayer(playlistPath: String?) {
    private val playlist = playlistPath?.let {
        try {
            File(it).readLines()
        } catch (e: IOException) {
            Log.e("loadPlaylist", "Cannot load $playlistPath", e)
            null
        }
    } ?: emptyList()
    private var currentTrack = 0
    private val player = MediaPlayer()
    private var playerPaused = false

    fun cddaStart(track: Int, loop: Int) {
        val f = playlist.elementAtOrNull(track)
        if (f.isNullOrEmpty()) {
            Log.w("cddaStart", "No playlist entry for track $track")
            return
        }
        Log.v("cddaStart", f)
        try {
            player.apply {
                reset()
                setDataSource(f)
                isLooping = loop == 0
                prepare()
                start()
            }
            currentTrack = track
        } catch (e: IOException) {
            Log.e("cddaStart", "Cannot play $f", e)
            player.reset()
        }
    }

    fun cddaStop() {
        if (currentTrack > 0 && player.isPlaying)
            player.stop()
    }

    fun cddaCurrentPosition(): Int {
        if (currentTrack == 0)
            return 0
        val frames = player.currentPosition * 75 / 1000
        return currentTrack or (frames shl 8)
    }

    fun onStop() {
        if (currentTrack > 0 && player.isPlaying) {
            player.pause()
            playerPaused = true
        }
    }

    fun onResume() {
        if (playerPaused) {
            player.start()
            playerPaused = false
        }
    }
}