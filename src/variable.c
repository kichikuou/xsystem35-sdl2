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
#include "variable.h"
#include "xsystem35.h"

/* システム変数 */
int *sysVar;
/* 配列変数の情報 */
arrayVarStruct *sysVarAttribute;
/* 配列本体 */
arrayVarBufferStruct *arrayVarBuffer;
/* 64bit変数 */
double longVar[SYSVARLONG_MAX];
/* 文字列変数 */
static char *strVar;
/* 文字列変数の属性(最大,1つあたりの大きさ) */
int strvar_cnt = STRVAR_MAX;
int strvar_len = STRVAR_LEN;


/* 配列バッファの確保 DC ,page = 1~ */
extern boolean v_allocateArrayBuffer(int page, int size, boolean flg) {
	void *buf;
	
	if (page <= 0 || page > 256)   { return false; }
	if (size <= 0 || size > 65536) { return false; }
	
	if (NULL == (buf = malloc(size * sizeof(int) + 257))) {
		NOMEMERR();
	}
	memset(buf, 0, size * sizeof(int) + 257);
	if (arrayVarBuffer[page - 1].value != NULL) {
		int len = min(size, arrayVarBuffer[page - 1].max);
		memcpy(buf, arrayVarBuffer[page - 1].value, len * sizeof(int));
	}
	
	arrayVarBuffer[page - 1].value    = buf;
	arrayVarBuffer[page - 1].max      = size;
	arrayVarBuffer[page - 1].saveflag = flg;
	
	return true;
}

/*　配列変数の割り当て DS */
extern boolean v_defineArrayVar(int datavar, int *pointvar, int offset, int page) {
	if (datavar < 0 || datavar > SYSVAR_MAX - 1)                   { return false; }
	if (page    < 0 || page    > ARRAYVAR_PAGEMAX - 1)             { return false; }
	if (offset  < 0 || offset  > arrayVarBuffer[page - 1].max - 1) { return false; }
	
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

/* 指定ページの最大変数の取得 page = 1~ */
extern int v_getArrayBufferCnt(int page) {
	return arrayVarBuffer[page - 1].max;
}

/* 指定ページは使用中 page = 1~ */
extern boolean v_getArrayBufferStatus(int page) {
	return (arrayVarBuffer[page - 1].value != NULL) ? true : false;
}

/* 文字列変数の再初期化 */
extern void v_initStringVars(int cnt,int len) {
	strVar = realloc(strVar, cnt * len);
	if (strVar == NULL) {
		NOMEMERR();
	}
	strvar_cnt = cnt;
	strvar_len = len;
}

/* 変数の初期化 */
extern boolean v_initVars() {
	sysVar          = calloc(SYSVAR_MAX, sizeof(int));
	sysVarAttribute	= calloc(SYSVAR_MAX, sizeof(arrayVarStruct));
	arrayVarBuffer  = calloc(ARRAYVAR_PAGEMAX, sizeof(arrayVarBufferStruct));
	strVar          = calloc(STRVAR_MAX, STRVAR_LEN);
	
	if (strVar == NULL || sysVar == NULL || sysVarAttribute == NULL || arrayVarBuffer == NULL) {
		NOMEMERR();
	}
	
	return true;
}

/* 文字変数への代入 */
char *v_strcpy(int no, const char *str) {
	return strncpy(strVar + no * strvar_len, str, strvar_len - 1);
}

/* 文字変数への接続 */
char *v_strcat(int no, const char *str) {
	return strncat(strVar + no * strvar_len, str, strvar_len - 1);
}

/* 文字変数の長さ */
size_t v_strlen(int no) {
	return strlen(strVar + no * strvar_len);
}

/* 文字変数そのもの */
char *v_str(int no) {
	return strVar + no * strvar_len;
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
			fprintf(fp, "ArrayPage[%d],max=%d\n",i,arrayVarBuffer[i].max); 
			var = arrayVarBuffer[i].value;
			for (j = 0; j < arrayVarBuffer[i].max; j+=10) {
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
