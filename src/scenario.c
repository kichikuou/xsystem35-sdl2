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

const uint8_t *sl_sco;
int sl_page;
int sl_index;

static void pop_state(uint8_t tag);

static uint8_t *stack_buf;
static uint8_t *stack_top;
static int stack_size = 1024;

static char strbuf[512];

static dridata *dfile;   // dri object for current page
static dridata *datatbl; // dri object for data table

static void stack_reserve(int size) {
	if (stack_top + size < stack_buf + stack_size)
		return;
	int n = stack_top - stack_buf;
	stack_size *= 2;
	stack_buf = realloc(stack_buf, stack_size);
	if (!stack_buf)
		NOMEMERR();
	stack_top = stack_buf + n;
}

static void stack_push_byte(uint8_t v) {
	*stack_top++ = v;
}

static void stack_push_word(uint16_t v) {
	LittleEndian_putW(v, stack_top, 0);
	stack_top += 2;
}

static void stack_push_dword(uint32_t v) {
	LittleEndian_putDW(v, stack_top, 0);
	stack_top += 4;
}

static void stack_drop(int n) {
	stack_top -= n;
	if (stack_top < stack_buf)
		SYSERROR("stack underflow");
}

static uint8_t stack_pop_byte(void) {
	stack_drop(1);
	return *stack_top;
}

static int stack_pop_word(void) {
	stack_drop(2);
	return LittleEndian_getW(stack_top, 0);
}

static int stack_pop_dword(void) {
	stack_drop(4);
	return LittleEndian_getDW(stack_top, 0);
}

/* initilize stack and load first scenario data */
boolean sl_init(void) {
	stack_buf = malloc(stack_size);
	if (stack_buf == NULL)
		NOMEMERR();
	
	stack_top = stack_buf;
	sl_jmpFar(0);

	return TRUE;
}

/* UD 0 command, reinitilized scenario loader */
boolean sl_reinit(void) {
	free(stack_buf);
	return sl_init();
}

int sl_getw(void) {
	int c0 = sl_getc();
	int c1 = sl_getc();
	return c0 + (c1 << 8);
}

int sl_getcAt(int adr) {
	return sl_sco[adr];
}

int sl_getwAt(int adr) {
	int c0 = sl_sco[adr];
	int c1 = sl_sco[adr + 1];
	return c0 + (c1 << 8);
}

int sl_getdAt(int adr) {
	int c0 = sl_getwAt(adr);
	int c1 = sl_getwAt(adr + 2);
	return c0 + (c1 << 16);
}

int sl_getaddr(void) {
	int c0 = sl_getw();
	int c1 = sl_getw();
	return c0 + (c1 << 16);
}

void sl_ungetc(void) {
	sl_index--;
}

const char *sl_getString(char term) {
	int c0;
	char *index = strbuf;

	while ((c0 = sl_getc()) != (int)term) {
		*index++ = c0;
	}
	*index = '\0';
	return strbuf;
}

const char *sl_getConstString(void) {
	char *index = strbuf;
	int c0 = sl_getc();

	while ((c0 = sl_getc()) != 0) {
		*index++ = ((c0 & 0xf0) >> 4) + ((c0 & 0x0f) << 4);
	}
	*index = '\0';
	return strbuf;
}

/* @address */
void sl_jmpNear(int address) {
	sl_index = address;
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
		TRACE_MESSAGE("ald_getdata fail\n");
		return FALSE;
	}
	sl_sco   = dfile->data;
	sl_page  = page;
	sl_index = address;
	return TRUE;
}

void sl_callNear(int address) {
	stack_reserve(4 + 1);
	stack_push_dword(sl_index);
	stack_push_byte(STACK_NEARCALL);
	sl_jmpNear(address);
}

void sl_retNear(void) {
	uint8_t tag;
	while ((tag = stack_pop_byte()) != STACK_NEARCALL) {
		switch (tag) {
		case STACK_FARCALL:
			SYSERROR("label return from page call");
			break;
		default:
			pop_state(tag);
			break;
		}
	}
	int address = stack_pop_dword();
	sl_jmpNear(address);
}

void sl_callFar(int page) {
	stack_reserve(2 + 4 + 1);
	stack_push_word(sl_page + 1);
	stack_push_dword(sl_index);
	stack_push_byte(STACK_FARCALL);

	if (!sl_jmpFar(page))
		sl_retFar();
}

void sl_callFar2(int page, int address) {
	stack_reserve(2 + 4 + 1);
	stack_push_word(sl_page + 1);
	stack_push_dword(sl_index);
	stack_push_byte(STACK_FARCALL);

	if (!sl_jmpFar2(page, address))
		sl_retFar();
}

void sl_retFar(void) {
	uint8_t tag;
	while ((tag = stack_pop_byte()) != STACK_FARCALL) {
		switch (tag) {
		case STACK_NEARCALL:
			stack_drop(4);
			break;
		default:
			pop_state(tag);
			break;
		}
	}

	int address = stack_pop_dword();
	int page = stack_pop_word() - 1;
	sl_jmpFar2(page, address);
}

/* UC0 */
void sl_clearStack(bool restore) {
	if (!restore) {
		stack_top = stack_buf;
		return;
	}
	while (stack_top > stack_buf) {
		uint8_t tag = stack_pop_byte();
		switch (tag) {
		case STACK_NEARCALL:
			stack_drop(4);
			break;
		case STACK_FARCALL:
			stack_drop(4 + 2);
			break;
		default:
			pop_state(tag);
			break;
		}
	}
}

/* UC 2 */
void sl_dropPageCalls(int cnt) {
	while (cnt > 0) {
		uint8_t tag = stack_pop_byte();
		switch (tag) {
		case STACK_NEARCALL:
			stack_drop(4);
			break;
		case STACK_FARCALL:
			stack_drop(4 + 2);
			cnt--;
			break;
		default:
			pop_state(tag);
			break;
		}
	}
}

/* UC 1 */
void sl_dropLabelCalls(int cnt) {
	while (cnt > 0) {
		uint8_t tag = stack_pop_byte();
		switch (tag) {
		case STACK_NEARCALL:
			stack_drop(4);
			cnt--;
			break;
		case STACK_FARCALL:
			stack_push_byte(tag);
			return;
		default:
			pop_state(tag);
			break;
		}
	}
}

/* US */
void sl_pushVar(struct VarRef *vref, int cnt) {
	stack_reserve((2 + 2 + 2 + 1) * cnt);
	int *val = v_resolveRef(vref);
	for (int i = 0; i < cnt; i++) {
		stack_push_word(vref->index + i);
		stack_push_word(vref->page);
		stack_push_word(val[i]);
		stack_push_byte(STACK_VARIABLE);
	}
}

/* UG */
void sl_popVar(struct VarRef *vref, int cnt) {
	int *val = v_resolveRef(vref) + cnt;
	while (cnt-- > 0) {
		if (stack_pop_byte() != STACK_VARIABLE)
			SYSERROR("stack top is not a variable push");
		*--val = stack_pop_word();
		stack_drop(2 + 2);
	}
}

enum save_stack_frame_type {
	SAVE_NEARJMP = 1,
	SAVE_FARJMP = 2,
	SAVE_VARIABLE = 3,
	SAVE_TXXSTATE = 4
};

enum txx_type {
	TxxTEXTCOLOR = 1,
	TxxTEXTSIZE = 2,
	TxxTEXTLOC = 3
};

struct save_stack_frame {
	struct save_stack_frame *next;
	int len;
	int buf[];
};

struct save_stack_frame *push_save_stack_frame(int size, struct save_stack_frame *next) {
	struct save_stack_frame *f =
		malloc(sizeof(struct save_stack_frame) + sizeof(int) * size);
	f->next = next;
	f->len = size;
	return f;
}

uint8_t *sl_saveStack(enum save_format format, int *size_out) {
	if (format != SAVEFMT_XSYS35) {
		int size = stack_top - stack_buf;
		uint8_t *buf = malloc(size);
		memcpy(buf, stack_buf, size);
		*size_out = size;
		return buf;
	}

	// Serialize to the stack format used in old versions of xsystem35.
	struct save_stack_frame *frame = NULL;
	uint8_t *sp = stack_top;
	while (sp > stack_buf) {
		switch (sp[-1]) {
		case STACK_FARCALL:
			sp -= 7;
			frame = push_save_stack_frame(4, frame);
			frame->buf[3] = SAVE_FARJMP;
			frame->buf[2] = 2;
			frame->buf[1] = LittleEndian_getW(sp, 0) - 1; // page
			frame->buf[0] = LittleEndian_getDW(sp, 2);    // addr
			break;
		case STACK_NEARCALL:
			sp -= 5;
			frame = push_save_stack_frame(3, frame);
			frame->buf[2] = SAVE_NEARJMP;
			frame->buf[1] = 1;
			frame->buf[0] = LittleEndian_getDW(sp, 0); // addr
			break;
		case STACK_VARIABLE:
			sp -= 7;
			{
				int index = LittleEndian_getW(sp, 0);
				int page  = LittleEndian_getW(sp, 2);
				int value = LittleEndian_getW(sp, 4);
				if (frame && frame->buf[frame->len - 1] == SAVE_VARIABLE) {
					int count = frame->buf[frame->len - 2];
					int fpage = frame->buf[frame->len - 3];
					int base  = frame->buf[frame->len - 4];
					// Merge into existing SAVE_VARIABLE frame.
					if (page == fpage && index == base - 1) {
						frame = realloc(frame, sizeof(struct save_stack_frame) + sizeof(int) * (frame->len + 1));
						frame->len++;
						frame->buf[frame->len - 1] = SAVE_VARIABLE;
						frame->buf[frame->len - 2] = count + 1;
						frame->buf[frame->len - 3] = page;
						frame->buf[frame->len - 4] = base - 1;
						frame->buf[frame->len - 5] = value;
						break;
					}
				}
				frame = push_save_stack_frame(5, frame);
				frame->buf[4] = SAVE_VARIABLE;
				frame->buf[3] = 3;
				frame->buf[2] = page;
				frame->buf[1] = index;
				frame->buf[0] = value;
			}
			break;
		case STACK_TEXTCOLOR:
			sp -= 3;
			frame = push_save_stack_frame(5, frame);
			frame->buf[4] = SAVE_TXXSTATE;
			frame->buf[3] = 3;
			frame->buf[2] = TxxTEXTCOLOR;
			frame->buf[1] = sp[0]; // type
			frame->buf[0] = sp[1]; // color
			break;
		case STACK_TEXTSIZE:
			sp -= 6;
			frame = push_save_stack_frame(5, frame);
			frame->buf[4] = SAVE_TXXSTATE;
			frame->buf[3] = 3;
			frame->buf[2] = TxxTEXTSIZE;
			frame->buf[1] = sp[0];                     // type
			frame->buf[0] = LittleEndian_getDW(sp, 1); // size
			break;
		case STACK_TEXTLOC:
			sp -= 9;
			frame = push_save_stack_frame(5, frame);
			frame->buf[4] = SAVE_TXXSTATE;
			frame->buf[3] = 3;
			frame->buf[2] = TxxTEXTLOC;
			frame->buf[1] = LittleEndian_getDW(sp, 0); // x
			frame->buf[0] = LittleEndian_getDW(sp, 4); // y
			break;
		default:
			SYSERROR("broken stack");
		}
	}
	int total_len = 0;
	for (struct save_stack_frame *f = frame; f; f = f->next)
		total_len += f->len;

	int *buf = malloc(total_len * sizeof(int));
	int *p = buf;
	for (struct save_stack_frame *f = frame; f;) {
		memcpy(p, f->buf, f->len * sizeof(int));
		p += f->len;
		struct save_stack_frame *next = f->next;
		free(f);
		f = next;
	}

	*size_out = total_len * sizeof(int);
#if 0
	// Testing code. Call sl_loadStack with the serialized stack, and compare
	// the deserialized stack with the original stack.
	{
		int orig_size = stack_top - stack_buf;
		uint8_t *orig_stack = malloc(orig_size);
		memcpy(orig_stack, stack_buf, orig_size);

		sl_loadStack(format, (uint8_t *)buf, *size_out);

		if (stack_top - stack_buf != orig_size || memcmp(stack_buf, orig_stack, orig_size)) {
			FILE *fp = fopen("orig_stack", "wb");
			fwrite(orig_stack, 1, orig_size, fp);
			fclose(fp);
			fp = fopen("deserialized_stack", "wb");
			fwrite(stack_buf, 1, stack_top - stack_buf, fp);
			fclose(fp);
			SYSERROR("stack mismatch");
		}
	}
#endif
	return (uint8_t *)buf;
}

void sl_loadStack(enum save_format format, uint8_t *unaligned_data, int size) {
	if (format != SAVEFMT_XSYS35) {
		if (size > stack_size) {
			while (size > stack_size)
				stack_size *= 2;
			free(stack_buf);
			stack_buf = malloc(stack_size);
			if (!stack_buf)
				NOMEMERR();
		}
		memcpy(stack_buf, unaligned_data, size);
		stack_top = stack_buf + size;
		return;
	}

	// Deserialize from the stack format used in old versions of xsystem35.
	int *data = malloc(size);
	memcpy(data, unaligned_data, size);
	struct save_stack_frame *frame = NULL;

	for (int *p = data + size / sizeof(int); p > data;) {
		int len = p[-2] + 2;
		p -= len;
		frame = push_save_stack_frame(len, frame);
		memcpy(frame->buf, p, len * sizeof(int));
	}
	free(data);

	stack_top = stack_buf;
	for (struct save_stack_frame *f = frame; f;) {
		switch (f->buf[f->len - 1]) {
		case SAVE_NEARJMP:
			if (f->len != 3)
				SYSERROR("broken stack data");
			stack_reserve(4 + 1);
			stack_push_dword(f->buf[0]);
			stack_push_byte(STACK_NEARCALL);
			break;
		case SAVE_FARJMP:
			if (f->len != 4)
				SYSERROR("broken stack data");
			stack_reserve(2 + 4 + 1);
			stack_push_word(f->buf[1] + 1);
			stack_push_dword(f->buf[0]);
			stack_push_byte(STACK_FARCALL);
			break;
		case SAVE_VARIABLE:
			if (f->len < 5) {
				SYSERROR("broken stack data");
			} else {
				int count = f->len - 4;
				int page = f->buf[f->len - 3];
				int base = f->buf[f->len - 4];
				stack_reserve((2 + 2 + 2 + 1) * count);
				for (int i = 0; i < count; i++) {
					stack_push_word(base + i);
					stack_push_word(page);
					stack_push_word(f->buf[count - 1 - i]);
					stack_push_byte(STACK_VARIABLE);
				}
			}
			break;
		case SAVE_TXXSTATE:
			if (f->len != 5)
				SYSERROR("broken stack data");
			switch (f->buf[2]) {
			case TxxTEXTCOLOR:
				stack_reserve(1 + 1 + 1);
				stack_push_byte(f->buf[1]);  // type
				stack_push_byte(f->buf[0]);  // color
				stack_push_byte(STACK_TEXTCOLOR);
				break;
			case TxxTEXTSIZE:
				stack_reserve(1 + 4 + 1);
				stack_push_byte(f->buf[1]);  // type
				stack_push_dword(f->buf[0]); // size
				stack_push_byte(STACK_TEXTSIZE);
				break;
			case TxxTEXTLOC:
				stack_reserve(4 + 4 + 1);
				stack_push_dword(f->buf[1]); // x
				stack_push_dword(f->buf[0]); // y
				stack_push_byte(STACK_TEXTLOC);
				break;
			default:
				SYSERROR("broken stack data");
			}
			break;
		default:
			SYSERROR("broken stack data");
		}
		struct save_stack_frame *next = f->next;
		free(f);
		f = next;
	}
}

void sl_getStackInfo(struct stack_info *info) {
	memset(info, 0, sizeof(struct stack_info));

	struct stack_frame_info *sfi = NULL;
	while ((sfi = sl_next_stack_frame(sfi)) != NULL) {
		switch (sfi->tag) {
		case STACK_NEARCALL:
			info->label_calls++;
			if (!info->page_calls)
				info->label_calls_after_page_call++;
			break;
		case STACK_FARCALL:
			info->page_calls++;
			break;
		case STACK_VARIABLE:
			info->var_pushes++;
			if (info->page_calls + info->label_calls == 0)
				info->var_pushes_after_call++;
			break;
		default:
			break;
		}
	}
}

/* TPC */
void sl_pushTextColor(uint8_t type, uint8_t color) {
	stack_reserve(1 + 1 + 1);
	stack_push_byte(type);
	stack_push_byte(color);
	stack_push_byte(STACK_TEXTCOLOR);
}

/* TPS */
void sl_pushTextSize(uint8_t type, int size) {
	stack_reserve(1 + 4 + 1);
	stack_push_byte(type);
	stack_push_dword(size);
	stack_push_byte(STACK_TEXTSIZE);
}

/* TPP */
void sl_pushTextLoc(int x, int y) {
	stack_reserve(4 + 4 + 1);
	stack_push_dword(x);
	stack_push_dword(y);
	stack_push_byte(STACK_TEXTLOC);
}

/* TOx */
void sl_popState(uint8_t expected_type) {
	uint8_t tag = stack_pop_byte();
	if (tag != expected_type) {
		// No-op if the stack top is of an unexpected type.
		stack_push_byte(tag);
		WARNING("unexpected stack top type");
		return;
	}
	pop_state(tag);
}

static void pop_state(uint8_t tag) {
	switch (tag) {
	case STACK_VARIABLE:
		{
			int value = stack_pop_word();
			int page = stack_pop_word();
			int index = stack_pop_word();
			varPage[page].value[index] = value;
		}
		break;
	case STACK_TEXTCOLOR:
		{
			uint8_t color = stack_pop_byte();
			uint8_t type = stack_pop_byte();
			if (type)
				nact->sel.MsgFontColor = color;
			else
				nact->msg.MsgFontColor = color;
		}
		break;
	case STACK_TEXTSIZE:
		{
			int size = stack_pop_dword();
			uint8_t type = stack_pop_byte();
			if (type)
				nact->sel.MsgFontSize = size;
			else
				nact->msg.MsgFontSize = size;
		}
		break;
	case STACK_TEXTLOC:
		{
			int x = stack_pop_dword();
			int y = stack_pop_dword();
			msg_setMessageLocation(x, y);
		}
		break;
	default:
		SYSERROR("broken stack");
	}
}

struct stack_frame_info *sl_next_stack_frame(struct stack_frame_info *sfi) {
	static struct stack_frame_info frame_info;
	if (!sfi) {
		sfi = &frame_info;
		sfi->p = stack_top;
	}
	if (sfi->p <= stack_buf)
		return NULL;
	sfi->tag = sfi->p[-1];
	switch (sfi->tag) {
	case STACK_NEARCALL:
		sfi->p -= 5;
		sfi->addr = LittleEndian_getDW(sfi->p, 0);
		break;
	case STACK_FARCALL:
		sfi->p -= 7;
		sfi->page = LittleEndian_getW(sfi->p, 0) - 1;
		sfi->addr = LittleEndian_getDW(sfi->p, 2);
		break;
	case STACK_VARIABLE:  sfi->p -= 7; break;
	case STACK_TEXTCOLOR: sfi->p -= 3; break;
	case STACK_TEXTSIZE:  sfi->p -= 6; break;
	case STACK_TEXTLOC:   sfi->p -= 9; break;
	default:
		SYSERROR("broken stack");
	}
	return sfi;
}

void *sl_setDataTable(int page, int index) {
	if (datatbl) {
		ald_freedata(datatbl);
	}
	datatbl = ald_getdata(DRIFILE_SCO, page);
	return (void *)(datatbl->data + index);
}

void sl_returnGoto(int address) {
	uint8_t tag;
	while ((tag = stack_pop_byte()) != STACK_FARCALL && tag != STACK_NEARCALL)
		pop_state(tag);

	if (tag == STACK_FARCALL) {
		stack_drop(4);
		int page = stack_pop_word() - 1;
		sl_jmpFar2(page, address);
	} else {
		stack_drop(4);
		sl_jmpNear(address);
	}
}
