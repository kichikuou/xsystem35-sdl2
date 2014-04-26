/*
 * windowframe.h  message / selection window frame definition
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
/* $Id: windowframe.h,v 1.5 2006/04/21 16:40:48 chikama Exp $ */

#ifndef __WINDOWFRAME__
#define __WINDOWFRAME__

#include "portab.h"

/* 枠の種類 */
#define WINDOW_FRAME_EMPTY 0
#define WINDOW_FRAME_LINE  1
#define WINDOW_FRAME_CG    2

/* 選択・メッセージウィンドの最大 */
#define SELWINMAX 128
#define MSGWINMAX 128

/* メッセージ・選択ウィンドの情報 */
typedef struct {
	int x;
	int y;
	int width;
	int height;
	boolean save;
	void *savedimg;
} Bcom_WindowInfo;

// extern Bcom_WindowInfo *sel_winInfo;

#endif /* !__WINDOWFRAME__ */
