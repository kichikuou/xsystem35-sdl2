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

/* システム変数 */
int sysVar[SYSVAR_MAX];
/* 配列変数の情報 */
arrayVarStruct sysVarAttribute[SYSVAR_MAX];
/* 配列本体 */
arrayVarBufferStruct arrayVarBuffer[ARRAYVAR_PAGEMAX];
/* 64bit変数 */
double longVar[SYSVARLONG_MAX];
/* 文字列変数 */
static char **strVar;
/* 文字列変数の属性(最大,1つあたりの大きさ) */
int strvar_cnt = STRVAR_MAX;
int strvar_len = STRVAR_LEN;

static char *advance(const char *s, int n) {
	while (*s && n > 0) {
		s += (CHECKSJIS1BYTE(*s) && *(s + 1)) ? 2 : 1;
		n--;
	}
	return (char *)s;
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
	
	sysVarAttribute[datavar].pointvar = pointvar;
	sysVarAttribute[datavar].page     = page;
	sysVarAttribute[datavar].offset   = offset;
	return true;
}

/* 配列変数の割り当て解除 DR */
extern boolean v_releaseArrayVar(int datavar) {
	sysVarAttribute[datavar].page = 0;
	return true;
}

/* 指定ページは使用中 page = 1~ */
extern boolean v_getArrayBufferStatus(int page) {
	return (arrayVarBuffer[page - 1].value != NULL) ? true : false;
}

/* 文字列変数の再初期化 */
extern void v_initStringVars(int cnt,int len) {
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

/* 変数の初期化 */
extern boolean v_initVars() {
	strVar = calloc(STRVAR_MAX, sizeof(char *));
	if (strVar == NULL) {
		NOMEMERR();
	}
	return true;
}

/* 文字変数への代入 */
void v_strcpy(int no, const char *str) {
	if (no < 0 || no >= strvar_cnt) {
		WARNING("string index out of range: %d", no);
		return;
	}
	if (strVar[no])
		free(strVar[no]);
	strVar[no] = strdup(str);
}

void v_strncpy(int dstno, int dstpos, int srcno, int srcpos, int len) {
	if (dstno < 0 || dstno >= strvar_cnt) {
		WARNING("string index out of range: %d", dstno);
		return;
	}
	if (!strVar[dstno])
		strVar[dstno] = strdup("");

	char *buf = NULL;
	const char *src;
	if (srcno == dstno)
		src = buf = strdup(strVar[srcno]);
	else
		src = v_str(srcno);

	dstpos = advance(strVar[dstno], dstpos) - strVar[dstno];  // #chars -> #bytes
	src = advance(src, srcpos);
	len = advance(src, len) - src;  // #chars -> #bytes

	strVar[dstno] = realloc(strVar[dstno], dstpos + len + 1);
	strncpy(strVar[dstno] + dstpos, src, len);
	strVar[dstno][dstpos + len] = '\0';

	if (buf)
		free(buf);
}

/* 文字変数への接続 */
void v_strcat(int no, const char *str) {
	if (no < 0 || no >= strvar_cnt) {
		WARNING("string index out of range: %d", no);
		return;
	}
	if (!strVar[no]) {
		strVar[no] = strdup(str);
		return;
	}
	int len1 = strlen(strVar[no]);
	int len2 = strlen(str);
	strVar[no] = realloc(strVar[no], len1 + len2 + 1);
	strcpy(strVar[no] + len1, str);
}

/* 文字変数の長さ */
size_t v_strlen(int no) {
	if (no < 0 || no >= strvar_cnt) {
		WARNING("string index out of range: %d", no);
		return 0;
	}
	return strVar[no] ? sjis_count_char(strVar[no]) : 0;
}

/* 文字変数そのもの */
const char *v_str(int no) {
	if (no < 0 || no >= strvar_cnt) {
		WARNING("string index out of range: %d", no);
		return "";
	}
	return strVar[no] ? strVar[no] : "";
}

int v_strstr(int no, int start, const char *str) {
	if (no < 0 || no >= strvar_cnt) {
		WARNING("string index out of range: %d", no);
		return -1;
	}
	if (!*str)
		return 0;
	if (!strVar[no])
		return -1;
	const char *p = advance(strVar[no], start);
	const char *found = strstr(p, str);
	if (!found)
		return -1;
	int n = start;
	while (p < found) {
		p += CHECKSJIS1BYTE(*p) ? 2 : 1;
		n++;
	}
	return n;
}

void v_strFromVars(int no, const int *vars) {
	if (no < 0 || no >= strvar_cnt) {
		WARNING("string index out of range: %d", no);
		return;
	}
	int len = 0;
	for (const int *c = vars; *c; c++)
		len += (*c < 256) ? 1 : 2;

	if (strVar[no])
		free(strVar[no]);
	char *p = strVar[no] = malloc(len + 1);

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

int v_strToVars(int no, int *vars) {
	if (no < 0 || no >= strvar_cnt) {
		WARNING("string index out of range: %d", no);
		return 0;
	}
	if (!strVar[no]) {
		*vars = 0;
		return 1;
	}

	int count = 0;
	for (const char *p = strVar[no]; *p; p++, count++) {
		vars[count] = *p & 0xff;
		if (CHECKSJIS1BYTE(*p) && *(p + 1))
			vars[count] |= (*++p & 0xff) << 8;
	}
	vars[count++] = 0;
	return count;
}

#ifdef DEBUG

void debug_showvariable() {
	int i,j,k;
	int *var;
	FILE *fp = fopen("VARIABLES.TXT","a");
	if (fp == NULL) return;

	fprintf(fp, "Page = %d, index = %x\n", sl_getPage(), sl_getIndex());

	var = &sysVar[0];
	fprintf(fp, "sysVar\n");
	for (i = 0; i < SYSVAR_MAX; i+=10) {
		for (j = 0; j < 10; j++) {
			fprintf(fp, "%d,", *var); var++;
		}
		fprintf(fp, "\n");
	}

	for (i = 0; i < ARRAYVAR_PAGEMAX; i++) {
		if (arrayVarBuffer[i].value != NULL) {
			fprintf(fp, "ArrayPage[%d],size=%d\n",i,arrayVarBuffer[i].size);
			var = arrayVarBuffer[i].value;
			for (j = 0; j < arrayVarBuffer[i].size; j+=10) {
				for (k = 0; k < 10; k++) {
					fprintf(fp, "%d,", *var); var++;
				}
				fprintf(fp, "\n");
			}
		}
	}

	fclose(fp);
}

#endif
