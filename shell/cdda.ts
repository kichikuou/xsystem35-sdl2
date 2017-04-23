/// <reference path="util.ts" />
/// <reference path="volume.ts" />

namespace xsystem35 {
    export class CDPlayer {
        private audio = <HTMLAudioElement>$('audio');
        private blobCache: Blob[];
        private currentTrack: number;
        private isVolumeSupported: boolean;
        private unmute: () => void;  // Non-null if emulating mute by pause

        constructor(private imageLoader: ImageLoader, volumeControl: VolumeControl) {
            // Volume control of <audio> is not supported in iOS
            this.audio.volume = 0.5;
            this.isVolumeSupported = this.audio.volume !== 1;

            this.blobCache = [];
            volumeControl.addEventListener(this.onVolumeChanged.bind(this));
            this.audio.volume = volumeControl.volume();
            this.audio.addEventListener('error', this.onAudioError.bind(this));
            this.removeUserGestureRestriction();
            if (!this.isVolumeSupported) {
                volumeControl.hideSlider();
                if (this.audio.volume === 0)
                    this.unmute = () => {};
            }
            document.addEventListener('visibilitychange', this.onVisibilityChange.bind(this));
        }

        play(track: number, loop: number) {
            this.currentTrack = track;
            if (this.unmute) {
                this.unmute = () => { this.play(track, loop); };
                return;
            }
            if (this.blobCache[track]) {
                this.startPlayback(this.blobCache[track], loop);
                return;
            }
            this.audio.currentTime = 0;
            this.imageLoader.getCDDA(track).then((blob) => {
                this.blobCache[track] = blob;
                this.startPlayback(blob, loop);
            });
        }

        stop() {
            this.audio.pause();
            this.currentTrack = null;
            if (this.unmute)
                this.unmute = () => {};
        }

        getPosition(): number {
            if (!this.currentTrack)
                return 0;
            let time = Math.round(this.audio.currentTime * 75);
            if (this.unmute || this.audio.error)
                time += 750;  // unblock Kichikuou OP
            return this.currentTrack | time << 8;
        }

        private startPlayback(blob: Blob, loop: number) {
            this.audio.setAttribute('src', URL.createObjectURL(blob));
            this.audio.loop = (loop !== 0);
            this.audio.load();
            this.audio.play();
        }

        private onVisibilityChange() {
            if (document.hidden)
                this.blobCache = [];
        }

        private onVolumeChanged(evt: CustomEvent) {
            if (this.isVolumeSupported) {
                this.audio.volume = evt.detail;
                return;
            }
            let muted = evt.detail === 0;
            if (!!this.unmute === muted)
                return;
            if (muted) {
                this.audio.pause();
                this.unmute = () => { this.audio.play(); };
            } else {
                let unmute = this.unmute;
                this.unmute = null;
                unmute();
            }
        }

        private onAudioError(err: ErrorEvent) {
            let clone = document.importNode((<HTMLTemplateElement>$('#cdda-error')).content, true);
            let toast = xsystem35.shell.addToast(clone, 'danger');
            toast.querySelector('.cdda-reload-button').addEventListener('click', () => {
                this.imageLoader.reloadImage().then(() => {
                    this.play(this.currentTrack, this.audio.loop ? 1 : 0);
                    (<HTMLElement>toast.querySelector('.btn-clear')).click();
                });
            });
        }

        private removeUserGestureRestriction() {
            let hanlder = () => {
                if (!this.currentTrack) {
                    this.audio.load();
                    console.log('CDDA unlocked');
                }
                window.removeEventListener('touchend', hanlder);
            };
            window.addEventListener('touchend', hanlder);
        }
    }
}
