// draw rectangle

#include "portab.h"
#include "surface.h"
#include "ngraph.h"
#include "ags.h"

void gr_drawrect(surface_t *dst, int x, int y, int w, int h, int r, int g, int b) {
	uint8_t *dp;
	int i;
	int col = 0;
	
	if (!gr_clip_xywh(dst, &x, &y, &w, &h)) {
		return;
	}

	dp = GETOFFSET_PIXEL(dst, x, y);
	
	switch(dst->depth) {
	case 8:
		col = r; break;
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
			*((uint8_t *)dp + i) = col;
		}
		
		/* side */
		h-=2;
		for (i = 0; i < h; i++) {
			dp += dst->bytes_per_line;
			*((uint8_t *)dp)         = col;
			*((uint8_t *)dp + w - 1) = col;
		}
		
		/* bottom */
		dp += dst->bytes_per_line;
		for (i = 0; i < w; i++) {
			*((uint8_t *)dp + i) = col;
		}
		break;
	case 16:
		/* top */
		for (i = 0; i < w; i++) {
			*((uint16_t *)dp + i) = col;
		}
		
		/* side */
		h-=2;
		for (i = 0; i < h; i++) {
			dp += dst->bytes_per_line;
			*((uint16_t *)dp)         = col;
			*((uint16_t *)dp + w - 1) = col;
		}
		
		/* bottom */
		dp += dst->bytes_per_line;
		for (i = 0; i < w; i++) {
			*((uint16_t *)dp + i) = col;
		}
		
		break;
	case 24:
	case 32:
		/* top */
		for (i = 0; i < w; i++) {
			*((uint32_t *)dp + i) = col;
		}
		
		/* side */
		h-=2;
		for (i = 0; i < h; i++) {
			dp += dst->bytes_per_line;
			*((uint32_t *)dp)         = col;
			*((uint32_t *)dp + w - 1) = col;
		}
		
		/* bottom */
		dp += dst->bytes_per_line;
		for (i = 0; i < w; i++) {
			*((uint32_t *)dp + i) = col;
		}
		
		break;
	}
}
