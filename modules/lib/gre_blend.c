// dst の上に src をブレンド率 lv でブレンドしたものを write へ書き出し

#include "config.h"

#include "portab.h"
#include "surface.h"
#include "ngraph.h"
#include "ags.h"

int gre_Blend(surface_t *write, int wx, int wy, surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int width, int height, int lv) {
	uint8_t *sp, *dp, *wp;
	int x, y;
	
	wp = GETOFFSET_PIXEL(write, wx, wy);
	sp = GETOFFSET_PIXEL(src,   sx, sy);
	dp = GETOFFSET_PIXEL(dst,   dx, dy);
	
	switch(dst->depth) {
	case 16:
		{
			uint16_t *yls, *yld, *ylw;
			
			for (y = 0; y < height; y++) {
				yls = (uint16_t *)(sp + y * src->bytes_per_line);
				yld = (uint16_t *)(dp + y * dst->bytes_per_line);
				ylw = (uint16_t *)(wp + y * write->bytes_per_line);
				
				for (x = 0; x < width; x++) {
					*ylw = ALPHABLEND16(*yls, *yld, lv);
					yls++; yld++; ylw++;
				}
			}
		}	
		break;
	case 24:
	case 32:
	{
		uint32_t *yls, *yld, *ylw;
		
		for (y = 0; y < height; y++) {
			yls = (uint32_t *)(sp + y * src->bytes_per_line);
			yld = (uint32_t *)(dp + y * dst->bytes_per_line);
			ylw = (uint32_t *)(wp + y * write->bytes_per_line);
			
			for (x = 0; x < width; x++) {
				*ylw = ALPHABLEND24(*yls, *yld, lv);
				yls++; yld++; ylw++;
			}
		}
		break;
	}
	}
	
	return OK;
}
