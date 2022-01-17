#include "config.h"

#include <stdio.h>

#include "portab.h"
#include "system.h"
#include "surface.h"
// #include "gleffectcopy.h"
#include "sdl_core.h"
#include "ags.h"
#include "input.h"
#include "graph.h"
#include "ngraph.h"

/* saved parameter */
struct ecopyparam {
	surface_t *src;
	int sx;
	int sy;
	surface_t *dst;
	int dx;
	int dy;
	int w;
	int h;
	surface_t *write;
	int wx;
	int wy;
	int time;
	boolean cancel;
	int spCol;
	int extp[10];
	
	// for ec11
	surface_t *ss[8];
	surface_t *sd[8];
};
typedef struct ecopyparam ecopyparam_t;
static ecopyparam_t ecp;



static void ec11_cb(int step);
static void ec11_prepare(void);

void gpx_effect(int no,
		int wx, int wy,
		surface_t *dst, int dx, int dy,
		surface_t *src, int sx, int sy,
		int width, int height,
		int time,
		int *endtype) {
	surface_t *write = nact->ags.dib;
	if (!gr_clip(dst, &dx, &dy, &width, &height, write, &wx, &wy)) return;
	if (!gr_clip(src, &sx, &sy, &width, &height, write, &wx, &wy)) return;

	*endtype = 0;

	enum sdl_effect_type sdl_effect = from_sact_effect(no);
	if (sdl_effect != EFFECT_INVALID) {
		SDL_Rect rect = { wx, wy, width, height };
		struct sdl_effect *eff = sdl_effect_init(&rect, dst, dx, dy, src, sx, sy, sdl_effect);
		if (!time)
			time = (no == SACT_EFFECT_CROSSFADE_DOWN || SACT_EFFECT_CROSSFADE_UP) ? 1150 : 2700;
		ags_fade2(time, FALSE, (ags_fade2_callback)sdl_effect_step, eff);
		sdl_effect_finish(eff);

		switch (no) {
		case SACT_EFFECT_FADEOUT:
			gr_fill(write, wx, wy, width, height, 0, 0, 0);
			break;
		case SACT_EFFECT_WHITEOUT:
			gr_fill(write, wx, wy, width, height, 255, 255, 255);
			break;
		default:
			gr_copy(write, wx, wy, src, sx, sy, width, height);
			break;
		}
		ags_updateArea(wx, wy, width, height);
		return;
	}

	ecp.write = write;
	ecp.wx  = wx;
	ecp.wy  = wy;
	ecp.dst = dst;
	ecp.dx  = dx;
	ecp.dy  = dy;	
	ecp.src = src;
	ecp.sx  = sx;
	ecp.sy  = sy;
	ecp.w = width;
	ecp.h = height;
	ecp.time = time;

	ags_faderinfo_t i;

	switch(no) {
	case 11:
		ec11_prepare();
		i.step_max = 6;
		i.effect_time = time == 0 ? 2700 : time;
		i.cancel = FALSE;
		i.callback = ec11_cb;
		ags_fader(&i);
		break;
	default:
		gr_copy(write, wx, wy, src, sx, sy, width, height);
		ags_updateArea(wx, wy, width, height);
	}
}


static void ec11_prepare() {
	int i;
	
	ecp.ss[0] = sf_create_surface(ecp.w, ecp.h, ecp.write->depth);
	ecp.sd[0] = sf_create_surface(ecp.w, ecp.h, ecp.write->depth);
	gr_buller(ecp.ss[0], 0, 0, ecp.src, ecp.sx, ecp.sy, ecp.w, ecp.h, 1 << 2);
	gr_buller(ecp.sd[0], 0, 0, ecp.dst, ecp.dx, ecp.dy, ecp.w, ecp.h, 1 << 2);
	
	for (i = 1; i < 6; i++) {
		ecp.ss[i] = sf_create_surface(ecp.w, ecp.h, ecp.write->depth);
		ecp.sd[i] = sf_create_surface(ecp.w, ecp.h, ecp.write->depth);
		gr_buller(ecp.ss[i], 0, 0, ecp.ss[i-1], 0, 0, ecp.w, ecp.h, 1 << (i+2));
		gr_buller(ecp.sd[i], 0, 0, ecp.sd[i-1], 0, 0, ecp.w, ecp.h, 1 << (i+2));
	}
}

static void ec11_cb(int step) {
	int i;
	
	if (step == 6) {
		for (i = 0; i < 6; i++) {
			sf_free(ecp.ss[i]);
			sf_free(ecp.sd[i]);
		}
		gr_copy(ecp.write, ecp.wx, ecp.wy, ecp.src, ecp.sx, ecp.sy, ecp.w, ecp.h);
		ags_updateArea(ecp.wx, ecp.wy, ecp.w, ecp.h);
		return;
	}
	
	gre_Blend(ecp.write, ecp.wx, ecp.wy, ecp.sd[step], 0, 0, ecp.ss[5-step], 0, 0, ecp.w, ecp.h, ((step+1)*256)/7);
	
	ags_updateArea(ecp.wx, ecp.wy, ecp.w, ecp.h);
}

#if 0
static void ec11_cb(int step) {
	static surface_t *ss;
	static surface_t *sd;
	
	if (step == 0) {
		ss = sf_create_surface(ecp.w, ecp.h, ecp.write->depth);
		sd = sf_create_surface(ecp.w, ecp.h, ecp.write->depth);
		gr_copy(ss, 0, 0, ecp.src, ecp.sx, ecp.sy, ecp.w, ecp.h);
		gr_copy(sd, 0, 0, ecp.dst, ecp.dx, ecp.dy, ecp.w, ecp.h);
		return;
	}
	if (step == 6) {
		sf_free(ss);
		sf_free(sd);
		gr_copy(ecp.write, ecp.wx, ecp.wy, ecp.src, ecp.sx, ecp.sy, ecp.w, ecp.h);
		ags_updateArea(ecp.wx, ecp.wy, ecp.w, ecp.h);
		return;
	}
	
	gr_buller(ss, 0, 0, ecp.src, ecp.sx, ecp.sy, ecp.w, ecp.h, 1 << (7-step));
	gr_buller(sd, 0, 0, ecp.dst, ecp.dx, ecp.dy, ecp.w, ecp.h, 1 << (step+1));
	gre_Blend(ecp.write, ecp.wx, ecp.wy, sd, 0, 0, ss, 0, 0, ecp.w, ecp.h, (step*256)/6);
	ags_updateArea(ecp.wx, ecp.wy, ecp.w, ecp.h);
}
#endif
