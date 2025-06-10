// 転送元 alpha map を参照して、転送元ピクセルと転送先ピクセルをブレンドし、
// 転送先ピクセルに描画

#include <stdio.h>

#include "portab.h"
#include "system.h"
#include "graph_blend_amap.h"
#include "ags.h"

static void gre_BlendUseAMap(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int width, int height) {
	uint8_t *sp = GETOFFSET_PIXEL(src, sx, sy);
	uint8_t *dp = GETOFFSET_PIXEL(dst, dx, dy);
	uint8_t *ap = GETOFFSET_ALPHA(src, sx, sy);

	switch (dst->sdl_surface->format->BitsPerPixel) {
	case 16:
		for (int y = 0; y < height; y++) {
			uint16_t *yls = (uint16_t *)(sp + y * src->sdl_surface->pitch);
			uint16_t *yld = (uint16_t *)(dp + y * dst->sdl_surface->pitch);
			uint8_t *yla = (uint8_t *)(ap + y * src->width);

			for (int x = 0; x < width; x++) {
				*yld = ALPHABLEND16(*yls, *yld, *yla);
				yls++; yld++; yla++;
			}
		}
		break;
	case 24:
	case 32:
		for (int y = 0; y < height; y++) {
			uint32_t *yls = (uint32_t *)(sp + y * src->sdl_surface->pitch);
			uint32_t *yld = (uint32_t *)(dp + y * dst->sdl_surface->pitch);
			uint8_t *yla = (uint8_t  *)(ap + y * src->width);

			for (int x = 0; x < width; x++) {
				*yld = ALPHABLEND24(*yls, *yld, *yla);
				yls++; yld++; yla++;
			}
		}
		break;
	}
}

void gr_blend_alpha_map(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int sw, int sh) {
	if (src == NULL || dst == NULL) return;
	SDL_Rect src_window = { 0, 0, src->width, src->height };
	SDL_Rect dst_window = { 0, 0, dst->width, dst->height };
	if (!ags_clipCopyRect(&src_window, &dst_window, &sx, &sy, &dx, &dy, &sw, &sh)) return;
	
	if (src->alpha == NULL) {
		WARNING("src alpha NULL");
		return;
	}
	
	gre_BlendUseAMap(dst, dx, dy, src, sx, sy, sw, sh);
}

