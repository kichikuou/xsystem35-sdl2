class FallbackTextDecoder {
    constructor(utfLabel?: string) {}

    decode(buffer: DataView): string {
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
}

if (!(<any>self).TextDecoder)
    (<any>self).TextDecoder = <TextEncoding.TextDecoderStatic>FallbackTextDecoder;
