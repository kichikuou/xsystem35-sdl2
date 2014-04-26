/*
 * selection.c  選択
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
/* $Id: selection.c,v 1.42 2003/04/22 16:34:28 chikama Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "portab.h"
#include "xsystem35.h"
#include "ags.h"
#include "scenario.h"
#include "windowframe.h"
#include "imput.h"
#include "message.h"
#include "selection.h"

/* ショートカット */
#define sel nact->sel

/* 選択ウィンドが開いた時のマウスの初期位置(最初の選択肢の囲み内) */
#define MOUSE_INIT_X_RATIO 5  /* from 0 to 100 */
#define MOUSE_INIT_Y_RATIO 30 /* from 0 to 100 */

/* 選択肢の最大数 */
#define ELEMENT_MAX 20
/* 選択肢の１つの最大長さ */
#define ELEMENT_LENGTH 101

/**************** Private Variables **********************/
/* 選択肢の要素 */
static char elm[ELEMENT_MAX][ELEMENT_LENGTH];
/* 選択したときに返す値 */
static int elmv[ELEMENT_MAX];
/* 現在登録中の選択肢の番号 */
static int regnum = 0;
/* 選択肢の要素の最大長さ */
static int maxElementLength = 0;
/* 選択肢ウィンド退避 */
static MyRectangle saveArea;
/* 選択ウィンドの退避用 */
static void *saveimg;
static void *saveimg2;
/* 選択時のワーク */
static MyRectangle *workR;
/* callback functions 初期化/選択/キャンセル時に呼ばれるシナリオ内関数*/
static int cb_select_page;
static int cb_select_address;
static int cb_cancel_page;
static int cb_cancel_address;
/* 選択肢 window を開いた時のマウスカーソルの動作 */
static int default_element = 1;
/* 最後に選んだ選択肢の要素番号 */
static int last_selected_element;
/* キーボードによる選択操作用 */
static int keymode = 0;


/**************** Private Instance Methods **********************/
static void drawLineFrame(int x, int y, int width, int height);
static int  whereElement(void);

/*
  各公開変数初期化
*/
void sel_init() {
	/* Window枠の種類 */
	sel.WindowFrameType = 0;
	sel.FrameCgNoTop = 0;
	sel.FrameCgNoMid = 0;
	sel.FrameCgNoBot = 0;
	sel.Framedot = 0;
	
	/* メッセージフォントの大きさ */
	sel.MsgFontSize = 16;
	
	/* 各種色 */
	sel.MsgFontColor             = 255;
	sel.WinFrameColor            = 255;
	sel.WinBackgroundColor       = 0;
	sel.WinElementEncloseColor   = 255;
	sel.SelectedElementColor     = 0;
	sel.WinBackgroundTransparent = 255;
	sel.EncloseType              = 0;
	
	/* 選択したあとメッセージ領域を初期化するか */
	sel.ClearMsgWindow  = TRUE;
	
	/* 選択Windowの大きさの変更 */
	sel.WinResizeWidth  = FALSE;
	sel.WinResizeHeight = TRUE;

#if 0
	sel.WinInfo.x = 464;
	sel.WinInfo.y = 80;
	sel.WinInfo.width  = 160;
	sel.WinInfo.height = 160;
	sel.WinInfo.save = TRUE;
	sel.savedImage = NULL;
#endif
	
	/* 選択肢を登録中 */
	sel.in_setting = FALSE;
}

/* 登録された選択肢の個数を削減する */
void sel_reduce(int no) {
	if (regnum > no) {
		regnum = no;
	}
}

/* 登録された選択肢の個数を取得する */
int sel_getnumberof() {
	return regnum;
}

/* 登録された選択肢文字列を取得する */
char *sel_gettext(int no) {
	return elm[no -1];
}

/* 登録された選択肢アドレスにジャンプする */
void sel_goto(int no, int flag) {
	sl_jmpNear(elmv[no -1]);
	
	if (flag) {
		sel.in_setting = FALSE;
		regnum = 0;
		maxElementLength = 0;
	}
}

/* 登録された選択肢アドレスに、関数リターン後ジャンプする */
void sel_returengoto(int no, int flag) {
	// 積んであるのが far call の時は、sl_retFarでもどり、farJumpする
	// sl_retNear();
	// sl_jmpNear(elmv[no -1]);
	sl_returnGoto(elmv[no -1]);
	
	if (flag) {
		sel.in_setting = FALSE;
		regnum = 0;
		maxElementLength = 0;
	}
}

/* 選択肢デフォルト番号指定 */
void sel_setDefaultElement(int type) {
	default_element = type;
}

/* 選択時コールバック指定 */
void sel_setCallback(int type, int page, int adr) {
	switch(type) {
	case 1:
		cb_select_page    = page;
		cb_select_address = adr;
		break;
	case 2:
		cb_cancel_page    = page;
		cb_cancel_address = adr;
		break;
	}
}

/* 最後に選択された選択番号を取得 */
int sel_getLastElement() {
	return last_selected_element;
}

/* 選択要素数を返す */
int sel_getRegistoredElementNumber() {
	return regnum;
}

/* 選択肢最大文字幅取得 */
int sel_getRegistoredElementWidth() {
	return maxElementLength;
}

/* 選択肢最大文字幅取得(ASCII) */
int sel_getRegistoredElement_strlen() {
	int i, _max = 0;
	
	for (i = 0; i < regnum; i++) {
		_max = max(_max, strlen(elm[i]));
	}
	return _max;
}

/* フォントの設定 */
void sel_setFontSize(int size) {
	sel.MsgFontSize = size;
}

/* 選択肢の登録 */
void sel_addElement(const char *str) {
	int catlen;
	
	catlen = ELEMENT_LENGTH - strlen(elm[regnum]) -1;
	strncat(elm[regnum], str, catlen);
}

/* 選択したときの返り値を登録 */
void sel_addRetValue(int val) {
	elm[regnum][0] = 0;
	elmv[regnum] = val;
}

/* １要素の登録の終了 */
void sel_fixElement() {
	maxElementLength = max(maxElementLength, strlen(elm[regnum]) /2);
	regnum++;
}

/* 選択肢ウィンドを開く為の準備 */
static void init_selwindow() {
	int i;
	MyRectangle r;
	
	r.x      = sel.win->x;
	r.y      = sel.win->y;
	r.width  = 4 + (sel.WinResizeWidth  ? sel.MsgFontSize * maxElementLength : sel.win->width);
	r.height = 2 + (sel.WinResizeHeight ? (sel.MsgFontSize +2) * regnum     : max((sel.MsgFontSize +2) * regnum, sel.win->height));
	
	if (sel.win->save) {
		saveArea.x = r.x - sel.Framedot;
		saveArea.y = r.y - sel.Framedot;
		saveArea.width  = r.width  + 2 * sel.Framedot;
		saveArea.height = r.height + 2 * sel.Framedot;
		saveimg = ags_saveRegion(saveArea.x, saveArea.y, saveArea.width, saveArea.height);
	} else {
		saveArea = r;
	}
	
	if (sel.WinBackgroundTransparent == 255) {
		ags_fillRectangle(r.x, r.y, r.width, r.height, sel.WinBackgroundColor);
	} else {
		ags_wrapColor(r.x, r.y, r.width, r.height, sel.WinBackgroundColor, sel.WinBackgroundTransparent);
	}
	
	switch(sel.WindowFrameType) {
	case WINDOW_FRAME_EMPTY:
		break;
	case WINDOW_FRAME_LINE:
		drawLineFrame(r.x, r.y, r.width, r.height);
		break;
	case WINDOW_FRAME_CG:
		printf("frameType is CG %d,%d,%d\n",sel.FrameCgNoTop, sel.FrameCgNoMid, sel.FrameCgNoBot);
		break;
	default:
		printf("frameType is Default");
		break;
	}
	ags_setFont(FONT_GOTHIC, sel.MsgFontSize);
	for (i = 0; i < regnum; i++) {
		DEBUG_MESSAGE("%d:%s\n", i +1, elm[i]);
		ags_drawString(r.x +2, r.y + i * (sel.MsgFontSize +2) +2, elm[i], sel.MsgFontColor);
	}
	ags_updateArea(saveArea.x, saveArea.y, saveArea.width, saveArea.height);
	
	/* マウスカーソルの自動移動 */
	if (default_element == 0) {
		MyPoint p;
		sys_getMouseInfo(&p, TRUE);
		if (p.y < r.y) {
			ags_setCursorLocation(r.x + r.width * MOUSE_INIT_X_RATIO/100,
					      r.y + (sel.MsgFontSize +2) * MOUSE_INIT_Y_RATIO/100,
					      TRUE);
		} else if (p.y > (r.y + r.height)) {
			ags_setCursorLocation(r.x + r.width * MOUSE_INIT_X_RATIO/100,
					      r.y + (sel.MsgFontSize +2) * regnum * MOUSE_INIT_Y_RATIO/100,
					      TRUE);
		} else {
			ags_setCursorLocation(r.x + r.width * MOUSE_INIT_X_RATIO/100,
					      p.y * MOUSE_INIT_Y_RATIO/100, 
					      TRUE);
		}
	} else if (default_element < regnum) {
		ags_setCursorLocation(r.x + r.width * MOUSE_INIT_X_RATIO/100,
				      r.y + (sel.MsgFontSize +2) * default_element * MOUSE_INIT_Y_RATIO/100, TRUE);
	} else if (default_element < 1000) {
		ags_setCursorLocation(r.x + r.width * MOUSE_INIT_X_RATIO/100,
				      r.y + (sel.MsgFontSize +2) * regnum * MOUSE_INIT_Y_RATIO/100, TRUE);
	}
}

static void remove_selwindow() {
	if (sel.win->save) {
		ags_restoreRegion(saveimg, saveArea.x, saveArea.y);
		ags_updateArea(saveArea.x, saveArea.y, saveArea.width, saveArea.height);
		saveimg = NULL;
	}
}

static int whereElement(void) {
	int i;
	MyPoint p;
	MyRectangle *r = workR;
	static int mpx, mpy;
	
	sys_getMouseInfo(&p, TRUE);
	if (keymode) {
		if (abs(mpx - p.x) > 2 || abs(mpy - p.y) > 2) {
			keymode = 0;
		} else {
			return -1;
		}
	}
	
	mpx = p.x;
	mpy = p.y;
	
	for (i = 0; i < regnum; i++) {
		if (p.x >= r[i].x && p.x < r[i].x + r[i].width &&
		    p.y >= r[i].y && p.y < r[i].y + r[i].height) {
			return i;
		}
	}
	return -1;
}

static void lineEncloseElement(MyRectangle *r, int col) {
	ags_drawRectangle(r->x   , r->y  , r->width +2, r->height +2, col);
	ags_drawRectangle(r->x +1, r->y+1, r->width   , r->height   , col);
	ags_updateArea   (r->x   , r->y  , r->width +2, r->height +2);
}

static void encloseElement(int sw, int no) {
	MyRectangle *r = &workR[no];
	
	if (sw == 0) { /* off */
		if (sel.WinBackgroundTransparent != 255) {
			ags_restoreRegion(saveimg2, r->x, r->y);
			ags_updateArea(r->x, r->y, r->width +2, r->height +2);
			saveimg2 = NULL;
		} else {
			switch(sel.EncloseType) {
			case 0:
				lineEncloseElement(r, sel.WinBackgroundColor); break;
			case 1:
			case 2:
				ags_fillRectangle(r->x, r->y, r->width +2, r->height +2, sel.WinBackgroundColor);
				ags_drawString(r->x +2, r->y +2, elm[no], sel.MsgFontColor);
				ags_updateArea(r->x, r->y, r->width +2, r->height +2);
				break;
			default:
				break;
			}
		}
		
	} else {       /* on */
		if (sel.WinBackgroundTransparent != 255) {
			saveimg2 = ags_saveRegion(r->x, r->y, r->width +2, r->height +2); 
		}
		switch(sel.EncloseType) {
		case 0:
			lineEncloseElement(r, sel.MsgFontColor); break;
		case 1:
		case 2:
			ags_fillRectangleNeg(r->x, r->y, r->width +2, r->height +2, sel.SelectedElementColor);
			ags_drawString(r->x +2, r->y +2, elm[no], sel.SelectedElementColor);
			ags_updateArea(r->x, r->y, r->width +2, r->height +2);
			break;
		default:
			break;
		}
	}
}

static void drawLineFrame(int x, int y, int width, int height) {
	ags_drawRectangle(x - 8, y - 8, width + 16, height + 16, sel.WinFrameColor);
	ags_drawRectangle(x - 7, y - 7, width + 14, height + 14, sel.WinFrameColor);
	ags_drawRectangle(x - 6, y - 6, width + 12, height + 12, sel.WinFrameColor);
	ags_drawRectangle(x - 5, y - 5, width + 10, height + 10, sel.WinBackgroundColor);
	ags_drawRectangle(x - 4, y - 4, width +  8, height +  8, sel.WinBackgroundColor);
	ags_drawRectangle(x - 3, y - 3, width +  6, height +  6, sel.WinFrameColor);
	ags_drawRectangle(x - 2, y - 2, width +  4, height +  4, sel.WinBackgroundColor);
	ags_drawRectangle(x - 1, y - 1, width +  2, height +  2, sel.WinBackgroundColor);
}

/* 実際に選択 */
void sel_select() {
	int curElement = -1;
	int preElement = -1;
	int key, i;
	
	saveimg2 = NULL;
	keymode = 0;
	
	/* 選択肢ウィンドの初期化 */
	init_selwindow();

	/* マウス領域の初期化 */
	if (NULL == (workR = malloc(sizeof(MyRectangle) * regnum))) {
		NOMEMERR();
	}

	for (i = 0; i < regnum; i++) {
		workR[i].x      = sel.win->x;
		workR[i].y      = sel.win->y + i * (sel.MsgFontSize +2);
		workR[i].width  = 2 + (sel.WinResizeWidth ? sel.MsgFontSize * maxElementLength : sel.win->width);
		workR[i].height = 2 + sel.MsgFontSize;
	}
	
	sys_key_releasewait(SYS35KEY_RET, FALSE);
	while (1) {
		key = sys_keywait(25, TRUE);
		if (key == SYS35KEY_SPC) break;
		if (key == SYS35KEY_RET && curElement != -1) break;
		if (key & 3) {
			curElement = -1;
			if ((key & 1)) {
				if (preElement > 0) curElement = preElement-1;
				if (preElement < 0) curElement = 0;
			}
			
			if ((key & 2) && preElement < (regnum-1)) {
				curElement = preElement+1;
			}
			if (curElement >= 0) {
				if (preElement >= 0) {
					encloseElement(0, preElement);
				}
				sys_key_releasewait(key, FALSE);
				encloseElement(1, curElement);
				preElement = curElement;
			}
			keymode = 1;
		} else {
			curElement = whereElement();
			fflush(stdout);
			if (keymode) {
				curElement = preElement;
				continue;
			}
			
			if (curElement != preElement) {
				if (preElement != -1) {
					encloseElement(0, preElement);
					preElement = -1;
				}
			}
			if (curElement != -1 && curElement != preElement) {
				encloseElement(1, curElement);
				preElement = curElement;
			}
		}
	}
	sys_key_releasewait(SYS35KEY_RET, FALSE);
	
	if (saveimg2 != NULL) {
		ags_delRegion(saveimg2);
		saveimg2 = NULL;
	}
	
	free(workR);
	
	/* sysVar[0] = key; 実際には入らない。thanx 田尻さん */
	
	regnum = 0;
	maxElementLength = 0;
	remove_selwindow();
	
	if (sel.ClearMsgWindow) {
		msg_nextPage(TRUE);
	}
	
	if (key == SYS35KEY_RET) {
		sl_jmpNear(elmv[curElement]);
		if (cb_select_page != 0) {
			sl_callFar2(cb_select_page -1, cb_select_address);
		}
		last_selected_element = curElement + 1;
	} else {
		sl_jmpNear(sl_getIndex());
		if (cb_cancel_page != 0) {
			sl_callFar2(cb_cancel_page -1, cb_cancel_address);
		}
		last_selected_element = 0;
	}
	
	if (get_skipMode2()) set_skipMode(FALSE);
}
