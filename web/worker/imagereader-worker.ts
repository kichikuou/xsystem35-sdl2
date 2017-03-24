importScripts('cdimage.js');

declare function postMessage(data: any, transfers: any): void;

class Installer {
    private imgFile: File;
    private cueFile: File;
    private imageReader: CDImageReader;

    constructor() {
        addEventListener('message', this.onMessage.bind(this));
    }

    private onMessage(evt: MessageEvent) {
        switch (evt.data.command) {
        case 'setFile':
            this.setFile(evt.data.file);
            let imgName = this.imgFile && this.imgFile.name;
            let cueName = this.cueFile && this.cueFile.name;
            postMessage({command: 'readyState', img: imgName, cue: cueName});
            break;
        case 'extractFiles':
            if (this.ready())
                this.extractFiles();
            break;
        case 'getTrack':
            if (this.ready())
                this.getTrack(evt.data.track);
            break;
        default:
            throw('unknown command: ' + evt.data.command);
        }
    }

    private setFile(file: File) {
        let name = file.name.toLowerCase();
        if (name.endsWith('.img') || name.endsWith('.mdf'))
            this.imgFile = file;
        else if (name.endsWith('.cue') || name.endsWith('.mds'))
            this.cueFile = file;
        if (this.imgFile && this.cueFile) {
            if (this.cueFile.name.endsWith('.cue'))
                this.imageReader = new ImgCueReader(this.imgFile, this.cueFile);
            else
                this.imageReader = new MdfMdsReader(this.imgFile, this.cueFile);
        }
    }

    private ready(): boolean {
        return !!this.imageReader;
    }

    private extractFiles() {
        let isofs = new ISO9660FileSystem(this.imageReader);
        // this.walk(isofs, isofs.rootDir(), '/');
        let gamedata = isofs.getDirEnt('gamedata', isofs.rootDir());
        if (!gamedata) {
            postMessage({command: 'error', message: 'インストールできません。GAMEDATAフォルダが見つかりません。'});
            return;
        }
        let files: any = {};
        let transfers = [];
        for (let e of isofs.readDir(gamedata)) {
            if (e.name.toLowerCase().endsWith('.ald')) {
                let buffer = new ArrayBuffer(e.size);
                let uint8 = new Uint8Array(buffer);
                let ptr = 0;
                for (let buf of isofs.readFile(e)) {
                    uint8.set(buf, ptr);
                    ptr += buf.byteLength;
                }
                if (ptr !== e.size)
                    throw('expected ' + e.size + ' bytes, but read ' + ptr + 'bytes');
                files[e.name] = buffer;
                transfers.push(buffer);
            }
        }
        postMessage({command: 'extractFiles',
                     volumeLabel: isofs.volumeLabel(),
                     files}, transfers);
    }

    private walk(isofs: ISO9660FileSystem, dir: DirEnt, dirname: string) {
        for (let e of isofs.readDir(dir)) {
            if (e.name !== '\0' && e.name !== '\x01') {
                console.log(dirname + e.name);
                if (e.isDirectory)
                    this.walk(isofs, e, dirname + e.name + '/');
            }
        }
    }

    private getTrack(track: number) {
        let blob = this.imageReader.extractTrack(track);
        postMessage({command: 'complete', track, wav: blob});
    }
}

let installer = new Installer();
