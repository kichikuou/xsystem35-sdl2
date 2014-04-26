/*
 * scenario.c  シナリオ管理実行
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
/* $Id: scenario.c,v 1.30 2003/04/25 17:23:55 chikama Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "portab.h"
#include "ald_manager.h"
#include "variable.h"
#include "scenario.h"
#include "LittleEndian.h"
#include "xsystem35.h"

/* stack data type */
#define STACK_NEARJMP 1
#define STACK_FARJMP 2
#define STACK_VARIABLE 3
#define STACK_DATA 4

/* static mathods */
static void popVars(int *tmp);
static void popDatas(int *tmp);
static void showStackInfo();
static void sl_push(int type, int *val, int cnt);
static int* sl_pop();

/* static variables */
static BYTE *sl_sco;              /* scenario data */ 
static int  *sco_stackbuf;        /* stack data    */
static int  *sco_stackindex;      /* stack index   */
static int   sco_stacksize = 1024; /* default stack size */

static int   sl_page;              /* current scenario page no */
static int   sl_index;             /* cureent scenario address in page */

static int labelCallCnt = 0;
static int pageCallCnt  = 0;
static int labelCallCnt_afterPageCall = 0;
static int dataPushCnt = 0;

/* driobject */
static dridata *dfile;

/* debug: show stack information */
static void showStackInfo() {
	DEBUG_MESSAGE("stack top = %p, cur = %p, size=%d\n", sco_stackbuf, sco_stackindex, sco_stacksize);
}

/* debbug: show current scenario pointer information */
static void showIndexInfo() {
	DEBUG_MESSAGE("page = %d, index = %x\n", sl_page, sl_index);
}

#ifdef DEBUG
static void showStackData() {
	int i, j;
	int *var;
	FILE *fp = fopen("STACKS.TXT","a");
	if (fp == NULL) return;
	
	fprintf(fp, "Page = %d, index = %x\n", sl_getPage(), sl_getIndex());
	fprintf(fp, "stack top = %p, cur = %p, size=%d, index=%d\n", sco_stackbuf, sco_stackindex, sco_stacksize, (sco_stackindex - sco_stackbuf));
	
	var = sco_stackbuf;
	for (i = 0; i < sco_stacksize; i+=10) {
		for (j = 0; j < 10; j++) {
			fprintf(fp, "%d,", *var); var++;
		}
		fprintf(fp, "\n");
	}
	fprintf(fp, "\n");

	fclose(fp);
}
#endif

/* initilize stack and load first scenario data */
boolean sl_init() {
	sco_stackbuf = calloc(sco_stacksize, sizeof(int));
	if (sco_stackbuf == NULL)
		NOMEMERR();
	
	sco_stackindex = sco_stackbuf;
	sl_jmpFar(0);

	labelCallCnt = 0;
	pageCallCnt = 0;
	labelCallCnt_afterPageCall = 0;
	return TRUE;
}

/* UD 0 command, reinitilized scenario loader */
boolean sl_reinit() {
	free(sco_stackbuf);
	return sl_init();
}

static void sl_push(int type, int *val, int cnt) {
	int i;

	if (sco_stackindex + cnt >= sco_stackbuf + sco_stacksize) {
		i = sco_stackindex - sco_stackbuf;
		sco_stacksize <<= 1;
		sco_stackbuf    = realloc(sco_stackbuf, sco_stacksize * sizeof(int));
		if (sco_stackbuf == NULL)
			NOMEMERR();
		sco_stackindex = sco_stackbuf + i;
	}
	val += (cnt - 1);
	for (i = 0 ; i < cnt; i++) {
		*sco_stackindex++ = *val--;
	}
	*sco_stackindex++ = cnt;
	*sco_stackindex++ = type;
	// showStackInfo();
	// showStackData();
}

static int* sl_pop() {
	int i, type, cnt;
	int *tmp, *_tmp;
	
	type = *--sco_stackindex;
	cnt  = *--sco_stackindex;
	
	tmp  = _tmp = malloc(sizeof(int) * (cnt + 2));
	if (_tmp == NULL)
		NOMEMERR();
	
	*tmp++ = type;
	*tmp++ = cnt;
	for (i = 0; i < cnt; i++) {
		*tmp++ = *--sco_stackindex;
	}
	if (sco_stackindex < sco_stackbuf) {
		SYSERROR("Stack buffer is illegal\n");
	}
	
	// showStackInfo();
	// showStackData();
	return _tmp;
}

int sl_getc() {
	return sl_sco[sl_index++];
}

int sl_getw() {
	int c0,c1;

	c0 = sl_getc();
	c1 = sl_getc();
	return c0 + (c1 << 8);
}

int sl_getcAt(int adr) {
	return sl_sco[adr];
}

int sl_getwAt(int adr) {
	int c0,c1;
	
	c0 = sl_sco[adr];
	c1 = sl_sco[adr + 1];
	return c0 + (c1 << 8);
}

int sl_getdAt(int adr) {
	int c0, c1;
	
	c0 = sl_getwAt(adr);
	c1 = sl_getwAt(adr + 2);
	return c0 + (c1 << 16);
}

int sl_getadr() {
	int c0,c1;
	c0 = sl_getw();
	c1 = sl_getw();
	return c0 + (c1 << 16);
}

/* @address */
void sl_jmpNear(int address) {
	sl_index = address;
//	showIndexInfo();
}

/*
   #page
   page = 0~
*/
boolean sl_jmpFar(int page) {
	if (dfile) {
		ald_freedata(dfile);
	}
	
	dfile = ald_getdata(DRIFILE_SCO, page);
	if (dfile == NULL) {
		return FALSE;
	}
		
	sl_sco   = dfile->data;
	sl_page  = page;
	sl_index = LittleEndian_getDW(sl_sco, 4);
//	showIndexInfo();
	return TRUE;
}

/*
  ~page,address
   page = 1~
*/
boolean sl_jmpFar2(int page, int address) {
	if (dfile) {
		ald_freedata(dfile);
	}
	
	dfile = ald_getdata(DRIFILE_SCO, page);
	if (dfile == NULL) {
		DEBUG_MESSAGE("ald_getdata fail\n");
		return FALSE;
	}
	sl_sco   = dfile->data;
	sl_page  = page;
	sl_index = address;
	return TRUE;
}

void sl_callNear(int address) {
	int val[1];
	val[0] = sl_index;
	sl_push(STACK_NEARJMP, val, 1);
	sl_jmpNear(address);
	
	labelCallCnt++;
	labelCallCnt_afterPageCall++;
}

void sl_retNear() {
	int *tmp = sl_pop();
	int index;
	
	while (*tmp != STACK_NEARJMP) {
		if (*tmp == STACK_FARJMP) {
			SYSERROR("Stack buffer is illegal\n");
		} else if (*tmp == STACK_VARIABLE) {
			popVars(tmp);
		} else {
			SYSERROR("Stack buffer is illegal\n");
		}
		free(tmp);
		tmp = sl_pop();
	}
	index = *(tmp + 2);
	free(tmp);
	sl_jmpNear(index);
	
	labelCallCnt--;
	labelCallCnt_afterPageCall--;
}

void sl_callFar(int page) {
	int val[2];
	boolean bool;

	val[0] = sl_page;
	val[1] = sl_index;
	sl_push(STACK_FARJMP, val, 2);
	bool = sl_jmpFar(page);
	pageCallCnt++;
	labelCallCnt_afterPageCall = 0;
	if (!bool) {
		sl_retFar();
	}
}

void sl_callFar2(int page, int address) {
	int val[2];
	boolean bool;
	
	val[0] = sl_page;
	val[1] = sl_index;
	sl_push(STACK_FARJMP, val, 2);
	bool = sl_jmpFar2(page, address);
	labelCallCnt_afterPageCall = 0;
	pageCallCnt++;
	if (!bool) {
		sl_retFar();
	}
}

void sl_retFar() {
	int *tmp = sl_pop();
	int page, index;

	while (*tmp != STACK_FARJMP) {
		if (*tmp == STACK_NEARJMP) {
			WARNING("Stack buffer is illegal\n");
		} else if (*tmp == STACK_DATA) {
			popDatas(tmp);
		} else if (*tmp == STACK_VARIABLE) {
			popVars(tmp);
		} else {
			SYSERROR("Stack buffer is illegal\n");
		}
		free(tmp);
		tmp = sl_pop();
	}
	page  = *(tmp + 2);
	index = *(tmp + 3);
	free(tmp);
	sl_jmpFar2(page, index);
	
	labelCallCnt_afterPageCall = 0;
	pageCallCnt--;
}

/* UD 1 */
void sl_retFar2() {
	int *tmp = sl_pop();
	int page, index;

	while (*tmp != STACK_FARJMP) {
		if (*tmp == STACK_NEARJMP) {
			labelCallCnt--;
		} else if (*tmp == STACK_DATA) {
			popDatas(tmp);
		} else if (*tmp == STACK_VARIABLE) {
			popVars(tmp);
		} else {
			SYSERROR("Stack buffer is illegal\n");
		}
		free(tmp);
		tmp = sl_pop();
	}
	page  = *(tmp + 2);
	index = *(tmp + 3);
	free(tmp);
	sl_jmpFar2(page, index);
	
	pageCallCnt--;
	labelCallCnt_afterPageCall = 0;
}

/* UC0 */
void sl_stackClear_allCall() {
	free(sco_stackbuf);
	sco_stackbuf = calloc(sco_stacksize, sizeof(int));
	if (sco_stackbuf == NULL)
		NOMEMERR();

	sco_stackindex = sco_stackbuf;
}

/* UC 2 */
void sl_stackClear_pageCall(int cnt) {
	int *tmp;
	
	while(cnt--) {
		tmp = sl_pop();
		if (*tmp == STACK_NEARJMP) {
			labelCallCnt--;
		} else if (*tmp == STACK_FARJMP) {
			pageCallCnt--;
		} else if (*tmp == STACK_VARIABLE) {
			popVars(tmp);
		} else {
			SYSERROR("Stack buffer is illegal\n");
		}
		free(tmp);
	}
	labelCallCnt_afterPageCall = 0;
}

/* UC 1 */
void sl_stackClear_labelCall(int cnt) {
	int *tmp;
	
	if (labelCallCnt_afterPageCall == 0) return;
	
	tmp = sl_pop();
	while(cnt--) {
		if (*tmp == STACK_NEARJMP) {
			labelCallCnt--;
			labelCallCnt_afterPageCall--;
		} else if (*tmp == STACK_VARIABLE) {
			popVars(tmp);
		} else {
			SYSERROR("Stack buffer is illegal\n");
		}
		free(tmp);
		if (labelCallCnt_afterPageCall == 0) break;
		
		tmp = sl_pop();
	}
}

/* US */
void sl_pushVar(int *topVar, int cnt) {
	int *tmp = malloc(sizeof(int) * (cnt + 2));

	if (tmp == NULL) {
		NOMEMERR();
	}
	
	*tmp     = preVarPage;
	*(tmp+1) = preVarIndex;
	memcpy(tmp + 2, topVar, sizeof(int) * cnt);
	sl_push(STACK_VARIABLE, tmp, cnt + 2);
	free(tmp);
}

/* UG */
void sl_popVar(int *topvar, int cnt) {
	int *tmp = sl_pop();

	if (*tmp != STACK_VARIABLE) {
		SYSERROR("Stack buffer is illegal\n");
	}
	if (*(tmp + 2) != preVarPage){
		WARNING("Variable is not match with stacked variable\n");
	}
	if (*(tmp + 3) != preVarIndex) {
		WARNING("Variable is not match with stacked variable\n");
	}
	if (*(tmp + 1) != cnt + 2) {
		WARNING("Variable count is not match with stacked variable\n");
	}
	memcpy(topvar, tmp + 4, sizeof(int) * cnt);
	free(tmp);
}

static void popVars(int *tmp) {
	int cnt, page, index;
	int *topVar;
	
	cnt   = *(tmp + 1) - 2;
	page  = *(tmp + 2);
	index = *(tmp + 3);

	if (page == 0) {
		topVar = sysVar + index;
	} else {
		if ((arrayVarBuffer + page - 1) -> value == NULL) {
			WARNING("Illegal Variable pop\n");
			return;
		}
		topVar = ((arrayVarBuffer + page - 1) -> value) + index;
	}
	
	memcpy(topVar, tmp + 4, sizeof(int) * cnt); 
}

int sl_getIndex() {
	return sl_index;
}

int sl_getPage() {
	return sl_page;
}

int *sl_getStackInfo(int *size) {
	*size = sco_stackindex - sco_stackbuf;
//	printf("get stack size = %d\n", *size);
	return sco_stackbuf;
}

void sl_putStackInfo(int *data, int size) {
	if (size > sco_stacksize) {
		sco_stacksize = size << 1;
		sco_stackbuf  = calloc(sco_stacksize, sizeof(int));
	}
	if (sco_stackbuf == NULL)
		NOMEMERR();
	memcpy(sco_stackbuf, data, size * sizeof(int));
	sco_stackindex = sco_stackbuf + size;
}


/* TPx */
void sl_pushData(int *data, int cnt) {
	dataPushCnt++;
	sl_push(STACK_DATA, data, cnt);
}

/* TOx */
void sl_popData(int *data, int cnt) {
	int *tmp;
	if (dataPushCnt == 0) return;
	
	tmp = sl_pop();
	dataPushCnt--;
	
	if (*tmp != STACK_DATA)
		SYSERROR("Stack buffer is illegal\n");
	if (*(tmp + 1) != cnt)
		WARNING("Variable count is not match with stacked variable\n");
	
	memcpy(data, tmp + 2, sizeof(int) * cnt);
	free(tmp);
}

static void popDatas(int *tmp) {
	int d1 = *(tmp + 2);
	int d2 = *(tmp + 3);
	int d3 = *(tmp + 4);

	if (*(tmp + 1) != 3)
		WARNING("Variable count is not match with stacked variable\n");
	
	switch(d1) {
	case TxxTEXTCOLOR:
		if (d2 == 0) {
			nact->msg.MsgFontColor = d3;
		} else {
			nact->sel.MsgFontColor = d3;
		}
		break;
	case TxxTEXTSIZE:
		if (d2 == 0) {
			nact->msg.MsgFontSize = d3;
		} else {
			nact->sel.MsgFontSize = d3;
		}
		break;
	case TxxTEXTLOC:
		msg_setMessageLocation(d2, d3);
		break;
	}
	dataPushCnt--;
}

static dridata *datatbl;

void *sl_setDataTable(int page, int index) {
	if (datatbl) {
		ald_freedata(datatbl);
	}
	datatbl = ald_getdata(DRIFILE_SCO, page);
	return (void *)(datatbl->data + index);
}

void sl_returnGoto(int address) {
	int *tmp = sl_pop();
	int page;

	while (TRUE) {
		if (*tmp != STACK_FARJMP || *tmp != STACK_NEARJMP) break;
		if (*tmp == STACK_DATA) {
			popDatas(tmp);
		} else if (*tmp == STACK_VARIABLE) {
			popVars(tmp);
		} else {
			fprintf(stderr, "%d \n", *tmp);
			SYSERROR("Stack buffer is illegal\n");
		}
		free(tmp);
		tmp = sl_pop();
	}

	if (*tmp == STACK_FARJMP) {
		page  = *(tmp + 2);
		// index = *(tmp + 3);
		free(tmp);
		sl_jmpFar2(page, address);
		labelCallCnt_afterPageCall = 0;
		pageCallCnt--;
	} else {
		// index = *(tmp + 2);
		free(tmp);
		sl_jmpNear(address);
		
		labelCallCnt--;
		labelCallCnt_afterPageCall--;
	}
}
