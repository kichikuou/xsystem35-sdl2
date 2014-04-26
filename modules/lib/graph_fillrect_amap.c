// alpha map の矩形塗りつぶし

#include <string.h>
#include "portab.h"
#include "surface.h"
#include "ngraph.h"
#include "ags.h"

int gr_fill_alpha_map(surface_t *dst, int dx, int dy, int dw, int dh, int lv) {
	BYTE *a;
	
	if (!gr_clip_xywh(dst, &dx, &dy, &dw, &dh)) {
		return NG;
	}
	
	a = GETOFFSET_ALPHA(dst, dx, dy);
	
	while(dh--){
		memset(a, lv, dw);
		a += dst->width;
	}
	
	return OK;
}
