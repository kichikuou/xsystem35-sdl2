/// <reference path="util.ts" />

namespace xsystem35 {
    export class Settings {
        private keyDownHandler: (ev: KeyboardEvent) => void;
        constructor() {
            $('#settings-button').addEventListener('click', this.openModal.bind(this));
            $('#settings-close').addEventListener('click', this.closeModal.bind(this));
            this.keyDownHandler = (ev: KeyboardEvent) => {
                if (ev.keyCode === 27)  // escape
                    this.closeModal();
            };
            $('.modal-overlay').addEventListener('click', this.closeModal.bind(this));
        }

        private openModal() {
            $('#settings-modal').classList.add('active');
            document.addEventListener('keydown', this.keyDownHandler);
        }

        private closeModal() {
            $('#settings-modal').classList.remove('active');
            document.removeEventListener('keydown', this.keyDownHandler);
        }
    }
}
