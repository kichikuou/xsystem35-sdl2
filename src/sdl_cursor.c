/*
 * sdl_cursor.c  SDL cursor 
 *
 * Copyright (C) 2000-     Fumihiko Murata       <fmurata@p1.tcnet.ne.jp>
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
/* $Id: sdl_cursor.c,v 1.4 2001/03/30 19:16:38 chikama Exp $ */

#include "config.h"

#include <SDL.h>

#include "portab.h"
#include "system.h"
#include "cursor.h"
#include "sdl_private.h"

/* マウスカーソルフォントイメージ*/
#include "bitmaps/cursor_uparrow.xpm"

static SDL_Cursor      *cursor[256];

/* Stolen from the SDL mailing list */
/* Creates a new mouse cursor from an XPM */

static SDL_Cursor *init_system_cursor(const char *image[]) {
	int i, row, col;
	Uint8 data[4*32];
	Uint8 mask[4*32];
	int hot_x, hot_y;
	
	i = -1;
	for (row = 0; row < 32; row++) {
		for (col = 0; col < 32; col++) {
			if (col % 8) {
				data[i] <<= 1;
				mask[i] <<= 1;
			} else {
				i++;
				data[i] = mask[i] = 0;
			}
			switch (image[4 + row][col]) {
			case 'X':
				data[i] |= 0x01;
				mask[i] |= 0x01;
				break;
			case '.':
				mask[i] |= 0x01;
				break;
			case ' ':
				break;
			}
		}
	}
	sscanf(image[4 + row], "%d,%d", &hot_x, &hot_y);
	return SDL_CreateCursor(data, mask, 32, 32, hot_x, hot_y);
}

void sdl_cursor_init(void) {
	cursor[CURSOR_ARROW]    = SDL_GetDefaultCursor();
	cursor[CURSOR_CROSS]    = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
	cursor[CURSOR_IBEAM]    = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
	cursor[CURSOR_ICON]     = SDL_GetDefaultCursor();
	cursor[CURSOR_NO]       = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NO);
	cursor[CURSOR_SIZE]     = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
	cursor[CURSOR_SIZEALL]  = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
	cursor[CURSOR_SIZENESW] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
	cursor[CURSOR_SIZENS]   = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
	cursor[CURSOR_SIZENWSE] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
	cursor[CURSOR_SIZEWE]   = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
	cursor[CURSOR_UPARROW]  = init_system_cursor(cursor_uparrow);
	cursor[CURSOR_WAIT]     = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAIT);
}

bool sdl_cursorNew(uint8_t* data, int no, CursorImage *cursorImage, TCursorDirEntry *cursordirentry) {
	int    xormasklen, andmasklen, xornum;
	int    i, j;
	int    h = 0;
	
	uint8_t   *buf1, *buf2, *buf3, *buf4;
	
	xornum = (cursordirentry->bWidth * cursordirentry->bHeight);
	xormasklen = (xornum * cursorImage->icHeader.biBitCount) / 8;
	NOTICE("Cursor:  xormasklen==%d,  xornum==%d", xormasklen, xornum);
	
	andmasklen = xornum / 8;
	cursorImage->xormasklen = xormasklen;
	cursorImage->andmasklen = andmasklen;
	
	buf1 = malloc(sizeof(uint8_t) * xornum);
	buf2 = malloc(sizeof(uint8_t) * xornum);
	buf3 = malloc(sizeof(uint8_t) * xornum);
	buf4 = malloc(sizeof(uint8_t) * xornum);
	
	memcpy(buf1, data, min(xormasklen, xornum));
	data += xormasklen;
	
	memcpy(buf2, data, min(andmasklen, xornum));
	data += andmasklen;
	
#define height cursordirentry->bHeight
#define width  cursordirentry->bWidth
	
	for (j = 0; j < height; j++) {
		for (i = 0; i < width * cursorImage->icHeader.biBitCount /8; i++) {
			buf3[h] = buf1[(height-j-1)*height*cursorImage->icHeader.biBitCount/8+i];
			buf4[h] = 0xff ^ buf2[(height-j-1)*height*cursorImage->icHeader.biBitCount/8+i];
			h++;
		}
	}
	
	if (cursor[no])
		SDL_FreeCursor(cursor[no]);
	cursor[no] = SDL_CreateCursor(buf3, buf4, 32, 32, cursordirentry->wxHotspot, cursordirentry->wyHotspot);
	
	free(buf1);
	free(buf2);
	free(buf3);
	free(buf4);
	
#undef height
#undef width
	
	return true;
}

MyPoint sdl_translateMouseCoords(int x, int y) {
	// scale mouse x and y
	float scalex, scaley;
	SDL_RenderGetScale(sdl_renderer, &scalex, &scaley);
	x *= scalex;
	y *= scaley;

	// calculate window borders
	int logw, logh;
	SDL_RenderGetLogicalSize(sdl_renderer, &logw, &logh);

	float scalew, scaleh;
	scalew = logw * scalex;
	scaleh = logh * scaley;

	int winw, winh;
	SDL_GetWindowSize(sdl_window, &winw, &winh);

	float border_left = (winw - scalew) / 2;
	float border_top  = (winh - scaleh) / 2;

	// offset x and y by window borders
	x += border_left;
	y += border_top;

	MyPoint p = { x, y };
	return p;
}

/* マウスの位置の移動 */
void sdl_setCursorLocation(int x, int y) {
	MyPoint t = sdl_translateMouseCoords(x, y);
	SDL_WarpMouseInWindow(sdl_window, t.x, t.y);
}

/* マウスカーソルの形状の設定 */
void sdl_setCursorType(int type) {
	if (cursor[type] != NULL) {
		SDL_SetCursor(cursor[type]);
	}
}
