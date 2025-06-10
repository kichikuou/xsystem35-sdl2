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

import android.app.*
import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.provider.OpenableColumns
import android.text.Html
import android.util.Log
import android.view.Menu
import android.view.MenuItem
import android.widget.ArrayAdapter
import android.widget.ListView
import android.widget.TextView
import android.widget.Toast
import java.io.*

private const val CONTENT_TYPE_ZIP = "application/zip"
private const val INSTALL_REQUEST = 1
private const val SAVEDATA_EXPORT_REQUEST = 2
private const val SAVEDATA_IMPORT_REQUEST = 3
private const val STATE_PROGRESS_TEXT = "progressText"

class LauncherActivity : Activity(), LauncherObserver {
    private lateinit var launcher: Launcher
    private var progressDialog: Dialog? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.launcher)

        launcher = Launcher.getInstance(filesDir)
        launcher.observer = this
        if (launcher.isInstalling) {
            showProgressDialog(savedInstanceState)
        }

        onGameListChange()
        val listView = findViewById<ListView>(R.id.list)
        listView.emptyView = findViewById(R.id.empty)
        findViewById<TextView>(R.id.usage).text = Html.fromHtml(getString(R.string.usage))
        listView.setOnItemClickListener { _, _, position, _ ->
            onListItemClick(position)
        }
        listView.setOnItemLongClickListener { _, _, position, _ ->
            onItemLongClick(position)
        }
    }

    override fun onDestroy() {
        launcher.observer = null
        dismissProgressDialog()
        super.onDestroy()
    }

    override fun onSaveInstanceState(outState: Bundle) {
        progressDialog?.let {
            outState.putCharSequence(STATE_PROGRESS_TEXT, it.findViewById<TextView>(R.id.text).text)
        }
        super.onSaveInstanceState(outState)
    }

    private fun onListItemClick(position: Int) {
        startGame(launcher.games[position].path, null)
    }

    private fun onItemLongClick(position: Int): Boolean {
        AlertDialog.Builder(this).setTitle(R.string.confirm)
                .setMessage(getString(R.string.uninstall_dialog_message, launcher.games[position].title))
                .setPositiveButton(R.string.ok) {_, _ -> uninstall(position)}
                .setNegativeButton(R.string.cancel) {_, _ -> }
                .show()
        return true
    }

    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        menuInflater.inflate(R.menu.launcher_menu, menu)
        return true
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        return when (item.itemId) {
            R.id.install_from_zip -> {
                val i = Intent(Intent.ACTION_GET_CONTENT)
                i.type = CONTENT_TYPE_ZIP
                startActivityForResult(Intent.createChooser(i, getString(R.string.choose_a_file)), INSTALL_REQUEST)
                true
            }
            R.id.export_savedata -> {
                val i = Intent(Intent.ACTION_CREATE_DOCUMENT)
                i.type = CONTENT_TYPE_ZIP
                i.putExtra(Intent.EXTRA_TITLE, "savedata.zip")
                startActivityForResult(i, SAVEDATA_EXPORT_REQUEST)
                true
            }
            R.id.import_savedata -> {
                val i = Intent(Intent.ACTION_GET_CONTENT)
                i.type = CONTENT_TYPE_ZIP
                startActivityForResult(Intent.createChooser(i, getString(R.string.choose_a_file)),
                                       SAVEDATA_IMPORT_REQUEST)
                true
            }
            R.id.clear_savedata -> {
                AlertDialog.Builder(this)
                    .setTitle(R.string.confirm)
                    .setMessage(R.string.clear_save_data_confirm)
                    .setPositiveButton(R.string.ok) { _, _ ->
                        if (launcher.clearSaveData()) {
                            Toast.makeText(this, R.string.save_data_clear_success, Toast.LENGTH_SHORT).show()
                        } else {
                            errorDialog(R.string.save_data_clear_error)
                        }
                    }
                    .setNegativeButton(R.string.cancel, null)
                    .show()
                true
            }
            R.id.help -> {
                AlertDialog.Builder(this)
                    .setMessage(Html.fromHtml(getString(R.string.usage)))
                    .show()
                true
            }
            R.id.licenses -> {
                val intent = Intent(this, LicensesMenuActivity::class.java)
                startActivity(intent)
                true
            }
            else -> super.onOptionsItemSelected(item)
        }
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        if (resultCode != RESULT_OK)
            return
        val uri = data?.data ?: return
        when (requestCode) {
            INSTALL_REQUEST -> {
                val input = contentResolver.openInputStream(uri) ?: return
                showProgressDialog()
                launcher.install(input, getArchiveName(uri))
            }
            SAVEDATA_EXPORT_REQUEST -> try {
                launcher.exportSaveData(contentResolver.openOutputStream(uri)!!)
                Toast.makeText(this, R.string.save_data_export_success, Toast.LENGTH_SHORT).show()
            } catch (e: IOException) {
                Log.e("launcher", "Failed to export savedata", e)
                errorDialog(R.string.save_data_export_error)
            }
            SAVEDATA_IMPORT_REQUEST -> {
                val input = contentResolver.openInputStream(uri) ?: return
                val errMsgId = launcher.importSaveData(input)
                if (errMsgId == null) {
                    Toast.makeText(this, R.string.save_data_import_success, Toast.LENGTH_SHORT).show()
                } else {
                    errorDialog(errMsgId)
                }
            }
        }

    }

    override fun onGameListChange() {
        val items = launcher.titles.toMutableList()
        val listView = findViewById<ListView>(R.id.list)
        listView.adapter = ArrayAdapter(this, android.R.layout.simple_list_item_1, items)
    }

    override fun onInstallProgress(path: String) {
        progressDialog?.findViewById<TextView>(R.id.text)?.text = getString(R.string.install_progress, path)
    }

    override fun onInstallSuccess(path: File, archiveName: String?) {
        dismissProgressDialog()
        startGame(path, archiveName)
    }

    override fun onInstallFailure(msgId: Int) {
        dismissProgressDialog()
        errorDialog(msgId)
    }

    private fun startGame(path: File, archiveName: String?) {
        val i = Intent()
        i.setClass(applicationContext, GameActivity::class.java)
        i.putExtra(GameActivity.EXTRA_GAME_ROOT, path.path)
        i.putExtra(GameActivity.EXTRA_SAVE_DIRECTORY, File(filesDir, Launcher.SAVE_DIR).path)
        i.putExtra(GameActivity.EXTRA_ARCHIVE_NAME, archiveName)
        startActivity(i)
    }

    private fun uninstall(id: Int) {
        launcher.uninstall(id)
    }

    private fun showProgressDialog(savedInstanceState: Bundle? = null) {
        progressDialog = Dialog(this)
        progressDialog!!.apply {
            setTitle(R.string.install_dialog_title)
            setCancelable(false)
            setContentView(R.layout.progress_dialog)
            savedInstanceState?.let {
                findViewById<TextView>(R.id.text)?.text = it.getCharSequence(STATE_PROGRESS_TEXT)
            }
            show()
        }
    }

    private fun dismissProgressDialog() {
        progressDialog?.dismiss()
        progressDialog = null
    }

    private fun errorDialog(msgId: Int) {
        AlertDialog.Builder(this).setTitle(R.string.error_dialog_title)
                .setMessage(msgId)
                .setPositiveButton(R.string.ok) {_, _ -> }
                .show()
    }

    private fun getArchiveName(uri: Uri): String? {
        val cursor = contentResolver.query(uri, null, null, null, null, null)
        cursor?.use {
            if (it.moveToFirst()) {
                val column = it.getColumnIndex(OpenableColumns.DISPLAY_NAME)
                if (column >= 0) {
                    val fname = it.getString(column)
                    return if (fname.endsWith(".zip", true)) fname.dropLast(4) else fname
                }
            }
        }
        return null
    }
}
