/*
 * input.c キーボードマウス関連
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
/* $Id: input.c,v 1.22 2001/03/30 19:16:38 chikama Exp $ */

#include <unistd.h>
#include <limits.h>
#include "config.h"
#include "input.h"
#include "sdl_core.h"
#include "counter.h"
#include "xsystem35.h"
#include "message.h"
#include "texthook.h"
#include "msgskip.h"

static int hak_ignore_mask      = 0xffffffff;
static int hak_releasewait_mask = (0 << 0) | (0 << 1) | (0 << 2) | (0 << 3) |
                                  (1 << 4) | (0 << 5) | (0 << 6) | (0 << 7) ;

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
	int key = sdl_getMouseInfo(&_p);
	
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
	return sdl_getKeyInfo();
}

int sys_getJoyInfo() {
	return sdl_getJoyInfo();
}

int sys_getInputInfo() {
	return sdl_getMouseInfo(NULL) | sdl_getKeyInfo() | sdl_getJoyInfo();
}

int sys_keywait(int msec, unsigned flags) {
	texthook_keywait();

	if ((flags & KEYWAIT_SKIPPABLE) && msgskip_isSkipping()) {
		if (msgskip_getFlags() & MSGSKIP_STOP_ON_CLICK) {
			int key = sys_getInputInfo();
			if (key)
				msgskip_activate(FALSE);
			return key;
		}
		return 0;
	}

	int key=0, n;
	int end = msec == INT_MAX ? INT_MAX : get_high_counter(SYSTEMCOUNTER_MSEC) + msec;
	while (!((flags & KEYWAIT_SKIPPABLE) && msgskip_isSkipping()) &&
		   (n = end - get_high_counter(SYSTEMCOUNTER_MSEC)) > 0) {
		if (n <= 16)
			sdl_sleep(n);
		else
			sdl_wait_vsync();
		nact->callback();
		key = sys_getInputInfo();
		nact->wait_vsync = FALSE;  // We just waited!
		if ((flags & KEYWAIT_CANCELABLE) && key) break;
	}

	return key;
}

void sys_hit_any_key() {
	int key=0;
	if (msgskip_isSkipping()) {
		if (msgskip_getFlags() & MSGSKIP_STOP_ON_CLICK && sys_getInputInfo())
			msgskip_activate(FALSE);
		sdl_sleep(30);
		return;
	}

	msg_hitAnyKey();
	
	/* message wait flag restore */
	if (nact->messagewait_cancelled) {
		nact->messagewait_cancelled = FALSE;
		/* consume the input that cancelled message wait */
		while(0 == (key & hak_ignore_mask)) {
			key = sys_keywait(INT_MAX, KEYWAIT_CANCELABLE);
		}
		while(key & hak_releasewait_mask) {
			key = sys_keywait(100, KEYWAIT_CANCELABLE);
		}
	}

	while (!msgskip_isSkipping() && 0 == (key & hak_ignore_mask)) {
		key = sys_keywait(100, KEYWAIT_CANCELABLE | KEYWAIT_SKIPPABLE);
	}
	
	while (!msgskip_isSkipping() && (key & hak_releasewait_mask)) {
		key = sys_keywait(100, KEYWAIT_CANCELABLE | KEYWAIT_SKIPPABLE);
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
		key = sys_keywait(50, KEYWAIT_NONCANCELABLE);
	}
}
