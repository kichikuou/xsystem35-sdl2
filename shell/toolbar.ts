/// <reference path="util.ts" />

namespace xsystem35 {
    export class ToolBar {
        private toolbar = $('#toolbar');
        private handler = $('#toolbar-handler');

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
    }
}
