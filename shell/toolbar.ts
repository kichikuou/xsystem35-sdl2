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

        open() {
            this.toolbar.classList.remove('closed');
        }

        close() {
            this.toolbar.classList.add('closed');
        }
    }
}
