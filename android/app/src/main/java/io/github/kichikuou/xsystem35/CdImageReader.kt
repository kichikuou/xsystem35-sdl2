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
import android.os.ParcelFileDescriptor
import java.io.Closeable
import java.io.EOFException
import java.io.File
import java.io.FileInputStream
import java.nio.ByteBuffer
import java.nio.channels.FileChannel

internal class CdImageReader private constructor(
    private val channel: FileChannel,
    private val descriptor: ParcelFileDescriptor?,
    private val tempFile: File?,
) : Closeable {
    fun readSector(sector: Int, buffer: ByteArray) {
        readFully(sector.toLong() * ISO_SECTOR_SIZE, buffer, 0, ISO_SECTOR_SIZE)
    }

    fun readFully(offset: Long, buffer: ByteArray, bufferOffset: Int, length: Int) {
        val byteBuffer = ByteBuffer.wrap(buffer, bufferOffset, length)
        var pos = offset
        while (byteBuffer.hasRemaining()) {
            val read = channel.read(byteBuffer, pos)
            if (read < 0) {
                throw EOFException("Unexpected end of CD image")
            }
            pos += read
        }
    }

    override fun close() {
        channel.close()
        descriptor?.close()
        tempFile?.delete()
    }

    companion object {
        const val ISO_SECTOR_SIZE = 2048

        fun openIso(
            contentResolver: ContentResolver,
            file: SelectedInstallFile,
            tempDir: File,
        ): CdImageReader {
            val descriptor = contentResolver.openFileDescriptor(file.uri, "r")
                ?: throw InstallFailureException(R.string.cd_image_read_error)
            try {
                val channel = FileInputStream(descriptor.fileDescriptor).channel
                channel.size()
                return CdImageReader(channel, descriptor, null)
            } catch (e: Exception) {
                descriptor.close()
            }

            val tempFile = File.createTempFile("cdimage-", ".iso", tempDir)
            try {
                contentResolver.openInputStream(file.uri)?.use { input ->
                    tempFile.outputStream().buffered().use { output ->
                        input.copyTo(output)
                    }
                } ?: throw InstallFailureException(R.string.cd_image_read_error)
                return CdImageReader(FileInputStream(tempFile).channel, null, tempFile)
            } catch (e: Exception) {
                tempFile.delete()
                throw e
            }
        }
    }
}
