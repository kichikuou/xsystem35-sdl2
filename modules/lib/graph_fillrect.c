// fill rectangle

#include <string.h>
#include "portab.h"
#include "surface.h"
#include "ngraph.h"
#include "ags.h"

void gr_fill(surface_t *dst, int dx, int dy, int dw, int dh, int r, int g, int b) {
	uint8_t *dp, *dp_;
	int x, y;
	
	if (!gr_clip_xywh(dst, &dx, &dy, &dw, &dh)) {
		return;
	}
	
	dp = dp_ = GETOFFSET_PIXEL(dst, dx, dy);
	
	switch(dst->depth) {
	case 8:
		memset(dp, r, dw);
		break;
		
	case 16:
	{
		uint16_t pic16 = PIX16(r, g, b);
		
		for (x = 0; x < dw; x++) {
			*((uint16_t *)dp + x) = pic16;
		}
		
		break;
	}
	case 24:
	case 32:
	{
		uint32_t pic24 = PIX24(r, g, b);
		
		for (x = 0; x < dw; x++) {
			*((uint32_t *)dp + x) = pic24;
		}
		
		break;
	}
	}
	
	dp += dst->bytes_per_line;
	for (y = 1; y < dh; y++) {
		memcpy(dp, dp_, dw * dst->bytes_per_pixel);
		dp += dst->bytes_per_line;
	}
}
