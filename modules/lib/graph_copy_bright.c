// 指定のsurface領域を明るさを lv/255 倍してコピー

#include <string.h>

#include "portab.h"
#include "surface.h"
#include "ngraph.h"
#include "ags.h"

void gr_copy_bright(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int width, int height, int lv) {
	int x, y;
	uint8_t *sp, *dp;
	
	if (!gr_clip(src, &sx, &sy, &width, &height, dst, &dx, &dy)) return;
	
	sp = GETOFFSET_PIXEL(src, sx, sy);
	dp = GETOFFSET_PIXEL(dst, dx, dy);
	
	if (sp == NULL || dp == NULL) return;
	
	switch(dst->depth) {
	case 16:
		{
			uint16_t *yls, *yld;
			
			for (y = 0; y < height; y++) {
				yls = (uint16_t *)(sp + y * src->bytes_per_line);
				yld = (uint16_t *)(dp + y * dst->bytes_per_line);
				
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
		uint32_t *yls, *yld;
		
		for (y = 0; y < height; y++) {
			yls = (uint32_t *)(sp + y * src->bytes_per_line);
			yld = (uint32_t *)(dp + y * dst->bytes_per_line);
			
			for (x = 0; x < width; x++) {
				*yld = ALPHALEVEL24(*yls, lv);
				yls++; yld++;
			}
		}
		break;
	}
	}
}

