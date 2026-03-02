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

enum save_format;
struct VarRef;

// Stack frame tags. Changing these will break savedata compatibility.
#define STACK_FARCALL   0xFF
#define STACK_NEARCALL  0xEE
#define STACK_VARIABLE  0xDD
#define STACK_TEXTCOLOR 0xCC
#define STACK_TEXTSIZE  0xBB
#define STACK_TEXTLOC   0xAA

struct stack_info {
	int top_attr;  // always zero?
	int page_calls;
	int label_calls;
	int var_pushes;
	int label_calls_after_page_call;
	int var_pushes_after_call;
};

struct stack_frame_info {
	uint8_t tag;
	uint16_t page;  // STACK_FARCALL
	int addr;       // STACK_FARCALL or STACK_NEARCALL
	uint8_t *p;     // for internal use
};

// Use functions below instead of accessing these variables directly.
extern const uint8_t *sl_sco; // scenario page buffer
extern int sl_page;        // current scenario page (0-based)
extern int sl_index;       // cureent scenario address

bool sl_init(void);
bool sl_reinit(void);
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
bool sl_jmpFar(int page);
bool sl_jmpFar2(int page, int address);
void sl_callNear(int address);
void sl_retNear(void);
void sl_callFar(int page);
void sl_callFar2(int page, int address);
void sl_retFar(void);
void sl_dropLabelCalls(int cnt);
void sl_dropPageCalls(int cnt);
void *sl_setDataTable(int page, int index);
void sl_returnGoto(int address);

void sl_clearStack(bool restore);
void sl_pushVar(struct VarRef *vref, int cnt);
void sl_popVar(struct VarRef *vref, int cnt);
uint8_t *sl_saveStack(enum save_format format, int *size);
void sl_loadStack(enum save_format format, uint8_t *data, int size);
void sl_getStackInfo(struct stack_info *info);
void sl_pushTextColor(uint8_t type, uint8_t color);
void sl_pushTextSize(uint8_t type, int size);
void sl_pushTextLoc(int x, int y);
void sl_popState(uint8_t expected_type);
struct stack_frame_info *sl_next_stack_frame(struct stack_frame_info *frame_info);

static inline int sl_getIndex(void) { return sl_index; }
static inline int sl_getPage(void) { return sl_page; }
static inline int sl_getc(void) { return sl_sco[sl_index++]; }

#define TRACE_UNIMPLEMENTED(fmt, ...) \
	sys_message(2, "[UNIMPLEMENTED] " fmt "\n", ##__VA_ARGS__)

#ifdef DEBUG
#define TRACE(fmt, ...) \
	sys_message(5, "[TRACE] " fmt "\n", ##__VA_ARGS__)
#define TRACE_MESSAGE(...) sys_message(6, __VA_ARGS__)
#else
#define TRACE(...)
#define TRACE_MESSAGE(...)
#endif

#endif /* !__SCENARIO_ */
