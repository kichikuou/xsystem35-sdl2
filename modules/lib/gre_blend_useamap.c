// dst の上に src をalphamapを参照しつつ、ブレンド率 lv でブレンド
// したものを write へ書き出し

#include "config.h"

#include "portab.h"
#include "surface.h"
#include "ngraph.h"
#include "ags.h"

void gre_BlendUseAMap(surface_t *write, int wx, int wy, surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int width, int height, surface_t *alpha, int ax, int ay, int lv) {
	uint8_t *sp, *dp, *wp, *ap;
	int x, y;
	
	wp = GETOFFSET_PIXEL(write, wx, wy);
	sp = GETOFFSET_PIXEL(src,   sx, sy);
	dp = GETOFFSET_PIXEL(dst,   dx, dy);
	ap = GETOFFSET_ALPHA(alpha, ax, ay);
	
	if (lv == 255) {
		switch(dst->depth) {
		case 16:
			{
				uint16_t *yls, *yld, *ylw;
				uint8_t *yla;
				
				for (y = 0; y < height; y++) {
					yls = (uint16_t *)(sp + y * src->bytes_per_line);
					yld = (uint16_t *)(dp + y * dst->bytes_per_line);
					ylw = (uint16_t *)(wp + y * write->bytes_per_line);
					yla = (uint8_t *)(ap + y * alpha->width);
					
					for (x = 0; x < width; x++) {
						*ylw = ALPHABLEND16(*yls, *yld, *yla);
						yls++; yld++; ylw++; yla++;
					}
				}
			}
			break;
		case 24:
		case 32:
		{
			uint32_t *yls, *yld, *ylw;
			uint8_t  *yla;
			
			for (y = 0; y < height; y++) {
				yls = (uint32_t *)(sp + y * src->bytes_per_line);
				yld = (uint32_t *)(dp + y * dst->bytes_per_line);
				ylw = (uint32_t *)(wp + y * write->bytes_per_line);
				yla = (uint8_t  *)(ap + y * alpha->width);
				
				for (x = 0; x < width; x++) {
					*ylw = ALPHABLEND24(*yls, *yld, *yla);
					yls++; yld++; ylw++; yla++;
				}
			}
			break;
		}
		}
		
	} else {
		switch(dst->depth) {
		case 16:
			{
				uint16_t *yls, *yld, *ylw;
				uint8_t *yla;
				
				for (y = 0; y < height; y++) {
					yls = (uint16_t *)(sp + y * src->bytes_per_line);
					yld = (uint16_t *)(dp + y * dst->bytes_per_line);
					ylw = (uint16_t *)(wp + y * write->bytes_per_line);
					yla = (uint8_t *)(ap + y * alpha->width);
					
					for (x = 0; x < width; x++) {
						*ylw = ALPHABLEND16(*yls, *yld, (*yla * lv) / 255);
						yls++; yld++; ylw++; yla++;
					}
				}
			}
			break;
		case 24:
		case 32:
		{
			uint32_t *yls, *yld, *ylw;
			uint8_t  *yla;
			
			for (y = 0; y < height; y++) {
				yls = (uint32_t *)(sp + y * src->bytes_per_line);
				yld = (uint32_t *)(dp + y * dst->bytes_per_line);
				ylw = (uint32_t *)(wp + y * write->bytes_per_line);
				yla = (uint8_t  *)(ap + y * alpha->width);
				
				for (x = 0; x < width; x++) {
					*ylw = ALPHABLEND24(*yls, *yld, (*yla * lv) / 255);
					yls++; yld++; ylw++; yla++;
				}
			}
			break;
		}
		}
	}
}
