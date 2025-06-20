/*
 * bmp.c  extract 8/24 bit BMP cg
 *
 * Copyright (C) 1999 TAJIRI,Yasuhiro <tajiri@wizard.elec.waseda.ac.jp>
 * rewrited      2000 CHIKAMA Masaki   <masaki-c@is.aist-nara.ac.jp>
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
 *          0.00    99/01/04 初版
 *          0.01    99/01/20 パレットを読み込む際のrgbの並びを直した
 * @version 1.1     00/09/17 rewrite for changing interface
 *
*/
/* $Id: bmp.c,v 1.3 2000/11/25 13:08:59 chikama Exp $ */

#include <stdlib.h>
#include "portab.h"
#include "LittleEndian.h"
#include "cg.h"
#include "bmp.h"

/*
 * static methods
*/
static bmp_header *extract_header(uint8_t *b);
static void extract_8bit(bmp_header *bmp, uint8_t *pic, uint8_t *b);
static void extract_24bit(bmp_header *bmp, uint16_t *pic, uint8_t *b);

/*
 * Get information from cg header
 *   b: raw data (pointer to header)
 *   return: acquired bmp information object
*/
static bmp_header *extract_header(uint8_t *b) {
	bmp_header *bmp = malloc(sizeof(bmp_header));
	
	bmp->bmpSize    = LittleEndian_getDW(b, 2);
	/*data[4]-data[7]reserv*/
	bmp->bmpDp = LittleEndian_getDW(b, 10);
	bmp->bmpXW = LittleEndian_getDW(b, 18);
 	bmp->bmpYW = LittleEndian_getDW(b, 22);
	bmp->bmpBpp = LittleEndian_getW(b, 28);
	bmp->bmpCp  = LittleEndian_getDW(b, 30);
	bmp->bmpImgSize  = LittleEndian_getDW(b, 34);
	bmp->bmpPp  = (bmp->bmpBpp==24 ? 0 :
		       14 + LittleEndian_getDW(b, 14));
	bmp->bmpTp = LittleEndian_getDW(b, 14) == 12 ? BMP_OS2 : BMP_WIN;
	
	return bmp;
}

/*
 * Get palette from raw data
 *   b  : raw data (pointer to palette)
*/
static SDL_Color *getpal(uint8_t *b) {
	SDL_Color *pal = malloc(sizeof(SDL_Color) * 256);
	for (int i = 0; i < 256; i++) {
		pal[i].b = *b++;
		pal[i].r = *b++;
		pal[i].g = *b++;
		pal[i].a = 255;
	}
	return pal;
}

/*
 * Do extract 8bit bmp image
 *   bmps: bmp header information
 *   pic: pixel to be stored
 *   b  : raw data (pointer to pixel)
*/
static void extract_8bit(bmp_header *bmp, uint8_t *pic, uint8_t *b) {
	int i, j;
	int pos;
	int LineNeed = (bmp->bmpXW * bmp->bmpBpp) / 8;
	
	if (LineNeed % 4)
		LineNeed = (((LineNeed / 4) + 1) * 4);   /* DWORD整列化 */
	
	pos = LineNeed * (bmp->bmpYW);           /* 最上行の位置 */
	for (i = 0; i < bmp->bmpYW; i++) {
		const uint8_t* p;
		pos -= LineNeed;
		p = b + pos;
		for (j = 0; j < bmp->bmpXW; j++)
			*pic++ = *p++;
	}
}

/*
 * Do extract 24bit bmp image
 *   pms: pms header information
 *   pic: pixel to be stored (16 bit converted)
 *   b  : raw data (pointer to pixel)
*/
static void extract_24bit(bmp_header *bmp, uint16_t *pic, uint8_t *b) {
	int i, j;
	int pos;
	int LineNeed = (bmp->bmpXW * bmp->bmpBpp) / 8;
	
	if (LineNeed % 4)
		LineNeed = (((LineNeed / 4) + 1) * 4);   /* DWORD整列化 */
	
	pos = LineNeed * (bmp->bmpYW); /* 最上行の位置 */
	for (i = 0; i < bmp->bmpYW; i++) {
		const uint8_t* p;
		pos -= LineNeed;
		p = b + pos;
		
		for (j = 0; j < bmp->bmpXW; j++) {
			uint16_t r,g,b;
			b = (*p++);
			g = (*p++);
			r = (*p++);
			r >>= 3;
			g >>= 2;
			b >>= 3;
			*pic = (r << 11 | g << 5 | b);
			/* 本当はPIX24なんだろうけど、その後表示する関数ないし、
			   とりあえず。
			   *pic = PIX16(r,g,b);
			   */
			pic++;
		}
	}
}

/*
 * Check data is 8bit bmp format cg or not
 *   data: raw data (pointer to data top)
 *   return: true if data is bmp
*/
bool bmp256_checkfmt(uint8_t *data) {
	int w, h, bpp;
	if (data[0] != 'B' || data[1] != 'M') return false;
	
	w = LittleEndian_getDW(data, 18);
	h = LittleEndian_getDW(data, 22);
	bpp = LittleEndian_getW(data, 28);
	
	if (bpp != 8) return false;
	if (w < 0 || h < 0) return false;
	
	return true;
}

/*
 * Extract 8bit bmp, header, palette and pixel
 *   data: raw data (pointer to data top)
 *   return: extracted image data and information
*/
cgdata *bmp256_extract(uint8_t *data) {
	bmp_header *bmp = extract_header(data);
	cgdata *cg = calloc(1, sizeof(cgdata));
	
	cg->pal = getpal(data + bmp->bmpPp);
	
	/* +10: margin for broken cg */
	cg->pic = malloc(sizeof(uint8_t) * ((bmp->bmpXW + 10) * (bmp->bmpYW + 10)));
	extract_8bit(bmp, cg->pic, data + bmp->bmpDp);
	
	cg->type = ALCG_BMP8;
	cg->x = 0;
	cg->y = 0;
	cg->width  = bmp->bmpXW;
	cg->height = bmp->bmpYW;
	cg->depth = 8;
	cg->alpha  = NULL;
	
	free(bmp);
	
	return cg;
}

/*
 * Check data is 24bit bmp format cg or not
 *   data: raw data (pointer to data top)
 *   return: true if data is pms
*/
bool bmp16m_checkfmt(uint8_t *data) {
	int w, h, bpp;
	
	if (data[0] != 'B' || data[1] != 'M') return false;
	
	w = LittleEndian_getDW(data, 18);
	h = LittleEndian_getDW(data, 22);
	bpp = LittleEndian_getW(data, 28);
	
	if (bpp != 24) return false;
	if (w < 0 || h < 0) return false;
	
	return true;
}

/*
 * Extract 24bit bmp, header, palette and pixel
 *   data: raw data (pointer to data top)
 *   return: extracted image data and information
*/
cgdata *bmp16m_extract(uint8_t *data) {
	cgdata *cg = calloc(1, sizeof(cgdata));
	bmp_header *bmp = extract_header(data);
	
	/* +10: margin for broken cg */
	cg->pic = (uint8_t *)malloc(sizeof(uint16_t) * (bmp->bmpXW + 10) * (bmp->bmpYW + 10));
	extract_24bit(bmp, (uint16_t *)cg->pic, data + bmp->bmpDp);
	
	cg->type = ALCG_BMP24;
	cg->x = 0;
	cg->y = 0;
	cg->width  = bmp->bmpXW;
	cg->height = bmp->bmpYW;
	cg->depth = 16;
	cg->alpha = NULL;
	cg->pal   = NULL;
	
	free(bmp);
	
	return cg;
}
