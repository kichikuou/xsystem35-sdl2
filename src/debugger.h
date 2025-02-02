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

#ifndef __DEBUGGER_H__
#define __DEBUGGER_H__

#include <stdarg.h>
#include "portab.h"

typedef enum {
	DBG_RUNNING,
	DBG_STOPPED_ENTRY,
	DBG_STOPPED_STEP,
	DBG_STOPPED_NEXT,
	DBG_STOPPED_BREAKPOINT,
	DBG_STOPPED_INTERRUPT,
	DBG_STOPPED_EXCEPTION,
} DebuggerState;

extern DebuggerState dbg_state;

#define BREAKPOINT 0x0f


#ifdef ENABLE_DEBUGGER

#define dbg_trapped() (dbg_state != DBG_RUNNING)
void dbg_init(const char *symbols_path, bool use_dap);
void dbg_quit(void);
void dbg_main(int bp_no);
void dbg_onsleep(void);
void dbg_on_palette_change(void);
uint8_t dbg_handle_breakpoint(int page, int addr);
bool dbg_console_vprintf(int lv, const char *format, va_list ap);
void dbg_post_command(void *data);

#else // ENABLE_DEBUGGER

#define dbg_trapped() false
#define dbg_init(symbols_path, use_dap)
#define dbg_quit()
#define dbg_main(bp_no)
#define dbg_onsleep()
#define dbg_on_palette_change()
#define dbg_handle_breakpoint(page, addr) BREAKPOINT
#define dbg_console_vprintf(lv, format, ap) false
#define dbg_post_command(data)

#endif // ENABLE_DEBUGGER

#endif // __DEBUGGER_H__
