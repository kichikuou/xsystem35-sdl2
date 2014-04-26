//  color expansion and blending
//  8bppのモノクロをcolでブレンド

#include "portab.h"
#include "surface.h"
#include "ngraph.h"
#include "ags.h"

int gr_expandcolor_blend(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int sw, int sh, int r, int g, int b) {
	BYTE *sp, *dp;
	int x, y;
	int col;
	
	if (FALSE == gr_clip(src, &sx, &sy, &sw, &sh, dst, &dx, &dy)) {
		return NG;
	}
	
	sp = GETOFFSET_PIXEL(src, sx, sy);
	dp = GETOFFSET_PIXEL(dst, dx, dy);
	
	switch(dst->depth) {
	case 15: {
		WORD *yd;
		BYTE *ys;
		col = PIX15(r, g, b);
		for (y = 0; y < sh; y++) {
			ys = (BYTE *)(sp + y * src->bytes_per_line);
			yd = (WORD *)(dp + y * dst->bytes_per_line);
			for (x = 0; x < sw; x++) {
				if (*ys) {
					*yd = ALPHABLEND15(col, *yd, (BYTE)*ys);
				}
				ys++; yd++;
			}
		}
		break;
	}
	case 16:
		col = PIX16(r, g, b);
		//if (nact->mmx_is_ok) {
		if (0) {
			//ablend16_dpp(col, dp, sp, sw, sh,
			//	     dst->bytes_per_line, src->bytes_per_line);
		} else {
			WORD *yd;
			BYTE *ys;
			for (y = 0; y < sh; y++) {
				ys = (BYTE *)(sp + y * src->bytes_per_line);
				yd = (WORD *)(dp + y * dst->bytes_per_line);
				for (x = 0; x < sw; x++) {
					if (*ys) {
						*yd = ALPHABLEND16(col, *yd, (BYTE)*ys);
					}
					ys++; yd++;
				}
			}
		}
		break;
	case 32:
	case 24: {
		DWORD *yd;
		BYTE *ys;
		col = PIX24(r, g, b);
		for (y = 0; y < sh; y++) {
			ys = (BYTE *)(sp + y * src->bytes_per_line);
			yd = (DWORD *)(dp + y * dst->bytes_per_line);
			for (x = 0; x < sw; x++) {
				if (*ys) {
					*yd = ALPHABLEND24(col, *yd, (BYTE)*ys);
				}
				ys++; yd++;
			}
		}
		break;
	}}
	
	return OK;
}
