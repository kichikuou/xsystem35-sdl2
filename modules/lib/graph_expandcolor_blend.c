//  color expansion and blending
//  8bppのモノクロをcolでブレンド

#include <SDL.h>
#include "portab.h"
#include "surface.h"
#include "ngraph.h"
#include "ags.h"

void gr_expandcolor_blend(surface_t *dst, int dx, int dy, SDL_Surface *src, int sx, int sy, int sw, int sh, int r, int g, int b) {
	uint8_t *sp, *dp;
	int x, y;
	int col;
	
	MyRectangle src_window = { 0, 0, src->w, src->h };
	MyRectangle dst_window = { 0, 0, dst->width, dst->height };
	if (!ags_clipCopyRect(&src_window, &dst_window, &sx, &sy, &dx, &dy, &sw, &sh)) {
		return;
	}
	
	sp = PIXEL_AT(src, sx, sy);
	dp = GETOFFSET_PIXEL(dst, dx, dy);
	
	switch(dst->depth) {
	case 16:
		col = PIX16(r, g, b);
		{
			uint16_t *yd;
			uint8_t *ys;
			for (y = 0; y < sh; y++) {
				ys = (uint8_t *)(sp + y * src->pitch);
				yd = (uint16_t *)(dp + y * dst->bytes_per_line);
				for (x = 0; x < sw; x++) {
					if (*ys) {
						*yd = ALPHABLEND16(col, *yd, (uint8_t)*ys);
					}
					ys++; yd++;
				}
			}
		}
		break;
	case 32:
	case 24: {
		uint32_t *yd;
		uint8_t *ys;
		col = PIX24(r, g, b);
		for (y = 0; y < sh; y++) {
			ys = (uint8_t *)(sp + y * src->pitch);
			yd = (uint32_t *)(dp + y * dst->bytes_per_line);
			for (x = 0; x < sw; x++) {
				if (*ys) {
					*yd = ALPHABLEND24(col, *yd, (uint8_t)*ys);
				}
				ys++; yd++;
			}
		}
		break;
	}}
}
