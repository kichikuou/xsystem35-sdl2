#include <stdio.h>
#include <string.h>
#include <glib.h>

#include "portab.h"
#include "surface.h"
#include "ngraph.h"
#include "ags.h"

int gr_buller(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int width, int height, int step) {
	BYTE *dp, *sp;
	int x, y;
	int r1, g1, b1;
	int r2, g2, b2;
	int r3, g3, b3;
	
	if (src == NULL || dst == NULL) return NG;
	if (!gr_clip(src, &sx, &sy, &width, &height, dst, &dx, &dy)) return NG;
	
	sp = GETOFFSET_PIXEL(src, sx, sy);
	dp = GETOFFSET_PIXEL(dst, dx, dy);
	switch(dst->depth) {
	case 15:
	{
		WORD *yld, *yls;
		
		for (y = 0; y < height; y++) {
			yls  = (WORD *)(sp + y * src->bytes_per_line);
			yld  = (WORD *)(dp + y * dst->bytes_per_line);
			for (x = 0; x < step; x++) {
				*yld = *(yls+step);
				yls++; yld++;
			}
			
			for (; x < (width - step*2); x++) {
				r1 = PIXR15((*(yls+step)));
				g1 = PIXG15((*(yls+step)));
				b1 = PIXB15((*(yls+step)));
				r2 = PIXR15((*(yls-step)));
				g2 = PIXG15((*(yls-step)));
				b2 = PIXB15((*(yls-step)));
				r3 = MIN(255, (r1+r2) >> 1);
				g3 = MIN(255, (g1+g2) >> 1);
				b3 = MIN(255, (b1+b2) >> 1);
				
				*yld = PIX15(r3, g3, b3);
				yld++; yls++;
			}
			for (; x < width; x++) {
				*yld = *(yls-step);
				yls++; yld++;
			}				
		}
		break;
	}
	case 16:
	{
		WORD *yld, *yls;
		
		for (y = 0; y < height; y++) {
			yls  = (WORD *)(sp + y * src->bytes_per_line);
			yld  = (WORD *)(dp + y * dst->bytes_per_line);
			for (x = 0; x < step; x++) {
				*yld = *(yls+step);
				yls++; yld++;
			}
			
			for (; x < (width - step*2); x++) {
				r1 = PIXR16((*(yls+step)));
				g1 = PIXG16((*(yls+step)));
				b1 = PIXB16((*(yls+step)));
				r2 = PIXR16((*(yls-step)));
				g2 = PIXG16((*(yls-step)));
				b2 = PIXB16((*(yls-step)));
				r3 = MIN(255, (r1+r2) >> 1);
				g3 = MIN(255, (g1+g2) >> 1);
				b3 = MIN(255, (b1+b2) >> 1);
				
				*yld = PIX16(r3, g3, b3);
				yld++; yls++;
			}
			for (; x < width; x++) {
				*yld = *(yls-step);
				yls++; yld++;
			}				
		}
		break;
	}
	case 24:
	case 32:
	{
		DWORD *yld, *yls;
		
		for (y = 0; y < height; y++) {
			yls  = (DWORD *)(sp + y * src->bytes_per_line);
			yld  = (DWORD *)(dp + y * dst->bytes_per_line);
			for (x = 0; x < step; x++) {
				*yld = *(yls+step);
				yls++; yld++;
			}
			
			for (; x < (width - step*2); x++) {
				r1 = PIXR24((*(yls+step)));
				g1 = PIXG24((*(yls+step)));
				b1 = PIXB24((*(yls+step)));
				r2 = PIXR24((*(yls-step)));
				g2 = PIXG24((*(yls-step)));
				b2 = PIXB24((*(yls-step)));
				r3 = MIN(255, (r1+r2) >> 1);
				g3 = MIN(255, (g1+g2) >> 1);
				b3 = MIN(255, (b1+b2) >> 1);
				
				*yld = PIX24(r3, g3, b3);
				yld++; yls++;
			}
			for (; x < width; x++) {
				*yld = *(yls-step);
				yls++; yld++;
			}				
		}
		break;
	}}
	
	return OK;
}

// 縦方向ブラー
int gr_buller_v(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int width, int height, int step) {
	BYTE *dp, *sp;
	int x, y;
	int r1, g1, b1;
	int r2, g2, b2;
	int r3, g3, b3;
	
	if (src == NULL || dst == NULL) return NG;
	if (!gr_clip(src, &sx, &sy, &width, &height, dst, &dx, &dy)) return NG;
	
	sp = GETOFFSET_PIXEL(src, sx, sy);
	dp = GETOFFSET_PIXEL(dst, dx, dy);
	switch(dst->depth) {
	case 15:
	{
		WORD *yld, *yls;
		
		for (x = 0; x < width; x++) {
			yls  = (WORD *)(sp + x * src->bytes_per_pixel);
			yld  = (WORD *)(dp + x * dst->bytes_per_pixel);
			for (y = 0; y < step; y++) {
				*(yld+y*dst->width) = *(yls+(y+step)*src->width);
			}
			
			for (; y < (height - step*2); y++) {
				r1 = PIXR15((*(yls+(y+step)*src->width)));
				g1 = PIXG15((*(yls+(y+step)*src->width)));
				b1 = PIXB15((*(yls+(y+step)*src->width)));
				r2 = PIXR15((*(yls+(y-step)*src->width)));
				g2 = PIXG15((*(yls+(y-step)*src->width)));
				b2 = PIXB15((*(yls+(y-step)*src->width)));
				r3 = MIN(255, (r1+r2) >> 1);
				g3 = MIN(255, (g1+g2) >> 1);
				b3 = MIN(255, (b1+b2) >> 1);
				
				*(yld+y*dst->width) = PIX15(r3, g3, b3);
			}
			for (; y < height; y++) {
				*(yld+y*dst->width) = *(yls+(y-step)*src->width);
			}				
		}
		break;
	}
	case 16:
	{
		WORD *yld, *yls;
		
		for (x = 0; x < width; x++) {
			yls  = (WORD *)(sp + x * src->bytes_per_pixel);
			yld  = (WORD *)(dp + x * dst->bytes_per_pixel);
			for (y = 0; y < step; y++) {
				*(yld+y*dst->width) = *(yls+(y+step)*src->width);
			}
			
			for (; y < (height - step*2); y++) {
				r1 = PIXR16((*(yls+(y+step)*src->width)));
				g1 = PIXG16((*(yls+(y+step)*src->width)));
				b1 = PIXB16((*(yls+(y+step)*src->width)));
				r2 = PIXR16((*(yls+(y-step)*src->width)));
				g2 = PIXG16((*(yls+(y-step)*src->width)));
				b2 = PIXB16((*(yls+(y-step)*src->width)));
				r3 = MIN(255, (r1+r2) >> 1);
				g3 = MIN(255, (g1+g2) >> 1);
				b3 = MIN(255, (b1+b2) >> 1);
				
				*(yld+y*dst->width) = PIX16(r3, g3, b3);
			}
			for (; y < height; y++) {
				*(yld+y*dst->width) = *(yls+(y-step)*src->width);
			}				
		}
		break;
	}
	case 24:
	case 32:
	{
		DWORD *yld, *yls;
		
		for (x = 0; x < width; x++) {
			yls  = (DWORD *)(sp + x * src->bytes_per_pixel);
			yld  = (DWORD *)(dp + x * dst->bytes_per_pixel);
			for (y = 0; y < step; y++) {
				*(yld+y*dst->width) = *(yls+(y+step)*src->width);
			}
			
			for (; y < (height - step*2); y++) {
				r1 = PIXR24((*(yls+(y+step)*src->width)));
				g1 = PIXG24((*(yls+(y+step)*src->width)));
				b1 = PIXB24((*(yls+(y+step)*src->width)));
				r2 = PIXR24((*(yls+(y-step)*src->width)));
				g2 = PIXG24((*(yls+(y-step)*src->width)));
				b2 = PIXB24((*(yls+(y-step)*src->width)));
				r3 = MIN(255, (r1+r2) >> 1);
				g3 = MIN(255, (g1+g2) >> 1);
				b3 = MIN(255, (b1+b2) >> 1);
				
				*(yld+y*dst->width) = PIX24(r3, g3, b3);
			}
			for (; y < height; y++) {
				*(yld+y*dst->width) = *(yls+(y-step)*src->width);
			}				
		}
		break;
	}}
	
	return OK;
}

