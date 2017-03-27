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
        }

        private openModal() {
            $('#settings-modal').classList.add('active');
            document.addEventListener('keydown', this.keyDownHandler);
        }

        private closeModal() {
            $('#settings-modal').classList.remove('active');
            document.removeEventListener('keydown', this.keyDownHandler);
        }

        private async downloadSaveData() {
            await xsystem35.saveDirReady;
            let zip = new JSZip();
            let folder = zip.folder('save');
            for (let name of FS.readdir('/save')) {
                if (!name.toLowerCase().endsWith('.asd'))
                    continue;
                let content = FS.readFile('/save/' + name, { encoding: 'binary' });
                folder.file(name, content);
            }
            let blob = await zip.generateAsync({type: 'blob', compression: 'DEFLATE'});
            let elem = document.createElement('a');
            elem.setAttribute('download', 'savedata.zip');
            elem.setAttribute('href', URL.createObjectURL(blob));
            elem.click();
        }

        private uploadSaveData() {
            let input = document.createElement('input');
            input.type = 'file';
            input.addEventListener('change', (evt: Event) => {
                this.extractSaveData(input.files[0]);
                document.body.removeChild(input);
            });
            input.style.display = 'none';
            document.body.appendChild(input);
            input.click();
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
                if (file.name.toLowerCase().endsWith('.zip')) {
                    let zip = new JSZip();
                    await zip.loadAsync(await readFileAsArrayBuffer(file));
                    for (let f of zip.file(/\.asd$/i))
                        addSaveFile(basename(f.name), await f.async('arraybuffer'));
                } else if (file.name.toLowerCase().endsWith('.asd')) {
                    addSaveFile(file.name, await readFileAsArrayBuffer(file));
                } else {
                    throw('ファイルを認識できません。');
                }
                xsystem35.shell.syncfs(0);
            } catch (err) {
                console.log(err);
            }
        }
    }
}
