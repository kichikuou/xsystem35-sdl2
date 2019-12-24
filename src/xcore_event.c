/*
 * xcore_event.c  X11 event handler
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
/* $Id: xcore_event.c,v 1.12 2004/10/31 04:18:06 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>

#include "portab.h"
#include "system.h"
#include "xcore.h"
#include "xcore_private.h"
#include "nact.h"
#include "counter.h"
#include "input.h"
#include "menu.h"
#include "joystick.h"
#include "xcore_input.c"

static boolean xcore_getEvent(void);
static int check_button(void);

/* pointer の状態 */
static int mousex, mousey;

static boolean xcore_getEvent(void) {
	XEvent e;
	Window root, child;
	int rx, ry, wx, wy;
	int mask;
	boolean getkey = FALSE, msg_skip = FALSE;
	
	while(XPending(x11_display)) {
		XNextEvent(x11_display, &e);
		switch(e.type) {
		case LeaveNotify:
			mousex = -1; mousey = -1; // mouseb = 0;
			memset(RawKeyInfo, 0, sizeof(RawKeyInfo));
			getkey = TRUE;
			if (x11_fs_on) {
				XUngrabPointer(x11_display, CurrentTime);
				XSync(x11_display, False);
			}
			break;
		case EnterNotify:
			if (x11_fs_on) {
				XGrabPointer(x11_display, x11_window, True, 0,
					     GrabModeAsync, GrabModeAsync,
					     x11_window, None, CurrentTime);
				XSync(x11_display, False);
			}
			break;
		case MotionNotify:
			if (!XQueryPointer(x11_display, x11_window, &root, &child, &rx, &ry, &wx, &wy, &mask))
				break;
			mousex = wx; mousey = wy;
			if (nact->ags.eventcb) {
				agsevent_t agse;
				agse.type = AGSEVENT_MOUSE_MOTION;
				agse.d1 = wx; agse.d2 = wy; agse.d3 = 0;
				nact->ags.eventcb(&agse);
			}
			break;
			
#if 0
		case ButtonPress:
			mouseb |= (1 << e.xbutton.button);
			getkey = TRUE;
			break;
			
		case ButtonRelease:
			mouseb &= (0xffffffff ^ (1 << e.xbutton.button));
			getkey = TRUE;
			break;
#endif
		case ButtonPress:
		case ButtonRelease:
			buttonEventProcess((XButtonEvent *)&e);
			getkey = TRUE;
			break;
			
		case Expose: {
			MyRectangle src;
			MyPoint dst;
			XExposeEvent *ev = (XExposeEvent *)&e;
			if (ev->count == 0) {
				src.x = view_x + ev->x - winoffset_x;
				src.y = view_y + ev->y - winoffset_y;
				src.width = min(DIB->width, ev->width);
				src.height = min(DIB->height, ev->height);
				dst.x = ev->x - winoffset_x;
				dst.y = ev->y - winoffset_y;
				if (src.x > view_w || src.x < 0) break;
				if (src.y > view_h || src.y < 0) break;
				Xcore_updateArea(&src, &dst);
			}
			break;
		}
			
		case KeyPress:
			keyEventProsess((XKeyEvent *)&e);
			getkey = TRUE;
			break;
			
		case KeyRelease: {
			KeySym sym;
			
			keyEventProsess((XKeyEvent *)&e);
			sym = XkbKeycodeToKeysym(x11_display, e.xkey.keycode, 0, 0);
			if (sym == XK_F4) {
				Xcore_fullScreen(!x11_fs_on);
			} else if (sym == XK_F1) {
				msg_skip = TRUE;
			}
			getkey = TRUE;
			break;
		}
		
		case ClientMessage:
			if (e.xclient.message_type == atom_wmprot &&
			    e.xclient.data.l[0]    == atom_delwin) {
				menu_quitmenu_open();
			}
			break;
		}
	}
		
	if (msg_skip) set_skipMode(!get_skipMode());
	return getkey;
}

int Xcore_keywait(int msec, boolean cancel) {
	boolean bool = FALSE;
	int key = 0, retval, rest = msec;
	int cnt = get_high_counter(SYSTEMCOUNTER_MSEC);

	if (msec < 0) return 0;
#if 1
	while (msec > (get_high_counter(SYSTEMCOUNTER_MSEC) - cnt)) {
		bool = xcore_getEvent();
		//key = check_button() | Xcore_getKeyInfo() | joy_getinfo();
		key |= check_button() | Xcore_getKeyInfo() | joy_getinfo();
		if( cancel && key ) break;
		rest = msec - (get_high_counter(SYSTEMCOUNTER_MSEC) - cnt);
		if (rest < 0) break;
		if (rest < 10) {
			usleep(1000 * rest);
		} else {
			usleep(10000);
		}
		nact->callback();
	}
#endif
#if 0
	while (rest > 0) {
		struct timeval tv;
		fd_set fdset;
		int x11_fd = ConnectionNumber(x11_display);
		
		FD_ZERO(&fdset);
		FD_SET(x11_fd, &fdset);
		
		cnt  = get_high_counter(SYSTEMCOUNTER_MSEC);
		tv.tv_sec = 0;
		if (rest < 100) {
			tv.tv_usec = rest * 1000;
		} else {
			tv.tv_usec = 100 * 1000;
		}
		retval = select(x11_fd + 1, &fdset, NULL, NULL, &tv);
		if (retval > 0) {
			if (FD_ISSET(x11_fd, &fdset)) {
				bool = xcore_getEvent();
			}
		} else if (retval < 0) {
			perror("select");
		}
		if (bool) {
			key |= (check_button() | Xcore_getKeyInfo());
			if (cancel) break;
		}
		rest -= (get_high_counter(SYSTEMCOUNTER_MSEC) - cnt);
		nact->callback();
	}
#endif
	return key;
}

/* キー情報の取得 */
int Xcore_getKeyInfo() {
	xcore_getEvent();
	
	return ((RawKeyInfo[KEY_UP]     || RawKeyInfo[KEY_PAD_8])       |
		((RawKeyInfo[KEY_DOWN]  || RawKeyInfo[KEY_PAD_2]) << 1) |
		((RawKeyInfo[KEY_LEFT]  || RawKeyInfo[KEY_PAD_4]) << 2) |
		((RawKeyInfo[KEY_RIGHT] || RawKeyInfo[KEY_PAD_6]) << 3) |
		(RawKeyInfo[KEY_ENTER] << 4) |
		(RawKeyInfo[KEY_SPACE] << 5) |
		(RawKeyInfo[KEY_ESC]   << 6) |
		(RawKeyInfo[KEY_TAB]   << 7));
}

/* mouse 情報の取得 */
int Xcore_getMouseInfo(MyPoint *p) {
	xcore_getEvent();
	
	if (mousex < 0) {
		if (p) {
			p->x=0; p->y=0;
		}
		return 0;
	}
	
	if (p) {
		p->x = mousex - winoffset_x;
		p->y = mousey - winoffset_y;
	}
	return check_button();
}

/* mouse のボタン状態の取得 */
static int check_button(void) {
	int m1, m2;
	
	if (RawKeyInfo[KEY_MOUSE_MIDDLE]) {
		menu_open();
	} else {
		m1 = RawKeyInfo[KEY_MOUSE_LEFT]  ? SYS35KEY_RET : 0;
		m2 = RawKeyInfo[KEY_MOUSE_RIGHT] ? SYS35KEY_SPC : 0;
		return m1|m2;
	}
	return 0;
}
