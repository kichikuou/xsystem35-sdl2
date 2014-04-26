/*
 * scenario.h  シナリオ管理実行
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
/* $Id: scenario.h,v 1.14 2003/04/22 16:34:28 chikama Exp $ */

#ifndef __SCENARIO__
#define __SCENARIO__

#include "portab.h"

extern boolean sl_init();
extern boolean sl_reinit();
extern int sl_getc();
extern int sl_getw();
#define sl_getdw sl_getadr
extern int sl_getdAt(int address);
extern int sl_getwAt(int address);
extern int sl_getcAt(int address);
extern int sl_getadr();
extern void sl_jmpNear(int address);
extern boolean sl_jmpFar(int page);
extern boolean sl_jmpFar2(int page, int address);
extern void sl_callNear(int address);
extern void sl_retNear();
extern void sl_callFar(int page);
extern void sl_callFar2(int page, int address);
extern void sl_retFar();
extern void sl_retFar2();
extern void sl_stackClear_allCall();
extern void sl_stackClear_labelCall(int cnt);
extern void sl_stackClear_pageCall(int cnt);
extern void sl_pushVar(int *topvar, int cnt);
extern void sl_popVar(int *topvar, int cnt);
extern int sl_getIndex();
extern int sl_getPage();
extern int *sl_getStackInfo(int *size);
extern void sl_putStackInfo(int *data, int size);
extern void sl_pushData(int *data, int cnt);
extern void sl_popData(int *data, int cnt);
extern void *sl_setDataTable(int page, int index);
extern void sl_returnGoto(int address);

#define TxxTEXTCOLOR 1
#define TxxTEXTSIZE  2
#define TxxTEXTLOC   3

#endif /* !__SCENARIO_ */
