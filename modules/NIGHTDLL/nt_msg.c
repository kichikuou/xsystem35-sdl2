#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <SDL.h>

#include "portab.h"
#include "system.h"
#include "utfsjis.h"
#include "ags.h"
#include "nact.h"
#include "input.h"
#include "drawtext.h"
#include "night.h"
#include "nt_msg.h"
#include "sprite.h"
#include "sactcg.h"
#include "sactstring.h"


extern int ntsel_dosel(void);

#define SNAME_RYO "亮　　　　"
#define SNAME_RYO_DEF "亮"

// Normal message window
static const SDL_Rect msgframe_0 = {
	.x = 6,
	.y = 347,
	.w = 628,
	.h = 125
};

// Full screen message window
static const SDL_Rect msgframe_1 = {
	.x = 0,
	.y = 0,
	.w = 640,
	.h = 480
};

// Normal message (with face) window
static const SDL_Rect msgframe_2 = {
	.x = 6 + 160,
	.y = 347,
	.w = 628 - 160,
	.h = 125
};

// キー入力アニメーション位置
#define HAKANIM_LOC_X 620
#define HAKANIM_LOC_Y 450
// キー入力アニメーション間隔
#define HAKANIM_INTERVAL 250

// デフォルトメッセージフォントサイズ
#define MSG_DEFFONTSIZE 24


static void ntmsg_out(int wNum, int wSize, int wColorR, int wColorG, int wColorB, int wFont, int wSpeed, int wLineSpace);
static void set_align(char *msg, sprite_t *sp, int wSize);
static uint8_t *get_char(uint8_t *msg, char *mbuf, int bufmax);
static void cb_keyrelease(agsevent_t *e);
static void cb_mousemove(agsevent_t *e);
static void setup_hakanim();
static void hakanim(int i);
static int ntmsg_keywait();
static void ntmsg_clear(int wNum);


// 初期化
void ntmsg_init() {
	sprite_t *sp;

	// メッセージスプライト
	sp = night.sp[SPNO_MSGFRAME_FG] = nt_sp_msg_new(SPNO_MSGFRAME_FG, 0, 0, main_surface->w, main_surface->h);
	nt_sp_add_updatelist(sp);
	
	// メッセージ背景用CG
	sp = night.sp[SPNO_MSGFRAME_BG] = nt_sp_new(SPNO_MSGFRAME_BG, CGNO_MSGFRAME_LCG, 0, 0, SPRITE_NORMAL);
	nt_sp_add_updatelist(sp);
	
	// 文字背景CG作成
	nt_scg_create(CGNO_MSGFR_BG,
		   main_surface->w, main_surface->h,
		   0, 0, 0, 255);
	// night.cg[CGNO_MSGFR_BG]->refcnt++;
	
	// 文字背景CGのためのsprite
	sp = night.sp[SPNO_MSGBG] = nt_sp_new(SPNO_MSGBG, CGNO_MSGFR_BG, 0, 0, SPRITE_NORMAL);
	nt_sp_add_updatelist(sp);
	
	// キー入力アニメーションCG作成
	nt_scg_cut(CGNO_MSGHAK_1, CGNO_MSGHAK_LCG, 0, 0, 9, 9);
	//night.cg[CGNO_MSGHAK_1]->refcnt++;
	nt_scg_cut(CGNO_MSGHAK_2, CGNO_MSGHAK_LCG, 0, 9, 9, 9);
	//night.cg[CGNO_MSGHAK_2]->refcnt++;
	sp = night.sp[SPNO_MSG_KEYANIM] = nt_sp_new(SPNO_MSG_KEYANIM, CGNO_MSGHAK_1, CGNO_MSGHAK_2, 0, SPRITE_ANIME);
	sp->u.anime.interval = HAKANIM_INTERVAL;
	nt_sp_add_updatelist(sp);
	nt_sp_set_loc(sp, HAKANIM_LOC_X, HAKANIM_LOC_Y);
	nt_sp_set_show(sp, false);
	sp->update = nt_sp_draw_scg;

	

	


	// 主人公名前
	nt_sstr_regist_replace(fromUTF8(SNAME_RYO), fromUTF8(SNAME_RYO_DEF));
	
	night.msg.cbmove = cb_mousemove;
	night.msg.cbrelease = cb_keyrelease;
	
	ntmsg_set_frame(0);
	ntmsg_set_place(0);
	
	night.selmode = -1;
	
}

// メッセージ枠の種類
//   type=0 消去
//   type=1 枠あり
//   type=2 中央
void ntmsg_set_frame(int type) {
	night.msgframe = type;

	SDL_Surface *sf = night.sp[SPNO_MSGBG]->curcg->sf;
	SDL_FillRect(sf, NULL, 0);
	
	switch(type) {
	case 0:
		nt_sp_set_show(night.sp[SPNO_MSGBG], false);
		nt_sp_set_show(night.sp[SPNO_MSGFRAME_BG], false);
		nt_sp_set_show(night.sp[SPNO_MSGFRAME_FG], false);
		break;
	case 1:
		nt_sp_set_show(night.sp[SPNO_MSGBG], true);
		nt_sp_set_show(night.sp[SPNO_MSGFRAME_BG], true);
		nt_sp_set_show(night.sp[SPNO_MSGFRAME_FG], true);
		
		SDL_FillRect(sf, &msgframe_0, SDL_MapRGBA(sf->format, 16, 32, 64, 192));
		ntmsg_clear(SPNO_MSGFRAME_FG);
		nt_sp_update_all(true);
		break;
	case 2:
		nt_sp_set_show(night.sp[SPNO_MSGBG], true);
		nt_sp_set_show(night.sp[SPNO_MSGFRAME_BG], false);
		nt_sp_set_show(night.sp[SPNO_MSGFRAME_FG], true);
		
		SDL_FillRect(sf, &msgframe_1, SDL_MapRGBA(sf->format, 32, 32, 32, 128));
		ntmsg_clear(SPNO_MSGFRAME_FG);
		nt_sp_update_all(true);
	
		break;
	}
}

/*
  メッセージ領域の設定
  type=0: 628x125+6+347
  type=1: 640x480+0+0
  type=2: (628-120)x125+(6+120)+347
  
*/
void ntmsg_set_place(int type) {
	night.msgplace = type;
}

void ntmsg_newline() {
	uint8_t buf[2] = {'\n', '\0'};
	ntmsg_add(buf);
}

void ntmsg_add(const char *msg) {
	int len;
	
	SACT_DEBUG("len = %d", strlen(msg));
	
	if (msg[0] == '\0') return;
	
	len = MSGBUFMAX - (int)strlen(night.msgbuf);
	if (len < 0) {
		WARNING("buf shortage (%d)", len);
		return;
	}
	strncat(night.msgbuf, msg, len);
	night.msgbuf[MSGBUFMAX -1] = '\0';
}

int ntmsg_ana(void) {
	int ret;
	
	if (night.selmode == 0) {
		// sel start
		ret = ntsel_dosel();
	} else {
		// msg newpage
		ntmsg_out(SPNO_MSGFRAME_FG, night.fontsize, 255, 255, 255, night.fonttype, 0, 2);
		nt_sp_update_clipped();
		ret = ntmsg_keywait();
		
		// clear msgsprite
		ntmsg_clear(SPNO_MSGFRAME_FG);
		//nt_sp_update_clipped();
	}
	
	night.selmode = -1;
	return ret;
}

static void ntmsg_out(int wNum, int wSize, int wColorR, int wColorG, int wColorB, int wFont, int wSpeed, int wLineSpace) {
	char *msg;
	sprite_t *sp;
	bool needupdate = false;
	SDL_Rect uparea = {0,0,0,0};
	int savex;
	
	if (night.msgbuf[0] == '\0') return;
	
	// shortcut
	sp = night.sp[wNum];
	
	msg = nt_sstr_replacestr(night.msgbuf);
	
	// 文字アラインメントの調整
	set_align(msg, sp, wSize);
	savex = sp->u.msg.dspcur.x;
	
	
	while (*msg) {
		char mbuf[20];
		int cw, delta, wcnt;
		
		wcnt = sys_get_ticks();
		
		mbuf[0] = '\0';
		msg = get_char(msg, mbuf, sizeof(mbuf) -1); 
		
		if (mbuf[0] == '\n') {
			sp->u.msg.dspcur.x = savex;
			sp->u.msg.dspcur.y += (wSize + wLineSpace);
			continue;
		}
		
		dt_setfont(wFont, wSize);

		cw = dt_drawtext_col(sp->u.msg.canvas,
				     sp->u.msg.dspcur.x,
				     sp->u.msg.dspcur.y,
				     mbuf,
				     wColorR, wColorG, wColorB);
		
		needupdate = true;
		
		if (wSpeed > 0) {
			nt_sp_updateme_part(sp,
					 sp->u.msg.dspcur.x,
					 sp->u.msg.dspcur.y,
					 cw,
					 wSize);
			nt_sp_update_clipped();
			needupdate = false;
			
			// keywait
			delta = sys_get_ticks() - wcnt;
			if (delta < wSpeed) {
				if (sys_keywait(wSpeed - delta, KEYWAIT_NONCANCELABLE)) {
					
					wSpeed = 0;
				}
			}
		}
		
		sp->u.msg.dspcur.x += cw;
	}
	
	// バッファリング中の文字のクリア
	night.msgbuf[0] = '\0';
	
	// Waitなしの出力は最後にupdate
	if (needupdate) {
		uparea.w = sp->width;
		uparea.h = min(sp->height, uparea.y - sp->u.msg.dspcur.y + wLineSpace + wLineSpace);
		nt_sp_updateme_part(sp, uparea.x, uparea.y, uparea.w, uparea.h);
	}
	
}

/*
  メッセージキー入力待ち
  @param: none
  @return: 入力されたキー 
 */
static int ntmsg_keywait() {
	int i = 0;
	
	if (night.waitskiplv > 0) {
		sys_getInputInfo();
		return 0;
	}
	
	// アニメパターンの初期化
	setup_hakanim();
	
	night.waittype = KEYWAIT_MESSAGE;
	night.waitkey = -1;
	
	while (night.waitkey == -1 && !nact->is_quit) {
		int st = sys_get_ticks();
		int interval = 25;
		
		if (!night.zhiding) {
			interval = night.sp[SPNO_MSG_KEYANIM]->u.anime.interval;
			hakanim(i++);
		} 
		sys_keywait(interval - (sys_get_ticks() - st), KEYWAIT_NONCANCELABLE);
	}
	
	night.waittype = KEYWAIT_NONE;
	
	return night.waitkey;
}


static void set_align(char *msg, sprite_t *sp, int wSize) {
	int line, maxchar, c;
	
	switch(night.msgplace) {
	case 0:
		sp->u.msg.dspcur.x = msgframe_0.x;
		sp->u.msg.dspcur.y = msgframe_0.y + 8;
		break;
		
	case 1:
                // もっとも長い行が中心にくるようにし、
		// 他の行はその行の先頭に先頭をあわせる
		line = 0; maxchar = 0;
		
		c = 0;
		while (*msg) {
			if (*msg == '\n') {
				line++; 
				maxchar = max(maxchar, c);
				c = 0;
				msg++;
				continue;
			}
			msg++; c++;
		}
		
		maxchar = max(maxchar, c);
		line++;
		sp->u.msg.dspcur.x = (sp->width - (maxchar * wSize/2)) /2;
		sp->u.msg.dspcur.y = (sp->height - (line * (wSize+2))) /2;
		break;
	case 2:
		sp->u.msg.dspcur.x = msgframe_2.x;
		sp->u.msg.dspcur.y = msgframe_2.y + 8;
		break;
	}
	
}

static uint8_t *get_char(uint8_t *msg, char *mbuf, int bufmax) {
	if (msg[0] == '\n') {
		mbuf[0] = '\n';
		mbuf[1] = '\0';
		return msg +1;
	}
	
	uint8_t *p = advance_char(msg, nact->encoding);
	while (msg < p)
		*mbuf++ = *msg++;
	*mbuf = '\0';
	
	return msg;
}

static void is_in_icon() {
	
}


static void cb_keyrelease(agsevent_t *e) {
	switch (e->code) {
	case AGSEVENT_BUTTON_LEFT:
#if 0
		if (is_in_icon()) {
			do_icon(e->mousex, e->mousey);
			break;
		}
#endif
		// fall through
	case AGSEVENT_BUTTON_RIGHT:
		if (night.zhiding) {
			// unhide();
			break;
		}
		night.waitkey = e->code;
		break;
	}
	
}

static void cb_mousemove(agsevent_t *e) {
	// 音声mute/メッセージスキップ/メッセージ枠消去の領域に
	// マウスが移動したら、その部分のアイコンを変化させる
}

// メッセージ表示時に、キー入力を促すアニメーションの設定
static void setup_hakanim() {
}

// メッセージ表示時のキー入力を促すアニメーション
static void hakanim(int i) {
	sprite_t *sp;
	bool show;
	
	sp = night.sp[SPNO_MSG_KEYANIM];
	if (sp == NULL) return;
	show = sp->show;
	if (i%2) {
		sp->curcg = sp->cg1;
	} else {
		sp->curcg = sp->cg2;
	}
	sp->show = true;
	nt_sp_updateme(sp);
	nt_sp_update_clipped();
	
	sp->show = show;
}

void ntmsg_update(sprite_t *sp, SDL_Rect *r) {
	SDL_Rect sp_rect = {0, 0, sp->width, sp->height};
	int sx = 0;
	int sy = 0;
	int dx = sp->cur.x;
	int dy = sp->cur.y;
	int w = sp->width;
	int h = sp->height;
	if (!ags_clipCopyRect(&sp_rect, r, &sx, &sy, &dx, &dy, &w, &h)) {
		return;
	}

	SDL_SetSurfaceBlendMode(sp->u.msg.canvas, SDL_BLENDMODE_BLEND);
	SDL_SetSurfaceAlphaMod(sp->u.msg.canvas, sp->blendrate);
	SDL_BlitSurface(sp->u.msg.canvas, &(SDL_Rect){sx, sy, w, h}, main_surface, &(SDL_Rect){dx, dy, w, h});

	SACT_DEBUG("do update no=%d, sx=%d, sy=%d, w=%d, h=%d, dx=%d, dy=%d",
		sp->no, sx, sy, w, h, dx, dy);
}

static void ntmsg_clear(int wNum) {
	sprite_t *sp = night.sp[wNum];

	sp->u.msg.dspcur.x = 0;
	sp->u.msg.dspcur.y = 0;
	
	night.msgbuf[0]  = '\0';

	// Clear the canvas
	SDL_FillRect(sp->u.msg.canvas, NULL, 0);

	nt_sp_updateme(sp);
}
