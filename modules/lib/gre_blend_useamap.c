// dst の上に src をalphamapを参照しつつ、ブレンド率 lv でブレンド
// したものを write へ書き出し

#include "config.h"

#include "portab.h"
#include "surface.h"
#include "ngraph.h"
#include "ags.h"

int gre_BlendUseAMap(surface_t *write, int wx, int wy, surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int width, int height, surface_t *alpha, int ax, int ay, int lv) {
	BYTE *sp, *dp, *wp, *ap;
	int x, y;
	
	wp = GETOFFSET_PIXEL(write, wx, wy);
	sp = GETOFFSET_PIXEL(src,   sx, sy);
	dp = GETOFFSET_PIXEL(dst,   dx, dy);
	ap = GETOFFSET_ALPHA(alpha, ax, ay);
	
	if (lv == 255) {
		switch(dst->depth) {
		case 15:
		{
			WORD *yls, *yld, *ylw;
			BYTE *yla;
			
			for (y = 0; y < height; y++) {
				yls = (WORD *)(sp + y * src->bytes_per_line);
				yld = (WORD *)(dp + y * dst->bytes_per_line);
				ylw = (WORD *)(wp + y * write->bytes_per_line);
				yla = (BYTE *)(ap + y * alpha->width);
				
				for (x = 0; x < width; x++) {
					*ylw = ALPHABLEND15(*yls, *yld, *yla);
					yls++; yld++; ylw++; yla++;
				}
			}
			break;
		}
		case 16:
			if (nact->mmx_is_ok) {
//			if (0) {
#ifdef ENABLE_MMX
				ablend16_ppp(wp, sp, dp, ap,
					     width, height,
					     write->bytes_per_line,
					     src->bytes_per_line,
					     dst->bytes_per_line,
					     alpha->width, 255);
#endif
			} else {
				WORD *yls, *yld, *ylw;
				BYTE *yla;
				
				for (y = 0; y < height; y++) {
					yls = (WORD *)(sp + y * src->bytes_per_line);
					yld = (WORD *)(dp + y * dst->bytes_per_line);
					ylw = (WORD *)(wp + y * write->bytes_per_line);
					yla = (BYTE *)(ap + y * alpha->width);
					
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
			DWORD *yls, *yld, *ylw;
			BYTE  *yla;
			
			for (y = 0; y < height; y++) {
				yls = (DWORD *)(sp + y * src->bytes_per_line);
				yld = (DWORD *)(dp + y * dst->bytes_per_line);
				ylw = (DWORD *)(wp + y * write->bytes_per_line);
				yla = (BYTE  *)(ap + y * alpha->width);
				
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
		case 15:
		{
			WORD *yls, *yld, *ylw;
			BYTE *yla;
			
			for (y = 0; y < height; y++) {
				yls = (WORD *)(sp + y * src->bytes_per_line);
				yld = (WORD *)(dp + y * dst->bytes_per_line);
				ylw = (WORD *)(wp + y * write->bytes_per_line);
				yla = (BYTE *)(ap + y * alpha->width);
				
				for (x = 0; x < width; x++) {
					*ylw = ALPHABLEND15(*yls, *yld, (*yla * lv) / 255);
					yls++; yld++; ylw++; yla++;
				}
			}
			break;
	}
		case 16:
			if (nact->mmx_is_ok) {
//			if (0) {
#ifdef ENABLE_MMX
				ablend16_ppp(wp, sp, dp, ap,
					     width, height,
					     write->bytes_per_line,
					     src->bytes_per_line,
					     dst->bytes_per_line,
					     alpha->width, lv);
#endif
			} else {
				WORD *yls, *yld, *ylw;
				BYTE *yla;
				
				for (y = 0; y < height; y++) {
					yls = (WORD *)(sp + y * src->bytes_per_line);
					yld = (WORD *)(dp + y * dst->bytes_per_line);
					ylw = (WORD *)(wp + y * write->bytes_per_line);
					yla = (BYTE *)(ap + y * alpha->width);
					
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
			DWORD *yls, *yld, *ylw;
			BYTE  *yla;
			
			for (y = 0; y < height; y++) {
				yls = (DWORD *)(sp + y * src->bytes_per_line);
				yld = (DWORD *)(dp + y * dst->bytes_per_line);
				ylw = (DWORD *)(wp + y * write->bytes_per_line);
				yla = (BYTE  *)(ap + y * alpha->width);
				
				for (x = 0; x < width; x++) {
					*ylw = ALPHABLEND24(*yls, *yld, (*yla * lv) / 255);
					yls++; yld++; ylw++; yla++;
				}
			}
			break;
		}
		}
	}
	
	return OK;
}
