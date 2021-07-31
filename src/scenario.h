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
#include "system.h"

// Warning: changing these enum values will break savedata compatibility.
enum stack_frame_type {
	STACK_NEARJMP = 1,
	STACK_FARJMP = 2,
	STACK_VARIABLE = 3,
	STACK_DATA = 4
};

enum txx_type {
	TxxTEXTCOLOR = 1,
	TxxTEXTSIZE = 2,
	TxxTEXTLOC = 3
};

// Use functions below instead of accessing these variables directly.
extern const BYTE *sl_sco; // scenario page buffer
extern int sl_page;        // current scenario page (0-based)
extern int sl_index;       // cureent scenario address

boolean sl_init(void);
boolean sl_reinit(void);
int sl_getw(void);
#define sl_getdw sl_getaddr
int sl_getdAt(int address);
int sl_getwAt(int address);
int sl_getcAt(int address);
int sl_getaddr(void);
void sl_ungetc(void);
const char *sl_getString(char term);
const char *sl_getConstString(void);

void sl_jmpNear(int address);
boolean sl_jmpFar(int page);
boolean sl_jmpFar2(int page, int address);
void sl_callNear(int address);
void sl_retNear(void);
void sl_callFar(int page);
void sl_callFar2(int page, int address);
void sl_retFar(void);
void sl_retFar2(void);
void sl_stackClear_allCall(void);
void sl_stackClear_labelCall(int cnt);
void sl_stackClear_pageCall(int cnt);
void sl_pushVar(int *topvar, int cnt);
void sl_popVar(int *topvar, int cnt);
int *sl_getStackInfo(int *size);
void sl_putStackInfo(int *data, int size);
void sl_pushData(int *data, int cnt);
void sl_popData(int *data, int cnt);
void *sl_setDataTable(int page, int index);
void sl_returnGoto(int address);

static inline int sl_getIndex(void) { return sl_index; }
static inline int sl_getPage(void) { return sl_page; }
static inline int sl_getc(void) { return sl_sco[sl_index++]; }

#ifdef DEBUG
#define DEBUG_COMMAND_YET(fmt, ...) \
	sys_message(2, "%d,%x: " fmt, sl_getPage(), sl_getIndex(), ##__VA_ARGS__)
#define DEBUG_COMMAND(fmt, ...) \
	sys_message(5, "%d,%x: " fmt, sl_getPage(), sl_getIndex(), ##__VA_ARGS__)
#else
#define DEBUG_COMMAND(...)
#define DEBUG_COMMAND_YET(...)
#endif

#endif /* !__SCENARIO_ */
