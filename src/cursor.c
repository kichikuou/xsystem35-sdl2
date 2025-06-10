/*
 * cursor.c カーソル処理(general)
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
/* $Id: cursor.c,v 1.6 2001/04/04 21:55:39 chikama Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>

#include "portab.h"
#include "system.h"
#include "cursor.h"
#include "LittleEndian.h"
#include "ald_manager.h"

#include "bitmaps/cursor_uparrow.xpm"

#define CURSOR_ARROW     1
#define CURSOR_CROSS     2
#define CURSOR_IBEAM     3
#define CURSOR_ICON      4
#define CURSOR_NO        5
#define CURSOR_SIZE      6
#define CURSOR_SIZEALL   7
#define CURSOR_SIZENESW  8
#define CURSOR_SIZENS    9
#define CURSOR_SIZENWSE 10
#define CURSOR_SIZEWE   11
#define CURSOR_UPARROW  12
#define CURSOR_WAIT     13

typedef struct {
	short idReserved;  /* always set to 0 */
	short idType;      /* always set to 1 */
	short idCount;     /* number of cursor images,always set to 1 */
	/* immediately followed by idCount TCursorDirEntries */
} CursorHeader;

typedef struct {
	unsigned char  bWidth;       /* Width */
	unsigned char  bHeight;	     /* Height */
	unsigned char  bColorCount;
	unsigned char  bReserved;
	unsigned short wxHotspot;
	unsigned short wyHotspot;
	unsigned long  dwBytesInRes; /* total number of bytes in image */
	unsigned long  dwImageOffset;
} TCursorDirEntry;

typedef struct {
	long biSize;           /* sizeof(TBitmapInfoHeader) */
	long biWidth;          /* width of bitmap */
	long biHeight;	       /* height of bitmap, see notes */
	short biPlanes;	       /* planes, always 1 */
	short biBitCount;      /* number of color bits */
	long biCompression;    /* compression used, 0 */
	long biSizeImage;      /* size of the pixel data, see notes */
	long biXPelsPerMeter;  /* not used, 0 */
	long biYPelsPerMeter;  /* not used, 0 */
	long biClrUsed;	       /* # of colors used, set to 0 */
	long biClrImportant;   /* important colors, set to 0 */
	short hotX;
	short hotY;
} TBitmapInfoHeader;

/*
   biHeight=2*TIconDirEntry.bHeight;
   biSizeImage=ANDmask + XORmask;

   XORmask=(TIconDirEntry.bWidth * TIconDirEntry.bHeight * biBitCount)/8;
   ANDmask=(TIconDirEntry.bWidth * TIconDirEntry.bHeight)/8;
 */

typedef struct {
	unsigned char rgbBlue;     /* blue component of color */
	unsigned char rgbGreen;    /* green component of color */
	unsigned char rgbRed;      /* red component of color */
	unsigned char rgbReserved; /* reserved, 0 */
} TRGBQuad;

typedef struct {
	TBitmapInfoHeader icHeader; /* image header info */
	TRGBQuad *icColors;	    /* image palette */
	int xormasklen;
	int andmasklen;
} CursorImage;

typedef struct {
	int cbSizeof;
	int cFrames;
	int cSteps;
	int cx;
	int cy;
	int cBitCount;
	int cPlanes;
	int jiffRate;
	int fl;

	int *rate;
	int *seq;
} AnimationCursorHeader ;

typedef struct {
	AnimationCursorHeader *header;
	CursorImage *images;
} AniCursorImage;

static SDL_Cursor *cursor[256];

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

void cursor_init(void) {
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

static bool cursor_new(uint8_t* data, int no, CursorImage *cursorImage, TCursorDirEntry *cursordirentry) {
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

void cursor_set_type(int type) {
	if (cursor[type] != NULL) {
		SDL_SetCursor(cursor[type]);
	}
}

static CursorHeader    cursorHeader;
static TCursorDirEntry cursordirentry;
static CursorImage     cursorImage;
static AnimationCursorHeader anicurHeader;
static AniCursorImage  anicurImage;

typedef struct RIFFchunk {
	int size;
	char *data;
} RIFFchunk_t;

static bool search_chunk(uint8_t *src, char *key1, char *key2, RIFFchunk_t *c) {
	if (0 == strncmp(src, key1, 4)) {
		c->size = LittleEndian_getW(src, 4);
		if (key2) {
			if (0 == strncmp(src+8, key2, 4)) {
				c->data = src + 12;
				return true;
			}
			return false;
		}
		c->data = src + 8;
		return true;
	}
	return false;
}

static bool is_riff(uint8_t *data) {
	if (0 == strncmp(data, "RIFF", 4)) {
		return true;
	}
	return false;
}

static bool check_iconheader(uint8_t *data) {
	
	/* read Reserved bit, abort if not 0 */
	cursorHeader.idReserved = LittleEndian_getW(data, 0);
	if (cursorHeader.idReserved != 0) {
		return false;
	}
	
	data += 2;
	
	/* read Resource Type, 2 is for cursors, abort if different */
	cursorHeader.idType = LittleEndian_getW(data, 0);
	if (cursorHeader.idType != 2) {
		return false;
	}
	
	data += 2;
	
	/* read Number of images, abort if invalid value (0) */
	cursorHeader.idCount = LittleEndian_getW(data, 0);
	
	data += 2;
	
	/* Number of images (>0) */
	if (cursorHeader.idCount == 0) {
		WARNING("Cursor: no images in file!");
		return false;
	}
	
	if (cursorHeader.idCount > 1) {
		WARNING("Cursor:  warning:  too much images in file!"); 
	}
	return true;
}

static int read_direntries(uint8_t* data) {
	uint8_t *p = data;
	
	/* read Width, in pixels */
	cursordirentry.bWidth = *data;
	data++;
	
	/* read Height, in pixels */
	cursordirentry.bHeight = *data;
	data++;
	
	/* and validate data */
	NOTICE("Cursor:  bWidth==%d  bHeight==%d",
	       (int)cursordirentry.bWidth,
	       (int)cursordirentry.bHeight);
	
	if (cursordirentry.bWidth == 0 || cursordirentry.bHeight == 0) {
		return -1;
	}
	
	/* Bits per pixel, not used, 0 */
	cursordirentry.dwBytesInRes = LittleEndian_getW(data, 0);
	data += 2;
        
	cursordirentry.wxHotspot = LittleEndian_getW(data, 0);
	data += 2;
	
	cursordirentry.wyHotspot = LittleEndian_getW(data, 0);
	data += 2;
	
	/* size of image data, in bytes */
	cursordirentry.dwBytesInRes = LittleEndian_getDW(data, 0);
	data += 4;
	
	/* size of image data, in bytes */
	cursordirentry.dwImageOffset = LittleEndian_getDW(data, 0);
	data += 4;
	
	if (cursordirentry.dwImageOffset == 0) {
		return 0;
	}
	
	NOTICE("Cursor:  x==%d  y==%d",
	       (int)cursordirentry.wxHotspot,
	       (int)cursordirentry.wyHotspot);
	
	
	NOTICE("Width: %d", (int)(cursordirentry.bWidth));
	NOTICE("Height: %d", (int)(cursordirentry.bHeight));
	NOTICE("Bit Count (unused): %d", (int)(cursordirentry.dwBytesInRes));
	NOTICE("Total bytes: %ld", (long)(cursordirentry.dwBytesInRes));
	
	return (int)(data - p);
}

static int read_bitmapinfo(uint8_t* data) {
	uint8_t* p = data;
	
#define ih cursorImage.icHeader
	/* read bitmap info an perform some primitive sanity checks */

	/* sizeof(TBitmapInfoHeader) */
	ih.biSize = LittleEndian_getDW(data, 0);
	data += 4;
	
	/* width of bitmap */
	ih.biWidth = LittleEndian_getDW(data, 0);
	data += 4;
	
	/* height of bitmap, see notes (icon.h) */
	ih.biHeight=LittleEndian_getDW(data,0);
	data += 4;
	
	NOTICE("Cursor:  biWidth==%d  biHeight==%d", (int)ih.biWidth, (int)ih.biHeight);

	if (ih.biWidth == 0 || ih.biHeight == 0) {
		return 0;
	}
	
	/* planes, always 1 */
	ih.biPlanes = LittleEndian_getW(data, 0);
	data += 2;
	
	/* number of color bits (2,4,8) */
	ih.biBitCount = LittleEndian_getW(data, 0);
	if (ih.biBitCount != 1) {
		WARNING("Cursor: %d not supported color bit", ih.biBitCount);
		return 0;
	}
	data += 2;

	/* compression used, 0 */
	ih.biCompression = LittleEndian_getDW(data, 0);
	data += 4;
	
	if (ih.biCompression != 0) {
		WARNING("Cursor:  invalid compression value of %d", (int)ih.biCompression);
		return 0;
	}
	
	/* size of the pixel data, see icon.h */
	ih.biSizeImage = LittleEndian_getDW(data, 0);
	data += 4;
	
	NOTICE("Cursor:  biSizeImage==%d", (int)ih.biSizeImage);
	
	ih.biXPelsPerMeter = LittleEndian_getDW(data, 0);
	data += 4;

	/* not used, 0 */
	ih.biYPelsPerMeter = LittleEndian_getDW(data, 0);
	data += 4;
	
	/* # of colors used, set to 0 */
	ih.biClrUsed = LittleEndian_getDW(data, 0);
	data += 4;
	
	/* important colors, set to 0 */
	ih.biClrImportant = LittleEndian_getDW(data, 0);
	data += 4;
	
#undef ih
	
	return (int)(data - p);
}

static int read_rgbquad(uint8_t* data) {
	int j;
	const int colors=2;
	uint8_t* p = data;
	
	free(cursorImage.icColors);
	cursorImage.icColors = malloc(sizeof(TRGBQuad) * colors);
	
	if (cursorImage.icColors == NULL) {  /* shouldn't happen */
		NOMEMERR();
	}
	
#define cc cursorImage.icColors
	
	for (j = 0; j < colors; j++) {
		cc[j].rgbBlue = *data;
		data++;
		cc[j].rgbGreen = *data;
		data++;
		cc[j].rgbRed = *data;
		data++;
		cc[j].rgbReserved = *data;
		data++;
		NOTICE("#%d:  Red: %d  Green: %d  Blue: %d", j, 
		       cc[j].rgbRed,
		       cc[j].rgbGreen,
		       cc[j].rgbBlue);
	}
	
#undef cc
	return (int)(data - p);
}

static bool cursor_load_mono(uint8_t *d, int no) {
	int pos = 6, p1;
	
	/* check header information */
	if (!check_iconheader(d)) {
		WARNING("check_iconhdader fail");
		return false;
	}
	
	/* read dentries */
	if ((pos += read_direntries(d + pos)) <= 6) {
		WARNING("read dentries fail");
		return false;
	}
	
	/* read bitmap info */
	p1 = read_bitmapinfo(d + pos);
	if (p1 == 0) {
		WARNING("unable to read bitmap info");
		return false;
	}
	
	pos += p1;
	
	/* read rgb quad */
	p1 = read_rgbquad(d + pos);
	if (p1 == 0) {
		WARNING("unable to read palette table");
		return false;
	}
	
	pos += p1;
	
	/* read pixedl data */
	if (!cursor_new(d + pos, no, &cursorImage, &cursordirentry)) {
		WARNING("unable to read pixel data");
		return false;
	}
	return true;
}

static bool cursor_load_anim(uint8_t *data, int no) {
	uint8_t *b = data;
	int riffsize;
	RIFFchunk_t c;
	
	if (!search_chunk(b, "RIFF", "ACON", &c)) {
		WARNING("Not animation icon format");
		return false;
	}
	
	riffsize = c.size;
	// printf("size = %d\n", c.size);
	b = c.data;
	
	while (b < (data + riffsize)) {
		if (search_chunk(b, "LIST", "INFO", &c)) {
			NOTICE("LIST(INFO) ignore size = %d", c.size);
			b += c.size + 8;
		} else if (search_chunk(b, "anih", NULL, &c)) {
			uint8_t *src = c.data;
			NOTICE("anih size = %d", c.size);
			anicurHeader.cbSizeof  = LittleEndian_getW(src, 0);
			anicurHeader.cFrames   = LittleEndian_getW(src, 4);
			anicurHeader.cSteps    = LittleEndian_getW(src, 8);
			anicurHeader.cx        = LittleEndian_getW(src, 12);
			anicurHeader.cy        = LittleEndian_getW(src, 16);
			anicurHeader.cBitCount = LittleEndian_getW(src, 20);
			anicurHeader.cPlanes   = LittleEndian_getW(src, 24);
			anicurHeader.jiffRate  = LittleEndian_getW(src, 28);
			anicurHeader.fl        = LittleEndian_getW(src, 32);
			
			anicurImage.header = &anicurHeader;
			
			b += c.size + 8;
		} else if (search_chunk(b, "rate", NULL, &c)) {
			uint8_t *src = c.data;
			NOTICE("rate size = %d", c.size);
			if (anicurHeader.fl & 0x01) {
				int i;
				anicurHeader.rate = malloc(sizeof(int) * anicurHeader.cSteps);
				for (i = 0; i < anicurHeader.cSteps; i++) {
					anicurHeader.rate[i] = LittleEndian_getW(src, i*4);
					// printf("rate %d, %d\n", i, anicurHeader.rate[i]);
				}
			}
			b += c.size + 8;
		} else if (search_chunk(b, "icon", NULL, &c)) {
			NOTICE("icon size = %d", c.size);
			cursor_load_mono(c.data, no); /* last pattern is uesd */
			b += c.size + 8;
		} else if (search_chunk(b, "LIST", "frame", &c)) {
			NOTICE("LIST(frame) size = %d", c.size);
			b += 12;
		} else {
			WARNING("UnKnown chunk");
			b += c.size + 8;
		}
	}
	return true;
}

void cursor_load(int no, int linkno) {
	dridata *dfile;

	/* no must be from 100 to 255 */
	if (no < 100 || no > 256){
		WARNING("wrong cursor number(%d)", no);
	}
	
	/* load data */
	if (NULL == (dfile = ald_getdata(DRIFILE_RSC, linkno -1))) {
		WARNING("ald_getdata fail");
		return;
	}
	
	if (is_riff(dfile->data)) {
		cursor_load_anim(dfile->data, no);
	} else {
		cursor_load_mono(dfile->data, no);
	}
	
	ald_freedata(dfile);
	
	return;
}
