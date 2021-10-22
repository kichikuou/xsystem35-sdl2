/*
 * Copyright (C) 2021 kichikuou <KichikuouChrome@gmail.com>
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

#ifndef __DEBUGGER_PRIVATE_H__
#define __DEBUGGER_PRIVATE_H__

#include "portab.h"
#include "dri.h"

extern struct debug_symbols *symbols;

typedef struct breakpoint {
	struct breakpoint *next;
	int no;
	int page;
	int addr;
	BYTE restore_op;
	dridata *dfile;  // keeps modified scenario page alive in the cache
} Breakpoint;

typedef struct {
	int page;
	int addr;
	const char *src;
	int line;
	const char *name;
} StackFrame;

typedef struct {
	int nr_frame;
	StackFrame frames[];
} StackTrace;

typedef struct {
	void (*init)(void);
	void (*quit)(void);
	void (*repl)(void);
	void (*onsleep)(void);
} DebuggerImpl;

extern DebuggerImpl dbg_cui_impl;
extern DebuggerImpl dbg_dap_impl;
extern DebuggerImpl *dbg_impl;

Breakpoint *dbg_find_breakpoint(int page, int addr);
Breakpoint *dbg_set_breakpoint(int page, int addr, boolean is_internal);
boolean dbg_delete_breakpoint(int no);
void dbg_delete_breakpoints_in_page(int page);
void dbg_stepin(void);
void dbg_stepout(void);
void dbg_next(void);
int dbg_lookup_var(const char *name);
StackTrace *dbg_stack_trace(void);

#endif // __DEBUGGER_PRIVATE_H__
