/*
 * message.c  文字列表示関係
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
 *
*/
/* $Id: message.c,v 1.29 2003/01/25 01:34:50 chikama Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "portab.h"
#include "windowframe.h"
#include "sdl_core.h"
#include "xsystem35.h"
#include "message.h"
#include "variable.h"
#include "input.h"
#include "msgskip.h"
#include "ags.h"
#include "nact.h"
#include "texthook.h"

/* ショートカット */
#define msg nact->msg

/* Private Variables */
/* 現在使用中のメッセージWindow情報 */
// static Bcom_WindowInfo winInfo = {8,311,616,80,0};
/* Window枠の種類 */
static int frameType;
static int frameDot;
/* 現在の文字表示位置 */
static MyPoint msgcur;
static bool nextLineIsAfterKaigyou = false;

/* Private Methods */
static void drawLineFrame(Bcom_WindowInfo *info);
static void copyMsgToStrVar(const char *m);
static void msgget_at_r();
static void msgget_at_a();


void msg_init() {
	/* メッセージフォントの大きさ */
	msg.MsgFontSize = 16;
	
	/* 各種色 */
	msg.MsgFontColor             = 255;
	msg.WinFrameColor            = 255;
	msg.WinBackgroundColor       = 0;
	msg.HitAnyKeyMsgColor        = 255;
	msg.WinBackgroundTransparent = 255;
	
	msg.AutoPageChange = true;  /* 自動改ページ */
	msg.LineIncrement  = 2;     /* 改行幅 */
	msg.WinBackgroundTransparentColor = -1; /* Window背景の透過色 -1:指定無し*/
	
	/* MG コマンドによるメッセージの文字列変数への取り込み */
	msg.mg_getString     = false;
	msg.mg_dspMsg        = true;
	msg.mg_startStrVarNo = 1;
	msg.mg_curStrVarNo   = 1;
	msg.mg_policyR       = 0;
	msg.mg_policyA       = 0;

	for (int i = 0; i < MSGWINMAX; i++) {
		if (msg.wininfo[i].savedimg)
			ags_delRegion(msg.wininfo[i].savedimg);
	}
	memset(msg.wininfo, 0, sizeof(msg.wininfo));

	for (int i = 0; i < MSGWINMAX; i++) {
		msg.wininfo[i].x = 100;
		msg.wininfo[i].y = 300;
		msg.wininfo[i].width = 400;
		msg.wininfo[i].height = 90;
	}
	msg.winno = 1;
	msg.win = &msg.wininfo[1];
	msg.wininfo[1].save = true;

	// Private variables
	msgcur.x = 0;
	msgcur.y = 0;
	nextLineIsAfterKaigyou = false;
}

void msg_setFontSize(int size) {
	msg.MsgFontSize = size;
}

void msg_putMessage(const char *m) {
	if (nextLineIsAfterKaigyou) {
		sys_hit_any_key();
		msg_nextPage(true);
	}
	
	texthook_message(m);

	/* 表示文字列を文字列変数にコピーする */
	if (msg.mg_getString) {
		copyMsgToStrVar(m);
	}
	
	// fprintf(stdout, "x=%d, y = %d, msg=%s\n", msgcur.x,msgcur.y,msg);
	if (!msg.mg_dspMsg) return;
	
	ags_setFontWithWeight(nact->ags.font_type, msg.MsgFontSize, nact->ags.font_weight);

	MyRectangle drawn;
	msgcur.x += ags_drawString(msgcur.x, msgcur.y, m, msg.MsgFontColor, &drawn);

	if (nact->messagewait_enable && !nact->messagewait_cancelled && !msgskip_isSkipping()) {
		int x;
		for (x = 0; x < drawn.w; x+=16) {
			ags_updateArea(drawn.x + x, drawn.y, 16, drawn.h);
			if (nact->messagewait_cancel) {
				if (sys_getInputInfo()) {
					nact->messagewait_cancelled = true;
					ags_updateArea(drawn.x, drawn.y, drawn.w, drawn.h);
					break;
				}
				sdl_sleep(nact->messagewait_time * 10);
			}
			nact->callback();
		}
	} else {
		ags_updateArea(drawn.x, drawn.y, drawn.w, drawn.h);
	}
}

void msg_nextLine() {
	texthook_newline();

	// puts("next Line");
	if (msg.mg_getString) {
		msgget_at_r();
		return;
	}
	
	msgcur.x  = msg.win->x;
	msgcur.y += (msg.MsgFontSize + msg.LineIncrement);
	
	if ((msgcur.y + msg.MsgFontSize) > (msg.win->y + msg.win->height)) {
		nextLineIsAfterKaigyou = true;
	}
}

void msg_nextPage(bool innerclear) {
	texthook_nextpage();

	// puts("next Page");
	if (innerclear) {
		if (msg.WinBackgroundTransparent == 255) {
			ags_fillRectangle(msg.win->x,     msg.win->y,
					  msg.win->width, msg.win->height,
					  msg.WinBackgroundColor);
		} else {
			if (msg.win->savedimg != NULL) {
				if (frameType != WINDOW_FRAME_LINE) {
					ags_putRegion(msg.win->savedimg, msg.win->x, msg.win->y);
				} else {
					ags_putRegion(msg.win->savedimg, msg.win->x-8, msg.win->y-8);
				}
			}	
			ags_wrapColor(msg.win->x,     msg.win->y,
				      msg.win->width, msg.win->height,
				      msg.WinBackgroundColor, msg.WinBackgroundTransparent);
		}
	}
	if (frameType == WINDOW_FRAME_LINE) {
		drawLineFrame(msg.win);
	} else if (innerclear) {
		ags_updateArea(msg.win->x, msg.win->y,
			       msg.win->width, msg.win->height);
	}
	
	msgcur.x = msg.win->x;
	msgcur.y = msg.win->y +  msg.LineIncrement;
	
	if (nextLineIsAfterKaigyou) {
		if (msg.mg_getString) msgget_at_a();
	}
	
	nextLineIsAfterKaigyou = false;
}

void msg_openWindow(int W, int C1, int C2, int N, int M) {
	// winInfo    = *info;
	frameType  = W;
	
	switch(W) {
	case WINDOW_FRAME_EMPTY:
		if (M == 0) {
			/* show window */
			if (msg.win->savedimg != NULL) {
				ags_delRegion(msg.win->savedimg);
			}
			if (msg.win->save) {
				msg.win->savedimg = ags_saveRegion(msg.win->x, msg.win->y, msg.win->width, msg.win->height);
			} else {
				msg.win->savedimg = NULL;
			}
		} else {
			/* restore window */
			if (msg.win->savedimg != NULL) {
				ags_putRegion(msg.win->savedimg, msg.win->x, msg.win->y);
				ags_updateArea(msg.win->x, msg.win->y, msg.win->width, msg.win->height);
			}
		}
		frameDot = 0;
		break;
	case WINDOW_FRAME_LINE:
		if (M == 0) {
			/* show window*/
			if (msg.win->savedimg != NULL) {
				ags_delRegion(msg.win->savedimg);
			}
			if (msg.win->save) {
				msg.win->savedimg = ags_saveRegion(msg.win->x -8, msg.win->y -8, msg.win->width +16, msg.win->height +16);
			} else {
				msg.win->savedimg = NULL;
			}
		} else {
			/* restore window */
			if (msg.win->savedimg != NULL) {
				ags_putRegion(msg.win->savedimg, msg.win->x -8, msg.win->y -8);
				ags_updateArea(msg.win->x -8, msg.win->y -8, msg.win->width +16, msg.win->height +16);
			}
		}
		frameDot = 8;
		break;
	case WINDOW_FRAME_CG:
		frameDot     = M;
		break;
	default:
		break;
	}
	if (M == 0) msg_nextPage(N == 0);
}

void msg_setMessageLocation(int x, int y) {
	texthook_newline();
	msgcur.x = x;
	msgcur.y = y;
	nextLineIsAfterKaigyou = false;
}

void msg_getMessageLocation(MyPoint *loc) {
	loc->x = msgcur.x;
	loc->y = msgcur.y;
}

void msg_hitAnyKey() {
	const char *prompt[CHARACTER_ENCODING_MAX + 1] = {
		[SHIFT_JIS] = "\x81\xa5",
		[UTF8] = "▼",
	};
	
	MyRectangle r;
	int x = msg.win->x + msg.win->width - msg.MsgFontSize;
	int y = msg.win->y + msg.win->height - msg.MsgFontSize;
	ags_drawString(x, y, prompt[nact->encoding], msg.HitAnyKeyMsgColor, &r);
	ags_updateArea(r.x, r.y, r.w, r.h);
}

static void drawLineFrame(Bcom_WindowInfo *i) {
	ags_drawRectangle(i->x -8, i->y -8, i->width +16, i->height +16, msg.WinFrameColor);
	ags_drawRectangle(i->x -7, i->y -7, i->width +14, i->height +14, msg.WinFrameColor);
	ags_drawRectangle(i->x -6, i->y -6, i->width +12, i->height +12, msg.WinFrameColor);
	ags_drawRectangle(i->x -5, i->y -5, i->width +10, i->height +10, msg.WinBackgroundColor);
	ags_drawRectangle(i->x -4, i->y -4, i->width  +8, i->height  +8, msg.WinBackgroundColor);
	ags_drawRectangle(i->x -3, i->y -3, i->width  +6, i->height  +6, msg.WinFrameColor);
	ags_drawRectangle(i->x -2, i->y -2, i->width  +4, i->height  +4, msg.WinBackgroundColor);
	ags_drawRectangle(i->x -1, i->y -1, i->width  +2, i->height  +2, msg.WinBackgroundColor);
	ags_updateArea(i->x -8, i->y -8, i->width +16, i->height +16);
}

static void copyMsgToStrVar(const char *m) {
	if (svar_length(msg.mg_curStrVarNo) == 0) {
		svar_set(msg.mg_curStrVarNo, m);
	} else {
		svar_append(msg.mg_curStrVarNo, m);
	}
}

static void msgget_at_r() {
	if (msg.mg_policyR == 1) return;
	msg.mg_curStrVarNo++;
	// svar_set(msg.mg_curStrVarNo, NULL);
}

static void msgget_at_a() {
	switch(msg.mg_policyA) {
	case 0:
		msg.mg_curStrVarNo = msg.mg_startStrVarNo;
		break;
	case 1:
		msg.mg_curStrVarNo++;
		svar_set(msg.mg_curStrVarNo, "");
		break;
	case 2:
		svar_set(msg.mg_curStrVarNo, "");
		msg.mg_curStrVarNo++;
		break;
	case 3:
		break;
	default:
		break;
	}
}

void msg_mg6_command(int sw) {
	switch(sw) {
	case 0:
		msg.mg_curStrVarNo--;
		break;
	case 1:
		msg.mg_curStrVarNo++;
		break;
	case 2:
		/* まだ */
		break;
	default:
		break;
	}
}
