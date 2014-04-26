#ifndef __GLEFFECTCOPY_H__
#define __GLEFFECTCOPY_H__

#include "surface.h"

extern void gpx_effect(int no,
		       surface_t *write, int wx, int wy,
		       surface_t *dst, int dx, int dy,
		       surface_t *src, int sx, int sy,
		       int width, int height,
		       int time,
		       int *endtype);

#endif /* __GLEFFECTCOPY_H__ */
