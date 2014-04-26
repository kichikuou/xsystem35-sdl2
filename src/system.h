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

#include "config.h"

#ifndef HAVE_FUNC
#ifndef  __GNUC__
#define __func__ ""
#else 
#define __func__ __FUNCTION__
#endif
#endif

/* should define in somewhere */ 
extern void sys_error(char *format, ...);    /* show nessafe and exit system */
extern void sys_exit(int code);              /* exit system with code */
extern void sys_message(char *format, ...);  /* show various message */
extern void sys_reset();
extern int  sys_nextdebuglv;                 /* message level */

/*
 DEBUGLEVEL
  0: critical error message only (output to terminal or message box)
                                            [stable release default]
  1: warning only(output to terminal)
  2+: more message [devel relase default]
*/

#ifdef DEBUG
#define DEBUGLEVEL 2
#else
#define DEBUGLEVEL 0
#endif /* DEBUG */

#define NOMEMERR()        sys_error("Out of memory at %s()\n", __func__)
#define NOTICE            sys_nextdebuglv = 2, sys_message

#define WARNING           sys_nextdebuglv = 1, \
sys_message("*WARNING*(%s): ", __func__), sys_message

#define SYSERROR          fprintf(stderr, "*ERROR*(%s): ", __func__), sys_error

#endif /* !__SYSTEM__ */
