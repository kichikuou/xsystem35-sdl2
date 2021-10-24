/*
 * variable.c  変数管理
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
/* $Id: variable.c,v 1.15 2002/09/18 13:16:22 chikama Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "portab.h"
#include "utfsjis.h"
#include "variable.h"
#include "xsystem35.h"
#include "scenario.h"

typedef struct {
	int *pointvar;
	int page;
	int offset;
} VariableAttributes;

/* システム変数 */
int sysVar[SYSVAR_MAX];
/* 配列変数の情報 */
static VariableAttributes attributes[SYSVAR_MAX];
/* 配列本体 */
arrayVarBufferStruct arrayVarBuffer[ARRAYVAR_PAGEMAX];
/* 64bit変数 */
double longVar[SYSVARLONG_MAX];
/* 文字列変数 */
static char **strVar;
/* 文字列変数の属性(最大,1つあたりの大きさ) */
int strvar_cnt = STRVAR_MAX;
int strvar_len = STRVAR_LEN;

int preVarPage;      /* 直前にアクセスした変数のページ */
int preVarIndex;     /* 直前にアクセスした変数のINDEX */
int preVarNo;        /* 直前にアクセスした変数の番号 */

static char *advance(const char *s, int n) {
	while (*s && n > 0) {
		s = advance_char(s, nact->encoding);
		n--;
	}
	return (char *)s;
}

int *v_ref(int var) {
	VariableAttributes *attr = &attributes[var];
	preVarPage = attr->page;
	preVarNo   = var;

	if (attr->page == 0) {
		// Normal variable access
		preVarIndex = var;
		return sysVar + var;
	}

	// Implicit array access
	int *index = attr->pointvar;
	int page   = attr->page;
	int offset = attr->offset;
	if (*index + offset >= arrayVarBuffer[page - 1].size) {
		WARNING("%03d:%05x: ArrayIndexOutOfBounds (%d, %d, %d, %d)\n", sl_getPage(), sl_getIndex(), var, *index, page, offset);
		return NULL;
	}
	preVarIndex = offset + *index;
	return arrayVarBuffer[page - 1].value + offset + *index;
}

int *v_ref_indexed(int var, int index) {
	VariableAttributes *attr = &attributes[var];
	preVarPage = attr->page;
	preVarNo   = var;

	if (attr->page == 0) {
		// If VAR_n is not an array variable, VAR_n[i] points to VAR_(n+i).
		if ((var + index) >= SYSVAR_MAX) {
			WARNING("%03d:%05x: ArrayIndexOutOfBounds (%d, %d)\n", sl_getPage(), sl_getIndex(), var, index);
			return NULL;
		}
		preVarIndex = var + index;
		return sysVar + var + index;
	}

	// Indexed array access
	int page   = attr->page;
	int offset = attr->offset;
	if (offset + index >= arrayVarBuffer[page - 1].size) {
		WARNING("%03d:%05x: ArrayIndexOutOfBounds (%d, %d, %d, %d)\n", sl_getPage(), sl_getIndex(), var, index, page, offset);
		return NULL;
	}
	preVarIndex = offset + index;
	return arrayVarBuffer[page - 1].value + offset + index;
}

/* 配列バッファの確保 DC ,page = 1~ */
extern boolean v_allocateArrayBuffer(int page, int size, boolean saveflag) {
	if (page <= 0 || page > 256)   { return false; }
	if (size <= 0 || size > 65536) { return false; }
	
	void *buf = arrayVarBuffer[page - 1].value;
	if (buf != NULL)
		buf = realloc(buf, size * sizeof(int));
	else
		buf = calloc(size, sizeof(int));
	if (!buf)
		NOMEMERR();

	arrayVarBuffer[page - 1].value    = buf;
	arrayVarBuffer[page - 1].size     = size;
	arrayVarBuffer[page - 1].saveflag = saveflag;
	
	return true;
}

/*　配列変数の割り当て DS */
extern boolean v_defineArrayVar(int datavar, int *pointvar, int offset, int page) {
	if (datavar < 0 || datavar >  SYSVAR_MAX - 1)                { return false; }
	if (page    < 0 || page    >  ARRAYVAR_PAGEMAX - 1)          { return false; }
	if (offset  < 0 || offset  >= arrayVarBuffer[page - 1].size) { return false; }
	
	attributes[datavar].pointvar = pointvar;
	attributes[datavar].page     = page;
	attributes[datavar].offset   = offset;
	return true;
}

/* 配列変数の割り当て解除 DR */
extern boolean v_releaseArrayVar(int datavar) {
	attributes[datavar].page = 0;
	return true;
}

/* 指定ページは使用中 page = 1~ */
extern boolean v_getArrayBufferStatus(int page) {
	return (arrayVarBuffer[page - 1].value != NULL) ? true : false;
}

/* 文字列変数の再初期化 */
extern void svar_init(int cnt, int len) {
	for (int i = cnt + 1; i < strvar_cnt; i++) {
		if (strVar[i])
			free(strVar[i]);
	}
	strVar = realloc(strVar, cnt * sizeof(char *));
	if (strVar == NULL) {
		NOMEMERR();
	}
	for (int i = strvar_cnt + 1; i < cnt; i++)
		strVar[i] = NULL;
	strvar_cnt = cnt;
	strvar_len = len;
}

extern int svar_count(void) {
	return strvar_cnt;
}

/* 変数の初期化 */
extern boolean v_initVars() {
	strVar = calloc(STRVAR_MAX, sizeof(char *));
	if (strVar == NULL) {
		NOMEMERR();
	}
	return true;
}

/* 文字変数への代入 */
void svar_set(int no, const char *str) {
	if (no < 1 || no > strvar_cnt) {
		WARNING("string index out of range: %d", no);
		return;
	}
	if (strVar[no-1])
		free(strVar[no-1]);
	strVar[no-1] = strdup(str);
}

void svar_copy(int dstno, int dstpos, int srcno, int srcpos, int len) {
	if (dstno < 1 || dstno > strvar_cnt) {
		WARNING("string index out of range: %d", dstno);
		return;
	}
	if (!strVar[dstno-1])
		strVar[dstno-1] = strdup("");

	char *buf = NULL;
	const char *src;
	if (srcno == dstno)
		src = buf = strdup(strVar[srcno-1]);
	else
		src = svar_get(srcno);

	dstpos = advance(strVar[dstno-1], dstpos) - strVar[dstno-1];  // #chars -> #bytes
	src = advance(src, srcpos);
	len = advance(src, len) - src;  // #chars -> #bytes

	strVar[dstno-1] = realloc(strVar[dstno-1], dstpos + len + 1);
	strncpy(strVar[dstno-1] + dstpos, src, len);
	strVar[dstno-1][dstpos + len] = '\0';

	if (buf)
		free(buf);
}

/* 文字変数への接続 */
void svar_append(int no, const char *str) {
	if (no < 1 || no > strvar_cnt) {
		WARNING("string index out of range: %d", no);
		return;
	}
	if (!strVar[no-1]) {
		strVar[no-1] = strdup(str);
		return;
	}
	int len1 = strlen(strVar[no-1]);
	int len2 = strlen(str);
	strVar[no-1] = realloc(strVar[no-1], len1 + len2 + 1);
	strcpy(strVar[no-1] + len1, str);
}

/* 文字変数の長さ */
size_t svar_length(int no) {
	const char *s = svar_get(no);

	int c = 0;
	while (*s) {
		s = advance_char(s, nact->encoding);
		c++;
	}
	return c;
}

/* Width of a string (2 for full-width characters, 1 for half-width) */
int svar_width(int no) {
	return strlen(svar_get(no));
}

/* 文字変数そのもの */
const char *svar_get(int no) {
	if (no < 1 || no > strvar_cnt) {
		WARNING("string index out of range: %d", no);
		return "";
	}
	return strVar[no-1] ? strVar[no-1] : "";
}

int svar_find(int no, int start, const char *str) {
	if (no < 1 || no > strvar_cnt) {
		WARNING("string index out of range: %d", no);
		return -1;
	}
	if (!*str)
		return 0;
	if (!strVar[no-1])
		return -1;
	const char *p = advance(strVar[no-1], start);
	const char *found = strstr(p, str);
	if (!found)
		return -1;
	int n = start;
	while (p < found) {
		p = advance_char(p, nact->encoding);
		n++;
	}
	return n;
}

void svar_fromVars(int no, const int *vars) {
	if (no < 1 || no > strvar_cnt) {
		WARNING("string index out of range: %d", no);
		return;
	}
	int len = 0;
	for (const int *c = vars; *c; c++)
		len += (*c < 256) ? 1 : 2;

	if (strVar[no-1])
		free(strVar[no-1]);
	char *p = strVar[no-1] = malloc(len + 1);

	for (const int *v = vars; *v; v++) {
		if (*v < 256) {
			*p++ = *v;
		} else {
			*p++ = *v & 0xff;
			*p++ = *v >> 8;
		}
	}
	*p = '\0';
}

int svar_toVars(int no, int *vars) {
	if (no < 1 || no > strvar_cnt) {
		WARNING("string index out of range: %d", no);
		return 0;
	}
	if (!strVar[no-1]) {
		*vars = 0;
		return 1;
	}

	int count = 0;
	for (const char *p = strVar[no-1]; *p; p++, count++) {
		vars[count] = *p & 0xff;
		if (CHECKSJIS1BYTE(*p) && *(p + 1))
			vars[count] |= (*++p & 0xff) << 8;
	}
	vars[count++] = 0;
	return count;
}

int svar_getCharType(int no, int pos) {
	const char *p = svar_get(no);
	for (; *p && pos > 0; pos--)
		p += CHECKSJIS1BYTE(*p) ? 2 : 1;
	if (!*p)
		return 0;
	return CHECKSJIS1BYTE(*p) ? 2 : 1;
}

void svar_replaceAll(int no, int pattern, int replacement) {
	const char *pat = svar_get(pattern);
	if (!*pat)
		return;
	const char *repl = svar_get(replacement);

	if (no < 1 || no > strvar_cnt) {
		WARNING("string index out of range: %d", no);
		return;
	}
	if (!strVar[no-1])
		return;
	char *src = strVar[no-1];
	strVar[no-1] = NULL;

	char *start = src, *found;;
	while ((found = strstr(start, pat))) {
		char bak = *found;
		*found = '\0';
		svar_append(no, start);
		*found = bak;
		svar_append(no, repl);
		start = found + strlen(pat);
	}
	svar_append(no, start);
	free(src);
}
