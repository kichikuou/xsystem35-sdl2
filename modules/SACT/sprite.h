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

#include "sact.h"

#define DEFAULT_UPDATE sp_draw

// in sprite.c
void sp_init(void);
void sp_reset(void);
void sp_new(int no, int cg1, int cg2, int cg3, int type);
void sp_new_msg(int no, int x, int y, int width, int height);
void sp_set_wall_paper(int no);
void sp_free_all(void);
void sp_free(int no);
void sp_set_show(int no, int cnt, int flag);
void sp_set_pos(int wNum, int wX, int wY);
void sp_set_move(int wNum, int wX, int wY);
void sp_set_movetime(int wNum, int wTime);
void sp_set_movespeed(int wNum, int wTime);
void sp_add_zkey_hidesprite(int wNum);
void sp_clear_zkey_hidesprite_all(void);
void sp_freeze_sprite(int wNum, int wIndex);
void sp_thaw_sprite(int wNum);
void sp_add_quakesprite(int wNum);
void sp_clear_quakesprite_all(void);
void sp_set_animeinterval(int wNum, int wTime);
bool sp_is_insprite(sprite_t *sp, int x, int y);
void sp_set_blendrate(int wNum, int wCount, int rate);
bool sp_exists(int wNum);
bool sp_query_info(int wNum, int *vtype, int *vcg1, int *vcg2, int *vcg3);
bool sp_query_show(int wNum, int *vShow);
bool sp_query_pos(int wNum, int *vx, int *vy);
bool sp_query_size(int wNum, int *vw, int *vh);
bool sp_query_textpos(int wNum, int *vx, int *vy);
void sp_num_setcg(int nNum, int nIndex, int nCG);
void sp_num_getcg(int nNum, int nIndex, int *vCG);
void sp_num_setpos(int nNum, int nX, int nY);
void sp_num_getpos(int nNum, int *vX, int *vY);
void sp_num_setspan(int nNum, int nSpan);
void sp_num_getspan(int nNUm, int *vSpan);
void sp_exp_clear(void);
void sp_exp_add(int nNumSP1, int nNumSP2);
void sp_exp_del(int wNum);
void sp_sound_set(int wNumSP, int wNumWave1, int wNumWave2, int wNumWave3);
void sp_sound_wait(void);
void sp_sound_ob(int wNumWave);


// in sprite_update.c
void sp_update_all(bool syncscreen);
void sp_update_clipped();
void sp_updateme(sprite_t *sp);
void sp_updateme_part(sprite_t *sp, int x, int y, int w, int h);


// in sprite_draw.c
void sp_draw(sprite_t *sf);
void sp_draw_dmap(void* data, void* userdata);


// in sprite_msg.c
void smsg_add(const char *msg);
void smsg_newline(int wNum, int size);
void smsg_out(int wNum, int wSize, int wColorR, int wColorG, int wColorB, int wFont, int wSpeed, int wLineSpace, int wAlign, int wRSize, int wRFont, int wRLineSpace, int *wLength);
void smsg_clear(int wNum);
bool smsg_is_empty();
int smsg_peek(int nTopStringNum);
int smsg_keywait(int sp1, int sp2, int timeout);
void smsg_update(sprite_t *sp);

// in sprite_sel.c
void ssel_init();
void ssel_reset(void);
void ssel_clear();
void ssel_add(int nString, int wI);
int ssel_select(int wNum, int wChoiceSize, int wMenuOutSpc, int wChoiceLineSpace, int wChoiceAutoMoveCursor, int nAlign);

// in sprite_event.c
void spev_callback(agsevent_t *e);
void spev_add_eventlistener(sprite_t *sp, int (*cb)(sprite_t *, agsevent_t *));
void spev_remove_eventlistener(sprite_t *sp);

// in sprite_tevent.c
void spev_add_teventlistener(sprite_t *sp, int (*cb)(sprite_t *, agsevent_t *));
void spev_remove_teventlistener(sprite_t *sp);
void spev_main();

// in sprite_move.c
void spev_move_setup(void* data, void* userdata);
void spev_move_waitend(sprite_t *sp, int dx, int dy, int time);
void spev_wait4moving_sp();

// in sprite_switch.c
void sp_sw_setup(sprite_t *sp);

// in sprite_get.c
void sp_get_setup(sprite_t *sp);

// in sprite_put.c
void sp_put_setup(sprite_t *sp);

// in sprite_anime.c
void sp_anime_setup(sprite_t *sp);

// in sprite_eupdate.c
void sp_eupdate(int type, int time, int key);

// in sprite_quake.c
void sp_quake_sprite(int wType, int wAmplitudeX, int wAmplitude, int wCount, int cancel);

// in sprite_keywait.c
void sp_keywait(int *vOK, int *vRND, int *vRsv1, int *vRsv2, int *vRsv3, int timeout);

// in screen_quake.c
void sp_quake_screen(int type, int p1, int p2, int time, int cancel);

// in sprite_xmenu.c
void spxm_clear(void);
void spxm_register(int reginum, int menuid);
int spxm_getnum(int reginum);
void spxm_gettext(int regnum, int strno);
void spxm_titlereg(void);
void spxm_titleget(int strno);

#endif /* __SPRITE_H__ */
