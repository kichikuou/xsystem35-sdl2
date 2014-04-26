#include <stdio.h>

#include "portab.h"
#include "system.h"
#include "cg.h"
#include "ags.h"
#include "surface.h"
#include "ngraph.h"
#include "night.h"
#include "sprite.h"
#include "nt_scenario.h"
#include "sactcg.h"

// 壁紙スプライト表示位置
#define WALL_LOC_X 0
#define WALL_LOC_Y 0
// 風景スプライト表示位置
#define SCENERY_LOC_X 0
#define SCENERY_LOC_Y 64
// 顔スプライト表示位置
#define FACE_LOC_X 6
#define FACE_LOC_Y 272
// 立ち絵左スプライト表示位置
#define TACHI_L_LOC_X 0
#define TACHI_L_LOC_Y 0
// 立ち絵中央スプライト位置
#define TACHI_M_LOC_X 160
#define TACHI_M_LOC_Y 0
// 立ち絵右スプライト位置
#define TACHI_R_LOC_X 360
#define TACHI_R_LOC_Y 0

static int effecttime;

void nt_gr_init() {
}

void nt_gr_set_wallpaper(int no) {
	sprite_t *sp = night.sp[SPNO_WALL];
	
	if (sp) {
		sp_remove_updatelist(sp);
		sp_free(sp);
	}
	
	if (no == 1013 && nt_sco_is_natsu()) {
		no = 1011;
	}
	
	if (no == 1014 && nt_sco_is_natsu()) {
		no = 1012;
	}
	
	sp = sp_new(SPNO_WALL, no, 0, 0, SPRITE_WP);
	sp_add_updatelist(sp);
	if (no == 0) {
		sp->cursize.width  = sf0->width;
		sp->cursize.height = sf0->height;
		sp->update = sp_draw_wall;
	}
	
	night.sp[SPNO_WALL] = sp;
}

void nt_gr_set_scenery(int no) {
	sprite_t *sp = night.sp[SPNO_SCENERY];

	if (sp) {
		sp_remove_updatelist(sp);
		sp_free(sp);
		sp = NULL;
	}

	if (no) {
		sp = sp_new(SPNO_SCENERY, no, 0, 0, SPRITE_NORMAL);
		sp_add_updatelist(sp);
		sp_set_loc(sp, SCENERY_LOC_X, SCENERY_LOC_Y);
	}
	
	night.sp[SPNO_SCENERY] = sp;
}

void nt_gr_set_face(int no) {
	sprite_t *sp = night.sp[SPNO_FACE];
	
	if (sp) {
		sp_remove_updatelist(sp);
		sp_free(sp);
		sp = NULL;
	}
	if (no) {
		sp = sp_new(SPNO_FACE, no, 0, 0, SPRITE_NORMAL);
		sp_add_updatelist(sp);
		sp_set_loc(sp, FACE_LOC_X, FACE_LOC_Y);
		night.msgplace = 2;
	} else {
		night.msgplace = 0;
	}
	night.sp[SPNO_FACE] = sp;
}

void nt_gr_set_spL(int no) {
	sprite_t *sp = night.sp[SPNO_TACHI_L];
	
	if (sp) {
		sp_remove_updatelist(sp);
		sp_free(sp);
		sp = NULL;
	}
	if (no) {
		sp = sp_new(SPNO_TACHI_L, no, 0, 0, SPRITE_NORMAL);
		sp_add_updatelist(sp);
		sp_set_loc(sp, TACHI_L_LOC_X, TACHI_L_LOC_Y);
	}
	night.sp[SPNO_TACHI_L] = sp;
}

void nt_gr_set_spM(int no) {
	sprite_t *sp = night.sp[SPNO_TACHI_M];
	
	if (sp) {
		sp_remove_updatelist(sp);
		sp_free(sp);
		sp = NULL;
	}
	if (no) {
		sp = sp_new(SPNO_TACHI_M, no, 0, 0, SPRITE_NORMAL);
		sp_add_updatelist(sp);
		sp_set_loc(sp, TACHI_M_LOC_X, TACHI_M_LOC_Y);
	}
	night.sp[SPNO_TACHI_M] = sp;
}

void nt_gr_set_spR(int no) {
	sprite_t *sp = night.sp[SPNO_TACHI_R];

	if (sp) {
		sp_remove_updatelist(sp);
		sp_free(sp);
		sp = NULL;
	}
	if (no) {
		sp = sp_new(SPNO_TACHI_R, no, 0, 0, SPRITE_NORMAL);
		sp_add_updatelist(sp);
		sp_set_loc(sp, TACHI_R_LOC_X, TACHI_R_LOC_Y);
	}
	night.sp[SPNO_TACHI_R] = sp;
}

void nt_gr_set_spsL(int no) {
	if (no) {
		if (nt_sco_is_natsu()) no++;
	}
	nt_gr_set_spL(no);
}

void nt_gr_set_spsM(int no) {
	if (no) {
		if (nt_sco_is_natsu()) no++;
	}
	nt_gr_set_spM(no);
}

void nt_gr_set_spsR(int no) {
	if (no) {
		if (nt_sco_is_natsu()) no++;
	}
	nt_gr_set_spR(no);
}

void nt_gr_set_drawtime(int msec) {
	effecttime = msec;
}

void nt_gr_draw(int effectno) {
	switch(effectno) {
	case 0:
		// 全消し
		break;
	case 1:
		sp_update_all(TRUE);
		break;
	default:
		if (night.waitskiplv > 1) {
			sp_update_all(TRUE);
			break;
		}
		//sp_eupdate(effectno, effecttime, TRUE);
		sp_eupdate(effectno, 1000, TRUE);
	}
}

void nt_gr_screencg(int no, int x, int y) {
	surface_t *sf;
	
	ags_sync();
	
	sf = sf_loadcg_no(no -1);
	
	gre_BlendScreen(sf0, x, y, sf0, x, y, sf, 0, 0, sf->width, sf->height);
	
	ags_updateArea(x, y, sf->width, sf->height);
	
	sf_free(sf);
}


