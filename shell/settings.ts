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

            $('#downloadSaveData').addEventListener('click', this.downloadSaveData.bind(this));
            $('#uploadSaveData').addEventListener('click', this.uploadSaveData.bind(this));
            this.checkSaveData();
        }

        private openModal() {
            $('#settings-modal').classList.add('active');
            document.addEventListener('keydown', this.keyDownHandler);
        }

        private closeModal() {
            $('#settings-modal').classList.remove('active');
            document.removeEventListener('keydown', this.keyDownHandler);
        }

        private checkSaveData() {
            xsystem35.saveDirReady.then(() => {
                if ((<string[]>FS.readdir('/save')).some((name) => name.toLowerCase().endsWith('.asd')))
                    $('#downloadSaveData').removeAttribute('disabled');
            });
        }

        private async downloadSaveData() {
            let zip = new JSZip();
            let folder = zip.folder('save');
            for (let name of FS.readdir('/save')) {
                if (!name.toLowerCase().endsWith('.asd'))
                    continue;
                let content = FS.readFile('/save/' + name, { encoding: 'binary' });
                folder.file(name, content);
            }
            let blob = await zip.generateAsync({type: 'blob', compression: 'DEFLATE'});
            if (navigator.msSaveBlob) {  // Edge
                navigator.msSaveBlob(blob, 'savedata.zip');
            } else {
                let elem = document.createElement('a');
                elem.setAttribute('download', 'savedata.zip');
                elem.setAttribute('href', URL.createObjectURL(blob));
                document.body.appendChild(elem);
                elem.click();
                setTimeout(() => { document.body.removeChild(elem); }, 5000);
            }
        }

        private uploadSaveData() {
            openFileInput().then((file) => {
                this.extractSaveData(file);
            });
        }

        private async extractSaveData(file: File) {
            function basename(path: string) {
                return path.slice(path.lastIndexOf('/') + 1);
            }
            function addSaveFile(name: string, content: ArrayBuffer) {
                console.log(name);
                FS.writeFile('/save/' + name, new Uint8Array(content), { encoding: 'binary' });
            }
            try {
                await xsystem35.saveDirReady;
                if (file.name.toLowerCase().endsWith('.asd')) {
                    addSaveFile(file.name, await readFileAsArrayBuffer(file));
                } else {
                    let zip = new JSZip();
                    await zip.loadAsync(await readFileAsArrayBuffer(file));
                    for (let f of zip.file(/\.asd$/i))
                        addSaveFile(basename(f.name), await f.async('arraybuffer'));
                }
                xsystem35.shell.syncfs(0);
                xsystem35.shell.addToast('セーブデータの復元に成功しました。', 'success');
                this.checkSaveData();
            } catch (err) {
                xsystem35.shell.addToast('セーブデータを復元できませんでした。', 'danger');
                console.warn(err);
            }
        }
    }
}
