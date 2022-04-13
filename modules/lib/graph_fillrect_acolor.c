// 指定の色(rgb)をブレンド率(lv)を指定して矩形塗りつぶし

#include "config.h"

#include "portab.h"
#include "surface.h"
#include "ngraph.h"
#include "ags.h"

int gr_fill_alpha_color(surface_t *dst, int dx, int dy, int dw, int dh, int r, int g, int b, int lv) {
	int x, y;
	BYTE *dp;
	
	if (!gr_clip_xywh(dst, &dx, &dy, &dw, &dh)) {
		return NG;
	}
	
	dp = GETOFFSET_PIXEL(dst, dx, dy);
	
	switch(dst->depth) {
	case 16:
		{
			WORD pic16 = PIX16(r, g, b);
			WORD *yls;
			
			for (y = 0; y < dh; y++) {
				yls = (WORD *)(dp + y * dst->bytes_per_line);
				for (x = 0; x < dw; x++) {
					*yls = ALPHABLEND16(pic16, *yls, lv);
				yls++;
				}
			}
		}
		break;
	case 24:
	case 32:
	{
		DWORD pic24 = PIX24(r, g, b);
		DWORD *yls;
		
		for (y = 0; y < dh; y++) {
			yls = (DWORD *)(dp + y * dst->bytes_per_line);
			for (x = 0; x < dw; x++) {
				*yls = ALPHABLEND24(pic24, *yls, lv);
				yls++;
			}
		}
		break;
	}
	}
	
	return OK;
}
