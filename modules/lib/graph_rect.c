// draw rectangle

#include "portab.h"
#include "surface.h"
#include "ngraph.h"
#include "ags.h"

int gr_drawrect(surface_t *dst, int x, int y, int w, int h, int r, int g, int b) {
	BYTE *dp;
	int i;
	int col = 0;
	
	if (FALSE == gr_clip_xywh(dst, &x, &y, &w, &h)) {
		return NG;
	}

	dp = GETOFFSET_PIXEL(dst, x, y);
	
	switch(dst->depth) {
	case 8:
		col = r; break;
	case 15:
		col = PIX15(r, g, b); break;
	case 16:
		col = PIX16(r, g, b); break;
	case 24:
	case 32:
		col = PIX24(r, g, b); break;
	}
	
	switch(dst->depth) {
	case 8:
		/* top */
		for (i = 0; i < w; i++) {
			*((BYTE *)dp + i) = col;
		}
		
		/* side */
		h-=2;
		for (i = 0; i < h; i++) {
			dp += dst->bytes_per_line;
			*((BYTE *)dp)         = col;
			*((BYTE *)dp + w - 1) = col;
		}
		
		/* bottom */
		dp += dst->bytes_per_line;
		for (i = 0; i < w; i++) {
			*((BYTE *)dp + i) = col;
		}
		break;
	case 15:
	case 16:
		/* top */
		for (i = 0; i < w; i++) {
			*((WORD *)dp + i) = col;
		}
		
		/* side */
		h-=2;
		for (i = 0; i < h; i++) {
			dp += dst->bytes_per_line;
			*((WORD *)dp)         = col;
			*((WORD *)dp + w - 1) = col;
		}
		
		/* bottom */
		dp += dst->bytes_per_line;
		for (i = 0; i < w; i++) {
			*((WORD *)dp + i) = col;
		}
		
		break;
	case 24:
	case 32:
		/* top */
		for (i = 0; i < w; i++) {
			*((DWORD *)dp + i) = col;
		}
		
		/* side */
		h-=2;
		for (i = 0; i < h; i++) {
			dp += dst->bytes_per_line;
			*((DWORD *)dp)         = col;
			*((DWORD *)dp + w - 1) = col;
		}
		
		/* bottom */
		dp += dst->bytes_per_line;
		for (i = 0; i < w; i++) {
			*((DWORD *)dp + i) = col;
		}
		
		break;
	}
	
	return OK;
}
