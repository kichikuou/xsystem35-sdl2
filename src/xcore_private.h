/*
 * xcore_private.c  X11 only private data
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
/* $Id: xcore_private.h,v 1.5 2003/06/29 15:28:12 chikama Exp $ */

#ifndef __XCORE_PRIVATE_H__
#define __XCORE_PRIVATE_H__

#include "config.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>

#include "portab.h"
#include "font.h"
#include "ags.h"
#include "image.h"

/* dib/work image 情報 */
typedef struct {
	XImage          *ximg;  /* XImage/ShmXImage 使用時の XImage */
	XShmSegmentInfo info;   /* Shared Pixmap info */
	boolean         shared; /* is Shared Pixmap ? */
	agsurface_t     cimg;   /* Common Image for image manupilate */
} IMAGEINFO;

struct xcore_private_data {
	boolean local_X11;    /* flag: TRUE if local display */
	Display *dsp;         /* Used for event and window management */
	Visual  *vis;         /* Vidusl of Current Window */

	Window   win;         /* Main Window */
	int      win_depth;   /* Main Window Depth */
	int      winoffset_x; /* draw offset in Window x */
	int      winoffset_y; /*                       y */

	Atom     atom_wmprot; /* window manager  protocol atom  */
	Atom     atom_delwin; /* close-window protocol atom  */

	GC       gc_win;      /* The Graphics context for drawing to Window */
	
	GC       gc_pix;      /* The Graphics context for drawing to Pixmap */
	Pixmap   pix;         /* Pixmap for offscreen image */

	int      dib_depth_candidate; /* DIB depth if required pixmap depth isnt 8 */ 
	
	FONT *fontinfo;           /* font object */
	
	boolean noSHM;        /* dont have X-SHM extension */

	boolean packed24bpp;  /* window is packed 24 bpp? */
	
	/* SharedPixmapを使用中にXの描画が入った場合 XSyncを発行するためのフラグ */
	boolean needSync;     

	boolean fullscreen;   /* FullScreen Mode である */
	
	IMAGEINFO *dib;       /* DIB? information */
	int view_x;
	int view_y;
	int view_width;
	int view_height;

	IMAGEINFO *work;
	
	XColor    col[256];    /* pallet and color map */
	Colormap  cm;
};

extern void x11_init_cursor(void);
extern void x11_makeWorkImage(int w, int h);
extern void x11_vm_init(void);
extern void x11_vm_exit(void);
extern Pixmap x11_clip_from_DIB(int sx, int sy, int w, int h);


extern struct xcore_private_data *x11_videodev;

#define local_X11 (x11_videodev->local_X11)
#define x11_display (x11_videodev->dsp)
#define x11_visual (x11_videodev->vis)
#define x11_window (x11_videodev->win)
#define winoffset_x (x11_videodev->winoffset_x)
#define winoffset_y (x11_videodev->winoffset_y)
#define atom_delwin (x11_videodev->atom_delwin)
#define atom_wmprot (x11_videodev->atom_wmprot)
#define x11_gc_win (x11_videodev->gc_win)
#define x11_gc_pix (x11_videodev->gc_pix)
#define x11_pixmap (x11_videodev->pix)
#define dib_depth_candidate (x11_videodev->dib_depth_candidate)
#define x11_font (x11_videodev->fontinfo)
#define x11_noSHM (x11_videodev->noSHM)
#define packed24bpp (x11_videodev->packed24bpp)
#define x11_needSync (x11_videodev->needSync)
#define x11_dibinfo (x11_videodev->dib)
#define x11_workinfo (x11_videodev->work)
#define view_x (x11_videodev->view_x)
#define view_y (x11_videodev->view_y)
#define view_w (x11_videodev->view_width)
#define view_h (x11_videodev->view_height)
#define x11_cm (x11_videodev->cm)
#define x11_col (x11_videodev->col)
#define x11_fs_on (x11_videodev->fullscreen)
#define DRAW_BY_X (!x11_dibinfo->shared)
#define DIB       (&(x11_dibinfo->cimg))
#define DIB_DEPTH (x11_dibinfo->cimg.depth)
#define WIN_DEPTH (x11_videodev->win_depth)

#define WORK       (&(x11_workinfo->cimg))

// #define WROK_DEPTH (x11_workinfo->cimg.depth)

#define PAL2PIC(i) image_index2pixel(DIB_DEPTH, i)


#endif /* __XCORE_PRIVATE_H__ */
