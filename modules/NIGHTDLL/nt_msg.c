#include "config.h"

#include <stdio.h>
#include <string.h>
#include <glib.h>

#include "portab.h"
#include "system.h"
#include "counter.h"
#include "ags.h"
#include "nact.h"
#include "imput.h"
#include "ngraph.h"
#include "drawtext.h"
#include "night.h"
#include "nt_msg.h"
#include "sprite.h"
#include "sactcg.h"
#include "sjisname.h"
#include "sactstring.h"


extern int ntsel_dosel(void);

// 通常メッセージ表示位置と大きさ
#define MSGFRAME_0_X 6
#define MSGFRAME_0_Y 347
#define MSGFRAME_0_WIDTH 628
#define MSGFRAME_0_HEIGHT 125

// 画面全体メッセージ表示位置と大きさ
#define MSGFRAME_1_X 0
#define MSGFRAME_1_Y 0
#define MSGFRAME_1_WIDTH 640
#define MSGFRAME_1_HEIGHT 480

// 通常メッセージ(顔つき)表示位置と大きさ
#define MSGFRAME_2_X (6+160)
#define MSGFRAME_2_Y 347
#define MSGFRAME_2_WIDTH (628-160)
#define MSGFRAME_2_HEIGHT 125

// キー入力アニメーション位置
#define HAKANIM_LOC_X 620
#define HAKANIM_LOC_Y 450
// キー入力アニメーション間隔
#define HAKANIM_INTERVAL 250

// デフォルトメッセージフォントサイズ
#define MSG_DEFFONTSIZE 24


static void ntmsg_out(int wNum, int wSize, int wColorR, int wColorG, int wColorB, int wFont, int wSpeed, int wLineSpace);
static void set_align(char *msg, sprite_t *sp, int wSize);
static BYTE *get_char(BYTE *msg, char *mbuf, int bufmax);
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
	sp = night.sp[SPNO_MSGFRAME_FG] = sp_msg_new(SPNO_MSGFRAME_FG, 0, 0, sf0->width, sf0->height);
	sp_add_updatelist(sp);
	
	// メッセージ背景用CG
	sp = night.sp[SPNO_MSGFRAME_BG] = sp_new(SPNO_MSGFRAME_BG, CGNO_MSGFRAME_LCG, 0, 0, SPRITE_NORMAL);
	sp_add_updatelist(sp);
	
	// 文字背景CG作成
	scg_create(CGNO_MSGFR_BG,
		   sf0->width, sf0->height,
		   0, 0, 0, 255);
	// night.cg[CGNO_MSGFR_BG]->refcnt++;
	
	// 文字背景CGのためのsprite
	sp = night.sp[SPNO_MSGBG] = sp_new(SPNO_MSGBG, CGNO_MSGFR_BG, 0, 0, SPRITE_NORMAL);
	sp_add_updatelist(sp);
	
	// キー入力アニメーションCG作成
	scg_cut(CGNO_MSGHAK_1, CGNO_MSGHAK_LCG, 0, 0, 9, 9);
	//night.cg[CGNO_MSGHAK_1]->refcnt++;
	scg_cut(CGNO_MSGHAK_2, CGNO_MSGHAK_LCG, 0, 9, 9, 9);
	//night.cg[CGNO_MSGHAK_2]->refcnt++;
	sp = night.sp[SPNO_MSG_KEYANIM] = sp_new(SPNO_MSG_KEYANIM, CGNO_MSGHAK_1, CGNO_MSGHAK_2, 0, SPRITE_ANIME);
	sp->u.anime.interval = HAKANIM_INTERVAL;
	sp_add_updatelist(sp);
	sp_set_loc(sp, HAKANIM_LOC_X, HAKANIM_LOC_Y);
	sp_set_show(sp, FALSE);
	sp->update = sp_draw_scg;

	

	


	// 主人公名前
	sstr_regist_replace(SNAME_RYO, SNAME_RYO_DEF);
	
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
	surface_t *sf;
	
	night.msgframe = type;

	sf = night.sp[SPNO_MSGBG]->curcg->sf;
	memset(sf->pixel, 0, sf->bytes_per_line * sf->height);
	memset(sf->alpha, 0, sf->width * sf->height);
	
	switch(type) {
	case 0:
		sp_set_show(night.sp[SPNO_MSGBG], FALSE);
		sp_set_show(night.sp[SPNO_MSGFRAME_BG], FALSE);
		sp_set_show(night.sp[SPNO_MSGFRAME_FG], FALSE);
		break;
	case 1:
		sp_set_show(night.sp[SPNO_MSGBG], TRUE);
		sp_set_show(night.sp[SPNO_MSGFRAME_BG], TRUE);
		sp_set_show(night.sp[SPNO_MSGFRAME_FG], TRUE);
		
		gr_fill(sf, MSGFRAME_0_X, MSGFRAME_0_Y,
			MSGFRAME_0_WIDTH, MSGFRAME_0_HEIGHT,
			16, 32, 64);
		gr_fill_alpha_map(sf, MSGFRAME_0_X, MSGFRAME_0_Y,
				  MSGFRAME_0_WIDTH, MSGFRAME_0_HEIGHT, 192);
		ntmsg_clear(SPNO_MSGFRAME_FG);
		sp_update_all(TRUE);
		break;
	case 2:
		sp_set_show(night.sp[SPNO_MSGBG], TRUE);
		sp_set_show(night.sp[SPNO_MSGFRAME_BG], FALSE);
		sp_set_show(night.sp[SPNO_MSGFRAME_FG], TRUE);
		
		gr_fill(sf, MSGFRAME_1_X, MSGFRAME_1_Y,
			MSGFRAME_1_WIDTH, MSGFRAME_1_HEIGHT,
			32, 32, 32);
		gr_fill_alpha_map(sf, MSGFRAME_1_X, MSGFRAME_1_Y,
				  MSGFRAME_1_WIDTH, MSGFRAME_1_HEIGHT, 128);
		ntmsg_clear(SPNO_MSGFRAME_FG);
		sp_update_all(TRUE);
	
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
	BYTE buf[2] = {'\n', '\0'};
	ntmsg_add(buf);
}

void ntmsg_add(char *msg) {
	int len;
	
	WARNING("len = %d\n", strlen(msg));
	
	if (msg[0] == '\0') return;
	
	if (0) {
		char *b = sjis2euc(msg);
		fprintf(stderr, "add msg '%s'\n", b);
		free(b);
	}

	len = MSGBUFMAX - (int)strlen(night.msgbuf);
	if (len < 0) {
		WARNING("buf shortage (%d)\n", len);
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
		sp_update_clipped();
		ret = ntmsg_keywait();
		
		// clear msgsprite
		ntmsg_clear(SPNO_MSGFRAME_FG);
		//sp_update_clipped();
	}
	
	night.selmode = -1;
	return ret;
}

static void ntmsg_out(int wNum, int wSize, int wColorR, int wColorG, int wColorB, int wFont, int wSpeed, int wLineSpace) {
	char *msg;
	sprite_t *sp;
	boolean needupdate = FALSE;
	MyRectangle uparea = {0,0,0,0};
	int savex;
	
	if (night.msgbuf[0] == '\0') return;
	
	// shortcut
	sp = night.sp[wNum];
	
	msg = sstr_replacestr(night.msgbuf);
	
	// 文字アラインメントの調整
	set_align(msg, sp, wSize);
	savex = sp->u.msg.dspcur.x;
	
	
	while (*msg) {
		char mbuf[20];
		int cw, delta, wcnt;
		
		wcnt = get_high_counter(SYSTEMCOUNTER_MSEC);
		
		mbuf[0] = '\0';
		msg = get_char(msg, mbuf, sizeof(mbuf) -1); 
		
		if (mbuf[0] == '\n') {
			sp->u.msg.dspcur.x = savex;
			sp->u.msg.dspcur.y += (wSize + wLineSpace);
			continue;
		}
		
		dt_setfont(wFont, wSize);

		if (1) {
			char *b = sjis2euc(mbuf);
			fprintf(stderr, "msg '%s'\n", b);
			free(b);
		}
		
		cw = dt_drawtext_col(sp->u.msg.canvas,
				     sp->u.msg.dspcur.x,
				     sp->u.msg.dspcur.y,
				     mbuf,
				     wColorR, wColorG, wColorB);
		
		needupdate = TRUE;
		
		if (wSpeed > 0) {
			sp_updateme_part(sp,
					 sp->u.msg.dspcur.x,
					 sp->u.msg.dspcur.y,
					 cw,
					 wSize);
			sp_update_clipped();
			needupdate = FALSE;
			
			// keywait
			delta = get_high_counter(SYSTEMCOUNTER_MSEC) - wcnt;
			if (delta < wSpeed) {
				if (sys_keywait(wSpeed - delta, FALSE)) {
					
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
		uparea.width  = sp->cursize.width;
		uparea.height = min(sp->cursize.height, uparea.y - sp->u.msg.dspcur.y + wLineSpace + wLineSpace);
		sp_updateme_part(sp, uparea.x, uparea.y, uparea.width, uparea.height);
	}
	
}

/*
  メッセージキー入力待ち
  @param: none
  @return: 入力されたキー 
 */
static int ntmsg_keywait() {
	int i = 0;
	
	if (night.waitskiplv > 0) return 0;
	
	// アニメパターンの初期化
	setup_hakanim();
	
	night.waittype = KEYWAIT_MESSAGE;
	night.waitkey = -1;
	
	while (night.waitkey == -1) {
		int st = get_high_counter(SYSTEMCOUNTER_MSEC);
		int interval = 25;
		
		if (!night.zhiding) {
			interval = night.sp[SPNO_MSG_KEYANIM]->u.anime.interval;
			hakanim(i++);
		} 
		sys_keywait(interval - (get_high_counter(SYSTEMCOUNTER_MSEC) - st), FALSE);
	}
	
	night.waittype = KEYWAIT_NONE;
	
	return night.waitkey;
}


static void set_align(char *msg, sprite_t *sp, int wSize) {
	int line, maxchar, c;
	
	switch(night.msgplace) {
	case 0:
		sp->u.msg.dspcur.x = MSGFRAME_0_X;
		sp->u.msg.dspcur.y = MSGFRAME_0_Y + 8;
		break;
		
	case 1:
                // もっとも長い行が中心にくるようにし、
		// 他の行はその行の先頭に先頭をあわせる
		line = 0; maxchar = 0;
		
		c = 0;
		while (*msg) {
			if (*msg == '\n') {
				line++; 
				maxchar = MAX(maxchar, c);
				c = 0;
				msg++;
				continue;
			}
			msg++; c++;
		}
		
		maxchar = MAX(maxchar, c);
		line++;
		sp->u.msg.dspcur.x = (sp->cursize.width - (maxchar * wSize/2)) /2;
		sp->u.msg.dspcur.y = (sp->cursize.height - (line * (wSize+2))) /2;
		break;
	case 2:
		sp->u.msg.dspcur.x = MSGFRAME_2_X;
		sp->u.msg.dspcur.y = MSGFRAME_2_Y + 8;
		break;
	}
	
}

static BYTE *get_char(BYTE *msg, char *mbuf, int bufmax) {
	int c1;
	
	//  改行
	if (msg[0] == '\n') {
		mbuf[0] = '\n';
		mbuf[1] = '\0';
		return msg +1;
	}
	
	c1 = *msg++;
	
	*mbuf++ = c1;
	
	if ((c1 >= 0x81 && c1 < 0xa0) || (c1 >= 0xe0 && c1 <= 0xee)) {
		*mbuf++ = *msg++;
	}
	*mbuf = '\0';
	
	return msg;
}

static void is_in_icon() {
	
}


static void cb_keyrelease(agsevent_t *e) {
	int x = e->d1, y = e->d2;
	
	switch (e->d3) {
	case AGSEVENT_BUTTON_LEFT:
#if 0
		if (is_in_icon()) {
			do_icon(x, y);
			break;
		}
#endif
		// fall through
	case AGSEVENT_BUTTON_RIGHT:
		if (night.zhiding) {
			// unhide();
			break;
		}
		night.waitkey = e->d3;
		break;
	}
	
}

static void cb_mousemove(agsevent_t *e) {
	int x = e->d1, y = e->d2;
	
	// 音声mute/メッセージスキップ/メッセージ枠消去の領域に
	// マウスが移動したら、その部分のアイコンを変化させる

	
}

// メッセージ表示時に、キー入力を促すアニメーションの設定
static void setup_hakanim() {
}

// メッセージ表示時のキー入力を促すアニメーション
static void hakanim(int i) {
	sprite_t *sp;
	boolean show;
	
	sp = night.sp[SPNO_MSG_KEYANIM];
	if (sp == NULL) return;
	show = sp->show;
	if (i%2) {
		sp->curcg = sp->cg1;
	} else {
		sp->curcg = sp->cg2;
	}
	sp->show = TRUE;
	sp_updateme(sp);
	sp_update_clipped();
	
	sp->show = show;
}

int ntmsg_update(sprite_t *sp, MyRectangle *r) {
	int sx, sy, w, h, dx, dy;
	surface_t update;
	
	// canvas が clean のときはなにもしない
	//  -> 説明スプライトのように、SetShowされたときに対応できないからだめ 
	//if (sact.msgbufempty) return OK;
	
	update.width  = r->width;
	update.height = r->height;
	
	dx = sp->cur.x - r->x;
	dy = sp->cur.y - r->y;
	
	w = sp->cursize.width;
	h = sp->cursize.height;
	
	sx = 0; sy = 0;
	
	if (!gr_clip(sp->u.msg.canvas, &sx, &sy, &w, &h, &update, &dx, &dy)) {
		return NG;
	}
	
	dx += r->x;
	dy += r->y;
	
	gre_BlendUseAMap(sf0, dx, dy, sf0, dx, dy, sp->u.msg.canvas, sx, sy, w, h, sp->u.msg.canvas, sx, sy, sp->blendrate);
	
	WARNING("do update no=%d, sx=%d, sy=%d, w=%d, h=%d, dx=%d, dy=%d\n",
		sp->no, sx, sy, w, h, dx, dy);
	
	return OK;
}

static void ntmsg_clear(int wNum) {
	sprite_t *sp = night.sp[wNum];
	surface_t *sf;
	
	sp->u.msg.dspcur.x = 0;
	sp->u.msg.dspcur.y = 0;
	
	night.msgbuf[0]  = '\0';
	
	// キャンバスのクリア
	sf = sp->u.msg.canvas;
	memset(sf->pixel, 0, sf->bytes_per_line * sf->height);
	memset(sf->alpha, 0, sf->width * sf->height);
	
	sp_updateme(sp);
}
