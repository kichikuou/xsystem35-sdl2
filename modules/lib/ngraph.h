#ifndef __NGRAPH_H__
#define __NGRAPH_H__

#include "portab.h"
#include "nact.h"
#include "ags.h"
#include "surface.h"

#define sf0 nact->ags.dib

// DLL 用 graphic 関連関数 

/* in graph.c */
extern boolean gr_clip(surface_t *ss, int *sx, int *sy, int *sw, int *sh, surface_t *ds, int *dx, int *dy);
extern boolean gr_clip_xywh(surface_t *ss, int *sx, int *sy, int *sw, int *sh);


/* in graph_expandcolor_blend.c */
// 8bppのモノクロをcolでブレンド
extern int gr_expandcolor_blend(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int sw, int sh, int r, int g, int b);

/* in graph_fillrect.c */
extern int gr_fill(surface_t *dst, int dx, int dy, int dw, int dh, int r, int g, int b);

/* in graph_fillrect_amap.c */
extern int gr_fill_alpha_map(surface_t *dst, int dx, int dy, int dw, int dh, int lv);

/* in graph_fillrect_acolor.c */
extern int gr_fill_alpha_color(surface_t *dst, int dx, int dy, int dw, int dh, int r, int g, int b, int lv);

/* in graph_rect.c */
// 矩形枠描画
extern int gr_drawrect(surface_t *dst, int x, int y, int w, int h, int r, int g, int b);

/* in graph_copy.c */
extern int gr_copy(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int sw, int sh);

/* in graph_copy_bright.c */
void gr_copy_bright(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int width, int height, int lv);

/* in graph_copy_amap.c */
extern int gr_copy_alpha_map(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int sw, int sh);

/* in graph_blend_amap.c */
extern int gr_blend_alpha_map(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int sw, int sh);

/* in graph_saturadd_amap.c */
// alpha map の飽和加算
extern int gr_saturadd_alpha_map(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int sw, int sh);

/* in graph_draw_amap.c */
extern int gr_draw_amap(surface_t *dst, int dx, int dy, BYTE *src, int width, int height, int scanline);

// /* in graph_bright_dst_only.c */
#define gr_bright_dst_only(dst,dx,dy,w,h,lv) gr_copy_bright(dst,dx,dy,dst,dx,dy,w,h,lv)
// extern void gr_bright_dst_only(surface_t *dst, int dx, int dy, int w, int h, int lv);

/* in gre_blend_useamap.c */
extern int gre_BlendUseAMap(surface_t *write, int wx, int wy, surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int width, int height, surface_t *alpha, int ax, int ay, int lv);

/* in gre_blend.c */
extern int gre_Blend(surface_t *write, int wx, int wy, surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int width, int height, int lv);

/* in gre_blend_screen.c */
extern int gre_BlendScreen(surface_t *write, int wx, int wy, surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int width, int height);

/* in graph_buller */
extern int gr_buller(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int width, int height, int step);
extern int gr_buller_v(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int width, int height, int step);

/* in graph_stretch.c */
extern void gr_copy_stretch(surface_t *dst, int dx, int dy, int dw, int dh, surface_t *src, int sx, int sy, int sw, int sh);

/* in graph_copy_whiteout.c */
extern void gr_copy_whiteout(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int sw, int sh, int lv);


/* in graph_cg.c */
extern void gr_drawimage24(surface_t *ds, cgdata *cg, int x, int y);
extern void gr_drawimage16(surface_t *ds, cgdata *cg, int x, int y);




/* defined in cg.c */
extern surface_t *sf_getcg(void *buf);
extern surface_t *sf_loadcg_no(int no);


#endif /* __GRAPH_H__ */
