#ifndef __GRAPH_H__
#define __GRAPH_H__

#include "config.h"
#include "surface.h"

extern void gr_init();

extern void gr_copy_stretch_blend_alpha_map(surface_t *dst, int dx, int dy, int dw, int dh, surface_t *src, int sx, int sy, int sw, int sh);
extern void gr_fill_alpha_overborder(surface_t *dst, int dx, int dy, int dw, int dh, int s, int d);
extern void gr_fill_alpha_underborder(surface_t *dst, int dx, int dy, int dw, int dh, int s, int d);
extern void gr_copy_alpha_map_sprite(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int sw, int sh, int cl);
extern void gr_blend_alpha_wds_stretch2x2(surface_t *src1, int sx1, int sy1, surface_t *src2, int sx2, int sy2, int sw, int sh, surface_t *dst, int dx, int dy);
extern void gr_blend_alpha_wds(surface_t *src1, int sx1, int sy1, surface_t *src2, int sx2, int sy2, int sw, int sh, surface_t *dst, int dx, int dy);

#endif /* __GRAPH_H__ */

