// 拡大縮小
static surface_t *stretch(surface_t *src, int dw, int dh, int mirror) {
	surface_t *dst = calloc(1, sizeof(surface_t));
	float    a1, a2, xd, yd;
	int      sw, sh;
	int      *row, *col;
	int      x, y;
	uint8_t  *sdata, *ddata;
	
	dst->width = dw;
	dst->height = dh;
	dst->depth = src->depth;
	dst->bytes_per_line = dw * src->bytes_per_pixel;
	dst->bytes_per_pixel = src->bytes_per_pixel;
	
	if (src->pixel) {
		dst->sdl_surface = SDL_CreateRGBSurfaceWithFormat(0, dw, dh, src->depth, src->sdl_surface->format->format);
		dst->pixel = dst->sdl_surface->pixels;
		dst->bytes_per_line = dst->sdl_surface->pitch;
		dst->bytes_per_pixel = dst->sdl_surface->format->BytesPerPixel;
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
	uint8_t *_sl, *_dl;                                             \
	for (y = 0; y < dh; y++) {                                      \
		sl = (type *)(sdata + *(y + col) * src->bytes_per_line);\
		dl = (type *)(ddata +   y        * dst->bytes_per_line);\
		for (x = 0; x < dw; x++) {                              \
			*(dl + x) = *(sl + *(row + x));                 \
		}                                                       \
		_dl = (uint8_t *)dl;                                    \
		while (y < dh - 1 && *(col + y) == *(col + y + 1)) {	\
			_sl = _dl;                                      \
			_dl += dst->bytes_per_line;                     \
			memcpy(_dl, _sl, dw * sizeof(type));            \
			y++;                                            \
		}                                                       \
	}}
	
	switch(dst->depth) {
	case 8:	
		SCALEDCOPYAREA(uint8_t); break;
	case 16:
		SCALEDCOPYAREA(uint16_t); break;
	case 24:
	case 32:
		SCALEDCOPYAREA(uint32_t); break;
	default:
		break;
	}
	
	if (src->alpha) {
		int x, y;
		uint8_t *sl, *dl;
		uint8_t *_sl, *_dl;
		sdata = GETOFFSET_ALPHA(src, 0, 0);
		ddata = GETOFFSET_ALPHA(dst, 0, 0);
		for (y = 0; y < dh; y++) {
			sl = (uint8_t *)(sdata + *(y + col) * src->width);
			dl = (uint8_t *)(ddata +   y        * dst->width);
			for (x = 0; x < dw; x++) {
				*(dl + x) = *(sl + *(row + x));
			}
			_dl = (uint8_t *)dl;
			while (y < dh - 1 && *(col + y) == *(col + y + 1)) {
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
