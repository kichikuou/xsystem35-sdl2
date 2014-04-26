// dst の上に src をブレンド率 lv でブレンドしたものを write へ書き出し

#include "config.h"

#include "portab.h"
#include "surface.h"
#include "ngraph.h"
#include "ags.h"

int gre_Blend(surface_t *write, int wx, int wy, surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int width, int height, int lv) {
	BYTE *sp, *dp, *wp;
	int x, y;
	
	wp = GETOFFSET_PIXEL(write, wx, wy);
	sp = GETOFFSET_PIXEL(src,   sx, sy);
	dp = GETOFFSET_PIXEL(dst,   dx, dy);
	
	switch(dst->depth) {
	case 15:
	{
		WORD *yls, *yld, *ylw;
		
		for (y = 0; y < height; y++) {
			yls = (WORD *)(sp + y * src->bytes_per_line);
			yld = (WORD *)(dp + y * dst->bytes_per_line);
			ylw = (WORD *)(wp + y * write->bytes_per_line);
			
			for (x = 0; x < width; x++) {
				*ylw = ALPHABLEND15(*yls, *yld, lv);
				yls++; yld++; ylw++;
			}
		}
		break;
	}
	case 16:
		if (nact->mmx_is_ok) {
//		if (0) {
#ifdef ENABLE_MMX
			int alpha = lv | lv << 8 | lv << 16 | lv << 24;
			ablend16_ppd(wp, sp, dp, alpha,
				     width, height,
				     write->bytes_per_line,
				     src->bytes_per_line,
				     dst->bytes_per_line);
#endif
		} else {
			WORD *yls, *yld, *ylw;
			
			for (y = 0; y < height; y++) {
				yls = (WORD *)(sp + y * src->bytes_per_line);
				yld = (WORD *)(dp + y * dst->bytes_per_line);
				ylw = (WORD *)(wp + y * write->bytes_per_line);
				
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
		DWORD *yls, *yld, *ylw;
		
		for (y = 0; y < height; y++) {
			yls = (DWORD *)(sp + y * src->bytes_per_line);
			yld = (DWORD *)(dp + y * dst->bytes_per_line);
			ylw = (DWORD *)(wp + y * write->bytes_per_line);
			
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
