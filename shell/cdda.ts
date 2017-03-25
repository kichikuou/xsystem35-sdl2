/// <reference path="util.ts" />
/// <reference path="volume.ts" />

namespace xsystem35 {
    export class CDPlayer {
        private audio = <HTMLAudioElement>$('audio');
        private blobs: Blob[];
        private currentTrack: number;
        waiting: boolean;

        constructor(private imageLoader: ImageLoader, volumeControl: VolumeControl) {
            this.blobs = [];
            volumeControl.addEventListener(this.onVolumeChanged.bind(this));
            this.audio.volume = volumeControl.volume();
            this.waiting = false;
            this.removeUserGestureRestriction();
        }

        play(track: number, loop: number) {
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
            let time = Math.round(this.audio.currentTime * 75);
            return this.currentTrack | time << 8;
        }

        private startPlayback(blob: Blob, loop: number) {
            this.audio.setAttribute('src', URL.createObjectURL(blob));
            this.audio.loop = (loop !== 0);
            this.audio.load();
            this.audio.play();
            this.waiting = false;
        }

        private onVolumeChanged(evt: CustomEvent) {
            this.audio.volume = evt.detail;
        }

        private removeUserGestureRestriction() {
            let hanlder = () => {
                this.audio.load();
                console.log('CDDA unlocked');
                window.removeEventListener('touchend', hanlder);
            };
            window.addEventListener('touchend', hanlder);
        }
    }
}
