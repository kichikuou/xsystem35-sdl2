/*
 * sactstring.c: SACTの文字列操作関連
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
/* $Id: sactstring.c,v 1.1 2003/11/09 15:06:12 chikama Exp $ */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "portab.h"
#include "system.h"
#include "list.h"
#include "variable.h"

// 文字列置換用
typedef struct {
	char *src; // 置き換え元文字列
	char *dst; // 置き換え文字列
} strexchange_t;

#define DEFSTACKSIZE 100
static char **stack;   // stack本体
static int idx;        // stack pointer
static int idxmax;     // stack pointerの最大

// 文字列置き換え用 (表示時にon-the-flyで変換して表示)
#define REPLACEBUFSIZE 3000
static char repbuf[2][REPLACEBUFSIZE];
static SList *strreplace;
static char *replacesrc;
static char *replacedst;

/**
 * 文字列変数スタックの初期化
 */
void nt_sstr_init(void) {
	stack = malloc(sizeof(char *) * DEFSTACKSIZE);
	idx = 0;
	idxmax = DEFSTACKSIZE;
}

void nt_sstr_reset(void) {
	for (int i = 0; i < idx; i++)
		free(stack[i]);
	idx = 0;
	free(stack);
	stack = NULL;

	for (SList *l = strreplace; l; l = l->next) {
		strexchange_t *ex = l->data;
		free(ex->src);
		free(ex->dst);
		free(ex);
	}
	slist_free(strreplace);
	strreplace = NULL;
}

/**
 * 文字列の置き換え
 * @param sstrno: 変換元文字列変数番号
 * @param dstrno: 変換先文字列変数番号
 */
void nt_sstr_regist_replace(char *sstr, char *dstr) {
	strexchange_t *ex;
	
	if (sstr == dstr) return;
	
	ex = malloc(sizeof(strexchange_t));
	ex->src = sstr;
	ex->dst = dstr;
	strreplace = slist_append(strreplace, ex);
}

// 文字列の置き換え処理
static void replacestr_cb(void* data, void* userdata) {
	strexchange_t *ex = (strexchange_t *)data;
	char *start, *next, *out;
	
	if (ex == NULL) return;
	
	start = replacesrc;
	out   = replacedst;
	
	while (true) {
		next = strstr(start, ex->src);
		if (next == NULL) break;
		strncat(out, start, (size_t)(next - start));
		strncat(out, ex->dst, max(0, (REPLACEBUFSIZE - (int)strlen(out))));
		start = next + strlen(ex->src);
	}
	strncat(out, start, max(0, REPLACEBUFSIZE - (int)strlen(out)));
	
	replacedst = replacesrc;
	replacesrc = out;
	replacedst[0] = '\0';
}

// 文字列の置き換え
char *nt_sstr_replacestr(char *msg) {
	if (strreplace == NULL) return msg;
	
	repbuf[0][0] = '\0';
	repbuf[1][0] = '\0';
	strncpy(repbuf[0], msg, REPLACEBUFSIZE);
	replacesrc = repbuf[0];
	replacedst = repbuf[1];
	slist_foreach(strreplace, replacestr_cb, NULL);

	return (repbuf[0][0] == '\0') ? repbuf[1] : repbuf[0];
}
