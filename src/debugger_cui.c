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

#define CMD_BUFFER_SIZE 256

static const char whitespaces[] = " \t\r\n";
static char default_command[CMD_BUFFER_SIZE];

typedef enum { CONTINUE_REPL, EXIT_REPL } CommandResult;

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

static const char desc_backtrace[] = "Print backtrace of stack frames.";
static const char * const help_backtrace = NULL;

static CommandResult cmd_backtrace(void) {
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
	return CONTINUE_REPL;
}

static const char desc_break[] = "Set breakpoint at specified location.";
static const char help_break[] =
	"Syntax: break <filename>:<linenum> [if <condition>]\n"
	"        break <linenum> [if <condition>]\n"
	"        break <page>:<address> [if <condition>]\n"
	"        break <function> [if <condition>]";

static CommandResult cmd_break(void) {
	char *token = strtok(NULL, whitespaces);
	int page, addr;
	if (!token || !parse_address(token, &page, &addr)) {
		puts(help_break);
		return CONTINUE_REPL;
	}
	Breakpoint *bp = dbg_set_breakpoint(page, addr, false);
	if (!bp) {
		Breakpoint *bp = dbg_find_breakpoint(page, addr);
		if (bp)
			printf("Breakpoint %d is already set at %d:0x%x.\n", bp->no, page, addr);
		else
			printf("Failed to set breakpoint at %d:0x%x: invalid address\n", page, addr);
		return CONTINUE_REPL;
	}

	token = strtok(NULL, whitespaces);
	if (token && !strcmp(token, "if")) {
		char *condition = strtok(NULL, "");
		if (!condition)
			condition = "";

		char errmsg[256];
		if (!dbg_set_breakpoint_condition(bp, condition, errmsg, sizeof(errmsg))) {
			puts(errmsg);
			dbg_delete_breakpoint(bp->no);
			return CONTINUE_REPL;
		}
	}

	printf("Breakpoint %d at %s\n", bp->no, format_address(bp->page, bp->addr));
	return CONTINUE_REPL;
}

static const char desc_continue[] = "Continue program execution.";
static const char * const help_continue = NULL;

static CommandResult cmd_continue(void) {
	return EXIT_REPL;
}

static const char desc_delete[] = "Delete breakpoints.";
static const char help_delete[] =
	"Syntax: delete <breakpoint_no>...";

static CommandResult cmd_delete(void) {
	char *arg = strtok(NULL, whitespaces);
	if (!arg) {
		puts(help_delete);
		return CONTINUE_REPL;
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
	return CONTINUE_REPL;
}

static const char desc_help[] = "Print list of commands.";
static const char * const help_help = NULL;

static CommandResult cmd_help(void);

static const char desc_list[] = "List specified function or line.";
static const char help_list[] =
	"Syntax: list\n"
	"        list <linenum>\n"
	"        list <filename>:<linenum>\n"
	"        list <function>\n"
	"\n"
	"With no argument, lists ten lines around current location.";

static CommandResult cmd_list(void) {
	char *arg = strtok(NULL, whitespaces);
	int page, addr;
	if (arg) {
		if (!parse_address(arg, &page, &addr)) {
			puts(help_list);
			return CONTINUE_REPL;
		}
	} else {
		page = nact->current_page;
		addr = nact->current_addr;
	}

	int line_no = dsym_addr2line(symbols, page, addr);
	if (line_no <= 0) {
		printf("Cannot determine source location for %x:%x\n", page, addr);
		return CONTINUE_REPL;
	}

	if ((line_no -= 5) <= 0)
		line_no = 1;

	for (int i = 0; i < 10; i++) {
		const char *line = dsym_source_line(symbols, page, line_no + i);
		if (!line) {
			if (i == 0)
				printf("No source text for %s:%d\n", dsym_page2src(symbols, page), line_no);
			break;
		}
		printf("%d\t%s\n", line_no + i, line);
	}

	// Empty REPL input will print the next 10 lines.
	sprintf(default_command, "l %s:%d", dsym_page2src(symbols, page), line_no + 15);

	return CONTINUE_REPL;
}

static const char desc_step[] = "Step program until it reaches a different source line.";
static const char * const help_step = NULL;

static CommandResult cmd_step(void) {
	dbg_stepin();
	return EXIT_REPL;
}

static const char desc_finish[] = "Execute until current function returns.";
static const char * const help_finish = NULL;

static CommandResult cmd_finish(void) {
	dbg_stepout();
	return EXIT_REPL;
}

static const char desc_next[] = "Step program, proceeding through subroutine calls.";
static const char * const help_next = NULL;

static CommandResult cmd_next(void) {
	dbg_next();
	return EXIT_REPL;
}

static const char desc_print[] = "Print value of expression.";
static const char help_print[] =
	"Syntax: print <expr>";

static CommandResult cmd_print(void) {
	char *expr = strtok(NULL, "");
	if (!expr) {
		puts(help_print);
		return CONTINUE_REPL;
	}

	char buf[256];
	dbg_evaluate(expr, buf, sizeof(buf));
	puts(buf);
	return CONTINUE_REPL;
}

static const char desc_quit[] = "Exit " PACKAGE ".";
static const char * const help_quit = NULL;

static CommandResult cmd_quit(void) {
	sys_exit(0);
	return EXIT_REPL;
}

static const char desc_string[] = "Print value of string variable.";
static const char help_string[] =
	"Syntax: string <index>";

static CommandResult cmd_string(void) {
	char *arg = strtok(NULL, whitespaces);
	if (!arg) {
		puts(help_string);
		return CONTINUE_REPL;
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
	return CONTINUE_REPL;
}

static void sigint_handler(int sig_num) {
	dbg_state = DBG_STOPPED_INTERRUPT;
}

static void dbg_cui_init(const char *symbols_path) {
	symbols = dsym_load(symbols_path);
#ifdef HAVE_SIGACTION
	sys_set_signalhandler(SIGINT, sigint_handler);
#endif
#ifdef _WIN32
	win_alloc_console();
	win_set_ctrl_c_handler(sigint_handler);
#endif
	dbg_state = DBG_STOPPED_ENTRY;
}

static void dbg_cui_quit(bool restart) {
}

typedef struct {
	const char *name;
	const char *alias;
	const char *description;
	const char *help;
	CommandResult (*func)(void);
} Command;

const Command dbg_cui_commands[] = {
	{"backtrace", "bt",  desc_backtrace, help_backtrace, cmd_backtrace},
	{"break",     "b",   desc_break,     help_break,     cmd_break},
	{"continue",  "c",   desc_continue,  help_continue,  cmd_continue},
	{"delete",    "d",   desc_delete,    help_delete,    cmd_delete},
	{"help",      "h",   desc_help,      help_help,      cmd_help},
	{"list",      "l",   desc_list,      help_list,      cmd_list},
	{"step",      "s",   desc_step,      help_step,      cmd_step},
	{"finish",    NULL,  desc_finish,    help_finish,    cmd_finish},
	{"next",      "n",   desc_next,      help_next,      cmd_next},
	{"print",     "p",   desc_print,     help_print,     cmd_print},
	{"quit",      "q",   desc_quit,      help_quit,      cmd_quit},
	{"string",    "str", desc_string,    help_string,    cmd_string},
	{NULL}  // terminator
};

const Command *find_command(const char *str) {
	for (const Command *cmd = dbg_cui_commands; cmd->name; cmd++) {
		if (!strcmp(str, cmd->name) || (cmd->alias && !strcmp(str, cmd->alias)))
			return cmd;
	}
	printf("Unknown command \"%s\". Try \"help\".\n", str);
	return NULL;
}

static void cmd_print_help(const Command *cmd) {
	if (cmd->alias)
		printf("%s, %s -- %s\n", cmd->name, cmd->alias, cmd->description);
	else
		printf("%s -- %s\n", cmd->name, cmd->description);
}

static CommandResult cmd_help(void) {
	char *token = strtok(NULL, whitespaces);
	if (!token) {
		puts("List of commands:");
		puts("");
		for (const Command *cmd = dbg_cui_commands; cmd->name; cmd++)
			cmd_print_help(cmd);
		puts("");
		puts("Type \"help\" followed by command name for full documentation.");
		return CONTINUE_REPL;
	}
	const Command *cmd = find_command(token);
	if (cmd) {
		cmd_print_help(cmd);
		if (cmd->help) {
			puts("");
			puts(cmd->help);
		}
	}
	return CONTINUE_REPL;
}

static void dbg_cui_repl(void) {
	if (dbg_state == DBG_STOPPED_BREAKPOINT) {
		Breakpoint *bp = dbg_find_breakpoint(nact->current_page, nact->current_addr);
		if (bp)
			printf("Breakpoint %d\n", bp->no);
	}
	if (!print_source_line(nact->current_page, nact->current_addr))
		printf("Stopped at %s\n", format_address(nact->current_page, nact->current_addr));
	dbg_state = DBG_RUNNING;

	char buf[CMD_BUFFER_SIZE];
	while (printf("dbg> "), fflush(stdout), fgets(buf, CMD_BUFFER_SIZE, stdin)) {
		// If input line is empty, repeat the last command.
		if (buf[0] == '\n')
			strcpy(buf, default_command);
		else
			strcpy(default_command, buf);

		char *token = strtok(buf, whitespaces);
		if (!token)
			continue;

		const Command *cmd = find_command(token);
		if (cmd) {
			if (cmd->func() == EXIT_REPL)
				return;
		}
	}
}

static void dbg_cui_onsleep(void) {
	if (dbg_state == DBG_STOPPED_INTERRUPT)
		dbg_main();
}

DebuggerImpl dbg_cui_impl = {
	.init = dbg_cui_init,
	.quit = dbg_cui_quit,
	.repl = dbg_cui_repl,
	.onsleep = dbg_cui_onsleep,
};
