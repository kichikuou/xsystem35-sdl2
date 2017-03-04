var $: (selector:string)=>HTMLElement = document.querySelector.bind(document);

declare namespace Module {
    var noInitialRun: boolean;

    // Undocumented methods / attributes
    var canvas: HTMLElement;
    function setStatus(status: string): void;
    function monitorRunDependencies(left: number): void;
    function callMain(): void;

    var cdPlayer: CDPlayer;
};

declare namespace FS {
    function writeFile(path: string, data: ArrayBufferView | string, opts?: {encoding?: string; flags?: string}): void;
};

class Sys3Shell {
    private imageLoader: ImageLoader;
    status: HTMLElement = document.getElementById('status');
    progress: HTMLProgressElement = <HTMLProgressElement>document.getElementById('progress');

    constructor() {
        this.imageLoader = new ImageLoader(this);
        this.setStatus('Downloading...');
        window.onerror = () => {
            this.setStatus('Exception thrown, see JavaScript console');
            this.setStatus = (text: string) => {
                if (text) Module.printErr('[post-exception status] ' + text);
            };
        };

        // Initialize the Module object
        Module.print = Module.printErr = console.log.bind(console);
        Module.canvas = document.getElementById('canvas');
        Module.noInitialRun = true;
        Module.setStatus = this.setStatus.bind(this);
        Module.monitorRunDependencies = this.monitorRunDependencies.bind(this);

        Module.cdPlayer = new CDPlayer(this.imageLoader);
    }

    setStatus(text: string) {
        var m = text.match(/([^(]+)\((\d+(\.\d+)?)\/(\d+)\)/);
        var now = Date.now();
        if (m && now - Date.now() < 30) return; // if this is a progress update, skip it if too soon
        if (m) {
            text = m[1];
            this.progress.value = parseInt(m[2])*100;
            this.progress.max = parseInt(m[4])*100;
            this.progress.hidden = false;
        } else {
            this.progress.value = null;
            this.progress.max = null;
            this.progress.hidden = true;
        }
        this.status.innerHTML = text;
    }

    totalDependencies: number = 0;
    monitorRunDependencies(left: number) {
        this.totalDependencies = Math.max(this.totalDependencies, left);
        this.setStatus(left ? 'Preparing... (' + (this.totalDependencies-left) + '/' + this.totalDependencies + ')' : 'All downloads complete.');
    }
}

class CDPlayer {
    audio = <HTMLAudioElement>$('audio');
    private blobs: Blob[];

    constructor(private imageLoader:ImageLoader) {
        this.blobs = [];
    }

    play(track:number, loop:number) {
        if (this.blobs[track]) {
            this.startPlayback(this.blobs[track]);
            return;
        }
        this.imageLoader.getCDDA(track, (blob) => {
            this.blobs[track] = blob;
            this.startPlayback(blob);
        });
    }

    stop() {
        this.audio.pause();
    }

    private startPlayback(blob:Blob) {
        this.audio.setAttribute('src', URL.createObjectURL(blob));
        this.audio.load();
        this.audio.play();
    }
}

class ImageLoader {
    private worker: Worker;
    private cddaCallback: (wab:Blob)=>void;

    constructor(private shell:Sys3Shell) {
        this.initWorker();
        $('#fileselect').addEventListener('change', this.handleFileSelect.bind(this), false);
        document.body.ondragover = this.handleDragOver.bind(this);
        document.body.ondrop = this.handleDrop.bind(this);
    }

    getCDDA(track:number, callback:(wav:Blob)=>void) {
        this.send({command:'getTrack', track:track});
        this.cddaCallback = callback;
    }

    private initWorker() {
        this.worker = new Worker('imagereader-worker.js');
        this.worker.addEventListener('message', this.onMessage.bind(this));
        this.worker.addEventListener('error', this.onWorkerError.bind(this));
    }

    private setReadyState(imgName:string, cueName:string) {
        if (imgName) {
            $('#imgReady').classList.remove('notready');
            $('#imgReady code').textContent = imgName;
        }
        if (cueName) {
            $('#cueReady').classList.remove('notready');
            $('#cueReady code').textContent = cueName;
        }
    }

    private populateFiles(files:any, volumeLabel:string) {
        var grGenerator = new GameResourceGenerator();
        for (var name in files) {
            var data: ArrayBufferView = new Uint8Array(files[name]);
            FS.writeFile(name, data, { encoding: 'binary' });
            grGenerator.addFile(name);
        }
        var gr = grGenerator.generate();
        console.log(gr);
        FS.writeFile('xsystem35.gr', gr);
        Module.callMain();
    }

    private handleFileSelect(evt:Event) {
        var input = <HTMLInputElement>evt.target;
        var files = input.files;
        for (var i = 0; i < files.length; i++)
            this.setFile(files[i]);
        input.value = '';
    }

    private handleDragOver(evt:DragEvent) {
        evt.stopPropagation();
        evt.preventDefault();
        evt.dataTransfer.dropEffect = 'copy';
    }

    private handleDrop(evt:DragEvent) {
        evt.stopPropagation();
        evt.preventDefault();
        var files = evt.dataTransfer.files;
        for (var i = 0; i < files.length; i++)
            this.setFile(files[i]);
    }

    private setFile(file:File) {
        this.send({command:'setFile', file:file});
    }

    private startInstall() {
        this.send({command:'extractFiles'});
    }

    private send(msg:any) {
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
        }
    }

    private onWorkerError(evt: Event) {
        console.log('worker error', evt);
    }
}

class GameResourceGenerator {
    static resourceType: {[ch:string]:string} = {s:'Scenario', g:'Graphics', w:'Wave', d:'Data', r:'Resource', m:'Midi'};
    private basename:string;
    private lines:string[] = [];

    addFile(name:string) {
        var type = name.charAt(name.length - 6).toLowerCase();
        var id = name.charAt(name.length - 5);
        this.basename = name.slice(0, -6);
        this.lines.push(GameResourceGenerator.resourceType[type] + id.toUpperCase() + ' ' + name);
    }

    generate(): string {
        for (var i = 0; i < 26; i++) {
            var id = String.fromCharCode(65 + i);
            this.lines.push('Save' + id + ' save/' + this.basename + 's' + id.toLowerCase() + '.asd');
        }
        return this.lines.join('\n') + '\n';
    }

    isEmpty(): boolean {
        return this.lines.length == 0;
    }
}

var shell = new Sys3Shell();
