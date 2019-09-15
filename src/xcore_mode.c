/*
 * xcore_mode.c  X11 video mode and full-screen
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
/* $Id: xcore_mode.c,v 1.7 2003/01/31 12:58:28 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <limits.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#ifdef HAVE_XF86VMODE
# include <X11/extensions/xf86vmode.h>
#endif

#include "portab.h"
#include "system.h"
#include "xcore.h"
#include "xcore_private.h"

#ifdef HAVE_XF86VMODE
static int               vm_count;
static XF86VidModeModeInfo **vm_modelines;
#endif
static boolean have_vm;

void x11_vm_init(void) {
#ifdef HAVE_XF86VMODE
	int  foo, bar, i, ma, mi;
	
        if (XF86VidModeQueryExtension(x11_display, &foo, &bar)) {
		XF86VidModeQueryVersion(x11_display, &ma, &mi);

		NOTICE("VidMode  version %d.%d\n", ma, mi);
		have_vm = TRUE;

		XF86VidModeGetAllModeLines(x11_display, XDefaultScreen(x11_display),
					   &vm_count, &vm_modelines);
                NOTICE("  available video mode(s):");
		for (i = 0; i < vm_count; i++) {
			NOTICE(" %dx%d",
			       vm_modelines[i]->hdisplay, vm_modelines[i]->vdisplay);
		}           
		fprintf(stderr,"\n");
	}
#endif
}

void x11_vm_exit() {
#ifdef HAVE_XF86VMODE
	if (x11_fs_on) {
		XF86VidModeSwitchToMode(x11_display, XDefaultScreen(x11_display),
					vm_modelines[0]);
	}
#endif
		
}

static int search_preferable_fullscreen_mode() {
#ifdef HAVE_XF86VMODE
	int i, vm = 0, delta = INT_MAX;
	
	/* すべてのmodeのなかで最も適切なモードを選択 */
	for (i = 0; i < vm_count; i++) {
		if (vm_modelines[i]->hdisplay >= view_w && 
		    vm_modelines[i]->vdisplay >= view_y) {
			int deltaw = vm_modelines[i]->hdisplay - view_w;
			int deltah = vm_modelines[i]->vdisplay - view_h;
			if (delta > (deltaw + deltah)) {
				vm = i;
				delta = deltaw + deltah;
			}
		}
	}
	return vm;
#else
	return 0;
#endif
	
}

static void set_vidmode(int mode) {
#ifdef HAVE_XF86VMODE
	XF86VidModeSwitchToMode(x11_display, DefaultScreen(x11_display), vm_modelines[mode]);
#endif
}

/* Fullscreen 移行前の Window の座標 */
static int winsave_x, winsave_y;

static void enter_fullscreen() {
#ifdef HAVE_XF86VMODE
	int w, h, b, d;
	Window root;
	
	XGetGeometry(x11_display, x11_window, &root, &winsave_x, &winsave_y,
		     &w, &h, &b, &d);
	Xcore_setWindowSize(view_x, view_y, view_w, view_h);
	XSync(x11_display, False);
#endif
}

static void quit_fullscreen() {
#ifdef HAVE_XF86VMODE
	XF86VidModeSwitchToMode(x11_display, DefaultScreen(x11_display), vm_modelines[0]);
	XUngrabPointer(x11_display, CurrentTime);
	XMoveWindow(x11_display, x11_window, winsave_x, winsave_y);
	XSync(x11_display, False);
#endif
}

void Xcore_fullScreen(boolean on) {
	if (!have_vm) return;
	
#if HAVE_XF86VMODE
	if (on && !x11_fs_on) {
		x11_fs_on = TRUE;
		enter_fullscreen();
	} else if (!on && x11_fs_on) {
		quit_fullscreen();
		x11_fs_on = FALSE;
	}
	XSync(x11_display, False);
#endif
}

/* Windowの大きさの変更 */
void Xcore_setWindowSize(int x, int y, int width, int height) {
	
	view_x = x;
	view_y = y;
	
	if (width == view_w && height == view_h && !x11_fs_on) return;
	
	view_w = width;
	view_h = height;
	
#ifdef HAVE_XF86VMODE
	if (x11_fs_on) {
		int mode = search_preferable_fullscreen_mode();
		NOTICE("width = %d, height = %d\n",
		       vm_modelines[mode]->hdisplay,
		       vm_modelines[mode]->vdisplay);
		if (vm_modelines[mode]->hdisplay != view_w ||
		    vm_modelines[mode]->vdisplay != view_h) {
			winoffset_x = (vm_modelines[mode]->hdisplay - view_w)/2;
			winoffset_y = (vm_modelines[mode]->vdisplay - view_h)/2;
			width  = vm_modelines[mode]->hdisplay;
			height = vm_modelines[mode]->vdisplay;
			XClearWindow(x11_display, x11_window);
		} else {
			winoffset_x = winoffset_y = 0;
		}
		XMoveWindow(x11_display, x11_window, 0, 0);
		set_vidmode(mode);
		XF86VidModeSetViewPort(x11_display, XDefaultScreen(x11_display), 0, 0);
		
		XGrabPointer(x11_display, x11_window, True, 0,
		     GrabModeAsync, GrabModeAsync, x11_window, None, CurrentTime);
		XSync(x11_display, False);
	}
#endif	

	XResizeWindow(x11_display, x11_window, width, height);
	x11_makeWorkImage(width, height);
	XFlush(x11_display);
}
