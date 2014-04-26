// 指定のsurface領域をlv=255を真っ白 lv=0を通常としてコピー

#include <string.h>

#include "portab.h"
#include "surface.h"
#include "ngraph.h"
#include "ags.h"

void gr_copy_whiteout(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int sw, int sh, int lv) {

	int x, y;
	BYTE *sp, *dp;
	
	if (!gr_clip(src, &sx, &sy, &sw, &sh, dst, &dx, &dy)) return;
	
	sp = GETOFFSET_PIXEL(src, sx, sy);
	dp = GETOFFSET_PIXEL(dst, dx, dy);

	if (sp == NULL || dp == NULL) return;
	
	switch(dst->depth) {
	case 15:
	{
		WORD *yls, *yld;
		
		for (y = 0; y < sh; y++) {
			yls = (WORD *)(sp + y * src->bytes_per_line);
			yld = (WORD *)(dp + y * dst->bytes_per_line);
			
			for (x = 0; x < sw; x++) {
				*yld = WHITELEVEL15(*yls, lv);
				yls++; yld++;
			}
		}
		break;
	}
	case 16:
	{
		WORD *yls, *yld;
		
		for (y = 0; y < sh; y++) {
			yls = (WORD *)(sp + y * src->bytes_per_line);
			yld = (WORD *)(dp + y * dst->bytes_per_line);
			
			for (x = 0; x < sw; x++) {
				*yld = WHITELEVEL16(*yls, lv);
				yls++; yld++;
			}
		}
		break;
	}
	case 24:
	case 32:
	{
		DWORD *yls, *yld;
		
		for (y = 0; y < sh; y++) {
			yls = (DWORD *)(sp + y * src->bytes_per_line);
			yld = (DWORD *)(dp + y * dst->bytes_per_line);
			
			for (x = 0; x < sw; x++) {
				*yld = WHITELEVEL24(*yls, lv);
				yls++; yld++;
			}
		}
		break;
	}
	}
}
