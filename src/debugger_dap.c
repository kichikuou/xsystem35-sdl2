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
#include <stdlib.h>
#include <string.h>
#include <SDL_thread.h>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

#include "cjson/cJSON.h"
#include "debugger.h"
#include "debugger_private.h"
#include "debug_symbol.h"
#include "msgqueue.h"
#include "sdl_core.h"
#include "system.h"
#include "nact.h"
#include "variable.h"

#define THREAD_ID 1

enum VariablesReference {
	VREF_GLOBALS = 1,
	VREF_STRINGS,
};

// Exception Breakpoint Filters.
static const char ebf_warnings[] = "warnings";

static bool initialized;
static char *symbols_path;
static char *src_dir;
static struct msgq *queue;
static bool break_on_warnings;

cJSON *create_source(const char *name) {
	cJSON *source = cJSON_CreateObject();
	cJSON_AddStringToObject(source, "name", name);
	if (src_dir) {
		char *buf = alloca(strlen(src_dir) + strlen(name) + 2);
		sprintf(buf, "%s/%s", src_dir, name);
		cJSON_AddStringToObject(source, "path", buf);
	}
	cJSON_AddNumberToObject(source, "sourceReference", 0);
	return source;
}

static void send_json(cJSON *json) {
	static int seq = 0;

	cJSON_AddNumberToObject(json, "seq", seq);
	char *str = cJSON_PrintUnformatted(json);
	printf("Content-Length: %zu\r\n\r\n%s", strlen(str), str);
	fflush(stdout);
	free(str);
	cJSON_free(json);
}

static void emit_initialized_event(void) {
	cJSON *event = cJSON_CreateObject();
	cJSON_AddStringToObject(event, "type", "event");
	cJSON_AddStringToObject(event, "event", "initialized");
	send_json(event);
}

static void emit_terminated_event(bool restart) {
	cJSON *event = cJSON_CreateObject(), *body;
	cJSON_AddStringToObject(event, "type", "event");
	cJSON_AddStringToObject(event, "event", "terminated");
	if (restart) {
		cJSON_AddItemToObjectCS(event, "body", body = cJSON_CreateObject());
		cJSON_AddBoolToObject(body, "restart", true);
	}
	send_json(event);
}

static void emit_stopped_event(void) {
	const char *reason;
	switch (dbg_state) {
	case DBG_STOPPED_ENTRY: reason = "entry"; break;
	case DBG_STOPPED_STEP: reason = "step"; break;
	case DBG_STOPPED_NEXT: reason = "step"; break;
	case DBG_STOPPED_BREAKPOINT: reason = "breakpoint"; break;
	case DBG_STOPPED_INTERRUPT: reason = "pause"; break;
	case DBG_STOPPED_EXCEPTION: reason = "exception"; break;
	default: reason = "unknown"; break;
	}

	cJSON *event = cJSON_CreateObject(), *body;
	cJSON_AddStringToObject(event, "type", "event");
	cJSON_AddStringToObject(event, "event", "stopped");
	cJSON_AddItemToObjectCS(event, "body", body = cJSON_CreateObject());
	cJSON_AddStringToObject(body, "reason", reason);
	cJSON_AddNumberToObject(body, "threadId", THREAD_ID);  // needed?
	send_json(event);
}

static void emit_output_event(int lv, const char *output) {
	cJSON *event = cJSON_CreateObject(), *body;
	cJSON_AddStringToObject(event, "type", "event");
	cJSON_AddStringToObject(event, "event", "output");
	cJSON_AddItemToObjectCS(event, "body", body = cJSON_CreateObject());
	cJSON_AddStringToObject(body, "output", output);

	const char *src = dsym_page2src(symbols, nact->current_page);
	if (src)
		cJSON_AddItemToObjectCS(body, "source", create_source(src));
	int line = dsym_addr2line(symbols, nact->current_page, nact->current_addr);
	if (line >= 0)
		cJSON_AddNumberToObject(body, "line", line);

	send_json(event);

	if (break_on_warnings && lv <= 1)
		dbg_state = DBG_STOPPED_EXCEPTION;
}

static void cmd_initialize(cJSON *args, cJSON *resp) {
	cJSON *body, *filters, *filter;
	cJSON_AddBoolToObject(resp, "success", true);
	cJSON_AddItemToObjectCS(resp, "body", body = cJSON_CreateObject());
	cJSON_AddBoolToObject(body, "supportsConditionalBreakpoints", true);
	cJSON_AddBoolToObject(body, "supportsConfigurationDoneRequest", true);
	cJSON_AddBoolToObject(body, "supportsEvaluateForHovers", true);
	cJSON_AddBoolToObject(body, "supportsSetVariable", true);
	cJSON_AddItemToObjectCS(body, "exceptionBreakpointFilters", filters = cJSON_CreateArray());
	cJSON_AddItemToArray(filters, filter = cJSON_CreateObject());
	cJSON_AddStringToObject(filter, "filter", ebf_warnings);
	cJSON_AddStringToObject(filter, "label", "Stop on warnings");
}

static void cmd_launch(cJSON *args, cJSON *resp) {
	cJSON *noDebug = cJSON_GetObjectItemCaseSensitive(args, "noDebug");
	if (cJSON_IsTrue(noDebug)) {
		cJSON_AddBoolToObject(resp, "success", true);
		initialized = true;
		return;
	}

	symbols = dsym_load(symbols_path);
	if (!symbols) {
		cJSON_AddBoolToObject(resp, "success", false);
		cJSON_AddStringToObject(resp, "message", "xsystem35: Cannot load debug symbols");
		return;
	}

	cJSON *srcDir = cJSON_GetObjectItemCaseSensitive(args, "srcDir");
	if (cJSON_IsString(srcDir))
		src_dir = strdup(srcDir->valuestring);
	if (cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(args, "stopOnEntry")))
		dbg_state = DBG_STOPPED_ENTRY;
	cJSON_AddBoolToObject(resp, "success", true);

	emit_initialized_event();
}

static void cmd_configurationDone(cJSON *args, cJSON *resp) {
	initialized = true;
	cJSON_AddBoolToObject(resp, "success", true);
}

static void cmd_stackTrace(cJSON *args, cJSON *resp) {
	StackTrace *trace = dbg_stack_trace();

	cJSON *body, *stackFrames;
	cJSON_AddBoolToObject(resp, "success", true);
	cJSON_AddItemToObjectCS(resp, "body", body = cJSON_CreateObject());
	cJSON_AddItemToObjectCS(body, "stackFrames", stackFrames = cJSON_CreateArray());
	for (int i = 0; i < trace->nr_frame; i++) {
		StackFrame *frame = &trace->frames[i];
		cJSON *item = cJSON_CreateObject();
		cJSON_AddItemToArray(stackFrames, item);
		cJSON_AddNumberToObject(item, "id", i);
		cJSON_AddStringToObject(item, "name", frame->name);
		cJSON_AddItemToObjectCS(item, "source", create_source(frame->src));
		cJSON_AddNumberToObject(item, "line", frame->line);
		cJSON_AddNumberToObject(item, "column", 0);
	}
	cJSON_AddNumberToObject(body, "totalFrames", trace->nr_frame);

	free(trace);
}

static void cmd_setBreakpoints(cJSON *args, cJSON *resp) {
	cJSON *source = cJSON_GetObjectItemCaseSensitive(args, "source");
	cJSON *source_name = cJSON_GetObjectItemCaseSensitive(source, "name");
	cJSON *in_bps = cJSON_GetObjectItemCaseSensitive(args, "breakpoints");
	if (!cJSON_IsString(source_name) || !cJSON_IsArray(in_bps)) {
		cJSON_AddBoolToObject(resp, "success", false);
		cJSON_AddStringToObject(resp, "message", "invalid arguments");
		return;
	}
	const char *filename = source_name->valuestring;
	int page = dsym_src2page(symbols, filename);

	dbg_delete_breakpoints_in_page(page);

	cJSON *body, *out_bps;
	cJSON_AddBoolToObject(resp, "success", true);
	cJSON_AddItemToObjectCS(resp, "body", body = cJSON_CreateObject());
	cJSON_AddItemToObjectCS(body, "breakpoints", out_bps = cJSON_CreateArray());

	char message[256];
	cJSON *srcbp;
	cJSON_ArrayForEach(srcbp, in_bps) {
		cJSON *item = cJSON_CreateObject();
		cJSON_AddItemToArray(out_bps, item);

		cJSON *line = cJSON_GetObjectItemCaseSensitive(srcbp, "line");
		int line_no = line->valueint;
		int addr = dsym_line2addr(symbols, page, line_no);
		if (page < 0) {
			snprintf(message, sizeof(message), "no source file named %s", filename);
			cJSON_AddBoolToObject(item, "verified", false);
			cJSON_AddStringToObject(item, "message", message);
			continue;
		}
		if (addr < 0) {
			snprintf(message, sizeof(message), "no line %d in file %s", line_no, filename);
			cJSON_AddBoolToObject(item, "verified", false);
			cJSON_AddStringToObject(item, "message", message);
			continue;
		}
		Breakpoint *bp = dbg_set_breakpoint(page, addr, false);
		if (!bp) {
			snprintf(message, sizeof(message), "failed to set breakpoint at %d:0x%x", page, addr);
			cJSON_AddBoolToObject(item, "verified", false);
			cJSON_AddStringToObject(item, "message", message);
			continue;
		}

		cJSON *condition = cJSON_GetObjectItemCaseSensitive(srcbp, "condition");
		if (condition && cJSON_IsString(condition)) {
			if (!dbg_set_breakpoint_condition(bp, condition->valuestring, message, sizeof(message))) {
				dbg_delete_breakpoint(bp->no);
				cJSON_AddBoolToObject(item, "verified", false);
				cJSON_AddStringToObject(item, "message", message);
				continue;
			}
		}

		line_no = dsym_addr2line(symbols, page, addr);
		cJSON_AddNumberToObject(item, "id", bp->no);
		cJSON_AddBoolToObject(item, "verified", true);
		cJSON_AddItemToObjectCS(item, "source", create_source(filename));
		cJSON_AddNumberToObject(item, "line", line_no);
	}
}

static void cmd_setExceptionBreakpoints(cJSON *args, cJSON *resp) {
	cJSON *filters = cJSON_GetObjectItemCaseSensitive(args, "filters");
	break_on_warnings = false;
	cJSON *filter;
	cJSON_ArrayForEach(filter, filters) {
		if (cJSON_IsString(filter) && !strcmp(filter->valuestring, ebf_warnings))
			break_on_warnings = true;
	}
	cJSON_AddBoolToObject(resp, "success", true);
}

static void cmd_evaluate(cJSON *args, cJSON *resp) {
	cJSON *expression = cJSON_GetObjectItemCaseSensitive(args, "expression");
	if (!cJSON_IsString(expression)) {
		cJSON_AddBoolToObject(resp, "success", false);
		cJSON_AddStringToObject(resp, "message", "invalid arguments");
		return;
	}
	char buf[256];
	if (!dbg_evaluate(expression->valuestring, buf, sizeof(buf))) {
		cJSON_AddBoolToObject(resp, "success", false);
		cJSON_AddStringToObject(resp, "message", buf);
		return;
	}

	cJSON *body;
	cJSON_AddBoolToObject(resp, "success", true);
	cJSON_AddItemToObjectCS(resp, "body", body = cJSON_CreateObject());
	cJSON_AddStringToObject(body, "result", buf);
	cJSON_AddNumberToObject(body, "variablesReference", 0);
}

static void cmd_continue(cJSON *args, cJSON *resp) {
	sdl_raiseWindow();
	cJSON_AddBoolToObject(resp, "success", true);
}

static void cmd_pause(cJSON *args, cJSON *resp) {
	dbg_state = DBG_STOPPED_INTERRUPT;
	cJSON_AddBoolToObject(resp, "success", true);
}

static void cmd_stepIn(cJSON *args, cJSON *resp) {
	dbg_stepin();
	cJSON_AddBoolToObject(resp, "success", true);
}

static void cmd_stepOut(cJSON *args, cJSON *resp) {
	dbg_stepout();
	cJSON_AddBoolToObject(resp, "success", true);
}

static void cmd_next(cJSON *args, cJSON *resp) {
	dbg_next();
	cJSON_AddBoolToObject(resp, "success", true);
}

static void cmd_threads(cJSON *args, cJSON *resp) {
	cJSON *body, *threads, *thread;
	cJSON_AddBoolToObject(resp, "success", true);
	cJSON_AddItemToObjectCS(resp, "body", body = cJSON_CreateObject());
	cJSON_AddItemToObjectCS(body, "threads", threads = cJSON_CreateArray());
	cJSON_AddItemToArray(threads, thread = cJSON_CreateObject());
	cJSON_AddNumberToObject(thread, "id", THREAD_ID);
	cJSON_AddStringToObject(thread, "name", "main thread");
}

static void cmd_scopes(cJSON *args, cJSON *resp) {
	cJSON *body, *scopes, *globals, *strings;
	cJSON_AddBoolToObject(resp, "success", true);
	cJSON_AddItemToObjectCS(resp, "body", body = cJSON_CreateObject());
	cJSON_AddItemToObjectCS(body, "scopes", scopes = cJSON_CreateArray());
	cJSON_AddItemToArray(scopes, globals = cJSON_CreateObject());
	cJSON_AddStringToObject(globals, "name", "All Variables");
	cJSON_AddNumberToObject(globals, "variablesReference", VREF_GLOBALS);
	cJSON_AddNumberToObject(globals, "namedVariables", dsym_num_variables(symbols));
	cJSON_AddBoolToObject(globals, "expensive", false);
	cJSON_AddItemToArray(scopes, strings = cJSON_CreateObject());
	cJSON_AddStringToObject(strings, "name", "Strings");
	cJSON_AddNumberToObject(strings, "variablesReference", VREF_STRINGS);
	cJSON_AddNumberToObject(strings, "indexedVariables", svar_maxindex() + 1);
	cJSON_AddBoolToObject(strings, "expensive", false);
}

char *format_string_value(const char *str) {
	char *utf = toUTF8(str);
	char *new_value = malloc(strlen(utf) + 3);
	sprintf(new_value, "\"%s\"", utf);
	free(utf);
	return new_value;
}

static void cmd_variables(cJSON *args, cJSON *resp) {
	cJSON *start_ = cJSON_GetObjectItemCaseSensitive(args, "start");
	cJSON *count_ = cJSON_GetObjectItemCaseSensitive(args, "count");
	int start = cJSON_IsNumber(start_) ? start_->valueint : 0;
	int count = cJSON_IsNumber(count_) ? count_->valueint : 65536;

	cJSON *vref = cJSON_GetObjectItemCaseSensitive(args, "variablesReference");
	if (cJSON_IsNumber(vref) && vref->valueint == VREF_GLOBALS) {
		cJSON *body, *variables;
		cJSON_AddBoolToObject(resp, "success", true);
		cJSON_AddItemToObjectCS(resp, "body", body = cJSON_CreateObject());
		cJSON_AddItemToObjectCS(body, "variables", variables = cJSON_CreateArray());
		int end = dsym_num_variables(symbols);
		if (end > start + count)
			end = start + count;
		for (int i = start; i < end; i++) {
			cJSON *var = cJSON_CreateObject();
			cJSON_AddItemToArray(variables, var);
			cJSON_AddStringToObject(var, "name", dsym_variable_name(symbols, i));
			char value[20];
			sprintf(value, "%d", *v_ref(i));
			cJSON_AddStringToObject(var, "value", value);
			cJSON_AddNumberToObject(var, "variablesReference", 0);
		}
	} else if (cJSON_IsNumber(vref) && vref->valueint == VREF_STRINGS) {
		cJSON *body, *variables;
		cJSON_AddBoolToObject(resp, "success", true);
		cJSON_AddItemToObjectCS(resp, "body", body = cJSON_CreateObject());
		cJSON_AddItemToObjectCS(body, "variables", variables = cJSON_CreateArray());
		int end = svar_maxindex() + 1;
		if (end > start + count)
			end = start + count;
		for (int i = start; i < end; i++) {
			char name[20];
			cJSON *var = cJSON_CreateObject();
			cJSON_AddItemToArray(variables, var);
			sprintf(name, "[%d]", i);
			cJSON_AddStringToObject(var, "name", name);
			char *value = format_string_value(svar_get(i));
			cJSON_AddStringToObject(var, "value", value);
			free(value);
			cJSON_AddNumberToObject(var, "variablesReference", 0);
		}
	} else {
		cJSON_AddBoolToObject(resp, "success", false);
		cJSON_AddStringToObject(resp, "message", "invalid variablesReference");
	}
}

static void cmd_setVariable(cJSON *args, cJSON *resp) {
	cJSON *name = cJSON_GetObjectItemCaseSensitive(args, "name");
	cJSON *value = cJSON_GetObjectItemCaseSensitive(args, "value");
	if (!cJSON_IsString(name) || !cJSON_IsString(value)) {
		cJSON_AddBoolToObject(resp, "success", false);
		cJSON_AddStringToObject(resp, "message", "invalid arguments");
		return;
	}

	cJSON *vref = cJSON_GetObjectItemCaseSensitive(args, "variablesReference");
	if (cJSON_IsNumber(vref) && vref->valueint == VREF_GLOBALS) {
		int var = dbg_lookup_var(name->valuestring);
		if (var < 0) {
			cJSON_AddBoolToObject(resp, "success", false);
			cJSON_AddStringToObject(resp, "message", "invalid variable name");
			return;
		}
		int parsed_value;
		if (sscanf(value->valuestring, "%i", &parsed_value) != 1) {
			cJSON_AddBoolToObject(resp, "success", false);
			cJSON_AddStringToObject(resp, "message", "syntax error");
			return;
		}
		*v_ref(var) = parsed_value & 0xffff;

		cJSON *body;
		cJSON_AddBoolToObject(resp, "success", true);
		cJSON_AddItemToObjectCS(resp, "body", body = cJSON_CreateObject());
		char new_value[20];
		sprintf(new_value, "%d", *v_ref(var));
		cJSON_AddStringToObject(body, "value", new_value);
	} else if (cJSON_IsNumber(vref) && vref->valueint == VREF_STRINGS) {
		int idx;
		if (sscanf(name->valuestring, "[%d]", &idx) != 1 || (unsigned)idx > svar_maxindex()) {
			cJSON_AddBoolToObject(resp, "success", false);
			cJSON_AddStringToObject(resp, "message", "invalid string index");
			return;
		}
		int len = strlen(value->valuestring);
		if (len < 2 || value->valuestring[0] != '"' || value->valuestring[len-1] != '"') {
			cJSON_AddBoolToObject(resp, "success", false);
			cJSON_AddStringToObject(resp, "message", "syntax error");
			return;
		}
		value->valuestring[len-1] = '\0';
		char *str = fromUTF8(value->valuestring + 1);
		svar_set(idx, str);
		free(str);

		cJSON *body;
		cJSON_AddBoolToObject(resp, "success", true);
		cJSON_AddItemToObjectCS(resp, "body", body = cJSON_CreateObject());
		char *new_value = format_string_value(svar_get(idx));
		cJSON_AddStringToObject(body, "value", new_value);
		free(new_value);
	} else {
		cJSON_AddBoolToObject(resp, "success", false);
		cJSON_AddStringToObject(resp, "message", "invalid variablesReference");
	}
}

static void cmd_disconnect(cJSON *args, cJSON *resp) {
	cJSON_AddBoolToObject(resp, "success", true);
	send_json(resp);
	sys_exit(0);
}

static boolean handle_request(cJSON *request) {
	boolean continue_repl = true;

	cJSON *resp = cJSON_CreateObject();
	cJSON_AddStringToObject(resp, "type", "response");
	cJSON *request_seq = cJSON_DetachItemFromObjectCaseSensitive(request, "seq");
	cJSON_AddItemToObjectCS(resp, "request_seq", request_seq);
	cJSON *command = cJSON_DetachItemFromObjectCaseSensitive(request, "command");
	cJSON_AddItemToObjectCS(resp, "command", command);
	cJSON *args = cJSON_GetObjectItemCaseSensitive(request, "arguments");

	if (!cJSON_IsString(command)) {
		fprintf(stderr, "protocol error: command is not a string\n");
		// FIXME: return an error response
		cJSON_free(resp);
		return continue_repl;
	}

	if (!strcmp(command->valuestring, "initialize")) {
		cmd_initialize(args, resp);
	} else if (!strcmp(command->valuestring, "launch")) {
		cmd_launch(args, resp);
	} else if (!strcmp(command->valuestring, "configurationDone")) {
		cmd_configurationDone(args, resp);
	} else if (!strcmp(command->valuestring, "continue")) {
		cmd_continue(args, resp);
		continue_repl = false;
	} else if (!strcmp(command->valuestring, "stackTrace")) {
		cmd_stackTrace(args, resp);
	} else if (!strcmp(command->valuestring, "stepIn")) {
		cmd_stepIn(args, resp);
		continue_repl = false;
	} else if (!strcmp(command->valuestring, "stepOut")) {
		cmd_stepOut(args, resp);
		continue_repl = false;
	} else if (!strcmp(command->valuestring, "next")) {
		cmd_next(args, resp);
		continue_repl = false;
	} else if (!strcmp(command->valuestring, "pause")) {
		cmd_pause(args, resp);
	} else if (!strcmp(command->valuestring, "evaluate")) {
		cmd_evaluate(args, resp);
	} else if (!strcmp(command->valuestring, "setBreakpoints")) {
		cmd_setBreakpoints(args, resp);
	} else if (!strcmp(command->valuestring, "setExceptionBreakpoints")) {
		cmd_setExceptionBreakpoints(args, resp);
	} else if (!strcmp(command->valuestring, "threads")) {
		cmd_threads(args, resp);
	} else if (!strcmp(command->valuestring, "scopes")) {
		cmd_scopes(args, resp);
	} else if (!strcmp(command->valuestring, "variables")) {
		cmd_variables(args, resp);
	} else if (!strcmp(command->valuestring, "setVariable")) {
		cmd_setVariable(args, resp);
	} else if (!strcmp(command->valuestring, "disconnect")) {
		cmd_disconnect(args, resp);
	} else {
		fprintf(stderr, "unknown command \"%s\"\n", command->valuestring);
	}
	send_json(resp);
	return continue_repl;
}

static boolean handle_message(char *msg) {
	cJSON *json = cJSON_Parse(msg);
	cJSON *type = cJSON_GetObjectItemCaseSensitive(json, "type");
	boolean continue_repl = true;
	if (cJSON_IsString(type) && !strcmp(type->valuestring, "request"))
		continue_repl = handle_request(json);
	cJSON_free(json);
	free(msg);
	return continue_repl;
}

static int read_command_thread(void *data) {
	int content_length = -1;
	char header[512];
	while (fgets(header, sizeof(header), stdin)) {
		if (sscanf(header, "Content-Length: %d", &content_length) == 1) {
			continue;
		} else if ((header[0] == '\r' && header[1] == '\n') || header[0] == '\n') {
			if (content_length < 0) {
				fprintf(stderr, "Debug Adapter Protocol error: no Content-Length header\n");
				continue;
			}
			char *buf = malloc(content_length);
			fread(buf, content_length, 1, stdin);
			msgq_enqueue(queue, buf);
			content_length = -1;
		} else {
			fprintf(stderr, "Unknown Debug Adapter Protocol header: %s", header);
		}
	}
	msgq_enqueue(queue, NULL);  // EOF
	return 0;
}

static void dbg_dap_init(const char *path) {
	symbols_path = strdup(path);
	queue = msgq_new();

#ifdef _WIN32
	_setmode(_fileno(stdin), _O_BINARY);
	_setmode(_fileno(stdout), _O_BINARY);
#endif

	SDL_CreateThread(read_command_thread, "Debugger", NULL);

	while (!initialized) {
		char *msg = msgq_dequeue(queue);
		if (!msg)
			break;
		handle_message(msg);
	}
}

static void dbg_dap_quit(bool restart) {
	emit_terminated_event(restart);
	if (restart)
		exit(0); // The front end will restart xsystem35, so we can just exit.
}

static void dbg_dap_repl(int bp_no) {
	emit_stopped_event();
	dbg_state = DBG_RUNNING;

	boolean continue_repl = true;
	while (continue_repl) {
		char *msg = msgq_dequeue(queue);
		if (!msg)
			break;
		continue_repl = handle_message(msg);
	}
}

static void dbg_dap_onsleep(void) {
	while (!msgq_isempty(queue)) {
		char *msg = msgq_dequeue(queue);
		if (!msg)
			break;
		handle_message(msg);
	}
	if (dbg_state == DBG_STOPPED_INTERRUPT || dbg_state == DBG_STOPPED_EXCEPTION)
		dbg_main(0);
}

DebuggerImpl dbg_dap_impl = {
	.init = dbg_dap_init,
	.quit = dbg_dap_quit,
	.repl = dbg_dap_repl,
	.onsleep = dbg_dap_onsleep,
	.console_output = emit_output_event,
};
