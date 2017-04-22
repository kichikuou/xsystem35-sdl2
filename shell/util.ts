let $: (selector: string) => HTMLElement = document.querySelector.bind(document);

function readFileAsArrayBuffer(blob: Blob): Promise<ArrayBuffer> {
    return new Promise((resolve, reject) => {
        let reader = new FileReader();
        reader.onload = () => { resolve(reader.result); };
        reader.onerror = () => { reject(reader.error); };
        reader.readAsArrayBuffer(blob);
    });
}

function readFileAsText(blob: Blob): Promise<string> {
    return new Promise((resolve, reject) => {
        let reader = new FileReader();
        reader.onload = () => { resolve(reader.result); };
        reader.onerror = () => { reject(reader.error); };
        reader.readAsText(blob);
    });
}

function ASCIIArrayToString(buffer: Uint8Array): string {
    return String.fromCharCode.apply(null, buffer);
}

function SJISArrayToString(buffer: DataView): string {
    if (typeof TextDecoder !== 'undefined')
        return new TextDecoder('shift_jis').decode(buffer);

    let out = [];
    for (let i = 0; i < buffer.byteLength; i++) {
        let c = buffer.getUint8(i);
        if (c >= 0xa0 && c <= 0xdf)
            out.push(0xff60 + c - 0xa0);
        else if (c < 0x80)
            out.push(c);
        else
            out.push(_sjis2unicode(c, buffer.getUint8(++i)));
    }
    return String.fromCharCode.apply(null, out);
}

function openFileInput(): Promise<File> {
    return new Promise((resolve) => {
        let input = document.createElement('input');
        input.type = 'file';
        input.addEventListener('change', (evt: Event) => {
            document.body.removeChild(input);
            resolve(input.files[0]);
        });
        input.style.display = 'none';
        document.body.appendChild(input);
        input.click();
    });
}

declare var WebAssembly: any;

// xsystem35 exported functions
declare function _musfade_setvolval_all(vol: number): void;
declare function _ags_setAntialiasedStringMode(on: number): void;
declare function _ald_getdata(type: number, no: number): number;
declare function _ald_freedata(data: number): void;
declare function _sjis2unicode(byte1: number, byte2: number): void;
declare function _sdl_getDisplaySurface(): number;

declare namespace Module {
    // Undocumented methods / attributes
    let canvas: HTMLCanvasElement;
    function getMemory(size: number): number;
    function setStatus(status: string): void;
    function setWindowTitle(title: string): void;
}

declare namespace FS {
    function readFile(path: string, opts?: {encoding?: string; flags?: string}): any;
    function writeFile(path: string, data: ArrayBufferView | string,
                       opts?: {encoding?: string; flags?: string; canOwn?: boolean}): void;
}

declare namespace EmterpreterAsync {
    function handle(asyncOp: (resume: () => void) => void): void;
}
