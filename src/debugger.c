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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debugger.h"
#include "debug_symbol.h"
#include "system.h"
#include "ald_manager.h"
#include "scenario.h"
#include "variable.h"
#include "nact.h"
#ifdef HAVE_SIGACTION
#include <signal.h>
#endif
#ifdef _WIN32
#include "win/console.h"
#endif

#define INTERNAL_BREAKPOINT_NO -1

DebuggerState dbg_state = DBG_RUNNING;

typedef struct breakpoint {
	struct breakpoint *next;
	int no;
	int page;
	int addr;
	char restore_op;
	dridata *dfile;  // keeps modified scenario page alive in the cache
} Breakpoint;

static struct debug_symbols *symbols;
static Breakpoint *breakpoints = NULL;
static int next_breakpoint_no = 1;
static Breakpoint *internal_breakpoint;

// Used in DBG_STOPPED_STEP and DBG_STOPPED_NEXT
static struct {
	int page;
	int line;
} step_exec_state;

static const char whitespaces[] = " \t\r\n";

static const char *format_address(int page, int addr, boolean is_return_addr) {
	static char buf[100];
	const char *src = dsym_page2src(symbols, page);

	// For return addresses, search with (addr - 1) so that the function call's
	// line number will be printed.
	int line = dsym_addr2line(symbols, page, is_return_addr ? addr - 1 : addr);
	const char *func = dsym_addr2func(symbols, page, is_return_addr ? addr - 1 : addr);

	if (src && line > 0) {
		if (func)
			snprintf(buf, sizeof(buf), "%s:%d in %s", src, line, func);
		else
			snprintf(buf, sizeof(buf), "%s:%d", src, line);
	} else {
		snprintf(buf, sizeof(buf), "%d:0x%x", page, addr);
	}
	return buf;
}

static boolean parse_address(const char *str, int *page, int *addr) {
	// <page>:<address>
	if (sscanf(str, "%i:%i", page, addr) == 2)
		return true;

	// <filename>:<linenum>
	char filename[100];
	int line_no;
	if (sscanf(str, "%[^:]:%i", filename, &line_no) == 2) {
		*page = dsym_src2page(symbols, filename);
		if (*page < 0) {
			printf("No source file named %s.\n", filename);
			return false;
		}
		*addr = dsym_line2addr(symbols, *page, line_no);
		if (*addr < 0) {
			printf("No line %d in file %s.\n", line_no, filename);
			return false;
		}
		return true;
	}

	*page = nact->current_page;

	// <linenum>
	char *endptr;
	line_no = strtol(str, &endptr, 0);
	if (!*endptr) {
		*addr = dsym_line2addr(symbols, nact->current_page, line_no);
		if (*addr < 0) {
			printf("No line %d in current file.\n", line_no);
			return false;
		}
		return true;
	}

	// <funcname>
	if (dsym_func2addr(symbols, str, page, addr))
		return true;

	return false;
}

static boolean print_source_line(int page, int addr) {
	int line_no = dsym_addr2line(symbols, page, addr);
	if (line_no <= 0)
		return false;
	const char *line = dsym_source_line(symbols, page, line_no);
	if (!line)
		return false;
	printf("%s:%d\t%s\n", dsym_page2src(symbols, page), line_no, line);
	return true;
}

static int lookup_var(const char *name) {
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

static Breakpoint *find_breakpoint(int page, int addr) {
	for (Breakpoint *bp = breakpoints; bp; bp = bp->next) {
		if (bp->page == page && bp->addr == addr)
			return bp;
	}
	return NULL;
}

static Breakpoint *set_breakpoint(int page, int addr, boolean is_internal) {
	dridata *dfile = ald_getdata(DRIFILE_SCO, page);
	if (!dfile) {
		printf("Invalid page number %d\n", page);
		return NULL;
	}
	if (addr < 0 || addr >= dfile->size) {
		printf("Invalid address 0x%x\n", addr);
		ald_freedata(dfile);
		return NULL;
	}

	if (dfile->data[addr] == BREAKPOINT) {
		Breakpoint *bp = find_breakpoint(page, addr);
		if (bp)
			printf("Breakpoint %d is already set at %d:0x%x.\n", bp->no, page, addr);
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

static void delete_breakpoint(int no) {
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
			return;
		}
		prev = bp;
	}
	printf("No breakpoint number %d.\n", no);
}

static void sigint_handler(int sig_num) {
	dbg_state = DBG_STOPPED_INTERRUPT;
}

void dbg_init(const char *symbols_path) {
#ifdef HAVE_SIGACTION
	sys_set_signalhandler(SIGINT, sigint_handler);
#endif
#ifdef _WIN32
	win_alloc_console();
	win_set_ctrl_c_handler(sigint_handler);
#endif

	dbg_state = DBG_STOPPED_ENTRY;
	symbols = dsym_load(symbols_path);
}

int dbg_handle_breakpoint(int page, int addr) {
	Breakpoint *bp = find_breakpoint(page, addr);
	if (!bp)
		SYSERROR("Illegal BREAKPOINT instruction");

	if (bp->no != INTERNAL_BREAKPOINT_NO) {
		printf("Breakpoint %d, %s\n", bp->no, format_address(page, addr, false));
		dbg_state = DBG_STOPPED_BREAKPOINT;
	} else {
		dbg_state = DBG_STOPPED_NEXT;
	}

	char restore_op = bp->restore_op;
	dbg_repl();  // this may destroy bp
	return restore_op;
}

static void cmd_backtrace(void) {
	int stack_size;
	const int *stack_base = sl_getStackInfo(&stack_size);

	int page = nact->current_page;
	printf("\t%s\n", format_address(page, nact->current_addr, false));

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
		if (addr >= 0)
			printf("\t%s\n", format_address(page, addr, true));
		p -= p[-1] + 2;
	}
}

static void cmd_break(void) {
	char *arg = strtok(NULL, whitespaces);
	int page, addr;
	if (!arg || !parse_address(arg, &page, &addr)) {
		printf("Syntax: break <filename>:<linenum>\n"
			   "        break <page>:<address>\n");
		return;
	}
	Breakpoint *bp = set_breakpoint(page, addr, false);
	if (bp)
		printf("Breakpoint %d at %s\n", bp->no, format_address(bp->page, bp->addr, false));
}

static void cmd_delete(void) {
	char *arg = strtok(NULL, whitespaces);
	if (!arg) {
		// Delete all breakpoints
		while (breakpoints)
			delete_breakpoint(breakpoints->no);
		return;
	}

	while (arg) {
		char *endptr;
		int bp_no = strtol(arg, &endptr, 0);
		if (*endptr)
			printf("Bad breakpoint number %s\n", arg);
		else
			delete_breakpoint(bp_no);
		arg = strtok(NULL, whitespaces);
	}
}

static void cmd_step(void) {
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

static int get_retaddr_if_funcall(void) {
	int orig_addr = sl_getIndex();
	int retaddr = -1;

	assert(nact->current_page == sl_getPage());
	sl_jmpNear(nact->current_addr);
	int c0 = sl_getc();
	if (c0 == BREAKPOINT) {
		Breakpoint *bp = find_breakpoint(nact->current_page, nact->current_addr);
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
		internal_breakpoint = set_breakpoint(step_exec_state.page, retaddr, true);
		dbg_state = DBG_RUNNING;
	}
}

static void cmd_next(void) {
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

static void cmd_print(void) {
	char *arg = strtok(NULL, whitespaces);
	if (!arg) {
		printf("Syntax: print <variable>\n");
		return;
	}
	int var = lookup_var(arg);
	if (var < 0) {
		printf("Unrecognized variable name \"%s\".\n", arg);
		return;
	}
	printf("%s = %d\n", arg, sysVar[var]);
}

static void cmd_quit(void) {
	sys_exit(0);
}

static void cmd_string(void) {
	char *arg = strtok(NULL, whitespaces);
	if (!arg) {
		printf("Syntax: string <index>\n");
		return;
	}

	while (arg) {
		int no = atoi(arg);
		if (no < 1) {
			printf("Bad string index %s\n", arg);
		} else {
			char *utf = toUTF8(svar_get(no));
			printf("string[%d] = \"%s\"\n", no, utf);  // TODO: escaping
			free(utf);
		}
		arg = strtok(NULL, whitespaces);
	}
}

void dbg_repl(void) {
	if (internal_breakpoint) {
		delete_breakpoint(INTERNAL_BREAKPOINT_NO);
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

	if (!print_source_line(nact->current_page, nact->current_addr))
		printf("Stopped at %s\n", format_address(nact->current_page, nact->current_addr, false));
	dbg_state = DBG_RUNNING;

	char buf[256];
	while (printf("dbg> "), fflush(stdout), fgets(buf, sizeof(buf), stdin)) {
		char *cmd = strtok(buf, whitespaces);
		if (!cmd) {
			// TODO: repeat last command
			continue;
		}
		if (!strcmp(cmd, "c") || !strcmp(cmd, "continue")) {
			break;
		} else if (!strcmp(cmd, "bt") || !strcmp(cmd, "backtrace")) {
			cmd_backtrace();
		} else if (!strcmp(cmd, "b") || !strcmp(cmd, "break")) {
			cmd_break();
		} else if (!strcmp(cmd, "d") || !strcmp(cmd, "delete")) {
			cmd_delete();
		} else if (!strcmp(cmd, "s") || !strcmp(cmd, "step")) {
			cmd_step();
			return;
		} else if (!strcmp(cmd, "n") || !strcmp(cmd, "next")) {
			cmd_next();
			return;
		} else if (!strcmp(cmd, "p") || !strcmp(cmd, "print")) {
			cmd_print();
		} else if (!strcmp(cmd, "q") || !strcmp(cmd, "quit")) {
			cmd_quit();
		} else if (!strcmp(cmd, "str") || !strcmp(cmd, "string")) {
			cmd_string();
		} else {
			printf("Unknown command \"%s\".\n", cmd);
		}
	}
}
