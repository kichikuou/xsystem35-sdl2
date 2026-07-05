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

import java.io.InputStream
import java.nio.charset.Charset
import java.util.Locale

internal class Iso9660FileSystem(private val image: CdImageReader) {
    private val descriptor = readBestVolumeDescriptor()

    data class Entry(
        val name: String,
        val extent: Int,
        val size: Int,
        val isDirectory: Boolean,
    )

    fun findGameDataDirectory(): Entry? {
        return listDirectory(descriptor.rootDirectory).firstOrNull {
            it.isDirectory && it.name.uppercase(Locale.US) == "GAMEDATA"
        }
    }

    suspend fun extractDirectory(
        directory: Entry,
        outputPath: String,
        writeFile: suspend (String, InputStream) -> Unit,
    ) {
        for (entry in listDirectory(directory)) {
            val childPath = "$outputPath/${entry.name}"
            if (entry.isDirectory) {
                extractDirectory(entry, childPath, writeFile)
            } else {
                writeFile(childPath, fileInputStream(entry))
            }
        }
    }

    private fun listDirectory(directory: Entry): List<Entry> {
        val bytes = ByteArray(directory.size)
        image.readFully(
            directory.extent.toLong() * CdImageReader.ISO_SECTOR_SIZE,
            bytes,
            0,
            bytes.size
        )
        val entries = mutableListOf<Entry>()
        var offset = 0
        while (offset < bytes.size) {
            val length = bytes[offset].toInt() and 0xff
            if (length == 0) {
                offset = ((offset / CdImageReader.ISO_SECTOR_SIZE) + 1) * CdImageReader.ISO_SECTOR_SIZE
                continue
            }
            parseDirectoryRecord(bytes, offset, length)?.let { entries.add(it) }
            offset += length
        }
        return entries
    }

    private fun parseDirectoryRecord(bytes: ByteArray, offset: Int, length: Int): Entry? {
        if (length < 34) {
            throw InstallFailureException(R.string.invalid_iso9660_image)
        }
        val nameLength = bytes[offset + 32].toInt() and 0xff
        if (33 + nameLength > length) {
            throw InstallFailureException(R.string.invalid_iso9660_image)
        }
        val rawName = bytes.copyOfRange(offset + 33, offset + 33 + nameLength)
        if (rawName.size == 1 && (rawName[0].toInt() == 0 || rawName[0].toInt() == 1)) {
            return null
        }
        val name = descriptor.decodeName(rawName)
        val extent = readLittleEndianInt(bytes, offset + 2)
        val size = readLittleEndianInt(bytes, offset + 10)
        val flags = bytes[offset + 25].toInt() and 0xff
        return Entry(name, extent, size, flags and 0x02 != 0)
    }

    private fun fileInputStream(entry: Entry): InputStream {
        return object : InputStream() {
            private var pos = 0

            override fun read(): Int {
                val buffer = ByteArray(1)
                val read = read(buffer, 0, 1)
                return if (read < 0) -1 else buffer[0].toInt() and 0xff
            }

            override fun read(buffer: ByteArray, offset: Int, length: Int): Int {
                if (pos >= entry.size) {
                    return -1
                }
                val count = minOf(length, entry.size - pos)
                image.readFully(
                    entry.extent.toLong() * CdImageReader.ISO_SECTOR_SIZE + pos,
                    buffer,
                    offset,
                    count
                )
                pos += count
                return count
            }
        }
    }

    private fun readBestVolumeDescriptor(): VolumeDescriptor {
        var primary: VolumeDescriptor? = null
        var joliet: VolumeDescriptor? = null
        val sector = ByteArray(CdImageReader.ISO_SECTOR_SIZE)
        var sectorNumber = 0x10
        while (true) {
            image.readSector(sectorNumber++, sector)
            if (String(sector, 1, 5, Charsets.US_ASCII) != "CD001") {
                throw InstallFailureException(R.string.invalid_iso9660_image)
            }
            when (sector[0].toInt() and 0xff) {
                1 -> primary = VolumeDescriptor(parseRootDirectory(sector), false)
                2 -> if (isJolietDescriptor(sector)) {
                    joliet = VolumeDescriptor(parseRootDirectory(sector), true)
                }
                255 -> return joliet ?: primary
                    ?: throw InstallFailureException(R.string.invalid_iso9660_image)
            }
        }
    }

    private fun parseRootDirectory(sector: ByteArray): Entry {
        return Entry(
            name = "",
            extent = readLittleEndianInt(sector, 156 + 2),
            size = readLittleEndianInt(sector, 156 + 10),
            isDirectory = true,
        )
    }

    private fun isJolietDescriptor(sector: ByteArray): Boolean {
        return sector[88] == 0x25.toByte() &&
            sector[89] == 0x2f.toByte() &&
            (sector[90] == 0x40.toByte() || sector[90] == 0x43.toByte() || sector[90] == 0x45.toByte())
    }

    private data class VolumeDescriptor(
        val rootDirectory: Entry,
        val joliet: Boolean,
    ) {
        fun decodeName(rawName: ByteArray): String {
            val charset = if (joliet) Charsets.UTF_16BE else Charset.forName("Shift_JIS")
            return String(rawName, charset).substringBefore(";")
        }
    }
}

private fun readLittleEndianInt(bytes: ByteArray, offset: Int): Int {
    return (bytes[offset].toInt() and 0xff) or
        ((bytes[offset + 1].toInt() and 0xff) shl 8) or
        ((bytes[offset + 2].toInt() and 0xff) shl 16) or
        ((bytes[offset + 3].toInt() and 0xff) shl 24)
}
