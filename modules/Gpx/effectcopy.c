#include "config.h"

#include <stdio.h>

#include "portab.h"
#include "system.h"
#include "surface.h"
#include "sdl_core.h"
#include "ags.h"
#include "input.h"
#include "graph.h"
#include "ngraph.h"

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

	enum sdl_effect_type type = from_sact_effect(no);
	if (type == EFFECT_INVALID) {
		WARNING("Unimplemented effect %d\n", no);
		type = EFFECT_CROSSFADE;
	}
	SDL_Rect rect = { wx, wy, width, height };
	struct sdl_effect *eff = sdl_effect_init(&rect, dst, dx, dy, src, sx, sy, type);
	if (!time)
		time = (no == SACT_EFFECT_CROSSFADE_DOWN || no == SACT_EFFECT_CROSSFADE_UP) ? 1150 : 2700;
	ags_runEffect(time, FALSE, (ags_EffectStepFunc)sdl_effect_step, eff);
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
}
