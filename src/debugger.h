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
} DebuggerState;

extern DebuggerState dbg_state;

#define BREAKPOINT 0x0f


#ifdef ENABLE_DEBUGGER

#define dbg_trapped() (dbg_state != DBG_RUNNING)
void dbg_init(const char *symbols_path, boolean use_dap);
void dbg_quit();
void dbg_main(void);
void dbg_onsleep(void);
BYTE dbg_handle_breakpoint(int page, int addr);
boolean dbg_console_vprintf(const char *format, va_list ap);

#else // ENABLE_DEBUGGER

#define dbg_trapped() false
#define dbg_init(symbols_path, use_dap)
#define dbg_quit()
#define dbg_main()
#define dbg_onsleep()
#define dbg_handle_breakpoint(page, addr) BREAKPOINT
#define dbg_console_vprintf(format, ap) false

#endif // ENABLE_DEBUGGER

#endif // __DEBUGGER_H__
