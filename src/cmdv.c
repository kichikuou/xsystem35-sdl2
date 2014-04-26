/*
 * cmdv.c  SYSTEM35 V command
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
/* $Id: cmdv.c,v 1.21 2001/05/09 04:11:24 chikama Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include "portab.h"
#include "xsystem35.h"
#include "ags.h"
#include "imput.h"

extern void sys_set_signalhandler(int SIG, void (*handler)(int));

typedef struct {
	int x0Unit;
	int y0Unit;
	int nxUnit;
	int nyUnit;
	int bSpCol;
	boolean fEnable;
	boolean useTTP;
	int TTPunit;
} UnitMapSrcImg;

typedef struct {
	int unitWidth;     /* Unitの大きさ */
	int unitHeight;
	int patternNum;    /* パターン数 */
	int intervaltime;  /* 書換え間隔 */
	int srcX;          /* 取得位置 */
	int srcY;
	int startX;        /* 表示位置 */
	int startY;
	int endX;          /* 移動先 */
	int endY;
	int saveX;         /* 背景退避位置 */
	int saveY;
	int spType;        /* スプライト方法 */
	int spCol;         /* スプライト色 */
	int state;         /* 現在の状態  0:停止 1:動 */
	int elaspCut;      /* 経過コマ数 */
	int quantmsec;     /* 経過秒数 */
	int totalCut;      /* 全コマ数 */
	int preX;          /* 前回の位置 */
	int preY;
	int curX;          /* 現在位置 */
	int curY;
	boolean draw;      /* UNITを描く？ */
	boolean nomove;    /* 移動あり・なし */
	boolean rewrite;   /* 画面更新の必要あり */
} VaParam;

typedef struct {
	int x;
	int y;
} VhMark;

#define UNITMAP_DISPLAY_PAGE_MAX       16  /* VSの最大ページ数 */
#define UNITMAP_VARIABLE_IMMOVALE     255  /* 歩数ペイントの移動不可能マーク */
#define UNITMAP_VARIABLE_OUTOFRANGE 65535  /* 範囲外のマーク */
#define UNITMAP_ATTRIB_DEPTH      (4)
#define UNITMAP_ATTRIB_UNITNUMBER (0)
#define UNITMAP_ATTRIB_VARIABLE   (1)
#define UNITMAP_ATTRIB_WALKPAINT  (2)
#define UNITMAP_ATTRIB_WALKRESULT (3)

/* UnitMAP 全体へのポインタ */
static int *UnitMap = NULL;
/* VC command */
static int nPageNum;
static int x0Map;
static int y0Map;
static int cxMap;
static int cyMap;
static int cxUnit;
static int cyUnit;
/* VH command */
static VhMark *vh_src, *vh_dst, *_vh_src, *_vh_dst;
static int     vh_cnt_src, vh_cnt_dst;
/* VP command */
static UnitMapSrcImg *srcimg;
/* VA command state */
#define VA_STOPPED     0
#define VA_RUNNING     1
#define VACMD_MAX 20                      /* Panyoで18まで */
static  VaParam VAcmd[VACMD_MAX];         
static  boolean inAnimation      = FALSE; /* 画面更新中 */

/* UnitMap 各種マクロ */
#define MAPSIZE_PER_ATTRIB (cxMap * cyMap)
#define MAPSIZE_PER_PAGE   ((MAPSIZE_PER_ATTRIB) * UNITMAP_ATTRIB_DEPTH)

#define UNITMAP_UNITNUMBER(page,x,y) (UnitMap + (page) * (MAPSIZE_PER_PAGE) + UNITMAP_ATTRIB_UNITNUMBER * (MAPSIZE_PER_ATTRIB) + (y) * cxMap + (x))
#define UNITMAP_VARIABLE(page,x,y)   (UnitMap + (page) * (MAPSIZE_PER_PAGE) + UNITMAP_ATTRIB_VARIABLE *   (MAPSIZE_PER_ATTRIB) + (y) * cxMap + (x))
#define UNITMAP_WALKPAINT(page,x,y)  (UnitMap + (page) * (MAPSIZE_PER_PAGE) + UNITMAP_ATTRIB_WALKPAINT *  (MAPSIZE_PER_ATTRIB) + (y) * cxMap + (x))
#define UNITMAP_WALKRESULT(page,x,y) (UnitMap + (page) * (MAPSIZE_PER_PAGE) + UNITMAP_ATTRIB_WALKRESULT * (MAPSIZE_PER_ATTRIB) + (y) * cxMap + (x))

#define UNITMAP_UNITNUMBER_PAGETOP(page) UNITMAP_UNITNUMBER((page),0,0)
#define UNITMAP_VARIABLE_PAGETOP(page)   UNITMAP_VARIABLE((page),0,0)
#define UNITMAP_WALKPAINT_PAGETOP(page)  UNITMAP_WALKPAINT((page),0,0)
#define UNITMAP_WALKRESULT_PAGETOP(page) UNITMAP_WALKRESULT((page),0,0)

static boolean vh_checkImmovableArea(int page, int x, int y, int w, int h);
static void    vh_append_pos(int x, int y);
static void    vh_copy_to_src();
static void    vh_check_udlr(int n, int nPage, int x, int y);
static void    va_drawUnit(int no);
static void    va_restoreUnit(int no);
static void    va_updateUnit(int i);
static void    va_updatePreArea(int i);
static void    va_animationAlone(int i);
static void    va_interval_process();
static void    va_init_itimer();
static void    va_pause_itimer();
static void    va_unpause_itimer();
static void    alarmHandler();

void commandVC() { /* from Rance4 */
	nPageNum = getCaliValue();
	x0Map    = getCaliValue();
	y0Map    = getCaliValue();
	cxMap    = getCaliValue();
	cyMap    = getCaliValue();
	cxUnit   = getCaliValue();
	cyUnit   = getCaliValue();
	
	DEBUG_COMMAND("VC %d,%d,%d,%d,%d,%d,%d:\n",nPageNum, x0Map, y0Map, cxMap, cyMap, cxUnit, cyUnit);
	
	if (NULL != UnitMap ) {
		free(UnitMap);
		free(srcimg);
		free(_vh_src);
		free(_vh_dst);
	}
	
	if (nPageNum > UNITMAP_DISPLAY_PAGE_MAX) {
		WARNING("VC nPageNum too big %d\n", nPageNum);
		sysVar[0] = 0;
		return;
	}
	
	UnitMap = (int *)calloc(cxMap * cyMap * nPageNum * UNITMAP_ATTRIB_DEPTH, sizeof(int));
	srcimg  = (UnitMapSrcImg *)calloc(nPageNum, sizeof(UnitMapSrcImg));
	
	if (NULL == UnitMap || NULL == srcimg) {
		NOMEMERR();
	}
	
	_vh_src = (VhMark *)calloc(3 * (cxMap + cyMap), sizeof(VhMark));
	_vh_dst = (VhMark *)calloc(3 * (cxMap + cyMap), sizeof(VhMark));
	if (NULL == _vh_src || NULL == _vh_dst) {
		NOMEMERR();
	}
	sysVar[0] = 1;
}

void commandVP() { /* from T2 */
	int nPage  = getCaliValue();
	int x0Unit = getCaliValue();
	int y0Unit = getCaliValue();
	int nxUnit = getCaliValue();
	int nyUnit = getCaliValue();
	int bSpCol = getCaliValue();
	
	DEBUG_COMMAND("VP %d,%d,%d,%d,%d,%d:\n",nPage, x0Unit, y0Unit, nxUnit, nyUnit, bSpCol);
	
	if (nPage >= nPageNum) {
		WARNING("VP nPage too large %d\n", nPage);
		return;
	}
	
	srcimg[nPage].x0Unit = x0Unit;
	srcimg[nPage].y0Unit = y0Unit;
	srcimg[nPage].nxUnit = nxUnit;
	srcimg[nPage].nyUnit = nyUnit;
	srcimg[nPage].bSpCol = bSpCol;
	srcimg[nPage].useTTP = FALSE;
	srcimg[nPage].fEnable = TRUE;
}

void commandVS() { /* from Rance4 */
	int nPage = getCaliValue();
	int nType = getCaliValue();
	int x     = getCaliValue();
	int y     = getCaliValue();
	int wData = getCaliValue();
	
	DEBUG_COMMAND("VS %d,%d,%d,%d,%d:\n",nPage, nType, x, y, wData);
	
	if (nPage >= nPageNum) {
		WARNING("VS nPage too large %d\n", nPage);
		return;
	}
	
	if (x >= cxMap || x < 0) {
		WARNING("VS x out of range %d\n", x);
		return;
	}
	
	if (y >= cyMap || y < 0) {
		WARNING("VS y out of range %d\n", y);
		return;
	}
	if (wData < 0) {
		WARNING("VS wData illegal value %d\n", wData);
		return;
	}

	/* どうやら、 sysVar[0] に値を返してはいけないらしい thanx 村田さん*/
	switch(nType) {
	case 1:
		/* sysVar[0] = *UNITMAP_UNITNUMBER(nPage, x, y); */
		*UNITMAP_UNITNUMBER(nPage, x, y) = wData;
		break;
	case 2:
		/* sysVar[0] = *UNITMAP_VARIABLE(nPage,   x, y); */
		*UNITMAP_VARIABLE(nPage,   x, y) = wData;
		break;
	case 3:
		/* sysVar[0] = *UNITMAP_WALKPAINT(nPage,  x, y); */
		*UNITMAP_WALKPAINT(nPage,  x, y) = wData;
		break;
	case 4:
		/* sysVar[0] = *UNITMAP_WALKRESULT(nPage, x, y); */
		*UNITMAP_WALKRESULT(nPage, x, y) = wData;
		break;
	default:
		WARNING("VS unknown type %d\n", nType);
	}
}

void commandVG() { /* from Rance4 */
	int nPage = getCaliValue();
	int nType = getCaliValue();
	int x = getCaliValue();
	int y = getCaliValue();
	
	DEBUG_COMMAND("VG %d,%d,%d,%d:\n",nPage, nType, x, y);

	if (nPage >= nPageNum) {
		WARNING("VG nPage too large %d\n", nPage);
		return;
	}
	
	if (x >= cxMap || x < 0) {
		NOTICE("VG x out of range %d\n", x);
		sysVar[0] = UNITMAP_VARIABLE_OUTOFRANGE;
		return;
	}
	
	if (y >= cyMap || y < 0) {
		NOTICE("VG y out of range %d\n", y);
		sysVar[0] = UNITMAP_VARIABLE_OUTOFRANGE;
		return;
	}
	
	switch(nType) {
	case 1:
		sysVar[0] = *UNITMAP_UNITNUMBER(nPage, x, y); break;
		break;
	case 2:
		sysVar[0] = *UNITMAP_VARIABLE(nPage,   x, y); break;
	case 3:
		sysVar[0] = *UNITMAP_WALKPAINT(nPage,  x, y); break;
	case 4:
		sysVar[0] = *UNITMAP_WALKRESULT(nPage, x, y); break;
	default:
		WARNING("VG unknown type %d\n", nType);
	}
}

void commandVH() { /* from Rance4 */
	int nPage  = getCaliValue();
	int x      = getCaliValue();
	int y      = getCaliValue();
	int width  = getCaliValue();
	int height = getCaliValue();
	int _max   = getCaliValue();
	int xx, yy, i, n;
	int maxfoot, lmtx, lmty, lmtw, lmth;
	
	DEBUG_COMMAND("VH %d,%d,%d,%d,%d,%d:\n",nPage, x, y, width, height, _max);
	
	if (nPage >= nPageNum) {
		WARNING("VH nPage too large %d\n", nPage);
		return;
	}
	if (x >= cxMap || x < 0) {
		WARNING("VH x out of range %d\n", x);
		return;
	}
	
	if (y >= cyMap || y < 0) {
		WARNING("VH y out of range %d\n", y);
		return;
	}

	if (_max == 0) {
		for (yy = 0; yy < cyMap; yy++) {
			for (xx = 0; xx < cxMap; xx++) {
				*UNITMAP_WALKRESULT(nPage, xx , yy) = UNITMAP_VARIABLE_IMMOVALE;;
			}
		}
		/* 自分自身は 0 */
		*UNITMAP_WALKRESULT(nPage, x , y) = 0;
		return;
	}
	/*
	for (yy = 0; yy < cyMap; yy++) {
		for (xx = 0; xx < cxMap; xx++) {
			*UNITMAP_WALKPAINT(nPage, xx, yy) == 255 ? putchar('*') : putchar(' '+ *UNITMAP_WALKPAINT(nPage, xx, yy)); 
		}
		printf("\n");
	}
	*/
	maxfoot = min(_max, UNITMAP_VARIABLE_IMMOVALE);
        lmtx = max(0, x - maxfoot);
        lmty = max(0, y - maxfoot);
        lmtw = min(maxfoot + cxMap - x, min(cxMap, 2 * maxfoot + 1));
        lmth = min(maxfoot + cyMap - y, min(cyMap, 2 * maxfoot + 1));
  
	for (yy = 0; yy < cyMap; yy++) {
		for (xx = 0; xx < cxMap; xx++) {
			/* 最大歩数の外は不可 */
			if (maxfoot < (abs(yy-y)+abs(xx-x))) {
				*UNITMAP_WALKRESULT(nPage, xx , yy) = UNITMAP_VARIABLE_IMMOVALE;
			} else if (*UNITMAP_WALKPAINT(nPage, xx, yy) >= UNITMAP_VARIABLE_IMMOVALE) {
				/* 障害物なら */
				*UNITMAP_WALKRESULT(nPage, xx , yy) = UNITMAP_VARIABLE_IMMOVALE;
			} else {
				/* いずれでもないときは 0 でならす */
				*UNITMAP_WALKRESULT(nPage, xx , yy) = 0;
			}
		}
	}
	
	/* 移動不可領域の決定 */
	if (width > 1 || height > 1) {
		for (yy = 0; yy < lmth; yy++) {
			for (xx = 0; xx < lmtw; xx++) {
				if (vh_checkImmovableArea(nPage, xx + lmtx, yy + lmty, width, height)) {
					*UNITMAP_WALKRESULT(nPage, xx + lmtx , yy + lmty) = UNITMAP_VARIABLE_IMMOVALE;
				}
			}
		}
	}

	/* list の初期化 */
	vh_src = _vh_src;
	vh_dst = _vh_dst;
	/* 初期位置の設定 */
	vh_src -> x = x;
	vh_src -> y = y;
	vh_cnt_src = n = 1;
	vh_cnt_dst = 0;
	
	while(TRUE) {
		for (i = 0; i < vh_cnt_src; i++) {
			vh_check_udlr(n, nPage, (vh_src + i)->x, (vh_src + i)->y);
		}
		if (vh_cnt_dst == 0) break;  /* 未踏地が無くなったら終了 */
		vh_copy_to_src();
		if (n >= maxfoot)    break;  /* maxfoot までマークしたら終了 */
		n++;
	}
	
	/* 閉領域を255に */
	for (yy = 0; yy < lmth; yy++) {
		for (xx = 0; xx < lmtw; xx++) {
			if (*UNITMAP_WALKRESULT(nPage, xx + lmtx, yy + lmty) == 0) {
				*UNITMAP_WALKRESULT(nPage, xx + lmtx , yy + lmty) = UNITMAP_VARIABLE_IMMOVALE;
			}
		}
	}
	
	/* 自分自身は 0 */
	*UNITMAP_WALKRESULT(nPage, x , y) = 0;
	
	/*
	for (yy = 0; yy < cyMap; yy++) {
		for (xx = 0; xx < cxMap; xx++) {
			*UNITMAP_WALKRESULT(nPage, xx, yy) == 255 ? putchar('*') :putchar('0' + *UNITMAP_WALKRESULT(nPage, xx, yy)); 
		}
		printf("\n");
	}
	*/
}

void commandVF() { /* from Panyo */
	int x, y, i;
	int unit, unit_x, unit_y;
	UnitMapSrcImg *img;
	
	for (i = 0; i < nPageNum; i++) {
		img = &srcimg[i];
		
		if (img->fEnable == FALSE) continue;
		
		for (y = 0; y < cyMap; y++) {
			for (x = 0; x < cxMap; x++) {
				unit = *UNITMAP_UNITNUMBER(i, x, y);
				if (img->useTTP == TRUE && img->TTPunit == unit) continue;
				// printf("i = %d, x = %d, y = %d, unit no = %d\n", i, x, y,unit);
				unit_x = unit % img->nxUnit;
				unit_y = unit / img->nxUnit;
				if (img->bSpCol == 256) {
					ags_copyArea(img->x0Unit + cxUnit * unit_x,
						     img->y0Unit + cyUnit * unit_y,
						     cxUnit, cyUnit,
						     x0Map + x * cxUnit,
						     y0Map + y * cyUnit);
				} else {
					ags_copyAreaSP(img->x0Unit + cxUnit * unit_x,
						       img->y0Unit + cyUnit * unit_y,
						       cxUnit, cyUnit,
						       x0Map + x * cxUnit,
						       y0Map + y * cyUnit, img->bSpCol);
				}
			}
		}
	}
	
	ags_updateArea(x0Map, y0Map, cxMap * cxUnit, cyMap * cyUnit);
	DEBUG_COMMAND("VF:\n");
}

void commandVV() { /* from T2 */
	int nPage   = getCaliValue();
	int fEnable = getCaliValue();
	
	DEBUG_COMMAND("VV %d,%d:\n",nPage, fEnable);
	
	srcimg[nPage].fEnable = (fEnable == 1 ? TRUE : FALSE);
}

void commandVR() { /* from Rance4 */
	int nPage = getCaliValue();
	int nType = getCaliValue();
	int *var  = getCaliVariable();
	int *dst;
	
	DEBUG_COMMAND("VR %d,%d,%p:\n",nPage, nType, var);
	
	if (nPage >= nPageNum) {
		WARNING("VR nPage too large %d\n", nPage);
		return;
	}
	
	switch(nType) {
	case 1:
		dst = UNITMAP_UNITNUMBER_PAGETOP(nPage); break;
	case 2:
		dst = UNITMAP_VARIABLE_PAGETOP(nPage); break;
	case 3:
		dst = UNITMAP_WALKPAINT_PAGETOP(nPage); break;
	case 4:
		dst = UNITMAP_WALKRESULT_PAGETOP(nPage); break;
	default:
		WARNING("VR unknown type %d\n", nType);
		return;
	}
	
	memcpy(dst, var, sizeof(int) * MAPSIZE_PER_ATTRIB);
}

void commandVW() { /* from Rance4 */
	int nPage = getCaliValue();
	int nType = getCaliValue();
	int *var  = getCaliVariable();
	int *src;
	
	DEBUG_COMMAND("VW %d,%d,%p:\n",nPage, nType, var);
	
	if (nPage >= nPageNum) {
		WARNING("VW nPage too large %d\n", nPage);
		return;
	}
	
	switch(nType) {
	case 1:
		src = UNITMAP_UNITNUMBER_PAGETOP(nPage); break;
	case 2:
		src = UNITMAP_VARIABLE_PAGETOP(nPage); break;
	case 3:
		src = UNITMAP_WALKPAINT_PAGETOP(nPage); break;
	case 4:
		src = UNITMAP_WALKRESULT_PAGETOP(nPage); break;
	default:
		WARNING("VW unknown type %d\n", nType);
		return;
	}
	
	memcpy(var, src, sizeof(int) * MAPSIZE_PER_ATTRIB);
}

void commandVE() { /* from T2 */
	int x       = getCaliValue();
	int y       = getCaliValue();
	int width   = getCaliValue();
	int height  = getCaliValue();
	int out_ptn = getCaliValue();
	int flag    = getCaliValue();
	int xx, yy, i;
	int unit, unit_x, unit_y;
	UnitMapSrcImg *img;
	
	DEBUG_COMMAND("VE %d,%d,%d,%d,%d,%d:\n", x, y, width, height, out_ptn, flag);
	if (flag == 1) {
		WARNING("VE flag1 is not yet\n");
	}
	
	for (i = 0; i < nPageNum; i++) {
		img = &srcimg[i];
		/* VV による */
		if (img->fEnable == FALSE) continue;
		
		for (yy = y; yy < (y+height); yy++) {
			for (xx = x; xx < (x+width); xx++) {
				if (xx >= cxMap || yy >= cyMap) {
					unit = out_ptn;
				} else {
					unit = *UNITMAP_UNITNUMBER(i, xx, yy);
				}

				if (img->useTTP == TRUE && img->TTPunit == unit) continue;
				//printf("i = %d, xx = %d, yy = %d, unit no = %d\n", i, xx, yy,unit);
				unit_x = unit % img->nxUnit;
				unit_y = unit / img->nxUnit;
				ags_copyAreaSP(img->x0Unit + cxUnit * unit_x,
					       img->y0Unit + cyUnit * unit_y,
					       cxUnit, cyUnit,
					       x0Map + (xx-x) * cxUnit,
					       y0Map + (yy-y) * cyUnit, img->bSpCol);
			}
		}
	}
	
	ags_updateArea(x0Map, y0Map, cxMap * cxUnit, cyMap * cyUnit);
}

void commandVZ() { /* from T2 */
	int p1 = sys_getc();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	
	DEBUG_COMMAND("VZ %d,%d,%d:\n", p1, p2, p3);
	
	switch(p1) {
	case 0:
		srcimg[p2].useTTP = FALSE; break;
	case 1:
		srcimg[p2].useTTP = TRUE;
		srcimg[p2].TTPunit = p3;
		break;
	case 2:
		x0Map = p2; y0Map = p3;
		break;
	case 3:
		cxUnit = p2; cyUnit = p3;
		break;
	default:
		WARNING("unknown VZ %d:\n", p1);
	}
}

void commandVX() { /* from T2 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	
	DEBUG_COMMAND("VX %d,%d,%d,%d:\n", p1, p2, p3, p4);
	
	switch(p1) {
	case 0:
		srcimg[p2].x0Unit = p3;
		srcimg[p2].y0Unit = p4; break;
	case 1:
		srcimg[p2].nxUnit = p3;
		srcimg[p2].nyUnit = p4; break;
	case 2:
		srcimg[p2].bSpCol = p3; break;
	case 3:
		/* かえるにょ国にょアリスで問題が出たのでとりあえず */
		// commandVF();
		break;
	default:
		WARNING("VX unknown command %d:\n", p1);
	}
}

void commandVT() { /* from Panyo */
	int sp     = getCaliValue();
	int sa     = getCaliValue();
	int sx     = getCaliValue();
	int sy     = getCaliValue();
	int width  = getCaliValue();
	int height = getCaliValue();
	int dp     = getCaliValue();
	int da     = getCaliValue();
	int dx     = getCaliValue();
	int dy     = getCaliValue();
	int x, y, u;
	
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			switch(sa) {
			case 1:
				u = *UNITMAP_UNITNUMBER(sp, sx + x, sy + y); break;
			case 2:
				u = *UNITMAP_VARIABLE(sp, sx + x, sy + y)  ; break;
			case 3:
				u = *UNITMAP_WALKPAINT(sp, sx + x, sy + y) ; break;
			case 4:
				u = *UNITMAP_WALKRESULT(sp, sx + x, sy + y); break;
			default:
				WARNING("VT unknown type %d\n", sa); u = 0;
			}
			switch(da) {
			case 1:
				*UNITMAP_UNITNUMBER(dp, dx + x, dy + y) = u; break;
			case 2:
				*UNITMAP_VARIABLE(dp, dx + x, dy + y)   = u; break;
			case 3:
				*UNITMAP_WALKPAINT(dp, dx + x, dy + y)  = u; break;
			case 4:
				*UNITMAP_WALKRESULT(dp, dx + x, dy + y) = u; break;
			}
		}
	}
	
	DEBUG_COMMAND("VT %d,%d,%d,%d,%d,%d,%d,%d,%d,%d:\n", sp, sa, sx, sy, width, height, dp, da, dx, dy);
}

void commandVB() {
	int page   = getCaliValue();
	int type   = getCaliValue();
	int x_pos  = getCaliValue();
	int y_pos  = getCaliValue();
	int x_size = getCaliValue();
	int y_size = getCaliValue();
	int data   = getCaliValue();
	
	DEBUG_COMMAND_YET("VB %d,%d,%d,%d,%d,%d,%d:\n", page, type, x_pos, y_pos, x_size, y_size, data);
}

void commandVIC() { /* from Panyo */
	int sx     = getCaliValue();
	int sy     = getCaliValue();
	int width  = getCaliValue();
	int height = getCaliValue();
	int unit, unit_x, unit_y;
	int x, y, i;
	UnitMapSrcImg *img;

	DEBUG_COMMAND("VIC %d,%d,%d,%d:\n", sx, sy, width, height);
	
	for (i = 0; i < nPageNum; i++) {
		img = &srcimg[i];
		if (img->fEnable == FALSE) continue;
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				unit = *UNITMAP_UNITNUMBER(i, sx + x, sy + y);
				if (img->useTTP == TRUE && img->TTPunit == unit) continue;
				//printf("i = %d, xx = %d, yy = %d, unit no = %d\n", i, xx, yy, unit);
				unit_x = unit % img->nxUnit;
				unit_y = unit / img->nxUnit;
				ags_copyAreaSP(img->x0Unit + cxUnit * unit_x,
					       img->y0Unit + cyUnit * unit_y,
					       cxUnit, cyUnit,
					       x0Map + (sx + x) * cxUnit,
					       y0Map + (sy + y) * cyUnit, img->bSpCol);
			}
		}
	}
	ags_updateArea(x0Map + sx * cxUnit, y0Map + sy * cyUnit, width * cxUnit, height * cyUnit);	
}

void commandVIP() {
	int x      = getCaliValue();
	int y      = getCaliValue();
	int width  = getCaliValue();
	int height = getCaliValue();
	
	DEBUG_COMMAND_YET("VIP %d,%d,%d,%d:\n", x, y, width, height);
}

void commandVJ() {
	/* 重み付き歩数ペイント */
	int page  = getCaliValue();
	int x     = getCaliValue();
	int y     = getCaliValue();
	int max   = getCaliValue();
	
	DEBUG_COMMAND_YET("VJ %d,%d,%d,%d:\n", page, x, y, max);
}

void commandVA() { /* from Panyo */
	static boolean startedItimer = FALSE;
	int no = sys_getc();
	int p1 = getCaliValue();
	int p2, p3;
	int *var1, *var2;
	
	if (no >= 10) {
		var1 = getCaliVariable();
		var2 = getCaliVariable();
		DEBUG_COMMAND("VA %d,%d,%p,%p:\n", no, p1, var1, var2);
	} else {
		p2 = getCaliValue();
		p3 = getCaliValue();
		DEBUG_COMMAND("VA %d,%d,%d,%d:\n", no, p1, p2, p3);
	}
	
	if (p1 > VACMD_MAX) {
		WARNING("VA p1 is too lagrge %d\n", p1);
		return;
	}
	
	p1--;
	switch(no) {
	case 0:
		if (p2 == 0) {
			/* 停止 */
			inAnimation = TRUE;
			VAcmd[p1].state = VA_STOPPED;
			VAcmd[p1].draw  = TRUE;
			if (p3 == 0) {
				/* ユニット消し */
				va_restoreUnit(p1);
				va_updatePreArea(p1);
			} else {
				/* ユニット書く */
				va_drawUnit(p1);
				va_updateUnit(p1);
			}
			VAcmd[p1].draw  = FALSE;
		} else {
			/* 開始 (p3=コマ数,0:無限(開始位置==終了位置のとき))*/
			inAnimation = TRUE;
			VAcmd[p1].elaspCut  = 0;
			VAcmd[p1].quantmsec = 0;
			VAcmd[p1].totalCut  = p3;
			
			if (p3 == 0) {
				VAcmd[p1].nomove = TRUE;
			} else {
				if (VAcmd[p1].startX == VAcmd[p1].endX && VAcmd[p1].startY == VAcmd[p1].endY) {
					VAcmd[p1].nomove = TRUE;
				} else {
					VAcmd[p1].nomove = FALSE;
				}
			}
			/* animation start */
			if (startedItimer) {
				va_unpause_itimer();
			} else {
				startedItimer = TRUE;
				va_init_itimer();
			}
	
			VAcmd[p1].state   = VA_RUNNING;
			VAcmd[p1].draw    = TRUE;
			va_drawUnit(p1);
			va_updateUnit(p1);
			if (p2 == 2) {
				VAcmd[p1].rewrite = TRUE;
				/* キー抜け無し ,p3=0は指定不可 */
				while(VAcmd[p1].state == VA_RUNNING) {
					va_animationAlone(p1);
					usleep(10*1000);
				}
				va_drawUnit(p1);
				va_updateUnit(p1);
			} else if (p2 == 3) {
				/* キー抜けあり ,p3=0は指定不可*/
				int key = 0;
				VAcmd[p1].rewrite = TRUE;
				while(VAcmd[p1].state == VA_RUNNING) {
					va_animationAlone(p1);
					usleep(10*1000);
					key = sys_getInputInfo();
					if (key != 0) {
						sysVar[0] = key;
						break;
					}
					
				}
				va_drawUnit(p1);
				va_updateUnit(p1);
			} else {
				/* すぐに制御を戻す */
			}
		}
		break;
	case 1:
		/* 表示位置 */
		VAcmd[p1].curX = VAcmd[p1].preX = VAcmd[p1].startX = p2;
		VAcmd[p1].curY = VAcmd[p1].preY = VAcmd[p1].startY = p3; break;
	case 2:
		/* 移動先(終ったら自動的に停止 */
		VAcmd[p1].endX = p2;
		VAcmd[p1].endY = p3; break;
	case 3:
		/* サイズ */
		VAcmd[p1].unitWidth  = p2;
		VAcmd[p1].unitHeight = p3; break;
	case 4:
		/* パターン数・描替間隔(1/100sec) */
		VAcmd[p1].patternNum   = p2;
		VAcmd[p1].intervaltime = p3; break;
	case 5:
		/* 取得位置 */
		VAcmd[p1].srcX = p2;
		VAcmd[p1].srcY = p3; break;
	case 6:
		/* 背景退避位置 */
		VAcmd[p1].saveX = p2;
		VAcmd[p1].saveY = p3; break;
	case 7:
		/* スプライト方法・色 */
		VAcmd[p1].spType = p2;             /* p2=0:通常コピー , p2=1:色指定スプライト, p2=2:影データスプライト */
		VAcmd[p1].spCol  = p3; break;
	case 10:
		/* 状態取得(var1=0:停止1:動,var2=番号) */
		*var1 = VAcmd[p1].state == 0 ? 0 : 1;
		*var2 = VAcmd[p1].elaspCut; break;
	case 11:
		/* 位置取得 */
		*var1 = VAcmd[p1].curX;
		*var2 = VAcmd[p1].curY; break;
	default:
		WARNING("Unknown VA command %d\n", no);
	}
}
	
static boolean vh_checkImmovableArea(int page, int x, int y, int w, int h) {
	int _x, _y;
	
	if (x + w > cxMap) return TRUE; 
	
	if (y + h > cyMap) return TRUE; 

	for (_y = y; _y < (y+h); _y++) {
		for (_x = x; _x < (x+w); _x++) {
			if (*UNITMAP_WALKRESULT(page, _x, _y) == 255) return TRUE;
		}
	}
	
	return FALSE;
}

static void vh_append_pos(int x, int y) {
	/* x, y を list に追加 */
	(vh_dst + vh_cnt_dst)->x = x;
	(vh_dst + vh_cnt_dst)->y = y;
	vh_cnt_dst++;
}

static void vh_copy_to_src() {
	VhMark *tmp;

	/* src list と dst list の入れ換え */
	tmp    = vh_src;
	vh_src = vh_dst;
	vh_dst = tmp;

	vh_cnt_src = vh_cnt_dst;
	vh_cnt_dst = 0;
}

static void vh_check_udlr(int n, int nPage, int x, int y) {
	/* 上下左右が未踏地なら値をセットしてlistに追加 */
	if (x > 0) {
                if (*UNITMAP_WALKRESULT(nPage, x -1, y) == 0) {
                        *UNITMAP_WALKRESULT(nPage, x -1, y) = n;
                        vh_append_pos(x -1, y);
                }
        }
        if (x < (cxMap-1)) {
                if (*UNITMAP_WALKRESULT(nPage, x +1, y) == 0) {
                        *UNITMAP_WALKRESULT(nPage, x +1, y) = n;
                        vh_append_pos(x +1, y);
                }
        }
        if (y > 0) {
                if (*UNITMAP_WALKRESULT(nPage, x, y -1) == 0) {
                        *UNITMAP_WALKRESULT(nPage, x, y -1) = n;
                        vh_append_pos(x, y -1);
                }
        }
        if (y < (cyMap-1)) {
                if (*UNITMAP_WALKRESULT(nPage, x, y +1) == 0) {
                        *UNITMAP_WALKRESULT(nPage, x, y +1) = n;
                        vh_append_pos(x, y +1);
                }
	}
}

static void va_drawUnit(int no) {
	int unitno = VAcmd[no].elaspCut % VAcmd[no].patternNum;
	int unitX = unitno % 10; /* し〜らないっと :-p */
	int unitY = unitno / 10;

	/* save */
	ags_copyArea(VAcmd[no].curX, VAcmd[no].curY, VAcmd[no].unitWidth, VAcmd[no].unitHeight, 
		     VAcmd[no].saveX, VAcmd[no].saveY);
	/* drawUnit */
	if (VAcmd[no].spType == 0) {
		ags_copyArea(VAcmd[no].srcX + VAcmd[no].unitWidth * unitX,
			     VAcmd[no].srcY + VAcmd[no].unitHeight * unitY,
			     VAcmd[no].unitWidth, VAcmd[no].unitHeight, 
			     VAcmd[no].curX, VAcmd[no].curY);
	} else if (VAcmd[no].spType == 1) {
		ags_copyAreaSP(VAcmd[no].srcX + VAcmd[no].unitWidth * unitX,
			       VAcmd[no].srcY + VAcmd[no].unitHeight * unitY,
			       VAcmd[no].unitWidth, VAcmd[no].unitHeight, 
			       VAcmd[no].curX, VAcmd[no].curY,
			       VAcmd[no].spCol);
	}
}

static void va_restoreUnit(int no) {
	ags_copyArea(VAcmd[no].saveX, VAcmd[no].saveY, VAcmd[no].unitWidth, VAcmd[no].unitHeight, 
		     VAcmd[no].preX, VAcmd[no].preY);
}

static void va_updateUnit(int i) {
	ags_updateArea(VAcmd[i].curX, VAcmd[i].curY, VAcmd[i].unitWidth, VAcmd[i].unitHeight);
}

static void va_updatePreArea(int i) {
	ags_updateArea(VAcmd[i].preX, VAcmd[i].preY, VAcmd[i].unitWidth, VAcmd[i].unitHeight);
}

void va_animation() {
	int i;
	int x, y, w, h;
	
	inAnimation = TRUE;
	
	for (i = 0; i < VACMD_MAX; i++) {
		if (VAcmd[i].state == VA_STOPPED) continue;
		if (!VAcmd[i].rewrite)            continue;
		if (VAcmd[i].draw) va_restoreUnit(i);
		if (VAcmd[i].draw) va_drawUnit(i);
		x = min(VAcmd[i].curX, VAcmd[i].preX);
		y = min(VAcmd[i].curY, VAcmd[i].preY);
		w = max(VAcmd[i].curX + VAcmd[i].unitWidth, VAcmd[i].preX + VAcmd[i].unitWidth) - x; 
		h = max(VAcmd[i].curY + VAcmd[i].unitHeight, VAcmd[i].preY + VAcmd[i].unitHeight) - y; 
		ags_updateArea(x, y, w, h);
		// printf("x = %d, y = %d, w = %d, h = %d\n", x, y, w, h);
		VAcmd[i].rewrite = FALSE;
	}
	
	inAnimation = FALSE;
}
	
static void va_animationAlone(int i) {
	int x, y, w, h; /* update region */
	
	if (!VAcmd[i].rewrite) return;

	inAnimation = TRUE;
	
	va_restoreUnit(i);
	va_drawUnit(i);

	x = min(VAcmd[i].curX, VAcmd[i].preX);
	y = min(VAcmd[i].curY, VAcmd[i].preY);
	w = max(VAcmd[i].curX + VAcmd[i].unitWidth, VAcmd[i].preX + VAcmd[i].unitWidth) - x; 
	h = max(VAcmd[i].curY + VAcmd[i].unitHeight, VAcmd[i].preY + VAcmd[i].unitHeight) - y; 
	ags_updateArea(x, y, w, h);
	
	VAcmd[i].rewrite = FALSE;
	inAnimation = FALSE;
}

static void va_interval_process() {
	boolean proceeding = FALSE;
	int i;
	
	for (i = 0; i < VACMD_MAX; i++) {
		if (VAcmd[i].state == VA_RUNNING) {
			proceeding = TRUE;
			VAcmd[i].quantmsec++;
			
			if (VAcmd[i].quantmsec >= VAcmd[i].intervaltime) {
				/* まだ更新していない場合はskip */
				if (VAcmd[i].rewrite) continue;
				VAcmd[i].rewrite = TRUE;
				VAcmd[i].quantmsec = 0;
 				VAcmd[i].elaspCut++;
				/* 古い場所 */
				VAcmd[i].preX = VAcmd[i].curX;
				VAcmd[i].preY = VAcmd[i].curY;
				/* 全コマ終ったら終了 */
				if (VAcmd[i].elaspCut >= VAcmd[i].totalCut) {
					VAcmd[i].state = VA_STOPPED;
					continue;
				}
				/* 移動なしの場合は skip */
				if (VAcmd[i].nomove)  continue;
				/* 新しい場所 */
				VAcmd[i].curX = VAcmd[i].startX + VAcmd[i].elaspCut * (VAcmd[i].endX - VAcmd[i].startX) / VAcmd[i].totalCut;
				VAcmd[i].curY = VAcmd[i].startY + VAcmd[i].elaspCut * (VAcmd[i].endY - VAcmd[i].startY) / VAcmd[i].totalCut;
 			}
		}
	}
	/* 更新するものが無い場合は、タイマーを止めて、アニメーションストップ */
	if (!proceeding) {
		va_pause_itimer();
		nact->is_va_animation = FALSE;
	}
}

static void alarmHandler() {
	if (!inAnimation) {
		va_interval_process();
	}
}

static void va_init_itimer() {
	sys_set_signalhandler(SIGALRM, alarmHandler);
	va_unpause_itimer();
}

static void va_pause_itimer() {
	struct itimerval value;

	value.it_interval.tv_sec  = 0;
	value.it_interval.tv_usec = 0;
	value.it_value.tv_sec  = 0;
	value.it_value.tv_usec = 0;
	setitimer(ITIMER_REAL, &value, NULL);
}

static void va_unpause_itimer() {
	struct itimerval value;
	
	value.it_interval.tv_sec  = 0;
	value.it_interval.tv_usec = 10 * 1000;
	value.it_value.tv_sec  = 0;
	value.it_value.tv_usec = 10 * 1000;
	setitimer(ITIMER_REAL, &value, NULL);
	nact->is_va_animation = TRUE;
}
