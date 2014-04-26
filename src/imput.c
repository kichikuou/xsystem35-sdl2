/*
 * imput.c キーボードマウス関連
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
/* $Id: imput.c,v 1.22 2001/03/30 19:16:38 chikama Exp $ */

#include <unistd.h>
#include <limits.h>
#include "config.h"
#include "imput.h"
#include "xsystem35.h"
#include "message.h"
#include "joystick.h"

static boolean skipToNextSel = FALSE;
static boolean skipModeInterruptable = TRUE;

static int hak_ignore_mask      = 0xffffffff;
static int hak_releasewait_mask = (0 << 0) | (0 << 1) | (0 << 2) | (0 << 3) |
                                  (1 << 4) | (0 << 5) | (0 << 6) | (0 << 7) ;

void set_skipMode(boolean bool) {
	skipToNextSel = bool;
}

boolean get_skipMode() {
	return skipToNextSel;
}

void set_skipMode2(boolean bool) {
	skipModeInterruptable = bool;
}

boolean get_skipMode2() {
	return skipModeInterruptable;
}

void set_hak_keymode(int key, int mode) {
	int flg = (1 << key);
	
	if (mode == 2) {
		hak_ignore_mask |= flg;
	} else {
		hak_ignore_mask ^= flg;
		if (mode == 0) {
			hak_releasewait_mask &= ~flg;
		} else {
			hak_releasewait_mask |= flg;
		}
	}
}

int sys_getMouseInfo(MyPoint *p, boolean is_dibgeo) {
	MyPoint _p;
	int key = GetMouseInfo(&_p);
	
	if (p) {
		p->x = _p.x;
		p->y = _p.y;
		if (is_dibgeo) {
			p->x += nact->sys_view_area.x;
			p->y += nact->sys_view_area.y;
		}
	}
	return key;
}

int sys_getKeyInfo() {
	return GetKeyInfo();
}

int sys_getJoyInfo() {
	return joy_getinfo();
}

int sys_getInputInfo() {
 	int key = GetMouseInfo(NULL) | GetKeyInfo() | joy_getinfo();

	if (key == SYS35KEY_SPC && skipModeInterruptable) {
		skipToNextSel = FALSE;		
	}

	/* 復活 !! */
        if (key == (SYS35KEY_SPC | SYS35KEY_RET | SYS35KEY_ESC)) sys_exit(0);
	
 	return key;
}

void sys_hit_any_key() {
	int key=0;
	if (skipToNextSel) return;

	msg_hitAnyKey();
	
	/* message wait flag restore */
	if (nact->messagewait_enable_save) {
		nact->messagewait_enable = nact->messagewait_enable_save ;
		
		while(0 == (key & hak_ignore_mask)) {
			key = sys_keywait(INT_MAX, TRUE);
		}
		while(key & hak_releasewait_mask) {
			key = sys_keywait(100, TRUE);
		}
	}

	while(0 == (key & hak_ignore_mask)) {
		key = sys_keywait(INT_MAX, TRUE);
	}
	
	while(key & hak_releasewait_mask) {
		key = sys_keywait(100, TRUE);
	}
	
	
}

void sys_key_releasewait(int key, boolean zi_mask_enabled) {
	int mask;
	
	if (zi_mask_enabled) {
		mask = hak_releasewait_mask;
	} else {
		mask = 0xffffffff;
	}
	
	while(key & mask) {
		key = sys_keywait(50, FALSE);
	}
}
