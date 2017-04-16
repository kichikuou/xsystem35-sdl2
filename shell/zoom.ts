/// <reference path="util.ts" />

namespace xsystem35 {
    export class ZoomManager {
        private canvas = <HTMLCanvasElement>$('#canvas');
        private zoomSelect = <HTMLInputElement>$('#zoom');
        private pixelateCheckbox = <HTMLInputElement>$('#pixelate');

        constructor() {
            this.zoomSelect.addEventListener('change', this.handleZoom.bind(this));
            this.zoomSelect.value = localStorage.getItem('zoom') || 'fit';
            if (CSS.supports('image-rendering', 'pixelated') || CSS.supports('image-rendering', '-moz-crisp-edges')) {
                this.pixelateCheckbox.addEventListener('change', this.handlePixelate.bind(this));
                if (localStorage.getItem('pixelate') === 'true') {
                    this.pixelateCheckbox.checked = true;
                    this.handlePixelate();
                }
            } else {
                this.pixelateCheckbox.setAttribute('disabled', 'true');
            }
        }

        handleZoom() {
            let value = this.zoomSelect.value;
            localStorage.setItem('zoom', value);
            let navbarStyle = $('.navbar').style;
            if (value === 'fit') {
                $('#xsystem35').classList.add('fit');
                navbarStyle.maxWidth = 'none';
                this.canvas.style.width = null;
            } else {
                $('#xsystem35').classList.remove('fit');
                let ratio = Number(value);
                navbarStyle.maxWidth = this.canvas.style.width = this.canvas.width * ratio + 'px';
            }
        }

        private handlePixelate() {
            localStorage.setItem('pixelate', String(this.pixelateCheckbox.checked));
            if (this.pixelateCheckbox.checked)
                this.canvas.classList.add('pixelated');
            else
                this.canvas.classList.remove('pixelated');
        }
    }
}
