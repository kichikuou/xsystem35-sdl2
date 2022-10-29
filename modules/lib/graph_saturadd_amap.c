// alpha map の飽和加算

#include <stdio.h>

#include "portab.h"
#include "system.h"
#include "surface.h"
#include "ngraph.h"
#include "ags.h"

int gr_saturadd_alpha_map(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int sw, int sh) {
	uint8_t *sp, *dp;
	int x, y;
	
	if (FALSE == gr_clip(src, &sx, &sy, &sw, &sh, dst, &dx, &dy)) {
		return NG;
	}
	
	if (src->alpha == NULL) {
		WARNING("src alpha NULL");
		return NG;
	}
	
	if (dst->alpha == NULL) {
		WARNING("dst alpha NULL");
		return NG;
	}
	
	sp = GETOFFSET_ALPHA(src, sx, sy);
	dp = GETOFFSET_ALPHA(dst, dx, dy);
	
	for (y = 0; y < sh; y++) {
		uint8_t *yls = sp + y * src->width;
		uint8_t *yld = dp + y * dst->width;
		for (x = 0; x < sw; x++) {
			int s = *yls, d = *yld;
			*yld = (uint8_t)(min(255, (s + d)));
			yls++; yld++;
		}
	}
	
	return OK;
}
