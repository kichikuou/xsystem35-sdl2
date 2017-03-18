var $: (selector:string)=>HTMLElement = document.querySelector.bind(document);

// xsystem35 exported functions
declare function _musfade_setvolval_all(vol: number): void;
declare function _ags_setAntialiasedStringMode(on: number): void;
declare function _sdl_rightButton(down: number): void;

declare namespace Module {
    var noInitialRun: boolean;

    // Undocumented methods / attributes
    var canvas: HTMLElement;
    function setStatus(status: string): void;
    function monitorRunDependencies(left: number): void;
    function callMain(): void;
    function getMemory(size: number): number;
}

declare namespace FS {
    function writeFile(path: string, data: ArrayBufferView | string, opts?: {encoding?: string; flags?: string; canOwn?: boolean}): void;
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
    private zoom:ZoomManager;
    private volumeControl: VolumeControl;
    private antialiasCheckbox: HTMLInputElement;

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
        Module.TOTAL_MEMORY = 96*1024*1024;
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
        this.volumeControl = new VolumeControl();
        this.volumeControl.addEventListener(this.updateVolume.bind(this));
        xsystem35.cdPlayer = new CDPlayer(this.imageLoader, this.volumeControl);
        this.zoom = new ZoomManager();
        this.antialiasCheckbox = <HTMLInputElement>$('#antialias');
        this.antialiasCheckbox.addEventListener('change', this.antialiasChanged.bind(this));
        this.antialiasCheckbox.checked = localStorage.getItem('antialias') != 'false';
    }

    run() {
        $('#loader').hidden = true;
        $('#xsystem35').hidden = false;
        setTimeout(() => {
            Module.callMain();
            this.updateVolume();
            this.antialiasChanged();
        }, 0);
        this.addRightClickEmulation();
    }

    setStatus(text: string) {
        console.log(text);
        this.status.innerHTML = text;
    }

    windowSizeChanged() {
        this.zoom.handleZoom();
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

    private updateVolume() {
        _musfade_setvolval_all(Math.round(this.volumeControl.volume() * 100));
    }

    private antialiasChanged() {
        localStorage.setItem('antialias', String(this.antialiasCheckbox.checked));
        _ags_setAntialiasedStringMode(this.antialiasCheckbox.checked ? 1 : 0);
    }

    private addRightClickEmulation() {
        var emulatingRightClick = false;
        document.body.addEventListener('touchstart', (e) => {
            if (e.target !== document.body)
                return;
            _sdl_rightButton(1);
            emulatingRightClick = true;
        });
        document.body.addEventListener('touchend', (e) => {
            if (!emulatingRightClick)
                return;
            _sdl_rightButton(0);
            emulatingRightClick = false;
        });
    }
}

class CDPlayer {
    private audio = <HTMLAudioElement>$('audio');
    private blobs: Blob[];
    private currentTrack: number;
    waiting: boolean;

    constructor(private imageLoader:ImageLoader, private volumeControl: VolumeControl) {
        this.blobs = [];
        this.volumeControl.addEventListener(this.onVolumeChanged.bind(this));
        this.audio.volume = this.volumeControl.volume();
        this.waiting = false;
    }

    play(track:number, loop:number) {
        this.currentTrack = track;
        this.waiting = true;
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
        this.waiting = false;
    }

    private onVolumeChanged(evt: CustomEvent) {
        this.audio.volume = evt.detail;
    }
}

class ZoomManager {
    private canvas: HTMLCanvasElement;
    private zoomSelect: HTMLInputElement;
    private smoothingCheckbox: HTMLInputElement;

    constructor() {
        this.canvas = <HTMLCanvasElement>$('#canvas');
        this.zoomSelect = <HTMLInputElement>$('#zoom');
        this.zoomSelect.addEventListener('change', this.handleZoom.bind(this));
        this.zoomSelect.value = localStorage.getItem('zoom') || '1';
        this.smoothingCheckbox = <HTMLInputElement>$('#smoothing');
        this.smoothingCheckbox.addEventListener('change', this.handleSmoothing.bind(this));
        if (localStorage.getItem('smoothing') == 'false') {
            this.smoothingCheckbox.checked = false;
            this.handleSmoothing();
        }
    }

    handleZoom() {
        var value = this.zoomSelect.value;
        localStorage.setItem('zoom', value);
        var contentsStyle = $('.contents').style;
        if (value == 'fit') {
            contentsStyle.maxWidth = 'none';
            contentsStyle.width = this.canvas.style.width = '100%';
        } else {
            var ratio = Number(value);
            contentsStyle.maxWidth = 'none';
            contentsStyle.width = this.canvas.style.width = this.canvas.width * ratio + 'px';
        }
    }

    private handleSmoothing() {
        localStorage.setItem('smoothing', String(this.smoothingCheckbox.checked));
        if (this.smoothingCheckbox.checked)
            this.canvas.classList.remove('pixelated');
        else
            this.canvas.classList.add('pixelated');
    }
}

class VolumeControl {
    private vol: number;  // 0.0 - 1.0
    private muted: boolean;
    private elem: HTMLElement;
    private icon: HTMLElement;
    private slider: HTMLInputElement;

    constructor() {
        this.vol = Number(localStorage.getItem('volume') || 1);
        this.muted = false;

        this.elem = document.getElementById('volume-control');
        this.icon = document.getElementById('volume-control-icon');
        this.slider = <HTMLInputElement>document.getElementById('volume-control-slider');
        this.slider.value = String(Math.round(this.vol * 100));

        this.icon.addEventListener('click', this.onIconClicked.bind(this));
        this.slider.addEventListener('input', this.onSliderValueChanged.bind(this));
        this.slider.addEventListener('change', this.onSliderValueSettled.bind(this));
    }

    volume(): number {
        return this.muted ? 0 : parseInt(this.slider.value) / 100;
    }

    addEventListener(handler: (evt: CustomEvent) => any) {
        this.elem.addEventListener('volumechange', handler);
    }

    private onIconClicked(e: Event) {
        this.muted = !this.muted;
        if (this.muted) {
            this.icon.classList.remove('fa-volume-up');
            this.icon.classList.add('fa-volume-off');
            this.slider.value = '0';
        } else {
            this.icon.classList.remove('fa-volume-off');
            this.icon.classList.add('fa-volume-up');
            this.slider.value = String(Math.round(this.vol * 100));
        }
        this.dispatchEvent();
    }

    private onSliderValueChanged(e: Event) {
        this.vol = parseInt(this.slider.value) / 100;
        if (this.vol > 0 && this.muted) {
            this.muted = false;
            this.icon.classList.remove('fa-volume-off');
            this.icon.classList.add('fa-volume-up');
        }
        this.dispatchEvent();
    }

    private onSliderValueSettled(e: Event) {
        localStorage.setItem('volume', this.vol + '');
    }

    private dispatchEvent() {
        var event = new CustomEvent('volumechange', {detail: this.volume()});
        this.elem.dispatchEvent(event);
    }
}

xsystem35.shell = new System35Shell();
