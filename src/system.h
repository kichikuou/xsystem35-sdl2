/*
 * system.h  general error/debug message
 *
 * Copyright (C) 1997-1998 Masaki Chikama (Wren) <chikama@kasumi.ipl.mech.nagoya-u.ac.jp>
 *               1998-                           <masaki-c@is.aist-nara.ac.jp>
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
/* $Id: system.h,v 1.12 2003/07/14 16:22:51 chikama Exp $ */

#ifndef __SYSTEM__
#define __SYSTEM__

#include <stdint.h>
#include "config.h"

void sys_error(char *format, ...);    /* show message and exit system */
void sys_exit(int code);              /* exit system with code */
void sys_message(int lv, char *format, ...);  /* show various message */

uint32_t sys_get_ticks(void);
void sys_sleep(int msec);
void sys_wait_vsync(void);

enum messagebox_type {
	MESSAGEBOX_ERROR,
	MESSAGEBOX_WARNING,
	MESSAGEBOX_INFO,
};
void sys_show_message_box(enum messagebox_type type, const char* title_utf8, const char* message_utf8);

void sys35_remove(void);

/*
 DEBUGLEVEL
  0: critical error message only (output to terminal or message box)
                                            [stable release default]
  1: warning only(output to terminal)
  2+: more message [devel relase default]
*/
void sys_set_debug_level(int level);

#define NOMEMERR()        sys_error("Out of memory at %s()\n", __func__)

#define SYSERROR(fmt, ...) \
	sys_error("[ERROR] %s: " fmt, __func__, ##__VA_ARGS__)
#define WARNING(fmt, ...) \
	sys_message(1, "[WARNING] %s: " fmt "\n", __func__, ##__VA_ARGS__)
#define NOTICE(fmt, ...) \
	sys_message(2, "[INFO] " fmt "\n", ##__VA_ARGS__)
#define SACT_DEBUG(fmt, ...) \
	sys_message(5, "[SACT] %s: " fmt "\n", __func__, ##__VA_ARGS__)

#ifdef HAVE_SIGACTION
void sys_set_signalhandler(int SIG, void (*handler)(int));
#endif

#endif /* !__SYSTEM__ */
