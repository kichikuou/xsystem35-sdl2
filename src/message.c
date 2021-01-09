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
static int frameCgNoTop;
static int frameCgNoMid;
static int frameCgNoBot;
static int frameDot;
/* 文字飾りの設定 */
static int msgDecorateColor   = 0;
static int msgDecorateType    = 0;
/* 現在の文字表示位置 */
static MyPoint msgcur;
static boolean nextLineIsAfterKaigyou = FALSE;

/* Private Methods */
static void drawLineFrame(Bcom_WindowInfo *info);
static void copyMsgToStrVar(const char *m);
static void msgget_at_r();
static void msgget_at_a();


void msg_init() {
	/* メッセージフォントの大きさ */
	msg.MsgFontSize     = 16;
	msg.MsgFontBoldSize = 0;
	msg.MsgFont         = FONT_GOTHIC;
	
	/* 各種色 */
	msg.MsgFontColor             = 255;
	msg.WinFrameColor            = 255;
	msg.WinBackgroundColor       = 0;
	msg.HitAnyKeyMsgColor        = 255;
	msg.WinBackgroundTransparent = 255;
	
	msg.AutoPageChange = TRUE;  /* 自動改ページ */
	msg.LineIncrement  = 2;     /* 改行幅 */
	msg.WinBackgroundTransparentColor = -1; /* Window背景の透過色 -1:指定無し*/
	
	/* MG コマンドによるメッセージの文字列変数への取り込み */
	msg.mg_getString     = FALSE;
	msg.mg_dspMsg        = TRUE;
	msg.mg_startStrVarNo = 1;
	msg.mg_curStrVarNo   = 1;
	msg.mg_policyR       = 0;
	msg.mg_policyA       = 0;

	msg.win = &msg.wininfo[0];
	msg.wininfo[0].x = 8;
	msg.wininfo[0].y = 311;
	msg.wininfo[0].width = 616;
	msg.wininfo[0].height = 80;
	msg.wininfo[0].save = TRUE;
}

void msg_setFontSize(int size) {
	msg.MsgFontSize = size;
}

void msg_setStringDecorationColor(int col) {
	msgDecorateColor = col;
}

void msg_setStringDecorationType(int type) {
	msgDecorateType = type;
}

void msg_putMessage(const char *m) {
	MyRectangle adj;
	
	if (nextLineIsAfterKaigyou) {
		sys_hit_any_key();
		msg_nextPage(TRUE);
	}
	
	texthook_message(m);

	/* 表示文字列を文字列変数にコピーする */
	if (msg.mg_getString) {
		copyMsgToStrVar(m);
	}
	
	// fprintf(stdout, "x=%d, y = %d, msg=%s\n", msgcur.x,msgcur.y,msg);
	if (!msg.mg_dspMsg) return;
	
	ags_setFont(msg.MsgFont, msg.MsgFontSize);
	switch(msgDecorateType) {
	case 0:
	default:
		adj.x = 0; adj.y = 0; adj.w = 0; adj.h = 0;
		break;
	case 1:
		ags_drawString(msgcur.x, msgcur.y +1, m, msgDecorateColor);
		adj.x = 0; adj.y = 0; adj.w = 0; adj.h = 1;
		break;
	case 2:
		ags_drawString(msgcur.x +1, msgcur.y, m, msgDecorateColor);
		adj.x = 0; adj.y = 0; adj.w = 1; adj.h = 0;
		break;
	case 3:
		ags_drawString(msgcur.x +1, msgcur.y +1, m, msgDecorateColor);
		adj.x = 0; adj.y = 0; adj.w = 1; adj.h = 1;
		break;
	case 4:
		ags_drawString(msgcur.x -1, msgcur.y, m, msgDecorateColor);
		ags_drawString(msgcur.x +1, msgcur.y, m, msgDecorateColor);
		ags_drawString(msgcur.x, msgcur.y -1, m, msgDecorateColor);
		ags_drawString(msgcur.x, msgcur.y +1, m, msgDecorateColor);
		adj.x = -1; adj.y = -1; adj.w = 2; adj.h = 2;
		break;
	case 6:
		ags_drawString(msgcur.x +1, msgcur.y, m, msg.MsgFontColor);
		adj.x = 0; adj.y = 0; adj.w = 1; adj.h = 0;
		break;
	case 7:
		ags_drawString(msgcur.x, msgcur.y +1, m, msg.MsgFontColor);
		adj.x = 0; adj.y = 0; adj.w = 0; adj.h = 1;
		break;
	case 8:
		ags_drawString(msgcur.x +1, msgcur.y +1, m, msg.MsgFontColor);
		adj.x = 0; adj.y = 0; adj.w = 1; adj.h = 1;
		break;
	case 10:
		ags_drawString(msgcur.x -1, msgcur.y   , m, msgDecorateColor);
		ags_drawString(msgcur.x +1, msgcur.y   , m, msgDecorateColor);
		ags_drawString(msgcur.x   , msgcur.y -1, m, msgDecorateColor);
		ags_drawString(msgcur.x   , msgcur.y +1, m, msgDecorateColor);
		ags_drawString(msgcur.x +2, msgcur.y +2, m, msgDecorateColor);
		adj.x = -1; adj.y = -1; adj.w = 3; adj.h = 3;
		break;
	}
	
	MyRectangle drawn = ags_drawString(msgcur.x, msgcur.y, m, msg.MsgFontColor);
	msgcur.x += drawn.w;
	drawn.x += adj.x;
	drawn.y += adj.y;
	drawn.w += adj.w;
	drawn.h += adj.h;

	if (nact->messagewait_enable && !nact->messagewait_cancelled && !msgskip_isSkipping()) {
		int x;
		for (x = 0; x < drawn.w; x+=16) {
			ags_updateArea(drawn.x + x, drawn.y, 16, drawn.h);
			if (nact->messagewait_cancel) {
				if (sys_getInputInfo()) {
					nact->messagewait_cancelled = TRUE;
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
		nextLineIsAfterKaigyou = TRUE;
	}
}

void msg_nextPage(boolean innerclear) {
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
	
	nextLineIsAfterKaigyou = FALSE;
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
		frameCgNoTop = C1;
		frameCgNoMid = C2;
		frameCgNoBot = N;
		frameDot     = M;
		break;
	default:
		break;
	}
	if (M == 0) msg_nextPage(N == 0 ? TRUE : FALSE);
}

void msg_setMessageLocation(int x, int y) {
	texthook_newline();
	msgcur.x = x;
	msgcur.y = y;
	nextLineIsAfterKaigyou = FALSE;
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
	
	MyRectangle r = ags_drawString(msg.win->x + msg.win->width - msg.MsgFontSize,
								   msg.win->y + msg.win->height - msg.MsgFontSize,
								   prompt[nact->encoding], msg.HitAnyKeyMsgColor);
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
