/// <reference path="util.ts" />
/// <reference path="loader.ts" />
/// <reference path="zoom.ts" />
/// <reference path="volume.ts" />
/// <reference path="cdda.ts" />
/// <reference path="audio.ts" />

namespace xsystem35 {
    const Font = { url: 'fonts/MTLc3m.ttf', fname: 'MTLc3m.ttf'};
    export const xsys35rc = [
        'font_device: ttf',
        'ttfont_mincho: ' + Font.fname,
        'ttfont_gothic: ' + Font.fname, '',
    ].join('\n');
    export let cdPlayer: CDPlayer;
    export let audio: AudioManager;

    class System35Shell {
        readonly fileSystemReady: Promise<any>;
        private imageLoader: ImageLoader;
        status: HTMLElement = document.getElementById('status');
        private zoom: ZoomManager;
        private volumeControl: VolumeControl;
        private antialiasCheckbox: HTMLInputElement;

        constructor() {
            let fileSystemReady: () => void;
            this.fileSystemReady = new Promise((resolve) => { fileSystemReady = resolve; });

            this.imageLoader = new ImageLoader(this.fileSystemReady);
            this.imageLoader.installed.then(this.run.bind(this));
            this.setStatus('Downloading...');
            window.onerror = () => {
                this.setStatus('Exception thrown, see JavaScript console');
                this.setStatus = (text: string) => {
                    if (text) Module.printErr('[post-exception status] ' + text);
                };
            };

            // Initialize the Module object
            Module.TOTAL_MEMORY = 96 * 1024 * 1024;
            Module.print = Module.printErr = console.log.bind(console);
            Module.canvas = document.getElementById('canvas');
            Module.noInitialRun = true;
            Module.setStatus = this.setStatus.bind(this);
            Module.preRun = [
                fileSystemReady,
                function loadFont() {
                    FS.createPreloadedFile('/', Font.fname, Font.url, true, false);
                },
                function prepareSaveDir() {
                    FS.mkdir('/save');
                    FS.mount(IDBFS, {}, '/save');
                    Module.addRunDependency('syncfs');
                    FS.syncfs(true, (err) => { Module.removeRunDependency('syncfs'); });
                },
            ];
            this.volumeControl = new VolumeControl();
            xsystem35.cdPlayer = new CDPlayer(this.imageLoader, this.volumeControl);
            this.zoom = new ZoomManager();
            this.antialiasCheckbox = <HTMLInputElement>$('#antialias');
            this.antialiasCheckbox.addEventListener('change', this.antialiasChanged.bind(this));
            this.antialiasCheckbox.checked = localStorage.getItem('antialias') !== 'false';
            xsystem35.audio = new AudioManager(this.volumeControl);
        }

        run() {
            $('#loader').hidden = true;
            $('#xsystem35').hidden = false;
            setTimeout(() => {
                Module.callMain();
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
                        console.log('FS.syncfs error: ', err);
                });
            }, 100);
        }

        private antialiasChanged() {
            localStorage.setItem('antialias', String(this.antialiasCheckbox.checked));
            _ags_setAntialiasedStringMode(this.antialiasCheckbox.checked ? 1 : 0);
        }

        private addRightClickEmulation() {
            let emulatingRightClick = false;
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

    export let shell = new System35Shell();
}
