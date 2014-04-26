/*
 * sprite.h: スプライト基本各種処理
 *
 * Copyright (C) 1997-1998 Masaki Chikama (Wren) <chikama@kasumi.ipl.mech.nagoya-u.ac.jp>
 *               1998-                           <masaki-c@is.aist-nara.ac.jp>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
*/
/* $Id: sprite.h,v 1.3 2003/07/14 16:22:51 chikama Exp $ */

#ifndef __SPRITE_H__
#define __SPRITE_H__

#include <glib.h>
#include "sact.h"

#define DEFAULT_UPDATE sp_draw

// in sprite.c
extern int sp_init();
extern int sp_new(int no, int cg1, int cg2, int cg3, int type);
extern int sp_new_msg(int no, int x, int y, int width, int height);
extern int sp_set_wall_paper(int no);
extern int sp_free_all();
extern int sp_free(int no);
extern int sp_set_show(int no, int cnt, int flag);
extern int sp_set_pos(int wNum, int wX, int wY);
extern int sp_set_move(int wNum, int wX, int wY);
extern int sp_set_movetime(int wNum, int wTime);
extern int sp_set_movespeed(int wNum, int wTime);
extern int sp_add_zkey_hidesprite(int wNum);
extern int sp_clear_zkey_hidesprite_all();
extern int sp_freeze_sprite(int wNum, int wIndex);
extern int sp_thaw_sprite(int wNum);
extern int sp_add_quakesprite(int wNum);
extern int sp_clear_quakesprite_all();
extern int sp_set_animeinterval(int wNum, int wTime);
extern boolean sp_is_insprite(sprite_t *sp, int x, int y);
extern int sp_set_blendrate(int wNum, int wCount, int rate);
extern int sp_query_isexist(int wNum, int *ret);
extern int sp_query_info(int wNum, int *vtype, int *vcg1, int *vcg2, int *vcg3);
extern int sp_query_show(int wNum, int *vShow);
extern int sp_query_pos(int wNum, int *vx, int *vy);
extern int sp_query_size(int wNum, int *vw, int *vh);
extern int sp_query_textpos(int wNum, int *vx, int *vy);
extern int sp_num_setcg(int nNum, int nIndex, int nCG);
extern int sp_num_getcg(int nNum, int nIndex, int *vCG);
extern int sp_num_setpos(int nNum, int nX, int nY);
extern int sp_num_getpos(int nNum, int *vX, int *vY);
extern int sp_num_setspan(int nNum, int nSpan);
extern int sp_num_getspan(int nNUm, int *vSpan);
extern int sp_exp_clear();
extern int sp_exp_add(int nNumSP1, int nNumSP2);
extern int sp_exp_del(int wNum);
extern int sp_sound_set(int wNumSP, int wNumWave1, int wNumWave2, int wNumWave3);
extern int sp_sound_wait();
extern int sp_sound_ob(int wNumWave);


// in sprite_update.c
extern int sp_update_all(boolean syncscreen);
extern int sp_update_clipped();
extern int sp_updateme(sprite_t *sp);
extern int sp_updateme_part(sprite_t *sp, int x, int y, int w, int h);


// in sprite_draw.c
extern int sp_draw(sprite_t *sf);
extern int sp_draw2(sprite_t *sf, cginfo_t *cg);
extern void sp_draw_dmap(gpointer data, gpointer userdata);


// in sprite_msg.c
extern void smsg_add(char *msg);
extern void smsg_newline(int wNum, int size);
extern void smsg_out(int wNum, int wSize, int wColorR, int wColorG, int wColorB, int wFont, int wSpeed, int wLineSpace, int wAlign, int wRSize, int wRFont, int wRLineSpace, int *wLength);
extern void smsg_clear(int wNum);
extern int smsg_is_empty();
extern int smsg_keywait(int sp1, int sp2, int timeout);
extern int smsg_update(sprite_t *sp);

// in sprite_sel.c
extern void ssel_init();
extern void ssel_clear();
extern void ssel_add(int nString, int wI);
extern int ssel_select(int wNum, int wChoiceSize, int wMenuOutSpc, int wChoiceLineSpace, int wChoiceAutoMoveCursor, int nAlign);

// in sprite_event.c
extern void spev_callback(agsevent_t *e);
extern void spev_add_eventlistener(sprite_t *sp, int (*cb)(sprite_t *, agsevent_t *));
extern void spev_remove_eventlistener(sprite_t *sp);

// in sprite_tevent.c
extern void spev_add_teventlistener(sprite_t *sp, int (*cb)(sprite_t *, agsevent_t *));
extern void spev_remove_teventlistener(sprite_t *sp);
extern void spev_main();

// in sprite_move.c
extern void spev_move_setup(gpointer data, gpointer userdata);
extern void spev_move_waitend(sprite_t *sp, int dx, int dy, int time);
extern void spev_wait4moving_sp();

// in sprite_switch.c
extern int sp_sw_setup(sprite_t *sp);

// in sprite_get.c
extern int sp_get_setup(sprite_t *sp);

// in sprite_put.c
extern int sp_put_setup(sprite_t *sp);

// in sprite_anime.c
extern int sp_anime_setup(sprite_t *sp);

// in sprite_eupdate.c
extern int sp_eupdate(int type, int time, int key);

// in sprite_quake.c
extern int sp_quake_sprite(int wType, int wAmplitudeX, int wAmplitude, int wCount, int cancel);

// in sprite_keywait.c
extern int sp_keywait(int *vOK, int *vRND, int *vRsv1, int *vRsv2, int *vRsv3, int timeout);

// in screen_quake.c
extern int sp_quake_screen(int type, int p1, int p2, int time, int cancel);

// in sactamask.c
extern int sp_eupdate_amap(int type, int time, int key);

// in sprite_xmenu.c
extern int spxm_clear(void);
extern int spxm_register(int reginum, int menuid);
extern int spxm_getnum(int reginum);
extern int spxm_gettext(int regnum, int strno);
extern int spxm_titlereg(void);
extern int spxm_titleget(int strno);

#endif /* __SPRITE_H__ */
