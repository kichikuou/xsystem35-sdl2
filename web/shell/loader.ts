/// <reference path="util.ts" />

namespace xsystem35 {
    export class ImageLoader {
        private worker: Worker;
        private cddaCallback: (wab: Blob) => void;

        constructor(private runMain: () => void) {
            this.initWorker();
            $('#fileselect').addEventListener('change', this.handleFileSelect.bind(this), false);
            document.body.ondragover = this.handleDragOver.bind(this);
            document.body.ondrop = this.handleDrop.bind(this);
        }

        getCDDA(track: number, callback: (wav: Blob) => void) {
            this.send({ command: 'getTrack', track });
            this.cddaCallback = callback;
        }

        private initWorker() {
            this.worker = new Worker('imagereader-worker.js');
            this.worker.addEventListener('message', this.onMessage.bind(this));
            this.worker.addEventListener('error', this.onWorkerError.bind(this));
        }

        private setReadyState(imgName: string, cueName: string) {
            if (imgName) {
                $('#imgReady').classList.remove('notready');
                $('#imgReady').textContent = imgName;
            }
            if (cueName) {
                $('#cueReady').classList.remove('notready');
                $('#cueReady').textContent = cueName;
            }
        }

        private populateFiles(files: any, volumeLabel: string) {
            let grGenerator = new GameResourceGenerator();
            for (let name in files) {
                let data: ArrayBuffer = files[name];
                // Store contents in the emscripten heap, so that it can be mmap-ed without copying
                let ptr = Module.getMemory(data.byteLength);
                Module.HEAPU8.set(new Uint8Array(data), ptr);
                FS.writeFile(name, Module.HEAPU8.subarray(ptr, ptr + data.byteLength),
                             { encoding: 'binary', canOwn: true });
                grGenerator.addFile(name);
            }
            FS.writeFile('xsystem35.gr', grGenerator.generate());
            FS.writeFile('.xsys35rc', xsystem35.xsys35rc);

            this.runMain();
        }

        private handleFileSelect(evt: Event) {
            let input = <HTMLInputElement>evt.target;
            let files = input.files;
            for (let i = 0; i < files.length; i++)
                this.setFile(files[i]);
            input.value = '';
        }

        private handleDragOver(evt: DragEvent) {
            evt.stopPropagation();
            evt.preventDefault();
            evt.dataTransfer.dropEffect = 'copy';
        }

        private handleDrop(evt: DragEvent) {
            evt.stopPropagation();
            evt.preventDefault();
            let files = evt.dataTransfer.files;
            for (let i = 0; i < files.length; i++)
                this.setFile(files[i]);
        }

        private setFile(file: File) {
            this.send({ command: 'setFile', file });
        }

        private startInstall() {
            this.send({ command: 'extractFiles' });
        }

        private send(msg: any) {
            this.worker.postMessage(msg);
        }

        private onMessage(evt: MessageEvent) {
            switch (evt.data.command) {
                case 'readyState':
                    this.setReadyState(evt.data.img, evt.data.cue);
                    if (evt.data.img && evt.data.cue)
                        this.startInstall();
                    break;
                case 'extractFiles':
                    this.populateFiles(evt.data.files, evt.data.volumeLabel);
                    break;
                case 'complete':
                    this.cddaCallback(evt.data.wav);
                    this.cddaCallback = null;
                    break;
                case 'error':
                    console.log(evt.data.message);
                    break;
                default:
                    throw('unknown command: ' + evt.data.command);
            }
        }

        private onWorkerError(evt: Event) {
            console.log('worker error', evt);
        }
    }

    class GameResourceGenerator {
        static resourceType: { [ch: string]: string } =
            { s: 'Scenario', g: 'Graphics', w: 'Wave', d: 'Data', r: 'Resource', m: 'Midi' };
        private basename: string;
        private lines: string[] = [];

        addFile(name: string) {
            let type = name.charAt(name.length - 6).toLowerCase();
            let id = name.charAt(name.length - 5);
            this.basename = name.slice(0, -6);
            this.lines.push(GameResourceGenerator.resourceType[type] + id.toUpperCase() + ' ' + name);
        }

        generate(): string {
            for (let i = 0; i < 26; i++) {
                let id = String.fromCharCode(65 + i);
                this.lines.push('Save' + id + ' save/' + this.basename + 's' + id.toLowerCase() + '.asd');
            }
            return this.lines.join('\n') + '\n';
        }

        isEmpty(): boolean {
            return this.lines.length === 0;
        }
    }
}
