/*
 * cmdy.c  SYSTEM35 Y command
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
/* $Id: cmdy.c,v 1.26 2001/04/29 23:30:25 chikama Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "portab.h"
#include "xsystem35.h"
#include "randMT.h"
#include "message.h"
#include "input.h"
#include "scenario.h"
#include "cmd_check.h"
#include "scheduler.h"
#include "music_cdrom.h"
#include "hacks.h"

unsigned Y3waitFlags = KEYWAIT_CANCELABLE;

/*
 * A hack to improve Rance4 map navigation.
 *
 * This fixes an issue where Rance4's map mode isn't responsive to mouse clicks.
 * Here's a simplified code of Rance4's map UI loop:
 *
 *         !COUNT : 0!
 *   *LOOP:
 *         Y 3, 1:             ; wait 16ms
 *         IM MX, MY:          ; get mouse position and button state
 *         !COUNT : COUNT + 1!
 *         { COUNT > 15 :
 *            { RND = 16 :     ; if left button is pressed
 *                ...          ; move to next map location
 *            }
 *            !COUNT : 0!
 *         }
 *         @LOOP:
 *
 * This means, a map move happens only if the button is pressed at the moment
 * COUNT reaches 16. In the original System3.x, this is not a problem because
 * `Y 3, 1:` returns immediately if a button is pressed, so COUNT quickly goes
 * up to 16. However, xsystem35 throttles input commands (to prevent busy loops)
 * so both Y and IM cause a 1-frame (16ms) delay. As a result, the user had to
 * hold down the button for up to 512ms (16ms * 2 * 16) to navigate the map.
 *
 * To fix this, this hack processes `Y 3, 1:` followed by the IM command without
 * delay, when a button is pressed.
 *
 * For Rance4 v2, this does not work because the UI loop also checks keyboard
 * status, so we use a different hack. See rance4v2_hack() in event.c.
 */
static void rance4_Y3_IM_hack() {
	static int count;

	int button_pressed = sys_getInputInfo();
	if (!button_pressed) {
		sys_wait_vsync();
		count = 0;
	}
	commandIM();
	if (button_pressed && ++count & 15)
		cancel_yield();
}

void commandY() {
	int i;
	unsigned int p1 = getCaliValue();
	unsigned int p2 = getCaliValue();
	
	switch (p1) {
	case 1:
		if (p2 == 0) {
			/* メッセージ領域の初期化と、文字の表示位置を左上端にセットする */
			msg_nextPage(true);
		} else if (p2 == 1) {
			/* メッセージ領域の文字の表示位置を左上端にセットする */
			msg_nextPage(false);
		}
		break;
	case 2:
		/*システム変数 D01〜D20 までを初期化する */ 
		for (i = 0; i < 20; i++) {
			sysVar[i + 1] = 0;
		}
		break;
	case 3:
	case 1003:  // Rance4 ver2.05
		{
			int orig_pc = sl_getIndex();
			if (p2 == 1 && sl_getc() == 'I' && sl_getc() == 'M') {
				rance4_Y3_IM_hack();
				return;
			}
			sl_jmpNear(orig_pc);
		}

		switch (p2) {
		case 10000:
			Y3waitFlags = KEYWAIT_NONCANCELABLE;
			break;
		case 10001:
			Y3waitFlags = KEYWAIT_CANCELABLE;
			break;
		case 0:
			sysVar[0] = sys_getInputInfo();
			break;
		default:
			if (p1 == 1003) {
				sys_key_releasewait(SYS35KEY_RET, false);
			} else if (game_id == GAME_RANCE4_V2 && p2 == 1) {
				// Return immediately if any key is pressed.
				sysVar[0] = sys_getInputInfo();
				if (sysVar[0])
					break;
			}
			sysVar[0] = sys_keywait(16 * p2, Y3waitFlags | KEYWAIT_SKIPPABLE);
			break;
		}
		break;
	case 4:
		/* 1 〜 n までの乱数を RND に返す。*/
		if (p2 == 0 || p2 == 1) {
			sysVar[0] = p2;
		} else {
			sysVar[0] = (int)(genrand() * p2) +1;
		}
		break;
	case 1900:
		nact->patch_ec = p2;
		// We're sure this game was built for System3.9 v5.50, which by default
		// routes CD-DA commands to DRIFILE_WAVE.
		muscd_init_bgm(DRIFILE_WAVE, 999);
		break;
	default:
		WARNING("Y undefined command %d", p1);
		break;
	}
	TRACE("Y %d,%d:",p1,p2);
}
