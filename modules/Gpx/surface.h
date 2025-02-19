#ifndef __SURFACE_H__
#define __SURFACE_H__

#include "portab.h"
#include "ags.h"

extern surface_t *sf_create_surface(int width, int height);
extern surface_t *sf_create_alpha(int width, int height);
extern surface_t *sf_create_pixel(int width, int height);
extern void       sf_free(surface_t *s);
extern surface_t *sf_dup(surface_t *in);

#endif /* __SURFACE_H__ */
