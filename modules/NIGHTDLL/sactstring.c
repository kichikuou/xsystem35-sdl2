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
#include <glib.h>
#include <string.h>
#include "portab.h"
#include "system.h"
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
static GSList *strreplace;
static char *replacesrc;
static char *replacedst;

/**
 * 文字列変数スタックの初期化
 */
int sstr_init() {
	stack = g_new(char *, DEFSTACKSIZE);
	idx = 0;
	idxmax = DEFSTACKSIZE;
	return OK;
}

/**
 * 文字列変数スタックに文字列を積む
 * @param strno: シナリオ上での文字列変数番号
 */
int sstr_push(char *str) {
	if (idx >= idxmax) {
		stack = g_renew(char *, stack, idx*2);
		idxmax = idx*2;
	}
	
	stack[idx++] = g_strdup(str);
	
	return OK;
}

/**
 * 文字列変数スタックから文字列を取り出す
 * @param strno: スタックから戻した文字列を格納する文字列変数番号
 */
int sstr_pop(char *str, int maxlen) {
	if (idx == 0) return NG;
	
	strncpy(str, stack[--idx], maxlen);
	g_free(stack[idx]);
	
	return OK;
}

/**
 * 文字列の置き換え
 * @param sstrno: 変換元文字列変数番号
 * @param dstrno: 変換先文字列変数番号
 */
int sstr_regist_replace(char *sstr, char *dstr) {
	strexchange_t *ex;
	
	if (sstr == dstr) return NG;
	
	ex = g_new(strexchange_t, 1);
	ex->src = strdup(sstr);
	ex->dst = strdup(dstr);
	strreplace = g_slist_append(strreplace, ex);
	return OK;
}

/**
 * 数値 -> 文字列化
 */
int sstr_num2str(int strno, int fig, int nzeropad, int num) {
	char s[256], ss[256];
	
	if (nzeropad) {
		char *sss = "%%0%dd";
		sprintf(ss, sss, fig);
	} else {
		char *sss = "%%%dd";
		sprintf(ss, sss, fig);
	}
	
	sprintf(s, ss, num);
	v_strcpy(strno -1, s);
	
	return OK;
}

// 文字列の置き換え処理
static void replacestr_cb(gpointer data, gpointer userdata) {
	strexchange_t *ex = (strexchange_t *)data;
	char *start, *next, *out;
	
	if (ex == NULL) return;
	
	start = replacesrc;
	out   = replacedst;
	
	while (TRUE) {
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
char *sstr_replacestr(char *msg) {
	if (strreplace == NULL) return msg;
	
	repbuf[0][0] = '\0';
	repbuf[1][0] = '\0';
	strncpy(repbuf[0], msg, REPLACEBUFSIZE);
	replacesrc = repbuf[0];
	replacedst = repbuf[1];
	g_slist_foreach(strreplace, replacestr_cb, NULL);

	return (repbuf[0][0] == '\0') ? repbuf[1] : repbuf[0];
}
