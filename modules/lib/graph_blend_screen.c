// srcのsurfaceの一部をdstに飽和加算

#include <stdio.h>

#include "portab.h"
#include "system.h"
#include "surface.h"
#include "ngraph.h"
#include "ags.h"

int gr_blend_screen(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int w, int h) {
	
	if (src == NULL || dst == NULL) return NG;
	if (!gr_clip(src, &sx, &sy, &w, &h, dst, &dx, &dy)) return NG;
	
	return gre_BlendScreen(dst, dx, dy, dst, dx, dy, src, sx, sy, w, h);
}

