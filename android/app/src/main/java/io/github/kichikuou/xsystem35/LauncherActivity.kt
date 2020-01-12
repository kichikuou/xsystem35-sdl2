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

import android.annotation.SuppressLint
import android.app.*
import android.content.Intent
import android.os.Build
import android.os.Bundle
import android.os.Handler
import android.os.Message
import android.util.Log
import android.view.View
import android.widget.AdapterView
import android.widget.ArrayAdapter
import android.widget.ListView
import java.io.*

import java.nio.charset.Charset
import java.util.Locale
import java.util.zip.ZipInputStream
import kotlin.collections.ArrayList

const val PROGRESS = 0
const val SUCCESS = 1
const val FAILURE = 2

class LauncherActivity : ListActivity(), AdapterView.OnItemLongClickListener {
    private lateinit var gm: GameManager
    private lateinit var adapter: ArrayAdapter<String>

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        gm = GameManager(filesDir)

        val items = gm.titles.toMutableList()
        items.add("Install from ZIP")
        adapter = ArrayAdapter(this, android.R.layout.simple_list_item_1, items)
        listAdapter = adapter
        listView.onItemLongClickListener = this
    }

    override fun onListItemClick(l: ListView?, v: View?, position: Int, id: Long) {
        super.onListItemClick(l, v, position, id)
        if (position < gm.games.size) {
            startGame(gm.games[position].path)
        } else {
            val i = Intent(Intent.ACTION_GET_CONTENT)
            i.type = "application/zip"
            startActivityForResult(Intent.createChooser(i, "Choose a file"), 0)
        }
    }

    override fun onItemLongClick(a: AdapterView<*>?, v: View?, position: Int, id: Long): Boolean {
        if (position < gm.games.size) {
            AlertDialog.Builder(this).setTitle("Confirm")
                    .setMessage("Uninstall ${gm.games[position].title}?")
                    .setPositiveButton("OK") {_, _ -> uninstall(position)}
                    .setNegativeButton("Cancel") {_, _ -> }
                    .show()
        }
        return true
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        if (resultCode != RESULT_OK)
            return
        val uri = data?.data ?: return
        val input = contentResolver.openInputStream(uri) ?: return
        val dialog = ProgressDialogFragment()
        dialog.show(fragmentManager, "progress_dialog")

        @SuppressLint("HandlerLeak")  // Should be okay as installer thread has a short lifetime
        val handler = object : Handler() {
            override fun handleMessage(msg: Message) {
                when (msg.what) {
                    PROGRESS -> {
                        dialog.setProgress(msg.obj as String)
                    }
                    SUCCESS -> {
                        dialog.dismiss()
                        startGame(msg.obj as File)
                    }
                    FAILURE -> {
                        dialog.dismiss()
                        errorDialog(msg.obj as String)
                    }
                }
            }
        }
        gm.install(input, handler)
    }

    private fun startGame(path: File) {
        val gameRoot = findGameRoot(path)
        if (gameRoot == null) {
            errorDialog("Cannot find game files (*.ald)")
            return
        }
        val i = Intent()
        i.setClass(applicationContext, GameActivity::class.java)
        i.putExtra(GameActivity.EXTRA_GAME_ROOT, gameRoot.path)
        i.putExtra(GameActivity.EXTRA_TITLE_FILE, File(path, GameManager.TITLE_FILE).path)
        startActivity(i)
    }

    private fun uninstall(id: Int) {
        gm.uninstall(id)
        adapter.remove(adapter.getItem(id))
    }

    private fun errorDialog(msg: String) {
        AlertDialog.Builder(this).setTitle("Error")
                .setMessage(msg)
                .setPositiveButton("OK") {_, _ -> }
                .show()
    }
}

@Suppress("DEPRECATION") // for ProgressDialog
class ProgressDialogFragment : DialogFragment() {
    private lateinit var dialog: ProgressDialog
    override fun onCreateDialog(savedInstanceState: Bundle?): Dialog {
        dialog = ProgressDialog(activity)
        return dialog.apply {
            setTitle("Installing")
            setCancelable(true)
        }
    }
    fun setProgress(msg: String) {
        dialog.setMessage(msg)
    }
}

private class GameManager(private val rootDir: File) {
    companion object {
        const val TITLE_FILE = "title.txt"
    }

    data class Entry(val path: File, val title: String)
    val games: ArrayList<Entry> = arrayListOf()
    val titles: List<String>
        get() = games.map(Entry::title)

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

    fun install(input: InputStream, handler: Handler) {
        var i = 0
        while (true) {
            val f = File(rootDir, i++.toString())
            if (!f.exists() && f.mkdir()) {
                val t = Thread {
                    extractFiles(input, f, handler)
                }
                t.start()
                return
            }
        }
    }

    fun uninstall(id: Int) {
        games[id].path.deleteRecursively()
        games.removeAt(id)
    }

    private fun extractFiles(input: InputStream, outDir: File, handler: Handler) {
        try {
            val zip = if (Build.VERSION.SDK_INT >= 24) {
                ZipInputStream(input.buffered(), Charset.forName("Shift_JIS"))
            } else {
                ZipInputStream(input.buffered())
            }
            while (true) {
                val zipEntry = zip.nextEntry ?: break
                Log.d("extractFiles", zipEntry.name)
                val path = File(outDir, zipEntry.name)
                if (zipEntry.isDirectory)
                    continue
                path.parentFile.mkdirs()
                handler.sendMessage(handler.obtainMessage(PROGRESS, "Extracting ${zipEntry.name}..."))
                val output = FileOutputStream(path).buffered()
                zip.copyTo(output)
                output.close()
            }
            zip.close()
            handler.sendMessage(handler.obtainMessage(SUCCESS, outDir))
        } catch (e: UTFDataFormatException) {
            // Attempted to read Shift_JIS zip in Android < 7
            handler.sendMessage(handler.obtainMessage(FAILURE, "This ZIP file is unsupported."))
        } catch (e: IOException) {
            Log.e("launcher", "Failed to extract ZIP", e)
            handler.sendMessage(handler.obtainMessage(FAILURE, "Failed to extract ZIP."))
        }
    }
}

private fun findGameRoot(path: File): File? {
    path.walkTopDown().forEach {
        val name = it.name.toLowerCase(Locale.US)
        if (name == "xsystem35.gr" || name.endsWith(".ald"))
            return it.parentFile
    }
    return null
}
