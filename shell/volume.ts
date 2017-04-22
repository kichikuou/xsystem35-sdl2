/// <reference path="util.ts" />

namespace xsystem35 {
    export class VolumeControl {
        private vol: number;  // 0.0 - 1.0
        private muted: boolean;
        private elem: HTMLElement;
        private icon: HTMLElement;
        private slider: HTMLInputElement;

        constructor() {
            this.vol = Number(localStorage.getItem('volume') || 1);
            this.muted = false;

            this.elem = $('#volume-control');
            this.icon = $('#volume-control-icon');
            this.slider = <HTMLInputElement>$('#volume-control-slider');
            this.slider.value = String(Math.round(this.vol * 100));

            this.icon.addEventListener('click', this.onIconClicked.bind(this));
            this.slider.addEventListener('input', this.onSliderValueChanged.bind(this));
            this.slider.addEventListener('change', this.onSliderValueSettled.bind(this));
        }

        volume(): number {
            return this.muted ? 0 : parseInt(this.slider.value, 10) / 100;
        }

        addEventListener(handler: (evt: CustomEvent) => any) {
            this.elem.addEventListener('volumechange', handler);
        }

        hideSlider() {
            this.slider.hidden = true;
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
            this.vol = parseInt(this.slider.value, 10) / 100;
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
            let event = new CustomEvent('volumechange', { detail: this.volume() });
            this.elem.dispatchEvent(event);
        }
    }
}
