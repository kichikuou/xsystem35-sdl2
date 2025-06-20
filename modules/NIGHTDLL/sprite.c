#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>

#include "portab.h"
#include "sprite.h"
#include "sactcg.h"
#include "nt_msg.h"

sprite_t *nt_sp_new(int no, int cg1, int cg2, int cg3, int type) {
	sprite_t *sp;
	
	sp = calloc(1, sizeof(sprite_t));
	
	sp->no = no;
	sp->type = type;
	
	sp->cg1 = cg1 ? nt_scg_addref(cg1) : NULL;
	sp->cg2 = cg2 ? nt_scg_addref(cg2) : NULL;
	sp->cg3 = cg3 ? nt_scg_addref(cg3) : NULL;
	
	sp->curcg = sp->cg1;
	sp->show = true;
	sp->blendrate = 255;
	sp->loc.x = 0;
	sp->loc.y = 0;
	sp->cur = sp->loc;
	
	if (sp->curcg == NULL) {
		sp->width  = 0;
		sp->height = 0;
	} else {
		sp->width  = sp->curcg->sf->w;
		sp->height = sp->curcg->sf->h;
	}
	sp->update = DEFAULT_UPDATE;
	
	switch (type) {
	case SPRITE_ANIME:
		// sp_anime_setup(sp);
		break;
	}

	
	return sp;
}

sprite_t *nt_sp_msg_new(int no, int x, int y, int width, int height) {
	sprite_t *sp;
	
	sp = calloc(1, sizeof(sprite_t));
	
	sp->no = no;
	sp->type = SPRITE_MSG;
	sp->show = true;
	sp->blendrate = 255;
	sp->loc.x = x;
	sp->loc.y = y;
	sp->cur = sp->loc;
	sp->width  = width;
	sp->height = height;
	sp->u.msg.dspcur.x = 0;
	sp->u.msg.dspcur.y = 0;
	sp->u.msg.canvas = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_ARGB8888);
	sp->update = ntmsg_update;
	
	return sp;
}

void nt_sp_free(sprite_t *sp) {
	if (sp == NULL) return;
	
	if (sp->cg1) nt_scg_deref(sp->cg1);
	if (sp->cg2) nt_scg_deref(sp->cg2);
	if (sp->cg3) nt_scg_deref(sp->cg3);

	if (sp->type == SPRITE_MSG) {
		SDL_FreeSurface(sp->u.msg.canvas);
	}
	
	free(sp);
}

void nt_sp_set_show(sprite_t *sp, bool show) {
	bool oldshow;
	
	if (sp == NULL) return;
	
	oldshow = sp->show;
	sp->show = show;

	if (oldshow != show) {
		nt_sp_updateme(sp);
	}
}

#if 0
void nt_sp_set_cg(sprite_t *sp, int no) {
	cginfo_t *cg;

	if (sp == NULL) return;

	if (sp->curcg)
		nt_scg_deref(sp->curcg);

	cg = no ? nt_scg_addref(no) : NULL;
	
	if (cg == NULL) {
		sp->width  = 0;
		sp->height = 0;
	} else {
		sp->width  = cg->sf->w;
		sp->height = cg->sf->h;
	}

	sp->curcg = cg;
}
#endif

void nt_sp_set_loc(sprite_t *sp, int x, int y) {
	if (sp == NULL) return;
	sp->cur.x = sp->loc.x = x;
	sp->cur.y = sp->loc.y = y;
}

// #include "nt_sprite_draw.c"
// #include "nt_sprite_update.c"
// #include "nt_sprite_eupdate.c"
