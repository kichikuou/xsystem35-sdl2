// 8bit surface (or data列)から surface の alpha map へコピー

#include <string.h>
#include "portab.h"
#include "surface.h"
#include "ngraph.h"
#include "ags.h"

int gr_draw_amap(surface_t *dst, int dx, int dy, BYTE *src, int width, int height, int scanline) {
	int y;
	BYTE *sp, *dp;
	
	sp = src;
	dp = GETOFFSET_ALPHA(dst, dx, dy);
	
	for (y = 0; y < height; y++) {
		memcpy(dp, sp, width);
		sp += scanline;
		dp += dst->width;
	}
	
	return OK;
}

