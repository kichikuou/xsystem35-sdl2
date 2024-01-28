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

import android.app.AlertDialog
import android.media.MediaPlayer
import android.os.Bundle
import android.text.InputType
import android.util.Log
import android.widget.EditText
import android.widget.NumberPicker
import org.libsdl.app.SDLActivity
import java.io.File
import java.io.IOException

// Intent for this activity must have the following extra:
// - EXTRA_GAME_ROOT (string): A path to the game installation.
class GameActivity : SDLActivity() {
    companion object {
        const val EXTRA_GAME_ROOT = "GAME_ROOT"
        const val EXTRA_ARCHIVE_NAME = "ARCHIVE_NAME"
    }

    private lateinit var gameRoot: File
    private val midi = MidiPlayer()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        gameRoot = File(intent.getStringExtra(EXTRA_GAME_ROOT)!!)
    }

    override fun onStop() {
        super.onStop()
        midi.onActivityStop()
    }

    override fun onResume() {
        super.onResume()
        midi.onActivityResume()
    }

    override fun getLibraries(): Array<String> {
        return arrayOf("SDL2", "xsystem35")
    }

    override fun getArguments(): Array<String> {
        return arrayOf(
            "-gamedir", intent.getStringExtra(EXTRA_GAME_ROOT)!!,
            "-playlist", Launcher.PLAYLIST_FILE)
    }

    override fun setTitle(title: CharSequence?) {
        super.setTitle(title)
        var str = title?.toString()?.substringAfter(':', "")
        if (str.isNullOrEmpty()) {
            str = intent.getStringExtra(EXTRA_ARCHIVE_NAME)
            if (str == null)
                return
        }
        File(gameRoot, Launcher.TITLE_FILE).writeText(str)
        Launcher.updateGameList()
    }

    private fun textInputDialog(msg: String, oldVal: String, maxLen: Int, result: Array<String?>) {
        val input = EditText(this)
        input.inputType = InputType.TYPE_CLASS_TEXT
        input.setText(oldVal)
        AlertDialog.Builder(this)
                .setMessage(msg)
                .setView(input)
                .setPositiveButton(R.string.ok) {_, _ ->
                    val s = input.text.toString()
                    result[0] = if (s.length <= maxLen) s else s.substring(0, maxLen)
                }
                .setNegativeButton(R.string.cancel) {_, _ -> }
                .setOnDismissListener {
                    synchronized(result) {
                        @Suppress("PLATFORM_CLASS_MAPPED_TO_KOTLIN") (result as Object).notify()
                    }
                }
                .show()
    }

    private fun numberInputDialog(msg: String, min: Int, max: Int, initial: Int, result: IntArray) {
        val input = NumberPicker(this)
        input.minValue = min
        input.maxValue = max
        input.value = initial
        AlertDialog.Builder(this)
                .setMessage(msg)
                .setView(input)
                .setPositiveButton(R.string.ok) {_, _ ->
                    input.clearFocus()
                    result[0] = input.value
                }
                .setNegativeButton(R.string.cancel) {_, _ -> }
                .setOnDismissListener {
                    synchronized(result) {
                        @Suppress("PLATFORM_CLASS_MAPPED_TO_KOTLIN") (result as Object).notify()
                    }
                }
                .show()
    }

    // The functions below are called in the SDL thread by JNI.
    @Suppress("unused") fun midiStart(path: String, loop: Boolean) = midi.start(path, loop)
    @Suppress("unused") fun midiStop() = midi.stop()
    @Suppress("unused") fun midiCurrentPosition() = midi.currentPosition()

    @Suppress("unused") fun inputString(msg: String, oldVal: String, maxLen: Int): String? {
        val result = arrayOfNulls<String?>(1)
        runOnUiThread { textInputDialog(msg, oldVal, maxLen, result) }
        // Block the calling thread.
        synchronized(result) {
            try {
                @Suppress("PLATFORM_CLASS_MAPPED_TO_KOTLIN") (result as Object).wait()
            } catch (ex: InterruptedException) {
                ex.printStackTrace()
            }
        }
        return result[0]
    }

    @Suppress("unused") fun inputNumber(msg: String, min: Int, max: Int, initial: Int): Int {
        val result = intArrayOf(-1)
        runOnUiThread { numberInputDialog(msg, min, max, initial, result) }
        // Block the calling thread.
        synchronized(result) {
            try {
                @Suppress("PLATFORM_CLASS_MAPPED_TO_KOTLIN") (result as Object).wait()
            } catch (ex: InterruptedException) {
                ex.printStackTrace()
            }
        }
        return result[0]
    }
}

private class MidiPlayer {
    private val player = MediaPlayer()
    private var playing = false
    private var playerPaused = false

    fun start(path: String, loop: Boolean) {
        try {
            player.apply {
                reset()
                setDataSource(path)
                isLooping = loop
                prepare()
                start()
            }
            playing = true
        } catch (e: IOException) {
            Log.e("midiStart", "Cannot play midi", e)
            player.reset()
        }
    }

    fun stop() {
        if (playing && player.isPlaying) {
            player.stop()
            playing = false
        }
    }

    fun currentPosition(): Int {
        return if (playing) player.currentPosition else 0
    }

    fun onActivityStop() {
        if (playing && player.isPlaying) {
            player.pause()
            playerPaused = true
        }
    }

    fun onActivityResume() {
        if (playerPaused) {
            player.start()
            playerPaused = false
        }
    }
}
