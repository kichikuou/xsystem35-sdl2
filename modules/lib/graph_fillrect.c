// fill rectangle

#include <string.h>
#include "portab.h"
#include "surface.h"
#include "ngraph.h"
#include "ags.h"

int gr_fill(surface_t *dst, int dx, int dy, int dw, int dh, int r, int g, int b) {
	BYTE *dp, *dp_;
	int x, y;
	
	if (!gr_clip_xywh(dst, &dx, &dy, &dw, &dh)) {
		return NG;
	}
	
	dp = dp_ = GETOFFSET_PIXEL(dst, dx, dy);
	
	switch(dst->depth) {
	case 8:
		memset(dp, r, dw);
		break;
		
	case 15:
	{
		WORD pic15 = PIX15(r, g, b);
		
		for (x = 0; x < dw; x++) {
			*((WORD *)dp + x) = pic15;
		}
		break;
	}
	case 16:
	{
		WORD pic16 = PIX16(r, g, b);
		
		for (x = 0; x < dw; x++) {
			*((WORD *)dp + x) = pic16;
		}
		
		break;
	}
	case 24:
	case 32:
	{
		DWORD pic24 = PIX24(r, g, b);
		
		for (x = 0; x < dw; x++) {
			*((DWORD *)dp + x) = pic24;
		}
		
		break;
	}
	}
	
	dp += dst->bytes_per_line;
	for (y = 1; y < dh; y++) {
		memcpy(dp, dp_, dw * dst->bytes_per_pixel);
		dp += dst->bytes_per_line;
	}

	return OK;
}
