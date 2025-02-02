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

#include "portab.h"
#include "system.h"
#include "cursor.h"
#include "LittleEndian.h"
#include "ald_manager.h"
#include "sdl_core.h"

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
	if (!sdl_cursorNew(d + pos, no, &cursorImage, &cursordirentry)) {
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
