#ifndef __SURFACE_H__
#define __SURFACE_H__

#include "portab.h"
#include "ags.h"

#define surface_t agsurface_t

extern surface_t *sf_create_surface(int width, int height, int depth);
extern surface_t *sf_create_alpha(int width, int height);
extern surface_t *sf_create_pixel(int width, int height, int depth);
extern void       sf_free(surface_t *s);
extern surface_t *sf_dup(surface_t *in);
extern surface_t *sf_dup2(surface_t *in, boolean copypixel, boolean copyalpha);
extern void       sf_copyall(surface_t *dst, surface_t *src);


#endif /* __SURFACE_H__ */
