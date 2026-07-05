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

import kotlinx.coroutines.runBlocking
import org.junit.Assert.assertArrayEquals
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Assert.fail
import org.junit.Rule
import org.junit.Test
import org.junit.rules.TemporaryFolder
import java.io.ByteArrayOutputStream
import java.io.File
import java.nio.charset.Charset

class Iso9660FileSystemTest {
    @get:Rule
    val tmp = TemporaryFolder()

    @Test
    fun extractsGameDataDirectoryFromPrimaryDescriptor() = runBlocking {
        val iso = SyntheticIsoBuilder()
            .addGameDataFile("SA.ALD;1", "ald".toByteArray())
            .addGameDataFile("SUB/FILE.TXT;1", "text".toByteArray())
            .writeTo(tmp.newFile("game.iso"))

        CdImageReader.openForFile(iso).use { reader ->
            val fs = Iso9660FileSystem(reader)
            val gameData = fs.findGameDataDirectory()
                ?: throw AssertionError("GAMEDATA directory was not found")
            val extracted = mutableMapOf<String, ByteArray>()

            fs.extractDirectory(gameData, "GAMEDATA") { path, input ->
                extracted[path] = input.readBytes()
            }

            assertArrayEquals("ald".toByteArray(), extracted["GAMEDATA/SA.ALD"])
            assertArrayEquals("text".toByteArray(), extracted["GAMEDATA/SUB/FILE.TXT"])
        }
    }

    @Test
    fun decodesPrimaryDescriptorNamesAsShiftJis() = runBlocking {
        val name = "表.TXT;1"
        val iso = SyntheticIsoBuilder()
            .addGameDataFile(name, "sjis".toByteArray())
            .writeTo(tmp.newFile("sjis.iso"))

        CdImageReader.openForFile(iso).use { reader ->
            val fs = Iso9660FileSystem(reader)
            val gameData = fs.findGameDataDirectory()
                ?: throw AssertionError("GAMEDATA directory was not found")
            val extracted = mutableMapOf<String, ByteArray>()

            fs.extractDirectory(gameData, "GAMEDATA") { path, input ->
                extracted[path] = input.readBytes()
            }

            assertArrayEquals("sjis".toByteArray(), extracted["GAMEDATA/表.TXT"])
        }
    }

    @Test
    fun prefersJolietDescriptorOverPrimaryDescriptor() = runBlocking {
        val iso = SyntheticIsoBuilder()
            .addGameDataFile("PRIMARY.TXT;1", "primary".toByteArray())
            .addJolietGameDataFile("日本語.TXT;1", "joliet".toByteArray())
            .writeTo(tmp.newFile("joliet.iso"))

        CdImageReader.openForFile(iso).use { reader ->
            val fs = Iso9660FileSystem(reader)
            val gameData = fs.findGameDataDirectory()
                ?: throw AssertionError("GAMEDATA directory was not found")
            val extracted = mutableMapOf<String, ByteArray>()

            fs.extractDirectory(gameData, "GAMEDATA") { path, input ->
                extracted[path] = input.readBytes()
            }

            assertFalse(extracted.containsKey("GAMEDATA/PRIMARY.TXT"))
            assertArrayEquals("joliet".toByteArray(), extracted["GAMEDATA/日本語.TXT"])
        }
    }

    @Test
    fun rejectsInvalidVolumeDescriptorSignature() {
        val iso = tmp.newFile("invalid.iso")
        iso.writeBytes(ByteArray(18 * CdImageReader.ISO_SECTOR_SIZE))

        val e = assertInstallFailure(R.string.invalid_iso9660_image) {
            CdImageReader.openForFile(iso).use { Iso9660FileSystem(it) }
        }
        assertEquals(R.string.invalid_iso9660_image, e.msgId)
    }
}

class CdImageReaderTest {
    @get:Rule
    val tmp = TemporaryFolder()

    @Test
    fun readsIsoSectorAt2048ByteOffset() {
        val sector0 = sectorFilledWith(0x10)
        val sector1 = sectorFilledWith(0x20)
        val image = tmp.newFile("plain.iso")
        image.writeBytes(sector0 + sector1)

        CdImageReader.openForFile(image).use { reader ->
            val buffer = ByteArray(CdImageReader.ISO_SECTOR_SIZE)

            reader.readSector(1, buffer)

            assertArrayEquals(sector1, buffer)
            assertTrue(reader.audioTracks().isEmpty())
        }
    }

    @Test
    fun cueMode12048ReadsDataSector() {
        val payload = sectorFilledWith(0x31)
        val image = tmp.newFile("mode2048.bin")
        image.writeBytes(payload)

        CdImageReader.openForFile(image, "mode2048.cue", cue("MODE1/2048").toByteArray()).use { reader ->
            val buffer = ByteArray(CdImageReader.ISO_SECTOR_SIZE)

            reader.readSector(0, buffer)

            assertArrayEquals(payload, buffer)
        }
    }

    @Test
    fun cueMode12352ReadsPayloadAtOffset16() {
        val payload = sectorFilledWith(0x42)
        val image = tmp.newFile("mode2352.bin")
        image.writeBytes(rawDataSector(payload, 0x11))

        CdImageReader.openForFile(image, "mode2352.cue", cue("MODE1/2352").toByteArray()).use { reader ->
            val buffer = ByteArray(CdImageReader.ISO_SECTOR_SIZE)

            reader.readSector(0, buffer)

            assertArrayEquals(payload, buffer)
        }
    }

    @Test
    fun cuePregapAdvancesDataTrackStart() {
        val pregapPayload = sectorFilledWith(0x21)
        val dataPayload = sectorFilledWith(0x22)
        val image = tmp.newFile("pregap.bin")
        image.writeBytes(rawDataSector(pregapPayload, 0x01) + rawDataSector(dataPayload, 0x02))
        val metadata = """
            FILE "pregap.bin" BINARY
              TRACK 01 MODE1/2352
                INDEX 00 00:00:00
                INDEX 01 00:00:01
        """.trimIndent()

        CdImageReader.openForFile(image, "pregap.cue", metadata.toByteArray()).use { reader ->
            val buffer = ByteArray(CdImageReader.ISO_SECTOR_SIZE)

            reader.readSector(1, buffer)

            assertArrayEquals(dataPayload, buffer)
        }
    }

    @Test
    fun extractsCueAudioTrackAsWav() {
        val dataPayload = sectorFilledWith(0x33)
        val audio = ByteArray(RAW_AUDIO_SECTOR_SIZE) { it.toByte() }
        val image = tmp.newFile("audio.bin")
        image.writeBytes(rawDataSector(dataPayload, 0) + audio)
        val metadata = """
            FILE "audio.bin" BINARY
              TRACK 01 MODE1/2352
                INDEX 01 00:00:00
              TRACK 02 AUDIO
                INDEX 01 00:00:01
        """.trimIndent()

        CdImageReader.openForFile(image, "audio.cue", metadata.toByteArray()).use { reader ->
            val output = ByteArrayOutputStream()

            assertEquals(listOf(2), reader.audioTracks())
            reader.extractAudioTrack(2, output)

            val wav = output.toByteArray()
            assertEquals("RIFF", wav.decodeAscii(0, 4))
            assertEquals(RAW_AUDIO_SECTOR_SIZE + 36, wav.readLeInt(4))
            assertEquals("WAVE", wav.decodeAscii(8, 4))
            assertEquals("data", wav.decodeAscii(36, 4))
            assertEquals(RAW_AUDIO_SECTOR_SIZE, wav.readLeInt(40))
            assertEquals(44100, wav.readLeInt(24))
            assertEquals(2, wav.readLeShort(22))
            assertEquals(16, wav.readLeShort(34))
            assertArrayEquals(audio, wav.copyOfRange(44, 44 + RAW_AUDIO_SECTOR_SIZE))
        }
    }

    @Test
    fun rejectsUnsupportedCueMode() {
        val image = tmp.newFile("unsupported.bin")
        image.writeBytes(ByteArray(CdImageReader.ISO_SECTOR_SIZE))

        assertInstallFailure(R.string.unsupported_cd_image) {
            CdImageReader.openForFile(image, "unsupported.cue", cue("MODE2/2352").toByteArray())
        }
    }

    @Test
    fun ccdUsesMode0ForAudioAndMode1PayloadOffsetForData() {
        val dataPayload = sectorFilledWith(0x52)
        val audio = ByteArray(RAW_AUDIO_SECTOR_SIZE) { (255 - it).toByte() }
        val image = tmp.newFile("disc.img")
        image.writeBytes(rawDataSector(dataPayload, 0x08) + audio)
        val metadata = """
            [TRACK 1]
            MODE=1
            INDEX 1=0
            [TRACK 2]
            MODE=0
            INDEX 1=1
        """.trimIndent()

        CdImageReader.openForFile(image, "disc.ccd", metadata.toByteArray()).use { reader ->
            val buffer = ByteArray(CdImageReader.ISO_SECTOR_SIZE)
            val output = ByteArrayOutputStream()

            reader.readSector(0, buffer)
            reader.extractAudioTrack(2, output)

            assertArrayEquals(dataPayload, buffer)
            assertEquals(listOf(2), reader.audioTracks())
            assertArrayEquals(audio, output.toByteArray().copyOfRange(44, 44 + RAW_AUDIO_SECTOR_SIZE))
        }
    }

    @Test
    fun mdsMapsModeAAToDataAndModeA9ToAudio() {
        val dataPayload = sectorFilledWith(0x61)
        val audio = ByteArray(RAW_AUDIO_SECTOR_SIZE) { (it * 3).toByte() }
        val image = tmp.newFile("disc.mdf")
        image.writeBytes(rawDataSector(dataPayload, 0x09) + audio)

        CdImageReader.openForFile(image, "disc.mds", mds(track1Sectors = 1, track2Sectors = 1)).use { reader ->
            val buffer = ByteArray(CdImageReader.ISO_SECTOR_SIZE)
            val output = ByteArrayOutputStream()

            reader.readSector(0, buffer)
            reader.extractAudioTrack(2, output)

            assertArrayEquals(dataPayload, buffer)
            assertEquals(listOf(2), reader.audioTracks())
            assertArrayEquals(audio, output.toByteArray().copyOfRange(44, 44 + RAW_AUDIO_SECTOR_SIZE))
        }
    }

    @Test
    fun rejectsMdsWithoutMediaDescriptorSignature() {
        val image = tmp.newFile("bad.mdf")
        image.writeBytes(ByteArray(RAW_AUDIO_SECTOR_SIZE))

        assertInstallFailure(R.string.unsupported_cd_image) {
            CdImageReader.openForFile(image, "bad.mds", ByteArray(0x70))
        }
    }
}

class GameInstallerCdImageTest {
    @get:Rule
    val tmp = TemporaryFolder()

    @Test
    fun installsDataOnlyIso() = runBlocking {
        val root = tmp.newFolder("root")
        val iso = SyntheticIsoBuilder()
            .addGameDataFile("SA.ALD;1", "ald".toByteArray())
            .addGameDataFile("TITLE.TXT;1", "title".toByteArray())
            .writeTo(tmp.newFile("game.iso"))

        CdImageReader.openForFile(iso).use { reader ->
            val gameDir = GameInstaller(GameStore(root)).installCdImageForTest(reader) {}

            assertEquals(File(root, "0/GAMEDATA").canonicalFile, gameDir.canonicalFile)
            assertEquals("GAMEDATA", File(root, "0/${Launcher.GAMEDIR_FILE}").readText())
            assertEquals("ald", File(gameDir, "SA.ALD").readText())
            assertEquals("title", File(gameDir, "TITLE.TXT").readText())
        }
    }

    @Test
    fun installsCueAudioTrackAndPlaylist() = runBlocking {
        val root = tmp.newFolder("root")
        val isoSector = SyntheticIsoBuilder()
            .addGameDataFile("SA.ALD;1", "ald".toByteArray())
            .toByteArray()
        val dataSectors = isoSector.toList().chunked(CdImageReader.ISO_SECTOR_SIZE)
            .joinToByteArray { rawDataSector(it.toByteArray(), 0) }
        val audio = ByteArray(RAW_AUDIO_SECTOR_SIZE) { (it and 0x7f).toByte() }
        val image = tmp.newFile("audio.bin")
        image.writeBytes(dataSectors + audio)
        val dataSectorCount = isoSector.size / CdImageReader.ISO_SECTOR_SIZE
        val metadata = """
            FILE "audio.bin" BINARY
              TRACK 01 MODE1/2352
                INDEX 01 00:00:00
              TRACK 02 AUDIO
                INDEX 01 ${dataSectorCount.toMsf()}
        """.trimIndent()

        CdImageReader.openForFile(image, "audio.cue", metadata.toByteArray()).use { reader ->
            val gameDir = GameInstaller(GameStore(root)).installCdImageForTest(reader) {}

            val wav = File(gameDir, "cdda/track02.wav")
            assertTrue(wav.exists())
            assertEquals("\ncdda/track02.wav", File(gameDir, Launcher.PLAYLIST_FILE).readText())
        }
    }

    @Test
    fun rejectsIsoWithoutGameDataDirectoryAndDeletesIncompleteInstall() = runBlocking {
        val root = tmp.newFolder("root")
        val iso = SyntheticIsoBuilder(includeGameData = false).writeTo(tmp.newFile("nogamedata.iso"))

        CdImageReader.openForFile(iso).use { reader ->
            assertInstallFailure(R.string.cannot_find_game_data_directory) {
                runBlocking {
                    GameInstaller(GameStore(root)).installCdImageForTest(reader) {}
                }
            }
        }

        assertFalse(File(root, "0").exists())
    }

    @Test
    fun rejectsIsoWithoutAldAndDeletesIncompleteInstall() = runBlocking {
        val root = tmp.newFolder("root")
        val iso = SyntheticIsoBuilder()
            .addGameDataFile("README.TXT;1", "readme".toByteArray())
            .writeTo(tmp.newFile("noald.iso"))

        CdImageReader.openForFile(iso).use { reader ->
            assertInstallFailure(R.string.cannot_find_ald) {
                runBlocking {
                    GameInstaller(GameStore(root)).installCdImageForTest(reader) {}
                }
            }
        }

        assertFalse(File(root, "0").exists())
    }

    @Test
    fun resolveOutputPathRejectsTraversal() {
        val base = tmp.newFolder("out")

        assertThrowsIOException {
            resolveOutputPath(base, "../evil")
        }
        assertThrowsIOException {
            resolveOutputPath(base, File(base.parentFile, "evil").absolutePath)
        }
    }
}

private class SyntheticIsoBuilder(
    private val includeGameData: Boolean = true,
) {
    private val primaryFiles = mutableMapOf<String, ByteArray>()
    private val jolietFiles = mutableMapOf<String, ByteArray>()

    fun addGameDataFile(path: String, content: ByteArray): SyntheticIsoBuilder {
        primaryFiles[path] = content
        return this
    }

    fun addJolietGameDataFile(path: String, content: ByteArray): SyntheticIsoBuilder {
        jolietFiles[path] = content
        return this
    }

    fun writeTo(file: File): File {
        file.writeBytes(toByteArray())
        return file
    }

    fun toByteArray(): ByteArray {
        val sectors = MutableList(80) { ByteArray(CdImageReader.ISO_SECTOR_SIZE) }
        var nextSector = 30
        val primaryTree = buildTree(primaryFiles, Charset.forName("Shift_JIS"))
        val primaryRoot = writeTree(sectors, primaryTree, ROOT_SECTOR, GAMEDATA_SECTOR, nextSector, false)
        nextSector = primaryRoot.nextSector

        writeVolumeDescriptor(sectors[16], 1, ROOT_SECTOR, CdImageReader.ISO_SECTOR_SIZE, false)
        var terminatorSector = 17
        if (jolietFiles.isNotEmpty()) {
            val jolietTree = buildTree(jolietFiles, Charsets.UTF_16BE)
            writeTree(sectors, jolietTree, JOLIET_ROOT_SECTOR, JOLIET_GAMEDATA_SECTOR, nextSector, true)
            writeVolumeDescriptor(sectors[17], 2, JOLIET_ROOT_SECTOR, CdImageReader.ISO_SECTOR_SIZE, true)
            terminatorSector = 18
        }
        writeTerminator(sectors[terminatorSector])
        return sectors.flattenToByteArray()
    }

    private fun buildTree(files: Map<String, ByteArray>, charset: Charset): DirectoryNode {
        val root = DirectoryNode("")
        if (!includeGameData) {
            return root
        }
        val gameData = DirectoryNode("GAMEDATA")
        root.directories["GAMEDATA"] = gameData
        for ((path, content) in files) {
            var directory = gameData
            val parts = path.split("/")
            for (part in parts.dropLast(1)) {
                directory = directory.directories.getOrPut(part.substringBefore(";")) {
                    DirectoryNode(part.substringBefore(";"))
                }
            }
            val rawName = parts.last().toByteArray(charset)
            directory.files.add(FileNode(rawName, content))
        }
        return root
    }

    private fun writeTree(
        sectors: MutableList<ByteArray>,
        root: DirectoryNode,
        rootSector: Int,
        gameDataSector: Int,
        firstFileSector: Int,
        joliet: Boolean,
    ): TreeWriteResult {
        var nextSector = firstFileSector
        root.extent = rootSector
        root.size = CdImageReader.ISO_SECTOR_SIZE
        if (includeGameData) {
            root.directories["GAMEDATA"]!!.extent = gameDataSector
            root.directories["GAMEDATA"]!!.size = CdImageReader.ISO_SECTOR_SIZE
        }
        assignDirectorySectors(root, gameDataSector + 1)
        nextSector = writeFileContents(sectors, root, nextSector)
        writeDirectorySector(sectors[rootSector], root, root, joliet)
        if (includeGameData) {
            writeDirectorySectors(sectors, root.directories["GAMEDATA"]!!, root, joliet)
        }
        return TreeWriteResult(nextSector)
    }

    private fun assignDirectorySectors(directory: DirectoryNode, nextDirectorySector: Int): Int {
        var sector = nextDirectorySector
        for (child in directory.directories.values) {
            if (child.name == "GAMEDATA") {
                sector = assignDirectorySectors(child, sector)
                continue
            }
            child.extent = sector++
            child.size = CdImageReader.ISO_SECTOR_SIZE
            sector = assignDirectorySectors(child, sector)
        }
        return sector
    }

    private fun writeFileContents(
        sectors: MutableList<ByteArray>,
        directory: DirectoryNode,
        firstFileSector: Int,
    ): Int {
        var sector = firstFileSector
        for (child in directory.directories.values) {
            sector = writeFileContents(sectors, child, sector)
        }
        for (file in directory.files) {
            file.extent = sector
            file.size = file.content.size
            val sectorCount = (file.content.size + CdImageReader.ISO_SECTOR_SIZE - 1) /
                CdImageReader.ISO_SECTOR_SIZE
            for (i in 0 until sectorCount) {
                file.content.copyInto(
                    sectors[sector + i],
                    0,
                    i * CdImageReader.ISO_SECTOR_SIZE,
                    minOf(file.content.size, (i + 1) * CdImageReader.ISO_SECTOR_SIZE)
                )
            }
            sector += maxOf(1, sectorCount)
        }
        return sector
    }

    private fun writeDirectorySectors(
        sectors: MutableList<ByteArray>,
        directory: DirectoryNode,
        parent: DirectoryNode,
        joliet: Boolean,
    ) {
        writeDirectorySector(sectors[directory.extent], directory, parent, joliet)
        for (child in directory.directories.values) {
            writeDirectorySectors(sectors, child, directory, joliet)
        }
    }

    private fun writeDirectorySector(
        sector: ByteArray,
        directory: DirectoryNode,
        parent: DirectoryNode,
        joliet: Boolean,
    ) {
        var offset = 0
        offset += writeRecord(sector, offset, byteArrayOf(0), directory.extent, directory.size, true)
        offset += writeRecord(sector, offset, byteArrayOf(1), parent.extent, parent.size, true)
        for (child in directory.directories.values) {
            val name = if (joliet) child.name.toByteArray(Charsets.UTF_16BE) else child.name.toByteArray()
            offset += writeRecord(sector, offset, name, child.extent, child.size, true)
        }
        for (file in directory.files) {
            offset += writeRecord(sector, offset, file.rawName, file.extent, file.size, false)
        }
    }

    private fun writeVolumeDescriptor(
        sector: ByteArray,
        type: Int,
        rootExtent: Int,
        rootSize: Int,
        joliet: Boolean,
    ) {
        sector[0] = type.toByte()
        "CD001".toByteArray(Charsets.US_ASCII).copyInto(sector, 1)
        sector[6] = 1
        if (joliet) {
            sector[88] = 0x25
            sector[89] = 0x2f
            sector[90] = 0x45
        }
        writeRecord(sector, 156, byteArrayOf(0), rootExtent, rootSize, true)
    }

    private fun writeTerminator(sector: ByteArray) {
        sector[0] = 255.toByte()
        "CD001".toByteArray(Charsets.US_ASCII).copyInto(sector, 1)
        sector[6] = 1
    }

    private fun writeRecord(
        buffer: ByteArray,
        offset: Int,
        rawName: ByteArray,
        extent: Int,
        size: Int,
        isDirectory: Boolean,
    ): Int {
        val length = 33 + rawName.size + if (rawName.size % 2 == 0) 1 else 0
        buffer[offset] = length.toByte()
        buffer.writeLeInt(offset + 2, extent)
        buffer.writeLeInt(offset + 10, size)
        buffer[offset + 25] = if (isDirectory) 0x02 else 0x00
        buffer[offset + 32] = rawName.size.toByte()
        rawName.copyInto(buffer, offset + 33)
        return length
    }

    companion object {
        private const val ROOT_SECTOR = 20
        private const val GAMEDATA_SECTOR = 21
        private const val JOLIET_ROOT_SECTOR = 24
        private const val JOLIET_GAMEDATA_SECTOR = 25
    }
}

private data class DirectoryNode(
    val name: String,
    val directories: MutableMap<String, DirectoryNode> = linkedMapOf(),
    val files: MutableList<FileNode> = mutableListOf(),
    var extent: Int = 0,
    var size: Int = 0,
)

private data class FileNode(
    val rawName: ByteArray,
    val content: ByteArray,
    var extent: Int = 0,
    var size: Int = 0,
)

private data class TreeWriteResult(val nextSector: Int)

private const val RAW_AUDIO_SECTOR_SIZE = 2352

private fun cue(mode: String): String = """
    FILE "image.bin" BINARY
      TRACK 01 $mode
        INDEX 01 00:00:00
""".trimIndent()

private fun mds(track1Sectors: Int, track2Sectors: Int): ByteArray {
    val bytes = ByteArray(0x70 + 2 * 0x50 + 2 * 8)
    "MEDIA DESCRIPTOR".toByteArray(Charsets.US_ASCII).copyInto(bytes, 0)
    bytes[0x62] = 2
    val track1 = 0x70
    bytes[track1] = 0xaa.toByte()
    bytes[track1 + 0x04] = 1
    bytes.writeLeShort(track1 + 0x10, RAW_AUDIO_SECTOR_SIZE)
    bytes.writeLeInt(track1 + 0x28, 0)
    bytes.writeLeInt(0x70 + 2 * 0x50 + 0x04, track1Sectors)
    val track2 = 0x70 + 0x50
    bytes[track2] = 0xa9.toByte()
    bytes[track2 + 0x04] = 2
    bytes.writeLeShort(track2 + 0x10, RAW_AUDIO_SECTOR_SIZE)
    bytes.writeLeInt(track2 + 0x28, track1Sectors * RAW_AUDIO_SECTOR_SIZE)
    bytes.writeLeInt(0x70 + 2 * 0x50 + 8 + 0x04, track2Sectors)
    return bytes
}

private fun sectorFilledWith(value: Int): ByteArray {
    return ByteArray(CdImageReader.ISO_SECTOR_SIZE) { value.toByte() }
}

private fun rawDataSector(payload: ByteArray, prefixValue: Int): ByteArray {
    val sector = ByteArray(RAW_AUDIO_SECTOR_SIZE) { prefixValue.toByte() }
    payload.copyInto(sector, 16, 0, CdImageReader.ISO_SECTOR_SIZE)
    return sector
}

private fun Int.toMsf(): String {
    val minutes = this / (60 * 75)
    val seconds = this / 75 % 60
    val frames = this % 75
    return "%02d:%02d:%02d".format(minutes, seconds, frames)
}

private fun Iterable<Byte>.toByteArray(): ByteArray {
    val result = ByteArray(count())
    forEachIndexed { index, byte -> result[index] = byte }
    return result
}

private fun List<ByteArray>.flattenToByteArray(): ByteArray {
    val result = ByteArray(sumOf { it.size })
    var offset = 0
    for (bytes in this) {
        bytes.copyInto(result, offset)
        offset += bytes.size
    }
    return result
}

private fun List<List<Byte>>.joinToByteArray(transform: (List<Byte>) -> ByteArray): ByteArray {
    return map(transform).flattenToByteArray()
}

private fun ByteArray.decodeAscii(offset: Int, length: Int): String {
    return String(this, offset, length, Charsets.US_ASCII)
}

private fun ByteArray.readLeShort(offset: Int): Int {
    return (this[offset].toInt() and 0xff) or ((this[offset + 1].toInt() and 0xff) shl 8)
}

private fun ByteArray.readLeInt(offset: Int): Int {
    return readLeShort(offset) or (readLeShort(offset + 2) shl 16)
}

private fun ByteArray.writeLeShort(offset: Int, value: Int) {
    this[offset] = value.toByte()
    this[offset + 1] = (value shr 8).toByte()
}

private fun ByteArray.writeLeInt(offset: Int, value: Int) {
    writeLeShort(offset, value)
    writeLeShort(offset + 2, value shr 16)
}

private fun assertInstallFailure(expectedMsgId: Int, block: () -> Unit): InstallFailureException {
    try {
        block()
    } catch (e: InstallFailureException) {
        assertEquals(expectedMsgId, e.msgId)
        return e
    }
    fail("Expected InstallFailureException")
    throw AssertionError()
}

private fun assertThrowsIOException(block: () -> Unit) {
    try {
        block()
    } catch (e: java.io.IOException) {
        return
    }
    fail("Expected IOException")
}
