/*
 * sactlog.c: バックログ
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
/* $Id: sactlog.c,v 1.3 2004/10/31 04:18:02 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>

#include "portab.h"
#include "system.h"
#include "list.h"
#include "menu.h"
#include "input.h"
#include "nact.h"
#include "sact.h"
#include "sprite.h"
#include "ngraph.h"
#include "drawtext.h"
#include "utfsjis.h"

#define LOGMSG_LINES 6

static const char *logmsg_utf8[LOGMSG_LINES] = {
	"\n",
	"※バックログ操作方法※",
	"[ESC]またはマウス右クリックでゲームに戻る",
	"[PageUp][PageDown]でページスクロール",
	"[↑][↓]またはマウスホイールで行スクロール",
	"\n",
};

/*
  ホイールで上下スクロール
  ESCキーで終了
  PageUP/Downで１ページ送り
*/

#define FONTSIZEINDEX 10
#define FONTSIZE 20
#define LOGLINENUM (sf0->height / FONTSIZE)
static int curline;
static surface_t *back;
static SDL_Surface *chr;

static void draw_log() {
	int i, y = 0, len;
	int cur = curline;
	char pinfo[256];
	List *node;

	SDL_FillRect(chr, NULL, 0);
	
	// ページ位置情報
	len = snprintf(pinfo, sizeof(pinfo) -1, "%d/%d", curline, list_length(sact.log));
	
	dt_setfont(FONT_GOTHIC, FONTSIZEINDEX);
	dt_drawtext(chr, sf0->width - FONTSIZEINDEX *len /2, 0, pinfo);
	
	// 表示始め位置
	node = list_nth(sact.log, list_length(sact.log) - curline);
	for (i = 0; i < LOGLINENUM; i++) {
		if (cur <= 0) continue;
		
		char *str = (char *)(node->data);
		if (0 == strcmp(str, "\n")) {
			SDL_FillRect(chr, &(SDL_Rect){0, y + FONTSIZE/2, sf0->width, 3}, 128);
		} else {
			if (cur < 6) {
				dt_setfont(FONT_MINCHO, FONTSIZE);
			} else {
				dt_setfont(FONT_GOTHIC, FONTSIZE);
			}
			dt_drawtext(chr, 0, y, str);
		}
		y += FONTSIZE;
		cur--;
		node = list_next(node);
	}
	
	gr_copy_bright(sf0, 0, 0, back, 0, 0, sf0->width, sf0->height, 128);
	gr_expandcolor_blend(sf0, 0, 0, chr, 0, 0, sf0->width, sf0->height,
			     255, 255, 255);
	ags_updateFull();
}



bool sblog_start(void) {
	if (sact.version < 120)
		return false;

	static char *logmsg[LOGMSG_LINES];
	if (!logmsg[0]) {
		for (int i = 0; i < LOGMSG_LINES; i++)
			logmsg[i] = fromUTF8(logmsg_utf8[i]);
	}

	// 説明文章を追加
	for (int i = 0; i < LOGMSG_LINES; i++)
		sact.log = list_append(sact.log, logmsg[i]);
	
	back = sf_dup(sf0);
	chr = SDL_CreateRGBSurfaceWithFormat(0, sf0->width, sf0->height, 8, SDL_PIXELFORMAT_INDEX8);
	curline = 6;
	draw_log();
	return true;
}

void sblog_end(void) {
	List *node;
	int i;
	
	sf_copyall(sf0, back);
	ags_updateFull();
	
	sf_free(back);
	if (chr) SDL_FreeSurface(chr);
	
	// 説明文章を削除
	for (i = 0; i < LOGMSG_LINES; i++) {
		node = list_last(sact.log);
		sact.log = list_remove(sact.log, node->data);
	}
}

void sblog_pageup(void) {
	curline = min(list_length(sact.log), curline + (LOGLINENUM -1));
	draw_log();
}

void sblog_pagedown(void) {
	curline = max(1, curline - (LOGLINENUM -1));
	draw_log();
}

void sblog_pagepre(void) {
	curline = max(1, curline - 1);
	draw_log();
}

void sblog_pagenext(void) {
	curline = min(list_length(sact.log), curline + 1);
	draw_log();
}
