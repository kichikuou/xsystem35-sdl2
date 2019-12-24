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

#include "portab.h"
#include "system.h"
#include "list.h"
#include "counter.h"
#include "menu.h"
#include "input.h"
#include "nact.h"
#include "key.h"
#include "sact.h"
#include "sprite.h"
#include "ngraph.h"
#include "drawtext.h"
#include "utfsjis.h"
#include "sactlog_sjismsg.c"

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
static surface_t *chr;

static void draw_log() {
	int i, y = 0, len;
	int cur = curline;
	char pinfo[256];
	List *node;

	// canvas clear
	memset(chr->pixel, 0, chr->bytes_per_line * chr->height);
	
	// ページ位置情報
	len = snprintf(pinfo, sizeof(pinfo) -1, "%d/%d", curline, list_length(sact.log));
	
	dt_setfont(FONT_GOTHIC, FONTSIZEINDEX);
	dt_drawtext(chr, sf0->width - FONTSIZEINDEX *len /2, 0, pinfo);
	
	// 表示始め位置
	node = list_nth(sact.log, list_length(sact.log) - curline);
	for (i = 0; i < LOGLINENUM; i++) {
		char *str, *streuc;
		if (cur <= 0) continue;
		
		str = (char *)(node->data);
		if (0 == strcmp(str, "\n")) {
			gr_fill(chr, 0, y + FONTSIZE/2, sf0->width, 3, 128, 0, 0);
		} else {
			streuc = sjis2lang(str);
			if (cur < 6) {
				dt_setfont(FONT_MINCHO, FONTSIZE);
			} else {
				dt_setfont(FONT_GOTHIC, FONTSIZE);
			}
			dt_drawtext(chr, 0, y, str);
			free(streuc);
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



int sblog_start(void) {
	// 説明文章を追加
	sact.log = list_append(sact.log, "\n");
	sact.log = list_append(sact.log, LOGMSG1);
	sact.log = list_append(sact.log, LOGMSG2);
	sact.log = list_append(sact.log, LOGMSG3);
	sact.log = list_append(sact.log, LOGMSG4);
	sact.log = list_append(sact.log, "\n");
	
	back = sf_dup(sf0);
	chr  = sf_create_surface(sf0->width, sf0->height, 8);
	curline = 6;
	draw_log();
	return OK;
}

int sblog_end(void) {
	List *node;
	int i;
	
	sf_copyall(sf0, back);
	ags_updateFull();
	
	sf_free(back);
	sf_free(chr);
	
	// 説明文章を削除
	for (i = 0; i < 6; i++) {
		node = list_last(sact.log);
		sact.log = list_remove(sact.log, node->data);
	}
	
	return OK;
}

int sblog_pageup(void) {
	curline = min(list_length(sact.log), curline + (LOGLINENUM -1));
	draw_log();
	return OK;
}

int sblog_pagedown(void) {
	curline = max(1, curline - (LOGLINENUM -1));
	draw_log();
	return OK;
}

int sblog_pagepre(void) {
	curline = max(1, curline - 1);
	draw_log();
	return OK;
}

int sblog_pagenext(void) {
	curline = min(list_length(sact.log), curline + 1);
	draw_log();
	return OK;
}
