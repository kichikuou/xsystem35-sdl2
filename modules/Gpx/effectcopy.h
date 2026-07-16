#ifndef __GLEFFECTCOPY_H__
#define __GLEFFECTCOPY_H__

#include "portab.h"
#include "surface.h"

extern void gpx_effect(int no,
		       int wx, int wy,
		       surface_t *dst, int dx, int dy,
		       surface_t *src, int sx, int sy,
		       int width, int height,
		       int time,
		       vmvar_t *endtype);

#endif /* __GLEFFECTCOPY_H__ */
