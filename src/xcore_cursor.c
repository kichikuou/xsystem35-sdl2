/*
 * xcore_cursor.c カーソル処理 for Xcore
 *
 * Copyright (C) 2000- TAJIRI Yasuhiro  <tajiri@venus.dti.ne.jp>
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
/* $Id: xcore_cursor.c,v 1.8 2001/03/30 19:16:38 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>

#include "portab.h"
#include "system.h"
#include "cursor.h"
#include "xcore_private.h"

/* マウスカーソルフォントイメージ*/
#include "bitmaps/curbm0.xbm"
#include "bitmaps/curbm1.xbm"
#include "bitmaps/curbm2.xbm"
#include "bitmaps/curbm3.xbm"
#include "bitmaps/curbm4.xbm"
#include "bitmaps/curbm5.xbm"
#include "bitmaps/curbm6.xbm"
#include "bitmaps/maskbm0.xbm"
#include "bitmaps/maskbm1.xbm"
#include "bitmaps/maskbm2.xbm"
#include "bitmaps/maskbm3.xbm"
#include "bitmaps/maskbm4.xbm"
#include "bitmaps/maskbm5.xbm"
#include "bitmaps/maskbm6.xbm"

static Cursor          cursor[256];
static GC              cursorGC;

/* mouse cursur の初期化 */
void x11_init_cursor(void) {
	XColor black, white;
	Pixmap curPix[7], maskPix[7],tmpPixmap;
	MyPoint spot[7] = {{10, 6}, {15, 15}, {14, 15}, {14, 15}, {14, 14}, {15, 15},{16,16}};
				  
	black.pixel = 0x00000000;
	black.red   = 0;
	black.green = 0;
	black.blue  = 0;
	white.pixel = 0xffffffff;
	white.red   = 0xffff;
	white.green = 0xffff;
	white.blue  = 0xffff;
	
	curPix[0]  = XCreateBitmapFromData(x11_display, x11_window, curbm0_bits,
					   curbm0_width, curbm0_height);
	maskPix[0] = XCreateBitmapFromData(x11_display, x11_window, maskbm0_bits,
					   maskbm0_width, maskbm0_height);
	curPix[1]  = XCreateBitmapFromData(x11_display, x11_window, curbm1_bits,
					   curbm1_width, curbm1_height);
	maskPix[1] = XCreateBitmapFromData(x11_display, x11_window, maskbm1_bits,
					   maskbm1_width, maskbm1_height);
	curPix[2]  = XCreateBitmapFromData(x11_display, x11_window, curbm2_bits,
					   curbm2_width, curbm2_height);
	maskPix[2] = XCreateBitmapFromData(x11_display, x11_window, maskbm2_bits,
					   maskbm2_width, maskbm2_height);
	curPix[3]  = XCreateBitmapFromData(x11_display, x11_window, curbm3_bits,
					   curbm3_width, curbm3_height);
	maskPix[3] = XCreateBitmapFromData(x11_display, x11_window, maskbm3_bits,
					   maskbm3_width, maskbm3_height);
	curPix[4]  = XCreateBitmapFromData(x11_display, x11_window, curbm4_bits,
					   curbm4_width, curbm4_height);
	maskPix[4] = XCreateBitmapFromData(x11_display, x11_window, maskbm4_bits,
					   maskbm4_width, maskbm4_height);
	curPix[5]  = XCreateBitmapFromData(x11_display, x11_window, curbm5_bits,
					   curbm5_width, curbm5_height);
	maskPix[5] = XCreateBitmapFromData(x11_display, x11_window, maskbm5_bits,
					   maskbm5_width, maskbm5_height);
	curPix[6]  = XCreateBitmapFromData(x11_display, x11_window, curbm6_bits,
					   curbm6_width, curbm6_height);
	maskPix[6] = XCreateBitmapFromData(x11_display, x11_window, maskbm6_bits,
					   maskbm6_width, maskbm6_height);
	
	cursor[CURSOR_ARROW] = XCreatePixmapCursor(x11_display, curPix[0], maskPix[0],
						&black, &white, spot[0].x, spot[0].y);
	cursor[CURSOR_CROSS] = XCreateFontCursor(x11_display, XC_tcross);
	cursor[CURSOR_IBEAM] = cursor[CURSOR_ICON] = XCreateFontCursor(x11_display, XC_icon);
	cursor[CURSOR_NO]    = XCreateFontCursor(x11_display, XC_X_cursor);
	cursor[CURSOR_SIZE]  = XCreatePixmapCursor(x11_display, curPix[6], maskPix[6],
						&black, &white, spot[6].x, spot[6].y);
	cursor[CURSOR_SIZEALL]  = cursor[CURSOR_SIZE];
	cursor[CURSOR_SIZENESW] = XCreatePixmapCursor(x11_display, curPix[4], maskPix[4],
						   &black, &white, spot[4].x, spot[4].y);
	cursor[CURSOR_SIZENS]   = XCreatePixmapCursor(x11_display, curPix[2], maskPix[2],
						   &black, &white, spot[2].x, spot[2].y);
	cursor[CURSOR_SIZENWSE] = XCreatePixmapCursor(x11_display, curPix[3], maskPix[3],
						   &black, &white, spot[3].x, spot[3].y);
	cursor[CURSOR_SIZEWE]   = XCreatePixmapCursor(x11_display, curPix[1], maskPix[1],
						   &black, &white, spot[1].x, spot[1].y);
	cursor[CURSOR_UPARROW]  = XCreateFontCursor(x11_display, XC_sb_up_arrow);
//	cursor[CURSOR_WAIT]     = XCreateFontCursor(x11_display, XC_watch);
	cursor[CURSOR_WAIT]     = XCreatePixmapCursor(x11_display, curPix[5], maskPix[5],
						   &black, &white, spot[5].x, spot[5].y);
	
	if ((tmpPixmap = XCreatePixmap(x11_display,x11_window, 1, 1, 1)))
	{
		cursorGC = XCreateGC( x11_display, tmpPixmap, 0, NULL );
		XSetGraphicsExposures( x11_display, cursorGC, False );
		XFreePixmap( x11_display, tmpPixmap );
	}
}

boolean Xcore_cursorNew(BYTE* data, int no, CursorImage *cursorImage,  TCursorDirEntry *cursordirentry) {
	int    xormasklen, andmasklen, xornum;
	int    i, j, k, l;
	int    h = 0;
	BYTE   *buf1, *buf2, *buf3,*buf4;
	Pixmap xorPix, andPix;
	XColor bg, fg;
	
	bg.pixel = 0xffffff;
	bg.red   = cursorImage->icColors[0].rgbRed*256;
	bg.green = cursorImage->icColors[0].rgbGreen*256;
	bg.blue  = cursorImage->icColors[0].rgbBlue*256;
	fg.pixel = 0xffffff;
	fg.red   = cursorImage->icColors[1].rgbRed*256;
	fg.green = cursorImage->icColors[1].rgbGreen*256;
	fg.blue  = cursorImage->icColors[1].rgbBlue*256;
	
	xornum = (cursordirentry->bWidth * cursordirentry->bHeight);
	xormasklen = (xornum * cursorImage->icHeader.biBitCount) / 8;
	NOTICE("Cursor:  xormasklen==%d,  xornum==%d\n", xormasklen, xornum);
	
	andmasklen = xornum / 8;
	cursorImage->xormasklen = xormasklen;
	cursorImage->andmasklen = andmasklen;
	

	buf1 = malloc(sizeof(BYTE) * xornum);
	buf2 = malloc(sizeof(BYTE) * xornum);
	buf3 = malloc(sizeof(BYTE) * xornum);
	buf4 = malloc(sizeof(BYTE) * xornum);
	
	memcpy(buf1, data, min(xormasklen, xornum));
	data += xormasklen;
	
	memcpy(buf2, data, min(andmasklen, xornum));
	data += andmasklen;
	
#define height cursordirentry->bHeight
#define width  cursordirentry->bWidth
	
	for (j = 0; j < height; j++) {
		for (i = 0; i < width * cursorImage->icHeader.biBitCount /8; i++) {
			k = buf1[(height-j-1)*height*cursorImage->icHeader.biBitCount/8+i];
			l = buf2[(height-j-1)*height*cursorImage->icHeader.biBitCount/8+i];
#define SWAPBITS(x) ((x&   1)<<7|(x&   2)<<5|(x&   4)<<3|(x&   8)<<1|\
                     (x&0x10)>>1|(x&0x20)>>3|(x&0x40)>>5|(x&0x80)>>7)
			buf3[h] = SWAPBITS(l);
			buf4[h] = SWAPBITS(k);
			h++;
#undef SWAPBITS
		}
	}
	xorPix = XCreateBitmapFromData(x11_display, x11_window, buf3, width, height);
	andPix = XCreateBitmapFromData(x11_display, x11_window, buf4, width, height);
	
	/*ここから、XOR,ANDがたから、X用のカーソルに作りかえ*/
	/*wine-991114 windows/x11drv/mouse.cより引用
	 * X11 mouse driver
	 *
	 * Copyright 1998 Ulrich Weigand
	 */
	{
		Pixmap pixmapBits, pixmapMask, pixmapMaskInv;
		pixmapBits = XCreatePixmap( x11_display, x11_window,
					    width, height, 1 );
		pixmapMask = XCreatePixmap( x11_display, x11_window,
					    width, height, 1 );
		pixmapMaskInv = XCreatePixmap( x11_display, x11_window,
					       width, height, 1 );
		
		if (pixmapBits && pixmapMask && xorPix && andPix)
		{
			XSetFunction( x11_display, cursorGC, GXcopy );
			XCopyArea( x11_display, xorPix, pixmapBits, cursorGC,
				   0, 0, width, height, 0, 0 );
			XCopyArea( x11_display, xorPix, pixmapMask, cursorGC,
				   0, 0, width, height, 0, 0 );
			XCopyArea( x11_display, xorPix, pixmapMaskInv, cursorGC,
				   0, 0, width, height, 0, 0 );
			XSetFunction( x11_display, cursorGC, GXand );
			XCopyArea( x11_display, andPix, pixmapMaskInv, cursorGC,
				   0, 0, width, height, 0, 0 );
			XSetFunction( x11_display, cursorGC, GXandReverse );
			XCopyArea( x11_display, andPix, pixmapBits, cursorGC,
				   0, 0, width, height, 0, 0 );
			XSetFunction( x11_display, cursorGC, GXorReverse );
			XCopyArea( x11_display, andPix, pixmapMask, cursorGC,
				   0, 0, width, height, 0, 0 );
			XSetFunction( x11_display, cursorGC, GXor );
			XCopyArea( x11_display, pixmapMaskInv, pixmapMask, cursorGC,
				   0, 0, width, height, 1, 1 );
			XCopyArea( x11_display, pixmapMaskInv, pixmapBits, cursorGC,
				   0, 0, width, height, 1, 1 );
			XSetFunction( x11_display, cursorGC, GXcopy );
			cursor[no] = XCreatePixmapCursor(x11_display, pixmapBits, pixmapMask,
							 &fg, &bg, cursordirentry->wxHotspot,
							 cursordirentry->wyHotspot);
		}
		
		if (xorPix) XFreePixmap( x11_display, xorPix );
		if (andPix) XFreePixmap(x11_display, andPix );
		if (pixmapBits) XFreePixmap( x11_display, pixmapBits );
		if (pixmapMask) XFreePixmap( x11_display, pixmapMask );
		if (pixmapMaskInv) XFreePixmap( x11_display, pixmapMaskInv );
	}
	
	free(buf1);
	free(buf2);
	free(buf3);
	free(buf4);
	
#undef height
#undef width

	return TRUE;
}

/* マウスの位置の移動 */
void Xcore_setCursorLocation(int x, int y) {
	XWarpPointer(x11_display, x11_window, x11_window,
		     0, 0, 0, 0,
		     x + winoffset_x, y + winoffset_y);
	XFlush(x11_display);
}

/* マウスカーソルの形状の設定 */
void Xcore_setCursorType(int type) {
	XDefineCursor(x11_display, x11_window, cursor[type]);
}
