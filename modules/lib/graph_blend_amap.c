// 転送元 alpha map を参照して、転送元ピクセルと転送先ピクセルをブレンドし、
// 転送先ピクセルに描画

#include <stdio.h>

#include "portab.h"
#include "system.h"
#include "surface.h"
#include "ngraph.h"
#include "ags.h"

int gr_blend_alpha_map(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int sw, int sh) {
	if (src == NULL || dst == NULL) return NG;
	if (!gr_clip(src, &sx, &sy, &sw, &sh, dst, &dx, &dy)) return NG;
	
	if (src->alpha == NULL) {
		WARNING("src alpha NULL\n");
		return NG;
	}
	
	return gre_BlendUseAMap(dst, dx, dy, dst, dx, dy, src, sx, sy, sw, sh, src, sx, sy, 255);
}

