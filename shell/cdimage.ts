/// <reference path="util.ts" />

namespace CDImage {
    export class ISO9660FileSystem {
        static async create(sectorReader: Reader): Promise<ISO9660FileSystem> {
            let pvd = new PVD(await sectorReader.readSector(0x10));
            return new ISO9660FileSystem(sectorReader, pvd);
        }

        private constructor(private sectorReader: Reader, private pvd: PVD) {
            if (this.pvd.type !== 1)
                throw new Error('PVD not found');
        }

        volumeLabel(): string {
            return this.pvd.volumeLabel();
        }

        rootDir(): DirEnt {
            return this.pvd.rootDirEnt();
        }

        async getDirEnt(name: string, parent: DirEnt): Promise<DirEnt> {
            name = name.toLowerCase();
            for (let e of await this.readDir(parent)) {
                if (e.name.toLowerCase() === name)
                    return e;
            }
            return null;
        }

        async readDir(dirent: DirEnt): Promise<DirEnt[]> {
            let sector = dirent.sector;
            let position = 0;
            let length = dirent.size;
            let entries: DirEnt[] = [];
            let buf: ArrayBuffer;
            while (position < length) {
                if (position === 0)
                    buf = await this.sectorReader.readSector(sector);
                let child = new DirEnt(buf, position);
                if (child.length === 0) {
                    // Padded end of sector
                    position = 2048;
                } else {
                    entries.push(child);
                    position += child.length;
                }
                if (position > 2048)
                    throw new Error('dirent across sector boundary');
                if (position === 2048) {
                    sector++;
                    position = 0;
                    length -= 2048;
                }
            }
            return entries;
        }

        readFile(dirent: DirEnt): Promise<Uint8Array[]> {
            return this.sectorReader.readSequentialSectors(dirent.sector, dirent.size);
        }
    }

    class PVD {
        private view: DataView;
        constructor(private buf: ArrayBuffer) {
            this.view = new DataView(buf);
        }
        get type(): number {
            return this.view.getUint8(0);
        }
        volumeLabel(): string {
            return SJISArrayToString(new DataView(this.buf, 40, 32)).trim();
        }
        rootDirEnt(): DirEnt {
            return new DirEnt(this.buf, 156);
        }
    }

    export class DirEnt {
        private view: DataView;
        constructor(private buf: ArrayBuffer, private offset: number) {
            this.view = new DataView(buf, offset);
        }
        get length(): number {
            return this.view.getUint8(0);
        }
        get sector(): number {
            return this.view.getUint32(2, true);
        }
        get size(): number {
            return this.view.getUint32(10, true);
        }
        get isDirectory(): boolean {
            return (this.view.getUint8(25) & 2) !== 0;
        }
        get name(): string {
            let len = this.view.getUint8(32);
            return SJISArrayToString(new DataView(this.buf, this.offset + 33, len)).split(';')[0];
        }
    }

    export interface Reader {
        readSector(sector: number): Promise<ArrayBuffer>;
        readSequentialSectors(startSector: number, length: number): Promise<Uint8Array[]>;
        maxTrack(): number;
        extractTrack(track: number): Promise<Blob>;
        resetImage(image: File): void;
    }

    export async function createReader(img: File, cue: File) {
        if (cue.name.endsWith('.cue')) {
            let reader = new ImgCueReader(img);
            await reader.parseCue(cue);
            return reader;
        } else {
            let reader = new MdfMdsReader(img);
            await reader.parseMds(cue);
            return reader;
        }
    }

    class ImageReaderBase {
        constructor(public image: File) { }

        async readSequential(startOffset: number,
                             bytesToRead: number,
                             blockSize: number,
                             sectorSize: number,
                             sectorOffset: number): Promise<Uint8Array[]> {
            let sectors = Math.ceil(bytesToRead / sectorSize);
            let blob = this.image.slice(startOffset, startOffset + sectors * blockSize);
            let buf = await readFileAsArrayBuffer(blob);
            let bufs: Uint8Array[] = [];
            for (let i = 0; i < sectors; i++) {
                bufs.push(new Uint8Array(buf, i * blockSize + sectorOffset, Math.min(bytesToRead, sectorSize)));
                bytesToRead -= sectorSize;
            }
            return bufs;
        }

        resetImage(image: File) {
            this.image = image;
        }
    }

    class ImgCueReader extends ImageReaderBase implements Reader {
        private tracks: Array<{ type: string; index: string[]; }>;

        constructor(img: File) {
            super(img);
        }

        readSector(sector: number): Promise<ArrayBuffer> {
            let start = sector * 2352 + 16;
            let end = start + 2048;
            return readFileAsArrayBuffer(this.image.slice(start, end));
        }

        readSequentialSectors(startSector: number, length: number): Promise<Uint8Array[]> {
            return this.readSequential(startSector * 2352, length, 2352, 2048, 16);
        }

        async parseCue(cueFile: File) {
            let lines = (await readFileAsText(cueFile)).split('\n');
            this.tracks = [];
            let currentTrack: number = null;
            for (let line of lines) {
                let fields = line.trim().split(/\s+/);
                switch (fields[0]) {
                    case 'TRACK':
                        currentTrack = Number(fields[1]);
                        this.tracks[currentTrack] = { type: fields[2], index: [] };
                        break;
                    case 'INDEX':
                        if (currentTrack)
                            this.tracks[currentTrack].index[Number(fields[1])] = fields[2];
                        break;
                    default:
                        // Do nothing
                }
            }
        }

        maxTrack(): number {
            return this.tracks.length - 1;
        }

        async extractTrack(track: number): Promise<Blob> {
            if (!this.tracks[track] || this.tracks[track].type !== 'AUDIO')
                return;

            let start = this.indexToSector(this.tracks[track].index[1]) * 2352;
            let end: number;
            if (this.tracks[track + 1]) {
                let index = this.tracks[track + 1].index[0] || this.tracks[track + 1].index[1];
                end = this.indexToSector(index) * 2352;
            } else {
                end = this.image.size;
            }
            let size = end - start;
            return new Blob([createWaveHeader(size), this.image.slice(start, start + size)], { type: 'audio/wav' });
        }

        private indexToSector(index: string): number {
            let msf = index.split(':').map(Number);
            return msf[0] * 60 * 75 + msf[1] * 75 + msf[2];
        }
    }

    enum MdsTrackMode { Audio = 0xa9, Mode1 = 0xaa }

    class MdfMdsReader extends ImageReaderBase implements Reader {
        private tracks: Array<{ mode: number; sectorSize: number; offset: number; sectors: number; }>;

        constructor(mdf: File) {
            super(mdf);
        }

        async parseMds(mdsFile: File) {
            let buf = await readFileAsArrayBuffer(mdsFile);

            let signature = ASCIIArrayToString(new Uint8Array(buf, 0, 16));
            if (signature !== 'MEDIA DESCRIPTOR')
                throw new Error(mdsFile.name + ': not a mds file');

            let header = new DataView(buf, 0, 0x70);
            let entries = header.getUint8(0x62);

            this.tracks = [];
            for (let i = 0; i < entries; i++) {
                let trackData = new DataView(buf, 0x70 + i * 0x50, 0x50);
                let extraData = new DataView(buf, 0x70 + entries * 0x50 + i * 8, 8);
                let mode = trackData.getUint8(0x00);
                let track = trackData.getUint8(0x04);
                let sectorSize = trackData.getUint16(0x10, true);
                let offset = trackData.getUint32(0x28, true); // >4GB offset is not supported.
                let sectors = extraData.getUint32(0x4, true);
                if (track < 100)
                    this.tracks[track] = { mode, sectorSize, offset, sectors };
            }
            if (this.tracks[1].mode !== MdsTrackMode.Mode1)
                throw new Error('track 1 is not mode1');
        }

        readSector(sector: number): Promise<ArrayBuffer> {
            let start = sector * this.tracks[1].sectorSize + 16;
            let end = start + 2048;
            return readFileAsArrayBuffer(this.image.slice(start, end));
        }

        readSequentialSectors(startSector: number, length: number): Promise<Uint8Array[]> {
            let track = this.tracks[1];
            return this.readSequential(track.offset + startSector * track.sectorSize,
                                       length, track.sectorSize, 2048, 16);
        }

        maxTrack(): number {
            return this.tracks.length - 1;
        }

        async extractTrack(track: number): Promise<Blob> {
            if (!this.tracks[track] || this.tracks[track].mode !== MdsTrackMode.Audio)
                return;

            let size = this.tracks[track].sectors * 2352;
            let chunks = await this.readSequential(this.tracks[track].offset, size,
                                                   this.tracks[track].sectorSize, 2352, 0);
            return new Blob([<any>createWaveHeader(size)].concat(chunks), { type: 'audio/wav' });
        }
    }

    function createWaveHeader(size: number): ArrayBuffer {
        let buf = new ArrayBuffer(44);
        let view = new DataView(buf);
        view.setUint32(0, 0x52494646, false); // 'RIFF'
        view.setUint32(4, size + 36, true); // filesize - 8
        view.setUint32(8, 0x57415645, false); // 'WAVE'
        view.setUint32(12, 0x666D7420, false); // 'fmt '
        view.setUint32(16, 16, true); // size of fmt chunk
        view.setUint16(20, 1, true); // PCM format
        view.setUint16(22, 2, true); // stereo
        view.setUint32(24, 44100, true); // sampling rate
        view.setUint32(28, 176400, true); // bytes/sec
        view.setUint16(32, 4, true); // block size
        view.setUint16(34, 16, true); // bit/sample
        view.setUint32(36, 0x64617461, false); // 'data'
        view.setUint32(40, size, true); // data size
        return buf;
    }
}
