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

#include <stdio.h>
#include <string.h>
#include "system.h"
#include "debugger.h"
#include "debugger_private.h"
#include "debug_symbol.h"
#include "nact.h"
#include "variable.h"
#ifdef HAVE_SIGACTION
#include <signal.h>
#endif
#ifdef _WIN32
#include "win/console.h"
#endif

static const char whitespaces[] = " \t\r\n";

static const char *format_address(int page, int addr) {
	static char buf[100];
	const char *src = dsym_page2src(symbols, page);
	int line = dsym_addr2line(symbols, page, addr);
	const char *func = dsym_addr2func(symbols, page, addr);

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

static void cmd_backtrace(void) {
	StackTrace *trace = dbg_stack_trace();

	for (int i = 0; i < trace->nr_frame; i++) {
		StackFrame *frame = &trace->frames[i];
		if (frame->src && frame->line > 0) {
			if (frame->name)
				printf("\t%s:%d in %s\n", frame->src, frame->line, frame->name);
			else
				printf("\t%s:%d\n", frame->src, frame->line);
		} else {
			printf("\t%d:0x%x\n", frame->page, frame->addr);
		}
	}

	free(trace);
}

static void cmd_break(void) {
	char *arg = strtok(NULL, whitespaces);
	int page, addr;
	if (!arg || !parse_address(arg, &page, &addr)) {
		printf("Syntax: break <filename>:<linenum>\n"
			   "        break <page>:<address>\n");
		return;
	}
	Breakpoint *bp = dbg_set_breakpoint(page, addr, false);
	if (bp) {
		printf("Breakpoint %d at %s\n", bp->no, format_address(bp->page, bp->addr));
	} else {
		Breakpoint *bp = dbg_find_breakpoint(page, addr);
		if (bp)
			printf("Breakpoint %d is already set at %d:0x%x.\n", bp->no, page, addr);
		else
			printf("Failed to set breakpoint at %d:0x%x: invalid address\n", page, addr);
	}
}

static void cmd_delete(void) {
	char *arg = strtok(NULL, whitespaces);
	if (!arg) {
		printf("Syntax: delete <breakpoint_no>...\n");
		return;
	}

	while (arg) {
		char *endptr;
		int bp_no = strtol(arg, &endptr, 0);
		if (*endptr)
			printf("Bad breakpoint number %s\n", arg);
		else {
			if (!dbg_delete_breakpoint(bp_no))
				printf("No breakpoint number %d.\n", bp_no);
		}
		arg = strtok(NULL, whitespaces);
	}
}

static void cmd_step(void) {
	dbg_step();
}

static void cmd_next(void) {
	dbg_next();
}

static void cmd_print(void) {
	char *arg = strtok(NULL, whitespaces);
	if (!arg) {
		printf("Syntax: print <variable>\n");
		return;
	}
	int var = dbg_lookup_var(arg);
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

static void sigint_handler(int sig_num) {
	dbg_state = DBG_STOPPED_INTERRUPT;
}

void dbg_cui_init(void) {
#ifdef HAVE_SIGACTION
	sys_set_signalhandler(SIGINT, sigint_handler);
#endif
#ifdef _WIN32
	win_alloc_console();
	win_set_ctrl_c_handler(sigint_handler);
#endif
}

void dbg_cui_repl(void) {
	if (dbg_state == DBG_STOPPED_BREAKPOINT) {
		Breakpoint *bp = dbg_find_breakpoint(nact->current_page, nact->current_addr);
		if (bp)
			printf("Breakpoint %d\n", bp->no);
	}
	if (!print_source_line(nact->current_page, nact->current_addr))
		printf("Stopped at %s\n", format_address(nact->current_page, nact->current_addr));
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
