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
        Module.preRun = [
            function loadFont() {
                FS.createPreloadedFile('/', xsystem35.Font.fname, xsystem35.Font.url, true, false);
            },
            function prepareSaveDir() {
                FS.mkdir('/save');
                FS.mount(IDBFS, {}, '/save');
                Module.addRunDependency('syncfs');
                FS.syncfs(true, (err) => { Module.removeRunDependency('syncfs') });
            }
        ];
        xsystem35.cdPlayer = new CDPlayer(this.imageLoader);
    }

    setStatus(text: string) {
        console.log(text);
        this.status.innerHTML = text;
    }

    private fsyncTimer: number;
    syncfs(fname: string) {
        window.clearTimeout(this.fsyncTimer);
        this.fsyncTimer = window.setTimeout(() => {
            FS.syncfs(false, (err) => {
                if (err)
                    console.log("FS.syncfs error: ", err);
            });
        }, 100);
    }
}

class CDPlayer {
    audio = <HTMLAudioElement>$('audio');
    private blobs: Blob[];
    private currentTrack: number;

    constructor(private imageLoader:ImageLoader) {
        this.blobs = [];
    }

    play(track:number, loop:number) {
        this.currentTrack = track;
        if (this.blobs[track]) {
            this.startPlayback(this.blobs[track], loop);
            return;
        }
        this.imageLoader.getCDDA(track, (blob) => {
            this.blobs[track] = blob;
            this.startPlayback(blob, loop);
        });
    }

    stop() {
        this.audio.pause();
        this.currentTrack = null;
    }

    getPosition(): number {
        if (!this.currentTrack)
            return 0;
        var time = Math.round(this.audio.currentTime * 75);
        return this.currentTrack | time << 8;
    }

    private startPlayback(blob:Blob, loop:number) {
        this.audio.setAttribute('src', URL.createObjectURL(blob));
        this.audio.loop = (loop != 0);
        this.audio.load();
        this.audio.play();
    }
}

xsystem35.shell = new System35Shell();
