// 拡大縮小
static surface_t *stretch(surface_t *src, int dw, int dh, int mirror) {
	surface_t *dst = calloc(1, sizeof(surface_t));
	float    a1, a2, xd, yd;
	int      sw, sh;
	int      *row, *col;
	int      x, y;
	BYTE    *sdata, *ddata;
	
	dst->width = dw;
	dst->height = dh;
	dst->depth = src->depth;
	dst->bytes_per_line = dw * src->bytes_per_pixel;
	dst->bytes_per_pixel = src->bytes_per_pixel;
	
	if (src->pixel) {
		dst->pixel = malloc(dh * dst->bytes_per_line);
	}
	if (src->alpha) {
		dst->alpha = malloc(dw * dh);
	}
	
	
	sdata = GETOFFSET_PIXEL(src, 0, 0);
	ddata = GETOFFSET_PIXEL(dst, 0, 0);
	sw = src->width;
	sh = src->height;
	
	a1  = (float)sw / (float)dw;
	a2  = (float)sh / (float)dh;
	row = malloc(sizeof(int) * dw);
	// 1おおきくして初期化しないと col[dw-1]とcol[dw]が同じになる
	// 可能性がある。
	col = calloc(dh +1, sizeof(int));
	
	if (mirror & 1) {
		/* 上下反転 added by  tajiri@wizard */
		for (yd = sh - a2, y = 0; y < dh; y++) {
			col[y] = yd; yd -= a2;
		}
	} else {
		for (yd = 0.0, y = 0; y < dh; y++) {
			col[y] = yd; yd += a2;
		}
	}
	if (mirror & 2) {
		/* 左右反転 added by  tajiri@wizard */
		for (xd = sw - a1, x = 0; x < dw; x++) {
			row[x] = xd; xd -= a1;
		}
	} else {
		for (xd = 0.0, x = 0; x < dw; x++) {
			row[x] = xd; xd += a1;
		}
	}
	
#define SCALEDCOPYAREA(type) {                                          \
	int x, y;                                                       \
	type *sl, *dl;                                                  \
	BYTE *_sl, *_dl;                                                \
	for (y = 0; y < dh; y++) {                                      \
		sl = (type *)(sdata + *(y + col) * src->bytes_per_line);\
		dl = (type *)(ddata +   y        * dst->bytes_per_line);\
		for (x = 0; x < dw; x++) {                              \
			*(dl + x) = *(sl + *(row + x));                 \
		}                                                       \
		_dl = (BYTE *)dl;                                       \
		while(*(col + y) == *(col + y + 1)) {                   \
			_sl = _dl;                                      \
			_dl += dst->bytes_per_line;                     \
			memcpy(_dl, _sl, dw * sizeof(type));            \
			y++;                                            \
		}                                                       \
	}}
	
	switch(dst->depth) {
	case 8:	
		SCALEDCOPYAREA(BYTE); break;
	case 15:
	case 16:
		SCALEDCOPYAREA(WORD); break;
	case 24:
	case 32:
		SCALEDCOPYAREA(DWORD); break;
	default:
		break;
	}
	
	if (src->alpha) {
		int x, y;
		BYTE *sl, *dl;
		BYTE *_sl, *_dl;
		sdata = GETOFFSET_ALPHA(src, 0, 0);
		ddata = GETOFFSET_ALPHA(dst, 0, 0);
		for (y = 0; y < dh; y++) {
			sl = (BYTE *)(sdata + *(y + col) * src->width);
			dl = (BYTE *)(ddata +   y        * dst->width);
			for (x = 0; x < dw; x++) {
				*(dl + x) = *(sl + *(row + x));
			}
			_dl = (BYTE *)dl;
			while(*(col + y) == *(col + y + 1)) {
				_sl = _dl;
				_dl += dst->width;
				memcpy(_dl, _sl, dw);
				y++;
			}
		}
	}
	
	free(row);
	free(col);
	
	return dst;
}
