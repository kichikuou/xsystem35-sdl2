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
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include "debugger.h"
#include "debugger_private.h"
#include "debug_symbol.h"
#include "system.h"
#include "ald_manager.h"
#include "scenario.h"
#include "nact.h"
#include "variable.h"

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
	dbg_impl->init(symbols_path);
}

void dbg_quit(bool restart) {
	if (dbg_impl)
		dbg_impl->quit(restart);
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

static const char *eval_input;
static jmp_buf eval_jmp_buf;
static char *eval_result;
static size_t eval_result_size;
static int eval_expr(void);

static void eval_error(char *format, ...) {
	va_list args;
	va_start(args, format);
	vsnprintf(eval_result, eval_result_size, format, args);
	va_end(args);

	longjmp(eval_jmp_buf, 1);
}

static char next_char(void) {
	while (isspace(*eval_input))
		eval_input++;
	return *eval_input;
}

static boolean consume(char c) {
	if (next_char() != c)
		return false;
	eval_input++;
	return true;
}

static void expect(char c) {
	if (next_char() != c)
		eval_error("syntax error");
	eval_input++;
}

static int clamp(int val) {
	return val > 0xffff ? 0xffff
		: val < 0 ? 0
		: val;
}

static boolean is_identifier(uint8_t c) {
	return isalnum(c) || !isascii(c) || c == '_' || c == '.';
}

static int parse_number(void) {
	int base = 10;
	if (eval_input[0] == '0' && tolower(eval_input[1]) == 'x') {
		base = 16;
		eval_input += 2;
	} else if (eval_input[0] == '0' && tolower(eval_input[1]) == 'b') {
		base = 2;
		eval_input += 2;
	}
	char *p;
	long val = strtol(eval_input, &p, base);
	eval_input = p;
	return clamp(val);
}

static int eval_variable(void) {
	const char *top = eval_input;
	while (is_identifier(*eval_input))
		eval_input++;
	if (top == eval_input)
		eval_error("syntax error");

	char *buf = alloca(eval_input - top + 1);
	strncpy(buf, top, eval_input - top);
	buf[eval_input - top] = '\0';
	int var = dbg_lookup_var(buf);
	if (var < 0)
		eval_error("unknown variable \"%s\"", buf);
	if (consume('[')) {
		int index = eval_expr();
		expect(']');
		return *v_ref_indexed(var, index);;
	} else {
		return *v_ref(var);
	}
}

static int eval_prim(void) {
	if (consume('(')) {
		int val = eval_expr();
		expect(')');
		return val;
	}
	if (isdigit(next_char()))
		return parse_number();

	return eval_variable();
}

static int eval_mul(void) {
	int val = eval_prim();
	for (;;) {
		if (consume('*')) {
			val = clamp(val * eval_prim());
		} else if (consume('/')) {
			int rhs = eval_prim();
			val = rhs ? val / rhs : 0;
		} else if (consume('%')) {
			int rhs = eval_prim();
			val = rhs ? val % rhs : 0;
		} else {
			break;
		}
	}
	return val;
}

static int eval_add(void) {
	int val = eval_mul();
	for (;;) {
		if (consume('+'))
			val = clamp(val + eval_mul());
		else if (consume('-'))
			val = clamp(val - eval_mul());
		else
			break;
	}
	return val;
}

static int eval_bit(void) {
	int val = eval_add();
	for (;;) {
		if (consume('&'))
			val = val & eval_add();
		else if (consume('|'))
			val = val | eval_add();
		else if (consume('^'))
			val = val ^ eval_add();
		else
			break;
	}
	return val;
}

static int eval_compare(void) {
	int val = eval_bit();
	for (;;) {
		if (consume('<')) {
			if (consume('='))
				val = val <= eval_bit() ? 1 : 0;
			else
				val = val < eval_bit() ? 1 : 0;
		} else if (consume('>')) {
			if (consume('='))
				val = val >= eval_bit() ? 1 : 0;
			else
				val = val > eval_bit() ? 1 : 0;
		} else {
			break;
		}
	}
	return val;
}

static int eval_expr(void) {
	int val = eval_compare();
	for (;;) {
		if (consume('=')) {
			val = val == eval_compare() ? 1 : 0;
		} else if (consume('\\')) {
			val = val != eval_compare() ? 1 : 0;
		} else {
			break;
		}
	}
	return val;
}

static int eval_condition(const char *expr) {
	eval_input = expr;
	eval_result = NULL;
	eval_result_size = 0;
	if (!setjmp(eval_jmp_buf)) {
		return eval_expr();
	} else {
		return 0;
	}
}

boolean dbg_evaluate(const char *expr, char *result, size_t result_size) {
	eval_input = expr;
	eval_result = result;
	eval_result_size = result_size;
	if (!setjmp(eval_jmp_buf)) {
		int val = eval_expr();
		expect('\0');
		snprintf(result, result_size, "%d", val);
		return true;
	} else {
		return false;
	}
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

boolean dbg_set_breakpoint_condition(Breakpoint *bp, const char *condition, char *err, size_t errsize) {
	if (!dbg_evaluate(condition, err, errsize))
		return false;
	if (bp->condition)
		free(bp->condition);
	bp->condition = strdup(condition);
	return true;
}

static Breakpoint *breakpoint_free(Breakpoint *bp) {
	assert(bp->dfile->data[bp->addr] == BREAKPOINT);
	bp->dfile->data[bp->addr] = bp->restore_op;
	if (bp->condition)
		free(bp->condition);
	ald_freedata(bp->dfile);
	Breakpoint *next = bp->next;
	free(bp);
	return next;
}

boolean dbg_delete_breakpoint(int no) {
	for (Breakpoint **p = &breakpoints; *p; p = &(*p)->next) {
		if ((*p)->no == no) {
			*p = breakpoint_free(*p);
			return true;
		}
	}
	return false;
}

void dbg_delete_breakpoints_in_page(int page) {
	Breakpoint **p = &breakpoints;
	while (*p) {
		if ((*p)->page == page)
			*p = breakpoint_free(*p);
		else
			p = &(*p)->next;
	}
}

BYTE dbg_handle_breakpoint(int page, int addr) {
	Breakpoint *bp = dbg_find_breakpoint(page, addr);
	if (!bp)
		SYSERROR("Illegal BREAKPOINT instruction");

	if (bp->condition && !eval_condition(bp->condition))
		return bp->restore_op;

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

boolean dbg_console_vprintf(int lv, const char *format, va_list ap) {
	if (!dbg_impl || !dbg_impl->console_output)
		return false;
	char buf[1024];
	vsnprintf(buf, sizeof(buf), format, ap);
	dbg_impl->console_output(lv, buf);
	return true;
}
