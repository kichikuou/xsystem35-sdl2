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

static int buf[CALI_DEPTH_MAX]; /* 計算式バッファ */
static int *cali = buf;         /* インデックス */

/* 変数番号を返す */
static int *getVar(int c0) {
	if ((c0 & 0x40) == 0)
		return v_ref(c0 & 0x3f);  // 0 - 0x3f

	int c1 = sl_getc();
	if (c0 != 0xc0)
		return v_ref(((c0 & 0x3f) * 256) + c1);  // 0x100 - 0x3fff

	if (c1 == 1) {
		c0 = sl_getc();
		c1 = sl_getc();
		int index = getCaliValue();
		return v_ref_indexed(c0 << 8 | c1, index);
	} else if (c1 >= 0x40) {
		return v_ref(c1);  // 0x40 - 0xff
	}
	SYSERROR("Invalid variable reference at %d:0x%x", sl_getPage(), sl_getIndex() - 2);
	return NULL;
}

/* 変数番号が返る */
int *getCaliVariable() {
	int *c0 = getVar(sl_getc());
	if (sl_getc() != 0x7f) {
		SYSERROR("Something is Wrong @ %03d:%05x", sl_getPage(), sl_getIndex());
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
					SYSERROR("Unknow Parameter @ %03d:%05x", sl_getPage(), sl_getIndex());
				}
			}
		l_var:
			t = getVar(c0);
			*cali++ = t ? *t : 0;
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
							SYSERROR("Unknown Parameter @ %03d:%05x", sl_getPage(), sl_getIndex());
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

	if (cali != bufc) {
		WARNING("Something is wrong @ %03d:%05x\n", sl_getPage(), sl_getIndex());
		cali = bufc;
		return 0;
	}

	return c0;
}
