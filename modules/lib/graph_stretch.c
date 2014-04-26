// 拡大・縮小

#include <string.h>
#include <glib.h>

#include "portab.h"
#include "surface.h"
#include "ngraph.h"
#include "ags.h"

void gr_copy_stretch(surface_t *dst, int dx, int dy, int dw, int dh, surface_t *src, int sx, int sy, int sw, int sh) {
	float    a1, a2, xd, yd;
	int      *row, *col;
	int      x, y;
	BYTE    *sp, *dp;
	
	if (!gr_clip_xywh(dst, &dx, &dy, &dw, &dh)) return;
	if (!gr_clip_xywh(src, &sx, &sy, &sw, &sh)) return;
	
	sp = GETOFFSET_PIXEL(src, sx, sy);
	dp = GETOFFSET_PIXEL(dst, dx, dy);
	
	a1  = (float)sw / (float)dw;
	a2  = (float)sh / (float)dh;
	
	// src width と dst width が同じときに問題があるので+1
	row = g_new0(int, dw+1);
	// 1おおきくして初期化しないと col[dw-1]とcol[dw]が同じになる
	// 可能性がある。
	col = g_new0(int, dh+1);
	
	for (yd = 0.0, y = 0; y < dh; y++) {
		col[y] = yd; yd += a2;
	}
	
	for (xd = 0.0, x = 0; x < dw; x++) {
		row[x] = xd; xd += a1;
	}
	
	switch(dst->depth) {
	case 15:
	case 16:
	{
		WORD *yls, *yld;
		BYTE *_yls, *_yld;
		
		for (y = 0; y < dh; y++) {
			yls = (WORD *)(sp + *(y + col) * src->bytes_per_line);
			yld = (WORD *)(dp +   y        * dst->bytes_per_line);
			for (x = 0; x < dw; x++) {
				*(yld + x) = *(yls + *(row + x));
			}
			_yld = (BYTE *)yld;
			while(*(col + y) == *(col + y + 1)) {
				_yls = _yld;
				_yld += dst->bytes_per_line;
				memcpy(_yld, _yls, dw * 2);
				y++;
			}
		}
		break;
	}
	case 24:
	case 32:
	{
		DWORD *yls, *yld;
		BYTE  *_yls, *_yld;
		
		for (y = 0; y < dh; y++) {
			yls = (DWORD *)(sp + *(y + col) * src->bytes_per_line);
			yld = (DWORD *)(dp +   y        * dst->bytes_per_line);
			for (x = 0; x < dw; x++) {
				*(yld + x) = *(yls+ *(row + x));
			}
			_yld = (BYTE *)yld;
			while(*(col + y) == *(col + y + 1)) {
				_yls = _yld;
				_yld += dst->bytes_per_line;
				memcpy(_yld, _yls, dw * 4);
				y++;
			}
		}
		break;
	}
	}
	
	g_free(row);
	g_free(col);
}
