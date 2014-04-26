/*
 * cursor.h カーソル処理(general)
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
/* $Id: cursor.h,v 1.3 2001/03/30 19:16:38 chikama Exp $ */

#ifndef __CURSOR_H__
#define __CURSOR_H__

#define BitSet(byte, bit)  (((byte) & (bit)) == (bit))

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

extern void cursor_load(int no, int linkno);

#endif  /* __CURSOR_H__ */
