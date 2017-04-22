/// <reference path="util.ts" />

namespace xsystem35 {
    export class ToolBar {
        private toolbar = $('#toolbar');
        private handler = $('#toolbar-handler');

        constructor() {
            $('#screenshot-button').addEventListener('click', this.saveScreenshot.bind(this));
        }

        setCloseable() {
            this.handler.hidden = false;
            this.handler.addEventListener('click', this.open.bind(this));
            $('#toolbar-close-button').addEventListener('click', this.close.bind(this));
            this.toolbar.classList.add('closeable');
            this.close();
        }

        private open() {
            this.toolbar.classList.remove('closed');
            this.handler.hidden = true;
        }

        private close() {
            this.toolbar.classList.add('closed');
            this.handler.hidden = false;
        }

        private async saveScreenshot() {
            let pixels = _sdl_getDisplaySurface();
            let canvas = document.createElement('canvas');
            canvas.width = Module.canvas.width;
            canvas.height = Module.canvas.height;
            let ctx = canvas.getContext('2d');
            let image = ctx.createImageData(canvas.width, canvas.height);
            let buffer = image.data;
            let num = image.data.length;
            for (let dst = 0; dst < num; dst += 4) {
                buffer[dst] = Module.HEAPU8[pixels + 2];
                buffer[dst + 1] = Module.HEAPU8[pixels + 1];
                buffer[dst + 2] = Module.HEAPU8[pixels];
                buffer[dst + 3] = 0xff;
                pixels += 4;
            }
            ctx.putImageData(image, 0, 0);

            let url;
            if (canvas.toBlob) {
                let blob = await new Promise((resolve) => canvas.toBlob(resolve));
                url = URL.createObjectURL(blob);
            } else if (canvas.msToBlob) {  // Edge
                let blob = canvas.msToBlob();
                navigator.msSaveBlob(blob, 'screenshot.png');
                return;
            } else {  // Safari
                url = canvas.toDataURL();
            }
            let elem = document.createElement('a');
            elem.setAttribute('download', 'screenshot.png');
            elem.setAttribute('href', url);
            elem.setAttribute('target', '_blank');  // Unless this, iOS safari replaces current page
            document.body.appendChild(elem);
            elem.click();
            setTimeout(() => { document.body.removeChild(elem); }, 5000);
        }
    }
}
