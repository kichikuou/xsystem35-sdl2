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
import java.io.OutputStream
import java.nio.ByteBuffer
import java.nio.channels.FileChannel
import java.util.Locale

internal class CdImageReader private constructor(
    private val image: RandomAccessImage,
    private val tracks: List<TrackInfo?>,
) : Closeable {
    fun readSector(sector: Int, buffer: ByteArray) {
        readDataFully(sector.toLong() * ISO_SECTOR_SIZE, buffer, 0, ISO_SECTOR_SIZE)
    }

    fun readDataFully(offset: Long, buffer: ByteArray, bufferOffset: Int, length: Int) {
        var logicalOffset = offset
        var outOffset = bufferOffset
        var remaining = length
        val sectorBuffer = ByteArray(ISO_SECTOR_SIZE)
        while (remaining > 0) {
            val sector = (logicalOffset / ISO_SECTOR_SIZE).toInt()
            val offsetInSector = (logicalOffset % ISO_SECTOR_SIZE).toInt()
            val count = minOf(remaining, ISO_SECTOR_SIZE - offsetInSector)
            readSectorPayload(sector, sectorBuffer)
            sectorBuffer.copyInto(buffer, outOffset, offsetInSector, offsetInSector + count)
            logicalOffset += count
            outOffset += count
            remaining -= count
        }
    }

    fun audioTracks(): List<Int> {
        return tracks.mapIndexedNotNull { trackNumber, track ->
            if (track?.isAudio == true) trackNumber else null
        }
    }

    fun extractAudioTrack(trackNumber: Int, output: OutputStream) {
        val track = tracks.getOrNull(trackNumber)
        if (track?.isAudio != true) {
            throw InstallFailureException(R.string.unsupported_cd_image)
        }
        val dataSize = track.numSectors.toLong() * RAW_AUDIO_SECTOR_SIZE
        writeWaveHeader(output, dataSize)
        val buffer = ByteArray(track.blockSize * 16)
        var sector = 0
        while (sector < track.numSectors) {
            val sectorsToRead = minOf(16, track.numSectors - sector)
            val bytesToRead = sectorsToRead * track.blockSize
            image.readFully(track.offset + sector.toLong() * track.blockSize, buffer, 0, bytesToRead)
            output.write(buffer, 0, sectorsToRead * RAW_AUDIO_SECTOR_SIZE)
            sector += sectorsToRead
        }
    }

    override fun close() {
        image.close()
    }

    private fun readSectorPayload(sector: Int, buffer: ByteArray) {
        val track = findDataTrack(sector)
        val offset = track.offset + (sector - track.startSector).toLong() * track.blockSize + track.blockOffset
        image.readFully(offset, buffer, 0, ISO_SECTOR_SIZE)
    }

    private fun findDataTrack(sector: Int): TrackInfo {
        for (track in tracks) {
            if (track == null || track.isAudio) {
                continue
            }
            if (sector >= track.startSector && sector < track.startSector + track.numSectors) {
                return track
            }
        }
        throw InstallFailureException(R.string.invalid_iso9660_image)
    }

    companion object {
        const val ISO_SECTOR_SIZE = 2048
        private const val RAW_AUDIO_SECTOR_SIZE = 2352
        private const val WAVE_HEADER_SIZE = 44

        fun open(
            contentResolver: ContentResolver,
            files: List<SelectedInstallFile>,
            tempDir: File,
        ): CdImageReader {
            val selected = selectImageFiles(files)
            val image = RandomAccessImage.open(contentResolver, selected.image, tempDir)
            try {
                if (selected.metadata == null) {
                    return CdImageReader(image, listOf(null, TrackInfo.iso(image.size)))
                }
                val cueText = contentResolver.openInputStream(selected.metadata.uri)?.bufferedReader()?.use {
                    it.readText()
                } ?: throw InstallFailureException(R.string.cd_image_read_error)
                return CdImageReader(image, parseCue(cueText, image.size))
            } catch (e: Exception) {
                image.close()
                throw e
            }
        }

        private fun selectImageFiles(files: List<SelectedInstallFile>): SelectedImageFiles {
            if (files.isEmpty()) {
                throw InstallFailureException(R.string.unsupported_cd_image)
            }
            val isoFiles = files.filter { it.displayName.lowercase(Locale.US).endsWith(".iso") }
            if (isoFiles.size == 1 && files.size == 1) {
                return SelectedImageFiles(isoFiles.single(), null)
            }

            val imageFiles = files.filter {
                val name = it.displayName.lowercase(Locale.US)
                name.endsWith(".bin") || name.endsWith(".img")
            }
            val cueFiles = files.filter { it.displayName.lowercase(Locale.US).endsWith(".cue") }
            if (imageFiles.size == 1 && cueFiles.isEmpty() && files.size == 1 ||
                imageFiles.isEmpty() && cueFiles.size == 1 && files.size == 1
            ) {
                throw InstallFailureException(R.string.missing_cd_image_metadata)
            }
            if (imageFiles.size != 1 || cueFiles.size != 1 || files.size != 2) {
                throw InstallFailureException(R.string.unsupported_cd_image)
            }
            val imageBase = imageFiles.single().baseName()
            val cueBase = cueFiles.single().baseName()
            if (imageBase != cueBase) {
                throw InstallFailureException(R.string.missing_cd_image_metadata)
            }
            return SelectedImageFiles(imageFiles.single(), cueFiles.single())
        }

        private fun parseCue(cueText: String, imageSize: Long): List<TrackInfo?> {
            val cueTracks = mutableListOf<CueTrack?>()
            var currentTrack: Int? = null
            for (line in cueText.lines()) {
                val fields = line.trim().split(Regex("\\s+"))
                if (fields.isEmpty()) {
                    continue
                }
                when (fields[0].uppercase(Locale.US)) {
                    "TRACK" -> {
                        if (fields.size < 3) {
                            throw InstallFailureException(R.string.unsupported_cd_image)
                        }
                        currentTrack = fields[1].toIntOrNull()
                            ?: throw InstallFailureException(R.string.unsupported_cd_image)
                        while (cueTracks.size <= currentTrack) {
                            cueTracks.add(null)
                        }
                        cueTracks[currentTrack] = when (fields[2].uppercase(Locale.US)) {
                            "MODE1/2048" -> CueTrack(false, 2048, 0)
                            "MODE1/2352" -> CueTrack(false, 2352, 16)
                            "AUDIO" -> CueTrack(true, 2352, 0)
                            else -> throw InstallFailureException(R.string.unsupported_cd_image)
                        }
                    }
                    "INDEX" -> {
                        val trackNumber = currentTrack ?: continue
                        if (fields.size < 3) {
                            throw InstallFailureException(R.string.unsupported_cd_image)
                        }
                        val indexNumber = fields[1].toIntOrNull()
                            ?: throw InstallFailureException(R.string.unsupported_cd_image)
                        cueTracks[trackNumber]?.index?.put(indexNumber, indexToSector(fields[2]))
                    }
                }
            }
            return makeTrackInfo(cueTracks, imageSize)
        }

        private fun makeTrackInfo(cueTracks: List<CueTrack?>, imageSize: Long): List<TrackInfo?> {
            val tracks = MutableList<TrackInfo?>(cueTracks.size) { null }
            var offset = 0L
            var startSector = 0
            for (trackNumber in 1 until cueTracks.size) {
                val cueTrack = cueTracks[trackNumber] ?: continue
                val index1 = cueTrack.index[1]
                    ?: throw InstallFailureException(R.string.unsupported_cd_image)
                cueTrack.index[0]?.takeIf { it != 0 }?.let { index0 ->
                    val gap = index1 - index0
                    if (gap > 0) {
                        startSector += gap
                        offset += gap.toLong() * cueTrack.blockSize
                    }
                }
                val nextTrack = cueTracks.drop(trackNumber + 1).firstOrNull { it != null }
                val nextStart = nextTrack?.let { it.index[0]?.takeIf { index -> index != 0 } ?: it.index[1] }
                val numSectors = if (nextStart != null) {
                    nextStart - index1
                } else {
                    ((imageSize - offset) / cueTrack.blockSize).toInt()
                }
                if (numSectors <= 0) {
                    throw InstallFailureException(R.string.unsupported_cd_image)
                }
                tracks[trackNumber] = TrackInfo(
                    isAudio = cueTrack.isAudio,
                    offset = offset,
                    blockSize = cueTrack.blockSize,
                    blockOffset = cueTrack.blockOffset,
                    startSector = startSector,
                    numSectors = numSectors,
                )
                startSector += numSectors
                offset += numSectors.toLong() * cueTrack.blockSize
            }
            return tracks
        }

        private fun indexToSector(index: String): Int {
            val parts = index.split(":").map {
                it.toIntOrNull() ?: throw InstallFailureException(R.string.unsupported_cd_image)
            }
            if (parts.size != 3) {
                throw InstallFailureException(R.string.unsupported_cd_image)
            }
            return parts[0] * 60 * 75 + parts[1] * 75 + parts[2]
        }

        private fun writeWaveHeader(output: OutputStream, dataSize: Long) {
            val header = ByteArray(WAVE_HEADER_SIZE)
            writeAscii(header, 0, "RIFF")
            writeLittleEndianInt(header, 4, dataSize + 36)
            writeAscii(header, 8, "WAVE")
            writeAscii(header, 12, "fmt ")
            writeLittleEndianInt(header, 16, 16)
            writeLittleEndianShort(header, 20, 1)
            writeLittleEndianShort(header, 22, 2)
            writeLittleEndianInt(header, 24, 44100)
            writeLittleEndianInt(header, 28, 44100 * 2 * 2)
            writeLittleEndianShort(header, 32, 2 * 2)
            writeLittleEndianShort(header, 34, 16)
            writeAscii(header, 36, "data")
            writeLittleEndianInt(header, 40, dataSize)
            output.write(header)
        }

        private fun writeAscii(buffer: ByteArray, offset: Int, value: String) {
            value.toByteArray(Charsets.US_ASCII).copyInto(buffer, offset)
        }

        private fun writeLittleEndianShort(buffer: ByteArray, offset: Int, value: Int) {
            buffer[offset] = value.toByte()
            buffer[offset + 1] = (value shr 8).toByte()
        }

        private fun writeLittleEndianInt(buffer: ByteArray, offset: Int, value: Long) {
            buffer[offset] = value.toByte()
            buffer[offset + 1] = (value shr 8).toByte()
            buffer[offset + 2] = (value shr 16).toByte()
            buffer[offset + 3] = (value shr 24).toByte()
        }
    }
}

private data class SelectedImageFiles(
    val image: SelectedInstallFile,
    val metadata: SelectedInstallFile?,
)

private data class CueTrack(
    val isAudio: Boolean,
    val blockSize: Int,
    val blockOffset: Int,
    val index: MutableMap<Int, Int> = mutableMapOf(),
)

private data class TrackInfo(
    val isAudio: Boolean,
    val offset: Long,
    val blockSize: Int,
    val blockOffset: Int,
    val startSector: Int,
    val numSectors: Int,
) {
    companion object {
        fun iso(imageSize: Long): TrackInfo {
            return TrackInfo(
                isAudio = false,
                offset = 0,
                blockSize = CdImageReader.ISO_SECTOR_SIZE,
                blockOffset = 0,
                startSector = 0,
                numSectors = (imageSize / CdImageReader.ISO_SECTOR_SIZE).toInt(),
            )
        }
    }
}

private class RandomAccessImage private constructor(
    private val channel: FileChannel,
    private val descriptor: ParcelFileDescriptor?,
    private val tempFile: File?,
) : Closeable {
    val size: Long = channel.size()

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
        fun open(
            contentResolver: ContentResolver,
            file: SelectedInstallFile,
            tempDir: File,
        ): RandomAccessImage {
            val descriptor = contentResolver.openFileDescriptor(file.uri, "r")
                ?: throw InstallFailureException(R.string.cd_image_read_error)
            try {
                val channel = FileInputStream(descriptor.fileDescriptor).channel
                channel.size()
                return RandomAccessImage(channel, descriptor, null)
            } catch (e: Exception) {
                descriptor.close()
            }

            val tempFile = File.createTempFile("cdimage-", ".img", tempDir)
            try {
                contentResolver.openInputStream(file.uri)?.use { input ->
                    tempFile.outputStream().buffered().use { output ->
                        input.copyTo(output)
                    }
                } ?: throw InstallFailureException(R.string.cd_image_read_error)
                return RandomAccessImage(FileInputStream(tempFile).channel, null, tempFile)
            } catch (e: Exception) {
                tempFile.delete()
                throw e
            }
        }
    }
}

private fun SelectedInstallFile.baseName(): String {
    return displayName.substringBeforeLast('.').lowercase(Locale.US)
}
