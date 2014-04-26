/*
 * xsystem35.h  SYSTEM35 デコーダ
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
/* $Id: xsystem35.h,v 1.26 2003/04/22 16:34:29 chikama Exp $ */

#ifndef __XSYSTEM35__
#define __XSYSTEM35__

#include "config.h"
#include "portab.h"
#include "scenario.h"
#include "system.h"
#include "nact.h"
#include "variable.h"
#include "ald_manager.h"
#include "gametitle.h"

/* logfile when debug level >= 5 */
#define DEBUGLOGFILE "./xsystem35.log"

/* 
 debug level in xsystem35
  0: critical error message only (output to terminal or message box)
  1: warning only(output to terminal) [stable release default]
  2: add not inplement message (output to terminal) [devel relase default]
  5: add command message (output to terminal or FILE)
  6: add message(output to terminal or FILE)
*/

#define DEBUG_SHOWSCOADR  sys_message("%d,%x: ", sl_getPage(), sl_getIndex())
#ifdef DEBUG
#define DEBUG_COMMAND_YET sys_nextdebuglv = 2, DEBUG_SHOWSCOADR, sys_message
#define DEBUG_COMMAND     sys_nextdebuglv = 5, DEBUG_SHOWSCOADR, sys_message
#define DEBUG_MESSAGE     sys_nextdebuglv = 6, sys_message
#else
#define DEBUG_MESSAGE
#define DEBUG_COMMAND
#define DEBUG_COMMAND_YET
#endif

/* 配列外参照チェック/cali異常値検出 */
#define DEBUG_CHECKALING

/* defined in cali.c */
extern int preVarPage;      /* 直前にアクセスした変数のページ */
extern int preVarIndex;     /* 直前にアクセスした変数のINDEX */
extern int preVarNo;        /* 直前にアクセスした変数の番号 */

#define System_idle(msec) usleep(1000l * (msec));
// extern void System_idle(int msec);

/* system35 画面デフォルト */
#define SYS35_DEFAULT_WIDTH 640
#define SYS35_DEFAULT_HEIGHT 480
#define SYS35_DEFAULT_DEPTH 8

#endif /* !__XSYSTEM35__ */
