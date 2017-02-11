/*
 * xcore_video.c  X11 video init
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
/* $Id: xcore_video.c,v 1.15 2003/06/29 15:28:12 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <glib.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#ifdef X_LOCALE
#include <X11/Xlocale.h>
#else
#include <locale.h>
#endif

#include <X11/extensions/XShm.h>

#include "portab.h"
#include "system.h"
#include "font.h"
#include "joystick.h"
#include "xsystem35.h"
#include "xcore.h"
#include "xcore_private.h"

static void check_SHMext(void);
static void check_packed24bpp(void);
static void init_window(void);
static void init_offscreen(void);
static void releaseDIB(void);
static void makeNonShmXImage(IMAGEINFO *simg, int w, int h, int depth);
static boolean makeShmXImage(IMAGEINFO *simg, int w, int h, int depth);
static void releaseNonShmXImage(IMAGEINFO *simg);
static void releaseShmXImage(IMAGEINFO *simg);
static void makeDIB(int w, int h, int depth);
static void releaseWorkImage();


struct xcore_private_data *x11_videodev;
static boolean noshm;

/* init/remove */

int Xcore_Initilize(void) {
	char *display;

	x11_videodev = g_new0(struct xcore_private_data, 1);
	x11_noSHM = noshm;
	
	/* locale init ( for no gtk option ) */
	if (setlocale(LC_ALL,"") == NULL) {
		if (setlocale(LC_ALL, "C") == NULL) {
			SYSERROR("locale not supported\n");
		}
	}
	

        /* Open the X11 display */
	display = NULL;     /* Get it from DISPLAY environment variable */
	
	if ((strncmp(XDisplayName(display), ":", 1) == 0) ||
	    (strncmp(XDisplayName(display), "unix:", 5) == 0)) {
		local_X11 = TRUE;
	} else {
		local_X11 = FALSE;
	}
	
	x11_display = XOpenDisplay(NULL);
	if (x11_display == NULL) {
		SYSERROR("Can't Open Display\n");
	}
	
	/* check packed 24 bpp */
	check_packed24bpp();
	
	/* check SHM extension is avaibale */
	check_SHMext();
	
	/* create toplevel window */
	init_window();
	
	/* map window to display */
	XMapWindow(x11_display, x11_window);

        /* discpriter init */
	//x11_selinfo.fd_x = ConnectionNumber(x11_display);
	
	/* event mask */
	XSelectInput(x11_display, x11_window, ExposureMask      | 
		               KeyPressMask      | KeyReleaseMask    |
		               LeaveWindowMask   | EnterWindowMask   | 
		               ButtonPressMask   | ButtonReleaseMask | 
//		               PointerMotionMask | PointerMotionHintMask);
		               PointerMotionMask);
	/* ???? */
	XSetGraphicsExposures(x11_display, x11_gc_win, False);

	/* create initial off-screen */
	init_offscreen();
	
	/* Window title bar */
	XStoreName(x11_display, x11_window, "XSystem3.5 Version "VERSION);
	
	/* wm hint */
	atom_wmprot = XInternAtom(x11_display, "WM_PROTOCOLS", False);
	atom_delwin = XInternAtom(x11_display, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(x11_display, x11_window, &atom_delwin, 1);
	
	/* init cursor */
	x11_init_cursor();

	/* final */
	XFlush(x11_display);

	/* window size set */
	Xcore_setWindowSize(0, 0, SYS35_DEFAULT_WIDTH, SYS35_DEFAULT_HEIGHT);

	return 0;
}

/* remove X */
void Xcore_Remove(void) {
	if (x11_videodev == NULL) return;

	if (x11_display != NULL) {
		NOTICE("Now X shutdown ... ");
		
		x11_vm_exit();
		
		if (x11_workinfo != NULL) {
			releaseWorkImage();
		}
		
		if (x11_dibinfo != NULL) {
			releaseDIB();
		}
		XCloseDisplay(x11_display);
		
		joy_close();
		
 		NOTICE("Done!\n");
	}
}



/* SHMext を使用しないモードに設定 */
void Xcore_setNoShmMode(void) {
	// x11_noSHM = TRUE;
	noshm = TRUE;
}

/* SHMext が使用できるかチェック */
static void check_SHMext(void) {
	int  major, minor;
	Bool pixmaps;

#if defined(__osf__)
	x11_noSHM = TRUE;   /* buggy X at least DIGITAL UNIX V4.0F */
#endif
	
	if (x11_noSHM) return;

	if (local_X11) {
		x11_noSHM = FALSE;
		return;
	}
	
	if (False == XShmQueryVersion(x11_display, &major, &minor, &pixmaps)) {
		WARNING("MIT-SHMextension is not supported\n");
		goto errexit;
	}
	
	if (False == pixmaps) {
		WARNING("SharedPixmap is not supported\n");
		goto errexit;
	}
	
	if (ZPixmap != XShmPixmapFormat(x11_display)) {
		WARNING("SharedPixmap's format is not ZPixmap\n");
		goto errexit;
	}
	
	x11_noSHM = FALSE;
	return;
	
 errexit:
	x11_noSHM = TRUE;
	return;
}

static void check_packed24bpp(void) {
	XPixmapFormatValues *list;
	int i, cnt;
	
	list = XListPixmapFormats(x11_display, &cnt);
	if (list == NULL) return;
	
	for (i = 0; i < cnt; i++) {
		if (list[i].depth == 24 && list[i].bits_per_pixel == 24) {
			packed24bpp = TRUE;
			break;
		}
	}
	XFree(list);
}


/* create toplevel window */
static void init_window(void) {
	int depth = DefaultDepth(x11_display, 0);
	boolean simple = FALSE;
	int attrib_mask;
	XSetWindowAttributes at;
	XVisualInfo vinfo;

	memset(&at, 0, sizeof(at));
	
	if (depth == 8) {
		/* 8bit の時 15/16/24bit True Color の visual を持っているか？ */
		if (True == XMatchVisualInfo(x11_display, None, 16, TrueColor, &vinfo)) {
			WIN_DEPTH = 16;
		} else if (True == XMatchVisualInfo(x11_display, None, 15, TrueColor, &vinfo)) {
			WIN_DEPTH = 15;
		} else if (True == XMatchVisualInfo(x11_display, None, 24, TrueColor, &vinfo)) {
			WIN_DEPTH = 24;
		} else if (True == XMatchVisualInfo(x11_display, None, 8, PseudoColor, &vinfo)) {
			WIN_DEPTH = 8;
			simple = TRUE;
		} else {
			SYSERROR("No required Visual\n");
		}
	} else {
		simple = TRUE;
		if (False == XMatchVisualInfo(x11_display, None, depth, TrueColor, &vinfo)) {
			SYSERROR("No required Visual\n");
		}
		WIN_DEPTH = depth;
	}
	
	x11_visual = vinfo.visual;
	
	if (simple) {
		x11_window = XCreateSimpleWindow(x11_display, RootWindow(x11_display, 0),
						 0, 0,
						 SYS35_DEFAULT_WIDTH, SYS35_DEFAULT_HEIGHT,
						 0, 0, 0);
		/* colormap */
		if (x11_visual->class == PseudoColor) {
			x11_cm = XCreateColormap(x11_display, RootWindow(x11_display,0), x11_visual, AllocAll);
			XSetWindowColormap(x11_display, x11_window, x11_cm);
		} else {
			x11_cm = DefaultColormap(x11_display, 0);
		}
		/* backing store */
		at.backing_store = WhenMapped;
		XChangeWindowAttributes(x11_display, x11_window, CWBackingStore, &at);
	} else {
		XVisualInfo *vinfo_g;
		int vinfo_n;
		
		vinfo.depth = WIN_DEPTH;
		vinfo_g = XGetVisualInfo(x11_display, VisualDepthMask | VisualClassMask| VisualRedMaskMask, &vinfo, &vinfo_n);
		if (vinfo_g == NULL) {
			SYSERROR("No required Visual\n");
		}
		x11_visual = vinfo_g->visual;
		XFree(vinfo_g);
		
		at.background_pixel = BlackPixel (x11_display, 0);
		at.border_pixel = BlackPixel (x11_display, 0);
		at.backing_store = WhenMapped;
		attrib_mask = CWBorderPixel | CWBackPixel | CWColormap | CWBackingStore;
		x11_cm = at.colormap = XCreateColormap(x11_display, RootWindow(x11_display, 0), x11_visual, AllocNone);
		x11_window = XCreateWindow(x11_display, RootWindow(x11_display, 0),
					   0, 0, SYS35_DEFAULT_WIDTH, SYS35_DEFAULT_HEIGHT,
					   0, WIN_DEPTH,
					   InputOutput, x11_visual, attrib_mask, &at);
	}
	
	/* determin dib depth candidate */
	dib_depth_candidate = (depth == 16 && vinfo.red_mask == 0x7c00) ? 15: WIN_DEPTH;
	
	/* create graphic context for window */
	x11_gc_win = XCreateGC(x11_display, x11_window, 0, 0);

	/* for fullscreen window init */
	x11_vm_init();

}

static void init_offscreen(void) {
	makeDIB(SYS35_DEFAULT_WIDTH, SYS35_DEFAULT_HEIGHT, SYS35_DEFAULT_DEPTH);
		
	x11_makeWorkImage(SYS35_DEFAULT_WIDTH, SYS35_DEFAULT_HEIGHT);
}

/* OffScreenImageに関する情報の解放 */
static void releaseDIB(void) {
	XFreePixmap(x11_display, x11_pixmap);
	XFreeGC(x11_display, x11_gc_pix);
	
	if (x11_dibinfo->shared) {
		releaseShmXImage(x11_dibinfo);
	} else {
		releaseNonShmXImage(x11_dibinfo);
	}
}

static void makeNonShmXImage(IMAGEINFO *simg, int w, int h, int depth) {
	simg->ximg = XCreateImage(x11_display, x11_visual, depth, ZPixmap, 0, NULL, w, h, packed24bpp ? 8 : 32, 0);

	/* XXXX */
	if (depth == 32 && packed24bpp) {
		simg->ximg->bytes_per_line = w * 4;  
	}
	
	if (NULL == (simg->ximg->data = malloc(simg->ximg->bytes_per_line * h))) {
		NOMEMERR();
	}
	
#ifdef WORDS_BIGENDIAN
	simg->ximg->byte_order = MSBFirst;
#else
	simg->ximg->byte_order = LSBFirst;
#endif
}

static boolean makeShmXImage(IMAGEINFO *simg, int w, int h, int depth) {
	simg->ximg = XShmCreateImage(x11_display, x11_visual, depth, ZPixmap, NULL, &simg->info, w, h);
	
	if ((simg->info.shmid = shmget(IPC_PRIVATE,
				       simg->ximg->bytes_per_line * h,
				       IPC_CREAT | 0666)) < 0) {
		WARNING("shmget: size:%x %s\n",
			simg->ximg->bytes_per_line * h, strerror(errno));
		XDestroyImage(simg->ximg);
		return FALSE;
	}
	
	if ((char *)-1 == (simg->info.shmaddr = simg->ximg->data = shmat(simg->info.shmid, 0, 0))) {
		SYSERROR("shmat: %s\n", strerror(errno));
	}
	
	simg->info.readOnly = False;
	XShmAttach(x11_display, &simg->info);
	
	return TRUE;
}

static void releaseNonShmXImage(IMAGEINFO *simg) {
	XDestroyImage(simg->ximg);
}

static void releaseShmXImage(IMAGEINFO *simg) {
	XShmDetach(x11_display, &simg->info);
	XDestroyImage(simg->ximg);
	
	if (0 > shmdt(simg->info.shmaddr)) {
		SYSERROR("shmdt: %s\n", strerror(errno));
	}
	
	if (0 > shmctl(simg->info.shmid, IPC_RMID, 0)) {
		SYSERROR("shmctl: %s\n", strerror(errno));
	}
}

static void makeDIB(int w, int h, int depth) {
	static boolean alloced = FALSE;
	
	if (WIN_DEPTH == 8 && depth != 8) {
		SYSERROR("You cannot play highcolor game in 256 color mode\n");
	}
	
	if (alloced) {
		releaseDIB();
		g_free(x11_dibinfo);
	}
	
	x11_dibinfo = g_new(IMAGEINFO, 1);
	
	if (depth != 8) {
		depth = dib_depth_candidate;
	}
	
	if (x11_noSHM ||
	    packed24bpp ||
	    depth != WIN_DEPTH || 
	    !makeShmXImage(x11_dibinfo, w, h, depth)) {
		makeNonShmXImage(x11_dibinfo, w, h, packed24bpp ? 32 : depth);
		x11_pixmap = XCreatePixmap(x11_display, x11_window, w, h, WIN_DEPTH);
		x11_dibinfo->shared = FALSE;
	} else {
		x11_pixmap = XShmCreatePixmap(x11_display, x11_window, x11_dibinfo->ximg->data, &x11_dibinfo->info, w, h, depth);
		x11_dibinfo->shared = TRUE;
	}
	
	x11_gc_pix = XCreateGC(x11_display, x11_pixmap, 0, 0);
	
	x11_dibinfo->cimg.width           = w;
	x11_dibinfo->cimg.height          = h;
	x11_dibinfo->cimg.pixel           = x11_dibinfo->ximg->data;
	x11_dibinfo->cimg.bytes_per_line  = x11_dibinfo->ximg->bytes_per_line;
	x11_dibinfo->cimg.depth           = depth;
	x11_dibinfo->cimg.bytes_per_pixel = (depth == 8                 ? 1 :
					     depth == 15 || depth == 16 ? 2 :
					     depth == 24 || depth == 32 ? 4 : 1);
	x11_dibinfo->cimg.alpha = NULL;
	
	image_setdepth(depth);
	alloced = TRUE;
}

/* fader用のImageを解放 */
static void releaseWorkImage() {
	if (x11_workinfo->shared) {
		releaseShmXImage(x11_workinfo);
	} else {
		releaseNonShmXImage(x11_workinfo);
	}
}

/* fader/ecopy用のImageを確保 */
void x11_makeWorkImage(int w, int h) {
	static boolean alloced = FALSE;
	
	if (alloced) {
		releaseWorkImage();
		g_free(x11_workinfo);
	}
	
	x11_workinfo = g_new(IMAGEINFO, 1);
	
	if (x11_noSHM ||
	    packed24bpp ||
	    !makeShmXImage(x11_workinfo, w, h, WIN_DEPTH)) {
		makeNonShmXImage(x11_workinfo, w, h, WIN_DEPTH);
		x11_workinfo->shared = FALSE;
	} else {
		x11_workinfo->shared = TRUE;
	}
	
	XSync(x11_display, False);

	x11_workinfo->cimg.width           = w;
	x11_workinfo->cimg.height          = h;
	x11_workinfo->cimg.pixel           = x11_workinfo->ximg->data;
	x11_workinfo->cimg.bytes_per_line  = x11_workinfo->ximg->bytes_per_line;
	x11_workinfo->cimg.depth           = x11_workinfo->ximg->depth;
	x11_workinfo->cimg.bytes_per_pixel = (WIN_DEPTH == 8  ? 1 :
					      WIN_DEPTH == 15 || WIN_DEPTH == 16 ? 2 :
					      WIN_DEPTH == 24 && packed24bpp     ? 3 :
					      WIN_DEPTH == 24 || WIN_DEPTH == 32 ? 4 : 1);
	x11_workinfo->cimg.alpha = NULL;
	
	alloced = TRUE;
}

void Xcore_setFontDevice(FONT *f) {
	x11_font = f;
}

/* name is EUC */
void Xcore_setWindowTitle(char *name) {
	XmbSetWMProperties (x11_display, x11_window,
			    name, name, NULL, 0, NULL, NULL, NULL);
}

/* offscreen の設定 */
void Xcore_setWorldSize(int width, int height, int depth) {
	makeDIB(width, height, depth);
	Xcore_fillRectangle(0, 0, width, height, 0);
	XSync(x11_display, False);
}

/* Windowの size と depth の取得 */
void Xcore_getWindowInfo(DispInfo *info) {
	info->width  = DisplayWidth(x11_display, 0);
	info->height = DisplayHeight(x11_display, 0);
	info->depth  = WIN_DEPTH;
}

/*
 DIBの取得
 SharedPixma / XImage / MemoryImageのいずれか
 */
agsurface_t *Xcore_getDIB(void) {
	return DIB;
}

/* AutoRepeat の設定 */
void Xcore_setAutoRepeat(boolean bool) {
	if (bool) {
		XAutoRepeatOn(x11_display);
	} else {
		XAutoRepeatOff(x11_display);
	}
}
