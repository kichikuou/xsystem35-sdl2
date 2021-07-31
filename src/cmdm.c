/*
 * cmdm.c  SYSTEM35 M command
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
/* $Id: cmdm.c,v 1.27 2002/12/21 12:28:35 chikama Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "portab.h"
#include "xsystem35.h"
#include "scenario.h"
#include "utfsjis.h"
#include "menu.h"
#include "ags.h"
#include "input.h"
#include "message.h"
#include "gametitle.h"
#include "hankaku.h"

/* defined by cmds.c */
extern boolean dummy_pcm_su_flag;
/* defined by cmdy.c */
extern boolean Y3waitFlags;


/* MI 用パラメータ */
INPUTSTRING_PARAM mi_param;

boolean have_eng_mp_patch = FALSE;

void commandMS() {
	/* Xコマンドで表示される文字列領域に文字列を入れる */
	int num = getCaliValue();
	const char *str = sl_getString(':');
	
	if (num > 0) { /* thanx tajiri@wizard */
		svar_set(num, str);
	} else {
        	WARNING("MS: num(%d) <= 0\n", num);
	}
	DEBUG_COMMAND("MS %d,%s:\n",num,str);
}

void commandMP() {
	/* 指定の文字列を指定文字数だけ表示する（Ｘコマンドの桁数指定） */
	int    num1 = getCaliValue();
	int    num2 = getCaliValue();
	const char *fullwidth_blank[CHARACTER_ENCODING_MAX + 1] = {
		[SHIFT_JIS] = "\x81\x40",
		[UTF8] = "　",
	};
	const char *src = svar_get(num1);
	int chars = num2;
	char *str;

	/* Patched English executable appends num2 spaces instead of truncating */
	if (have_eng_mp_patch) {
		str = calloc(strlen(src) + num2 * strlen(fullwidth_blank[nact->encoding]) + 1, 1);
		if (NULL == str) {
			NOMEMERR();
		}
		strcpy(str, src);
	}
	else {
		str = calloc(num2 * MAX_BYTES_PAR_CHAR(nact->encoding) + 1, 1);
		if (NULL == str) {
			NOMEMERR();
		}

		const char *p = src;
		while (*p && chars > 0) {
			p = advance_char(p, nact->encoding);
			chars--;
		}
		strncpy(str, src, p - src);
	}
	while (chars-- > 0) {
		strcat(str, fullwidth_blank[nact->encoding]);
	}

	sys_addMsg(str);
	DEBUG_COMMAND("MP %d,%d:\n",num1,num2);
	
	free(str);
}

void commandMI() { /* T2 */
	/* ユーザーによる文字列の入力 */
	int dst_no  = getCaliValue();
	int max_len = getCaliValue();
	const char *title = sl_getString(':');
	char *t1, *t2, *t3;
	
	t1 = toUTF8(title);
	t2 = toUTF8(svar_get(dst_no));
	
	mi_param.title = t1;
	mi_param.oldstring = t2;
	mi_param.max = max_len;
	
	menu_inputstring(&mi_param);
	if (mi_param.newstring == NULL) {
		svar_set(dst_no, NULL);
		free(t1); free(t2);
		return;
	}
	
	t3 = fromUTF8(mi_param.newstring);
	
	svar_set(dst_no, t3);

	free(t1);
	free(t2);
	free(t3);
	DEBUG_COMMAND("MI %d,%d,%s:\n",dst_no,max_len, title);
}

void commandMA() {
	/* num1 の文字列の後ろに num2 をつなげる */
	int num1 = getCaliValue();
	int num2 = getCaliValue();

	if (num1 == num2) {
		char *buf = strdup(svar_get(num2));
		svar_append(num1, buf);
		free(buf);
	} else {
		svar_append(num1, svar_get(num2));
	}
	
	DEBUG_COMMAND("MA %d,%d:\n",num1,num2);
}

void commandMC() {
	/* num1 , num2 を比較して結果を RND に返す (RND=0 不一致 , RND=1 一致) */
	int num1 = getCaliValue();
	int num2 = getCaliValue();
	
	sysVar[0] = strcmp(svar_get(num1), svar_get(num2)) == 0 ? 1 : 0;
	
	DEBUG_COMMAND("MC %d,%d:\n",num1,num2);
}

void commandMT() {
	/* ウインドウのタイトル文字列を設定する */
	const char *str = sl_getString(':');
	
	if (nact->game_title_utf8)
		free(nact->game_title_utf8);
	nact->game_title_utf8 = toUTF8(str);
	ags_setWindowTitle(str);
	
	/* 闘神都市II 対策 */
	if (0 == strcmp(nact->game_title_utf8, GT_TOSHIN2)) {
		dummy_pcm_su_flag = TRUE;
	}
	
	/* Rance4 対策？ */
	if (0 == strcmp(nact->game_title_utf8, GT_RANCE4)) {
		Y3waitFlags = KEYWAIT_NONCANCELABLE;
	}

	DEBUG_COMMAND("MT %s:\n",str);
}

void commandMM() {
	/* num1 の文字列に num2 をコピーする */
	int num1 = getCaliValue();
	int num2 = getCaliValue();
	
	if (num1 != num2)
		svar_set(num1, svar_get(num2));
	
	DEBUG_COMMAND("MM %d,%d:\n",num1, num2);
}

void commandMH() {
	/* 数値を文字列に変換する (参考 Hコマンド) */
	int num1 = getCaliValue();
	int fig  = getCaliValue();
	int num2 = getCaliValue();

	char buf[512];
	char *s = fromSJIS(format_number_zenkaku(num2, fig, buf));
	svar_set(num1, s);
	free(s);

	DEBUG_COMMAND("MH %d,%d,%d:\n",num1,fig,num2);
}

void commandMV() {
	/* シナリオバージョンをシステムへ通知する */
	int version = getCaliValue();

	nact->scenario_version = version;
	DEBUG_COMMAND("MV %d:\n",version);
}

void commandML() {
	/* 文字列の長さを取得する */
	int *var   = getCaliVariable();
	int str_no = getCaliValue();
	
	*var = svar_length(str_no);
	
	DEBUG_COMMAND("ML %p,%d:\n",var, str_no);
}

void commandMD() {
	/* 文字列変数を指定長さ分コピーする */
	int dst_str_no = getCaliValue();
	int src_str_no = getCaliValue();
	int len        = getCaliValue();
	
	svar_copy(dst_str_no, 0, src_str_no, 0, len);
	
	DEBUG_COMMAND("MD %d,%d,%d:\n",dst_str_no, src_str_no, len);
}

void commandME() {
	/* 位置指定つきの文字列コピー */
	int dst_str_no = getCaliValue();
	int dst_pos    = getCaliValue();
	int src_str_no = getCaliValue();
	int src_pos    = getCaliValue();
	int len        = getCaliValue();
	
	svar_copy(dst_str_no, dst_pos, src_str_no, src_pos, len);
	
	DEBUG_COMMAND("ME %d,%d,%d,%d,%d:\n",dst_str_no, dst_pos, src_str_no, src_pos, len);
}

void commandMF() {
	/* 文字列中から指定文字列の位置を探す */
	int *var      = getCaliVariable();
	int dst_no    = getCaliValue();
	int key_no    = getCaliValue();
	int start_pos = getCaliValue();
	
	int pos = svar_find(dst_no, start_pos, svar_get(key_no));
	
	if (pos < 0) {
		sysVar[0] = 255;
	} else {
		*var = pos;
		sysVar[0] = 0;
	}
	
	DEBUG_COMMAND("MF %p,%d,%d,%d:\n",var, dst_no, key_no, start_pos);
}

void commandMZ0() {
	/* 文字列変数の文字数・個数の設定の変更 */
	int max_len = getCaliValue();
	int max_num = getCaliValue();
	int rsv     = getCaliValue();
	
	DEBUG_COMMAND("MZ0 %d,%d,%d:\n",max_len, max_num, rsv);
	
	/* いつからか、文字列変数の最大長さは∞になったようだ */
	if (max_len == 0) max_len = STRVAR_LEN * 2;
	
	svar_init(max_num, max_len * 2 + 1);
}

void commandMG() {
	int no = sl_getc();
	int sw = 0, *var;
	
	switch(no) {
	case 0:
		sw = getCaliValue();
		nact->msg.mg_getString = sw == 1 ? TRUE : FALSE;
		break;
	case 1:
		sw = getCaliValue();
		nact->msg.mg_startStrVarNo = sw;
		break;
	case 2:
		sw = getCaliValue();
		nact->msg.mg_policyR = sw;
		break;
	case 3:
		sw = getCaliValue();
		nact->msg.mg_policyA = sw;
		break;
	case 4:
		sw = getCaliValue();
		nact->msg.mg_curStrVarNo = nact->msg.mg_startStrVarNo + sw;
		svar_set(nact->msg.mg_curStrVarNo, "");
		break;
	case 5:
		var = getCaliVariable();
		*var = nact->msg.mg_curStrVarNo;
		break;
	case 6:
		sw = getCaliValue();
		msg_mg6_command(sw);
		break;
	case 7:
		var = getCaliVariable();
		*var = svar_length(nact->msg.mg_curStrVarNo);
		break;
	case 100:
		sw = getCaliValue();
		nact->msg.mg_dspMsg = sw == 1 ? TRUE : FALSE;
		break;
	default:
		sw = getCaliValue();
		WARNING("Unknown MG command %d,%d\n", no, sw);
		break;
	}

	DEBUG_COMMAND("MG %d,%d:\n",no, sw);
}

void commandMJ() {
	/* 文字列入力 (ウィンド無し) */
	int num = getCaliValue();
	int x   = getCaliValue();
	int y   = getCaliValue();
	int h   = getCaliValue();
	int max_len = getCaliValue();
	INPUTSTRING_PARAM mj_param;
	char *t1, *t2;
	
	t1 = toUTF8(svar_get(num));
	mj_param.max = max_len;
	mj_param.x = x;
	mj_param.y = y;
	mj_param.h = h;
	mj_param.oldstring = t1;
	
	ags_setCursorLocation(x, y, FALSE); /* XXX */
	menu_inputstring2(&mj_param);
	if (mj_param.newstring == NULL) return;
	
	t2 = fromUTF8(mj_param.newstring);
	svar_set(num, t2);

	free(t1);
	free(t2);
	DEBUG_COMMAND("MJ %d,%d,%d,%d,%d:\n", num, x, y, h, max_len);
}

void commandMN() {
	int no   = sl_getc();
	int num  = getCaliValue();
	int *var = getCaliVariable();
	
	switch(no) {
	case 0:
		/* 文字列を配列に変換する */
		sysVar[0] = svar_toVars(num, var);
		break;
	case 1:
		/* 配列を文字列に変換する */
		svar_fromVars(num, var);
		break;
	default:
		WARNING("UnKnown MN command(%d)\n", no);
	}
	DEBUG_COMMAND("MN %d,%d,%d:\n",no, num, *var);
}
