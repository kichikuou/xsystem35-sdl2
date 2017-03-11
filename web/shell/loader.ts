class ImageLoader {
    private worker: Worker;
    private cddaCallback: (wab:Blob)=>void;

    constructor(private shell:System35Shell) {
        this.initWorker();
        $('#fileselect').addEventListener('change', this.handleFileSelect.bind(this), false);
        document.body.ondragover = this.handleDragOver.bind(this);
        document.body.ondrop = this.handleDrop.bind(this);
    }

    getCDDA(track:number, callback:(wav:Blob)=>void) {
        this.send({command:'getTrack', track:track});
        this.cddaCallback = callback;
    }

    private initWorker() {
        this.worker = new Worker('imagereader-worker.js');
        this.worker.addEventListener('message', this.onMessage.bind(this));
        this.worker.addEventListener('error', this.onWorkerError.bind(this));
    }

    private setReadyState(imgName:string, cueName:string) {
        if (imgName) {
            $('#imgReady').classList.remove('notready');
            $('#imgReady').textContent = imgName;
        }
        if (cueName) {
            $('#cueReady').classList.remove('notready');
            $('#cueReady').textContent = cueName;
        }
    }

    private populateFiles(files:any, volumeLabel:string) {
        var grGenerator = new GameResourceGenerator();
        for (var name in files) {
            var data: ArrayBufferView = new Uint8Array(files[name]);
            FS.writeFile(name, data, { encoding: 'binary' });
            grGenerator.addFile(name);
        }
        FS.writeFile('xsystem35.gr', grGenerator.generate());
        FS.writeFile('.xsys35rc', xsystem35.xsys35rc);

        this.shell.run();
    }

    private handleFileSelect(evt:Event) {
        var input = <HTMLInputElement>evt.target;
        var files = input.files;
        for (var i = 0; i < files.length; i++)
            this.setFile(files[i]);
        input.value = '';
    }

    private handleDragOver(evt:DragEvent) {
        evt.stopPropagation();
        evt.preventDefault();
        evt.dataTransfer.dropEffect = 'copy';
    }

    private handleDrop(evt:DragEvent) {
        evt.stopPropagation();
        evt.preventDefault();
        var files = evt.dataTransfer.files;
        for (var i = 0; i < files.length; i++)
            this.setFile(files[i]);
    }

    private setFile(file:File) {
        this.send({command:'setFile', file:file});
    }

    private startInstall() {
        this.send({command:'extractFiles'});
    }

    private send(msg:any) {
        this.worker.postMessage(msg);
    }

    private onMessage(evt: MessageEvent) {
        switch (evt.data.command) {
        case 'readyState':
            this.setReadyState(evt.data.img, evt.data.cue);
            if (evt.data.img && evt.data.cue)
                this.startInstall();
            break;
        case 'extractFiles':
            this.populateFiles(evt.data.files, evt.data.volumeLabel);
            break;
        case 'complete':
            this.cddaCallback(evt.data.wav);
            this.cddaCallback = null;
            break;
        case 'error':
            console.log(evt.data.message);
            break;
        }
    }

    private onWorkerError(evt: Event) {
        console.log('worker error', evt);
    }
}

class GameResourceGenerator {
    static resourceType: {[ch:string]:string} = {s:'Scenario', g:'Graphics', w:'Wave', d:'Data', r:'Resource', m:'Midi'};
    private basename:string;
    private lines:string[] = [];

    addFile(name:string) {
        var type = name.charAt(name.length - 6).toLowerCase();
        var id = name.charAt(name.length - 5);
        this.basename = name.slice(0, -6);
        this.lines.push(GameResourceGenerator.resourceType[type] + id.toUpperCase() + ' ' + name);
    }

    generate(): string {
        for (var i = 0; i < 26; i++) {
            var id = String.fromCharCode(65 + i);
            this.lines.push('Save' + id + ' save/' + this.basename + 's' + id.toLowerCase() + '.asd');
        }
        return this.lines.join('\n') + '\n';
    }

    isEmpty(): boolean {
        return this.lines.length == 0;
    }
}
