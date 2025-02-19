#ifndef __NGRAPH_H__
#define __NGRAPH_H__

#include "portab.h"
#include "nact.h"
#include "ags.h"
#include "surface.h"
#include "graph.h"

#define sf0 nact->ags.dib

struct SDL_Surface;

// DLL 用 graphic 関連関数 

/* in graph.c */
bool gr_clip(surface_t *ss, int *sx, int *sy, int *sw, int *sh, surface_t *ds, int *dx, int *dy);
bool gr_clip_xywh(surface_t *ss, int *sx, int *sy, int *sw, int *sh);

/* in graph_fillrect_amap.c */
void gr_fill_alpha_map(surface_t *dst, int dx, int dy, int dw, int dh, int lv);

/* in graph_copy_amap.c */
void gr_copy_alpha_map(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int sw, int sh);

/* in graph_blend_amap.c */
void gr_blend_alpha_map(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int sw, int sh);

/* in graph_draw_amap.c */
void gr_draw_amap(surface_t *dst, int dx, int dy, uint8_t *src, int width, int height, int scanline);

// /* in graph_bright_dst_only.c */
#define gr_bright_dst_only(dst,dx,dy,w,h,lv) gr_copy_bright(dst,dx,dy,dst,dx,dy,w,h,lv)
// void gr_bright_dst_only(surface_t *dst, int dx, int dy, int w, int h, int lv);

/* in graph_cg.c */
void gr_drawimage24(surface_t *ds, cgdata *cg, int x, int y);
void gr_drawimage16(surface_t *ds, cgdata *cg, int x, int y);




/* defined in cg.c */
surface_t *sf_loadcg_no(int no);
struct SDL_Surface *load_cg_to_sdlsurface(int no);

#endif /* __GRAPH_H__ */
