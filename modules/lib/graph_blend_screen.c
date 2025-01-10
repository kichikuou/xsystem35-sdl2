// srcのsurfaceの一部をdstに飽和加算

#include <stdio.h>

#include "portab.h"
#include "system.h"
#include "surface.h"
#include "ngraph.h"
#include "ags.h"

void gr_blend_screen(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int width, int height) {
	if (!gr_clip(src, &sx, &sy, &width, &height, dst, &dx, &dy)) return;

	uint8_t *sp = GETOFFSET_PIXEL(src,   sx, sy);
	uint8_t *dp = GETOFFSET_PIXEL(dst,   dx, dy);

	switch(dst->depth) {
	case 16:
		for (int y = 0; y < height; y++) {
			uint16_t *yls = (uint16_t *)(sp + y * src->bytes_per_line);
			uint16_t *yld = (uint16_t *)(dp + y * dst->bytes_per_line);
			for (int x = 0; x < width; x++) {
				*yld = SUTURADD16(*yls, *yld);
				yls++; yld++;
			}
		}
		break;

	case 32:
	case 24:
		for (int y = 0; y < height; y++) {
			uint32_t *yls = (uint32_t *)(sp + y * src->bytes_per_line);
			uint32_t *yld = (uint32_t *)(dp + y * dst->bytes_per_line);
			for (int x = 0; x < width; x++) {
				*yld = SUTURADD24(*yls, *yld);
				yls++; yld++;
			}
		}
		break;
	}
}

