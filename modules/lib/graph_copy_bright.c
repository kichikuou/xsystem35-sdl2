// 指定のsurface領域を明るさを lv/255 倍してコピー

#include <string.h>

#include "portab.h"
#include "surface.h"
#include "ngraph.h"
#include "ags.h"

void gr_copy_bright(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int width, int height, int lv) {
	int x, y;
	BYTE *sp, *dp;
	
	if (!gr_clip(src, &sx, &sy, &width, &height, dst, &dx, &dy)) return;
	
	sp = GETOFFSET_PIXEL(src, sx, sy);
	dp = GETOFFSET_PIXEL(dst, dx, dy);
	
	if (sp == NULL || dp == NULL) return;
	
	switch(dst->depth) {
	case 15:
	{
		WORD *yls, *yld;
		
		for (y = 0; y < height; y++) {
			yls = (WORD *)(sp + y * src->bytes_per_line);
			yld = (WORD *)(dp + y * dst->bytes_per_line);
			
			for (x = 0; x < width; x++) {
				*yld = ALPHALEVEL15(*yls, lv);
				yls++; yld++;
			}
		}
		break;
	}
	case 16:
		if (nact->mmx_is_ok) {
//		if (0) {
#ifdef ENABLE_MMX
			int alpha;
			lv = 255 - lv;
			alpha = lv | lv << 8 | lv << 16 | lv << 24;
			ablend16_dpd(dp, 0, sp, alpha, width, height, dst->bytes_per_line, src->bytes_per_line);
#endif
		} else {
			WORD *yls, *yld;
			
			for (y = 0; y < height; y++) {
				yls = (WORD *)(sp + y * src->bytes_per_line);
				yld = (WORD *)(dp + y * dst->bytes_per_line);
				
				for (x = 0; x < width; x++) {
					*yld = ALPHALEVEL16(*yls, lv);
					yls++; yld++;
				}
			}
		}
		break;
	case 24:
	case 32:
	{
		DWORD *yls, *yld;
		
		for (y = 0; y < height; y++) {
			yls = (DWORD *)(sp + y * src->bytes_per_line);
			yld = (DWORD *)(dp + y * dst->bytes_per_line);
			
			for (x = 0; x < width; x++) {
				*yld = ALPHALEVEL24(*yls, lv);
				yls++; yld++;
			}
		}
		break;
	}
	}
}

