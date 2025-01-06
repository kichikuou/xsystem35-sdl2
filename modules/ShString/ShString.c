/*
 * ShString.c  文字列操作 module
 *
 *   かえるにょ国にょアリス(未使用)
 *   大悪司
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
/* $Id: ShString.c,v 1.6 2004/10/31 04:18:03 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "portab.h"
#include "nact.h"
#include "system.h"
#include "xsystem35.h"
#include "modules.h"
#include "variable.h"
#include "ags.h"

static void ExchangeString(void) {
	/*
	  文字列(target)中の一部(pat)を別の文字列(patr)で置き換える
	*/
	int target = getCaliValue();
	int pat    = getCaliValue();
	int patr   = getCaliValue();
	svar_replaceAll(target, pat, patr);
	TRACE("ShString.ExchangeString: %d,%d,%d:", target, pat, patr);
}

static void SetNum16String(void) { /* 1 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();

	TRACE_UNIMPLEMENTED("ShString.SetNum16String: %d,%d:", p1, p2);
}

static void SetNum16HalfString(void) { /* 2 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();

	TRACE_UNIMPLEMENTED("ShString.SetNum16HalfString: %d,%d:", p1, p2);
}

static void SetNum32String(void) { /* 3 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();

	TRACE_UNIMPLEMENTED("ShString.SetNum32String: %d,%d:", p1, p2);
}

static void SetNum32HalfString(void) { /* 4 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();

	TRACE_UNIMPLEMENTED("ShString.SetNum32HalfString: %d,%d:", p1, p2);
}

static void GetArrayString(void) { /* 5 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();

	TRACE_UNIMPLEMENTED("ShString.GetArrayString: %d,%d,%d:", p1, p2, p3);
}

static void SetWindowTitle(void) { /* 6 */
	int strno = getCaliValue();
	int p2    = getCaliValue(); /* ISys3xSystem */ 
	
	char *title_utf8 = toUTF8(svar_get(strno));
	ags_setWindowTitle(title_utf8);
	free(title_utf8);
	
	TRACE("ShString.SetWindowTitle: %d,%d:", strno, p2);
}

static void FillString() {
	/*
	  指定の番号の文字列を他の文字列にコピー
	  
	  st:  コピー先の文字列の最初の番号
	  cnt: コピーする文字列の数
	  src: コピー元の文字列番号
	*/
	int st  = getCaliValue();
	int cnt = getCaliValue();
	int src = getCaliValue();
	int p4 = getCaliValue(); /* ISys3xStringTable */
	int i;
	
	for (i = 0; i < cnt; i++) {
		if (st + i != src)
			svar_set(st + i, svar_get(src));
	}
	
	TRACE("ShString.FillString: %d,%d,%d,%d:", st, cnt, src, p4);
}

static void SetStringNum16(void) {
	/*
	  文字列を数値に変換
	    大文字、小文字、混在可
	  
	  p1: 変換元文字列番号
	  p2: 変換された数値を格納する変数
	*/
	int st = getCaliValue();
	int *var = getCaliVariable();
	const char *str = svar_get(st);
	char _dst[100];
	char *dst = _dst;
	
	TRACE("ShString.SetStringNum16: %d,%p:", st, var);
	
	
	while(*str) {
		if (*str >= '0' && *str <= '9') {
			*dst = *str;
			dst++; str++;
		} else if ((unsigned char)*str == 0x82) {
			str++;
			if (*str >= '0' && *str <= '9') { 
				*dst = *str;
				dst++; str++;
			} else {
				*var = 0;
				return;
			}
		} else {
			*var = 0;
			return;
		}
	}
	*dst = '\0';
	
	*var = atoi(_dst);
}

static void SetStringNum32(void) {
	int p1 = getCaliValue();
	int *p2 = getCaliVariable();
	
	TRACE_UNIMPLEMENTED("ShString.SetStringNum32: %d,%p:", p1, p2);
}

static const ModuleFunc functions[] = {
	{"ExchangeString", ExchangeString},
	{"FillString", FillString},
	{"GetArrayString", GetArrayString},
	{"SetNum16HalfString", SetNum16HalfString},
	{"SetNum16String", SetNum16String},
	{"SetNum32HalfString", SetNum32HalfString},
	{"SetNum32String", SetNum32String},
	{"SetStringNum16", SetStringNum16},
	{"SetStringNum32", SetStringNum32},
	{"SetWindowTitle", SetWindowTitle},
};

const Module module_ShString = {"ShString", functions, sizeof(functions) / sizeof(ModuleFunc)};
