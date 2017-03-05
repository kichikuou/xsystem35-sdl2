var $: (selector:string)=>HTMLElement = document.querySelector.bind(document);

declare namespace Module {
    var noInitialRun: boolean;

    // Undocumented methods / attributes
    var canvas: HTMLElement;
    function setStatus(status: string): void;
    function monitorRunDependencies(left: number): void;
    function callMain(): void;
}

declare namespace FS {
    function writeFile(path: string, data: ArrayBufferView | string, opts?: {encoding?: string; flags?: string}): void;
}

namespace xsystem35 {
    export const Font = { url: 'fonts/MTLc3m.ttf', fname: 'MTLc3m.ttf'};
    export const xsys35rc = [
        'font_device: ttf',
        'ttfont_mincho: ' + Font.fname,
        'ttfont_gothic: ' + Font.fname, ''
    ].join('\n');
    export var cdPlayer: CDPlayer;
    export var shell: System35Shell;
}

class System35Shell {
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
        Module.preRun = [
            () => { FS.createPreloadedFile('/', xsystem35.Font.fname, xsystem35.Font.url, true, false); }
        ];
        xsystem35.cdPlayer = new CDPlayer(this.imageLoader);
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

xsystem35.shell = new System35Shell();
