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

typedef struct {
	int page;
	int addr;
	int refcnt;
	BYTE restore_op;
} PhysicalBreakpoint;

typedef struct breakpoint {
	struct breakpoint *next;
	PhysicalBreakpoint *phys;
	int no;
	char *condition;
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
	void (*init)(const char *symbols_path);
	void (*quit)(bool restart);
	void (*repl)(int bp_no);
	void (*onsleep)(void);
	void (*console_output)(int lv, const char *output);
} DebuggerImpl;

extern DebuggerImpl dbg_cui_impl;
extern DebuggerImpl dbg_dap_impl;
extern DebuggerImpl *dbg_impl;

Breakpoint *dbg_set_breakpoint(int page, int addr, boolean is_internal);
boolean dbg_delete_breakpoint(int no);
void dbg_delete_breakpoints_in_page(int page);
boolean dbg_set_breakpoint_condition(Breakpoint *bp, const char *condition, char *err, size_t errsize);
void dbg_stepin(void);
void dbg_stepout(void);
void dbg_next(void);
int dbg_lookup_var(const char *name);
boolean dbg_evaluate(const char *expr, char *result, size_t result_size);
StackTrace *dbg_stack_trace(void);

#endif // __DEBUGGER_PRIVATE_H__
