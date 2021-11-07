#include "config.h"
#include <stdio.h>
#include <stdlib.h>

#include "portab.h"
#include "sprite.h"
#include "sactcg.h"
#include "surface.h"
#include "ngraph.h"
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
	sp->show = TRUE;
	sp->blendrate = 255;
	sp->loc.x = 0;
	sp->loc.y = 0;
	sp->cur = sp->loc;
	
	if (sp->curcg == NULL) {
		sp->cursize.width  = 0;
		sp->cursize.height = 0;
	} else {
		sp->cursize.width  = sp->curcg->sf->width;
		sp->cursize.height = sp->curcg->sf->height;
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
	sp->show = TRUE;
	sp->blendrate = 255;
	sp->loc.x = x;
	sp->loc.y = y;
	sp->cur = sp->loc;
	sp->cursize.width  = width;
	sp->cursize.height = height;
	sp->u.msg.dspcur.x = 0;
	sp->u.msg.dspcur.y = 0;
	sp->u.msg.canvas = sf_create_surface(width, height, sf0->depth);
	sp->update = ntmsg_update;
	
	return sp;
}

void nt_sp_free(sprite_t *sp) {
	if (sp == NULL) return;
	
	if (sp->cg1) nt_scg_deref(sp->cg1);
	if (sp->cg2) nt_scg_deref(sp->cg2);
	if (sp->cg3) nt_scg_deref(sp->cg3);

	if (sp->type == SPRITE_MSG) {
		sf_free(sp->u.msg.canvas);
	}
	
	free(sp);
}

void nt_sp_set_show(sprite_t *sp, boolean show) {
	boolean oldshow;
	
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
		sp->cursize.width  = 0;
		sp->cursize.height = 0;
	} else {
		sp->cursize.width  = cg->sf->width;
		sp->cursize.height = cg->sf->height;
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
