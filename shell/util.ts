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

    let out = '';
    for (let i = 0; i < buffer.byteLength; i++) {
        let c = buffer.getUint8(i);
        if (c < 128)
            out += String.fromCharCode(c);
        else
            out += '%' + c.toString(16);
    }
    return out;
}

declare var WebAssembly: any;

// xsystem35 exported functions
declare function _musfade_setvolval_all(vol: number): void;
declare function _ags_setAntialiasedStringMode(on: number): void;
declare function _ald_getdata(type: number, no: number): number;
declare function _ald_freedata(data: number): void;

declare namespace Module {
    let noInitialRun: boolean;

    // Undocumented methods / attributes
    let canvas: HTMLElement;
    function setStatus(status: string): void;
    function monitorRunDependencies(left: number): void;
    function callMain(): void;
    function getMemory(size: number): number;
}

declare namespace FS {
    function readFile(path: string, opts?: {encoding?: string; flags?: string}): any;
    function writeFile(path: string, data: ArrayBufferView | string,
                       opts?: {encoding?: string; flags?: string; canOwn?: boolean}): void;
}

declare namespace EmterpreterAsync {
    function handle(asyncOp: (resume: () => void) => void): void;
}
