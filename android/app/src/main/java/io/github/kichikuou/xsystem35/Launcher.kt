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

import android.content.ContentResolver
import android.util.Log
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.io.*

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

class Launcher private constructor(rootDir: File) {
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

    }

    private val store = GameStore(rootDir)
    private val installer = GameInstaller(store)
    val games: List<GameStore.Entry>
        get() = store.games
    val titles: List<String>
        get() = store.titles
    var observer: LauncherObserver? = null
    private val scope = CoroutineScope(SupervisorJob() + Dispatchers.Main)
    private var installJob: Job? = null
    var installState: InstallState = InstallState.Idle
        private set

    fun installZip(input: InputStream, archiveName: String?) {
        if (installJob?.isActive == true) {
            input.close()
            return
        }
        startInstallJob {
            val gameDir = withContext(Dispatchers.IO) {
                input.use {
                    installer.installZip(it) { msg ->
                        withContext(Dispatchers.Main) {
                            setInstallProgress(msg)
                        }
                    }
                }
            }
            InstallResult(gameDir, archiveName)
        }
    }

    fun installCdImage(
        files: List<SelectedInstallFile>,
        contentResolver: ContentResolver,
        tempDir: File,
    ) {
        if (installJob?.isActive == true) {
            return
        }
        startInstallJob {
            val gameDir = withContext(Dispatchers.IO) {
                installer.installCdImage(files, contentResolver, tempDir) { msg ->
                    withContext(Dispatchers.Main) {
                        setInstallProgress(msg)
                    }
                }
            }
            InstallResult(gameDir, null)
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
                Log.e("launcher", "Failed to install game", e)
                setInstallFailed(R.string.install_error)
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
        store.uninstall(id)
        observer?.onGameListChange()
    }

    fun refreshGameList() {
        store.updateGameList()
        observer?.onGameListChange()
    }

    // Throws IOException
    fun exportSaveData(output: OutputStream) {
        store.exportSaveData(output)
    }

    fun importSaveData(input: InputStream): Int? {
        return store.importSaveData(input)
    }

    fun clearSaveData(): Boolean {
        return store.clearSaveData()
    }
}
