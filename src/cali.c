/*
 * cali.c  計算式の評価
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
 *
 *          0.00    97/11/27 初版
 *          0.01    97/12/06 計算式の評価順序がおかしかった
 * @version 0.01-01 97/12/06 メッセージから日本語を削除
 *
*/
/* $Id: cali.c,v 1.16 2002/12/31 04:11:19 chikama Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include "portab.h"
#include "variable.h"
#include "scenario.h"
#include "xsystem35.h"

#define CALI_TRUE       (1)            /* comparison is ture  */
#define CALI_FALSE      (0)            /* comparison is false */
#define CALI_MAX_VAL    (65535)        /* maxinum of value    */
#define CALI_MIN_VAL    (0)            /* mininum of value    */
#define CALI_NaN        (CALI_MIN_VAL) /* devied by 0         */
#define CALI_OF         (CALI_MAX_VAL) /* calculate over flow */
#define CALI_SubNG      (CALI_MIN_VAL) /* nagative result     */
#define CALI_TERMINATER (0x7f)         /* terminater          */

#define CALI_DEPTH_MAX  (256)

int getCaliValue();
int *getCaliVariable();
int *getVariable();

int preVarPage;      /* 直前にアクセスした変数のページ */
int preVarIndex;     /* 直前にアクセスした変数のINDEX */
int preVarNo;        /* 直前にアクセスした変数の番号 */

static int buf[CALI_DEPTH_MAX]; /* 計算式バッファ */
static int *cali = buf;         /* インデックス */

static void undeferr() {
	SYSERROR("Undefined Command:@ %03d,%05x\n", sl_getPage(), sl_getIndex());
}

/* 配列のオフセットを確定する */
static int *fixOffset(int base, int offset2) {
	int page, offset;
	int *index;
	
	preVarPage = (sysVarAttribute + base) -> page;
	preVarNo   = base;
	if ((sysVarAttribute + base) -> page == 0) {
		if (offset2 == -1) {
#ifdef DEBUG_CHECKALING
			if (base >= SYSVAR_MAX) {
				WARNING("sysVar[no] ArrayIndexOutofException (%d, %d)\n", base, offset2);
				return NULL;
			}
#endif
			preVarIndex = base;
			return sysVar + base;
		} else {
#ifdef DEBUG_CHECKALING
			if ((base + offset2) >= SYSVAR_MAX) {
				WARNING("sysVar[no] ArrayIndexOutofException (%d, %d)\n", base, offset2);
				return NULL;
			}
#endif
			preVarIndex = base + offset2;
			return sysVar + base + offset2;
		}
	} else {
		if (offset2 == -1) {
			index  = (sysVarAttribute + base) -> pointvar;
			page   = (sysVarAttribute + base) -> page;
			offset = (sysVarAttribute + base) -> offset;
#ifdef DEBUG_CHECKALING
			if ((*index) + offset > (arrayVarBuffer + page - 1)->max) {
				WARNING("sysVar[no] ArrayIndexOutofException (%d, %d, %d, %d, %d)\n", base, offset2, *index, page, offset);
				return NULL;
			}
#endif
			preVarIndex = offset + (*index);
			return ((arrayVarBuffer + page - 1) -> value) + offset + (*index);
		} else {
			page   = (sysVarAttribute + base) -> page;
			offset = (sysVarAttribute + base) -> offset;
#ifdef DEBUG_CHECKALING
			if (offset + offset2 > (arrayVarBuffer + page - 1)->max) {
				WARNING("sysVar[no] ArrayIndexOutofException (%d, %d, %d, %d)\n", base, offset2, page, offset);
				return NULL;
			}
#endif
			preVarIndex = offset + offset2;
			return ((arrayVarBuffer + page - 1) -> value) + offset + offset2;
		}
	}
}

/* 変数番号を返す */
static int *getVar(int c0) {
	int c1;
	
	if ((c0 & 0x40) != 0) { /* 2byte系 */
		c1 = sl_getc();
		if (c0 == 0xc0) {
			if (c1 == 0) {
				SYSERROR("Unknown Parameter\n");
			} else if (c1 == 1) {
				/* SYSTEM35拡張 */
				c0 = (sl_getc() << 8) + sl_getc();  /* varbase */
				c1 = getCaliValue();                /* offset */
				return fixOffset(c0, c1);
			} else if (c1 < 0x40) {
				WARNING("Unknown Parameter\n");
				undeferr();
			} else { /* 2byte系 40h - ffh */
				return fixOffset(c1, -1);
			}
		} else { /* 2byte系 100h - 3fffh */
			return fixOffset(((c0 & 0x3f) * 256) + c1, -1);
		}
	} else { /* 1byte系 */
		return fixOffset(c0 & 0x3f, -1);
	}
	/* 来ないはず */
	SYSERROR("Something was wrong\n");
	return NULL;
}

/* 変数番号が返る */
int *getCaliVariable() {
	int *c0 = getVar(sl_getc());
	if (sl_getc() != 0x7f) {
		SYSERROR("Something is Wrong @ %03d:%05x\n", sl_getPage(), sl_getIndex());
	}
	return c0;
}

/* 変数代入コマンド用 */
int *getVariable() {
	return getVar(sl_getc());
}

/* 計算式の評価後の値が返る */
int getCaliValue() {
	register int ingVal,edVal,rstVal;
	int c0,c1;
	int *bufc = cali;
	
	while((c0 = sl_getc()) != CALI_TERMINATER) {
		if ((c0 & 0x80) != 0) { /* variable */
			int c1 = sl_getcAt(sl_getIndex());
			int *t;
			if (c0 == 0xc0) {
				if (c1 == 1 || c1 >= 0x34) goto l_var;
				c1 = sl_getc();
				if (c1 == 2) { /* % */
					ingVal = *--cali;
					edVal  = *--cali;
					if (ingVal == 0) {
						*cali = CALI_NaN;
					} else {
						rstVal = edVal % ingVal;
						*cali = rstVal;
					}
					cali++;
					continue;
				} else if (c1 == 3) { /* <= */
					ingVal = *--cali;
					edVal  = *--cali;
					if (edVal <= ingVal) { *cali = CALI_TRUE;  }
					else                 { *cali = CALI_FALSE; }
					cali++;
					continue;
				} else if (c1 == 4) { /* >= */
					ingVal = *--cali;
					edVal  = *--cali;
					if (edVal >= ingVal) { *cali = CALI_TRUE;  }
					else                 { *cali = CALI_FALSE; }
					cali++;
					continue;
				} else if (c1 < 0x34) {
					SYSERROR("Unknow Parameter @ %03d:%05x\n", sl_getPage(), sl_getIndex());
				}
			}
		l_var:
			t = getVar(c0);
			if (t == NULL) continue;
			*cali = *t; cali++;
		} else {
			switch(c0) {
			case 0x7e: /* != */
				ingVal = *--cali;
				edVal  = *--cali;
				if (edVal != ingVal) { *cali = CALI_TRUE;  }
				else                 { *cali = CALI_FALSE; }
				cali++;
				break;
			case 0x7d: /* > */
				ingVal = *--cali;
				edVal  = *--cali;
				if (edVal > ingVal) { *cali = CALI_TRUE;  }
				else                { *cali = CALI_FALSE; }
				cali++;
				break;
			case 0x7c: /* < */
				ingVal = *--cali;
				edVal  = *--cali;
				if (edVal < ingVal) { *cali = CALI_TRUE;  }
				else                { *cali = CALI_FALSE; }
				cali++;
				break;
			case 0x7b: /* == */
				ingVal = *--cali;
				edVal  = *--cali;
				if (edVal == ingVal) { *cali = CALI_TRUE;  }
				else                 { *cali = CALI_FALSE; }
				cali++;
				break;
			case 0x7a: /* - */
				ingVal = *--cali;
				edVal  = *--cali;
				rstVal = edVal - ingVal;
				if (rstVal < CALI_MIN_VAL) { *cali = CALI_SubNG; }
				else                       { *cali = rstVal;     }
				cali++;
				break;
			case 0x79: /* + */
				ingVal = *--cali;
				edVal  = *--cali;
				rstVal = edVal + ingVal;
				if (rstVal > CALI_MAX_VAL) { *cali = CALI_OF; }
				else                       { *cali = rstVal;  }
				cali++;
				break;
			case 0x78: /* / */
				ingVal = *--cali;
				edVal  = *--cali;
				if (ingVal == 0) {
					*cali = CALI_NaN;
				} else {
					rstVal = edVal / ingVal;
					*cali = rstVal;
				}
				cali++;
				break;
			case 0x77: /* * */
				ingVal = *--cali;
				edVal  = *--cali;
				rstVal = edVal * ingVal;
				if (rstVal > CALI_MAX_VAL) { *cali = CALI_OF; }
				else                       { *cali = rstVal;  }
				cali++;
				break;
			case 0x76: /* XOR */
				ingVal = *--cali;
				edVal  = *--cali;
				*cali = (edVal ^ ingVal); cali++;
				break;
			case 0x75:  /* OR */
				ingVal = *--cali;
				edVal  = *--cali;
				*cali = (edVal | ingVal); cali++;
				break;
			case 0x74: /* AND */
				ingVal = *--cali;
				edVal  = *--cali;
				*cali = (edVal & ingVal); cali++;
				break;
			default:
				if ((c0 & 0x40) == 0) { /* WORD const */
					c1 = sl_getc();
					if (c0 == 0) { /* 34h-ffh */
						if (c1 <= 0x33) {
							SYSERROR("Unknown Parameter @ %03d:%05x\n", sl_getPage(), sl_getIndex());
						}
					} else { /* 100h- 3fff */
						c1 += ((c0 & 0x3f) * 256);
					}
				} else { /* byte const 0-33h */
					c1 = (c0 & 0x3f);
				}
				// printf("c1 = %d ",c1);
				*cali = c1; cali++;
			}
		}
	}
	c0 = *--cali;
#ifdef DEBUG_CHECKALING
	if (cali != bufc) {
		WARNING("Something is wrong @ %03d:%05x\n", sl_getPage(), sl_getIndex());
		cali = bufc;
		return 0;
	}
#endif
	return c0;
}
