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

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "debugger.h"
#include "debugger_private.h"
#include "debug_symbol.h"
#include "system.h"
#include "ald_manager.h"
#include "scenario.h"
#include "nact.h"

#define INTERNAL_BREAKPOINT_NO -1

DebuggerState dbg_state = DBG_RUNNING;
DebuggerImpl *dbg_impl;

struct debug_symbols *symbols;
static Breakpoint *breakpoints = NULL;
static int next_breakpoint_no = 1;
static Breakpoint *internal_breakpoint;

// Used in DBG_STOPPED_STEP and DBG_STOPPED_NEXT
static struct {
	int page;
	int line;
} step_exec_state;

void dbg_init(const char *symbols_path, boolean use_dap) {
	dbg_impl = use_dap ? &dbg_dap_impl : &dbg_cui_impl;
	dbg_impl->init();
	dbg_state = DBG_STOPPED_ENTRY;
	symbols = dsym_load(symbols_path);
}

void dbg_quit() {
	if (dbg_impl)
		dbg_impl->quit();
}

int dbg_lookup_var(const char *name) {
	if (symbols)
		return dsym_lookup_variable(symbols, name);

	// If debug information is not available, lookup from System39.ain.
	if (nact->ain.var) {
		for (int i = 0; i < nact->ain.varnum; i++) {
			if (!strcmp(name, nact->ain.var[i]))
				return i;
		}
	}
	return -1;
}

Breakpoint *dbg_find_breakpoint(int page, int addr) {
	for (Breakpoint *bp = breakpoints; bp; bp = bp->next) {
		if (bp->page == page && bp->addr == addr)
			return bp;
	}
	return NULL;
}

Breakpoint *dbg_set_breakpoint(int page, int addr, boolean is_internal) {
	dridata *dfile = ald_getdata(DRIFILE_SCO, page);
	if (!dfile)
		return NULL;
	if (addr < 0 || addr >= dfile->size || dfile->data[addr] == BREAKPOINT) {
		ald_freedata(dfile);
		return NULL;
	}

	Breakpoint *bp = calloc(1, sizeof(Breakpoint));
	bp->no = is_internal ? INTERNAL_BREAKPOINT_NO : next_breakpoint_no++;
	bp->page = page;
	bp->addr = addr;
	bp->dfile = dfile;
	bp->restore_op = dfile->data[addr];
	bp->next = breakpoints;
	breakpoints = bp;

	dfile->data[addr] = BREAKPOINT;

	return bp;
}

boolean dbg_delete_breakpoint(int no) {
	Breakpoint *prev = NULL;
	for (Breakpoint *bp = breakpoints; bp; bp = bp->next) {
		if (bp->no == no) {
			assert(bp->dfile->data[bp->addr] == BREAKPOINT);
			bp->dfile->data[bp->addr] = bp->restore_op;
			ald_freedata(bp->dfile);
			if (prev)
				prev->next = bp->next;
			else
				breakpoints = bp->next;
			free(bp);
			return true;
		}
		prev = bp;
	}
	return false;
}

void dbg_delete_breakpoints_in_page(int page) {
	Breakpoint *prev = NULL;
	for (Breakpoint *bp = breakpoints; bp;) {
		if (bp->page == page) {
			assert(bp->dfile->data[bp->addr] == BREAKPOINT);
			bp->dfile->data[bp->addr] = bp->restore_op;
			ald_freedata(bp->dfile);
			if (prev)
				prev->next = bp->next;
			else
				breakpoints = bp->next;
			bp = bp->next;
			free(bp);
		} else {
			prev = bp;
			bp = bp->next;
		}
	}
}

BYTE dbg_handle_breakpoint(int page, int addr) {
	Breakpoint *bp = dbg_find_breakpoint(page, addr);
	if (!bp)
		SYSERROR("Illegal BREAKPOINT instruction");

	dbg_state = bp->no == INTERNAL_BREAKPOINT_NO ?
		DBG_STOPPED_NEXT : DBG_STOPPED_BREAKPOINT;

	BYTE restore_op = bp->restore_op;
	dbg_main();  // this may destroy bp
	return restore_op;
}

static void set_stack_frame(StackFrame *frame, int page, int addr, boolean is_return_addr) {
	frame->page = page;
	frame->addr = addr;
	frame->src = dsym_page2src(symbols, page);

	// For return addresses, search with (addr - 1) so that the function call's
	// line number will be returned.
	frame->line = dsym_addr2line(symbols, page, is_return_addr ? addr - 1 : addr);
	frame->name = dsym_addr2func(symbols, page, is_return_addr ? addr - 1 : addr);
}

StackTrace *dbg_stack_trace(void) {
	int stack_size;
	const int *stack_base = sl_getStackInfo(&stack_size);

	int page = nact->current_page;

	int cap = 16;
	StackTrace *trace = malloc(sizeof(StackTrace) + cap * sizeof(StackFrame));
	set_stack_frame(&trace->frames[0], page, nact->current_addr, false);
	trace->nr_frame = 1;

	const int *p = stack_base + stack_size - 1;
	while (p >= stack_base) {
		int addr = -1;
		switch (*p) {
		case STACK_NEARJMP:
			addr = p[-2];
			break;
		case STACK_FARJMP:
			page = p[-2];
			addr = p[-3];
			break;
		}
		if (addr >= 0) {
			if (trace->nr_frame >= cap) {
				cap *= 2;
				trace = realloc(trace, sizeof(StackTrace) + cap * sizeof(StackFrame));
			}
			set_stack_frame(&trace->frames[trace->nr_frame++], page, addr, true);
		}
		p -= p[-1] + 2;
	}

	return trace;
}

void dbg_stepin(void) {
	step_exec_state.page = nact->current_page;
	step_exec_state.line = dsym_addr2line(symbols, step_exec_state.page, nact->current_addr);
	dbg_state = DBG_STOPPED_STEP;
}

static boolean should_continue_step(void) {
	if (step_exec_state.line < 0)
		return false;  // line info was not available
	return step_exec_state.page == nact->current_page
		&& step_exec_state.line == dsym_addr2line(symbols, step_exec_state.page, nact->current_addr);
}

void dbg_stepout(void) {
	// Set an internal breakpoint at the return address of current frame.
	int stack_size;
	const int *stack_base = sl_getStackInfo(&stack_size);
	const int *p = stack_base + stack_size - 1;
	while (p >= stack_base) {
		int page, addr = -1;
		switch (*p) {
		case STACK_NEARJMP:
			page = nact->current_page;
			addr = p[-2];
			break;
		case STACK_FARJMP:
			page = p[-2];
			addr = p[-3];
			break;
		}
		if (addr >= 0) {
			internal_breakpoint = dbg_set_breakpoint(page, addr, true);
			return;
		}
		p -= p[-1] + 2;
	}
	// No parent frame found, continue execution.
}

static int get_retaddr_if_funcall(void) {
	int orig_addr = sl_getIndex();
	int retaddr = -1;

	assert(nact->current_page == sl_getPage());
	sl_jmpNear(nact->current_addr);
	int c0 = sl_getc();
	if (c0 == BREAKPOINT) {
		Breakpoint *bp = dbg_find_breakpoint(nact->current_page, nact->current_addr);
		if (bp)
			c0 = bp->restore_op;
		else
			WARNING("Illegal BREAKPOINT instruction");
	}
	switch (c0) {
	case '%':
		if (getCaliValue() != 0)
			retaddr = sl_getIndex();
		break;
	case '\\':
		if (sl_getaddr() != 0)
			retaddr = sl_getIndex();
		break;
	case '~':
		{
			int page = sl_getw();
			if (page != 0 && page != 0xffff) {
				sl_getaddr();
				retaddr = sl_getIndex();
			}
		}
		break;
	}
	sl_jmpNear(orig_addr);
	return retaddr;
}

static void do_next(void) {
	assert(dbg_state == DBG_STOPPED_NEXT);

	int retaddr = get_retaddr_if_funcall();
	if (retaddr >= 0) {
		internal_breakpoint = dbg_set_breakpoint(step_exec_state.page, retaddr, true);
		dbg_state = DBG_RUNNING;
	}
}

void dbg_next(void) {
	step_exec_state.page = nact->current_page;
	step_exec_state.line = dsym_addr2line(symbols, step_exec_state.page, nact->current_addr);
	dbg_state = DBG_STOPPED_NEXT;
	do_next();
}

static boolean should_continue_next(void) {
	if (!should_continue_step())
		return false;
	do_next();
	return true;
}

void dbg_main(void) {
	if (internal_breakpoint) {
		dbg_delete_breakpoint(INTERNAL_BREAKPOINT_NO);
		internal_breakpoint = NULL;
	}

	switch (dbg_state) {
	case DBG_STOPPED_STEP:
		if (should_continue_step())
			return;
		break;
	case DBG_STOPPED_NEXT:
		if (should_continue_next())
			return;
		break;
	default:
		break;
	}
	dbg_impl->repl();
}

void dbg_onsleep(void) {
	if (dbg_impl)
		dbg_impl->onsleep();
}
