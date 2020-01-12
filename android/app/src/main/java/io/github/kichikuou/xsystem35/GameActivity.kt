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

import org.libsdl.app.SDLActivity
import java.io.File

// Intent for this activity must have two extras:
// - EXTRA_GAME_ROOT (string): A path to the game installation.
// - EXTRA_TITLE_FILE (string): A file to which the game title will be written.
class GameActivity : SDLActivity() {
    companion object {
        const val EXTRA_GAME_ROOT = "GAME_ROOT"
        const val EXTRA_TITLE_FILE = "TITLE_FILE"
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
}
