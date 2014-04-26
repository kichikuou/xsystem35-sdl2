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

#include <glib.h>
#include "portab.h"
#include "LittleEndian.h"
#include "graphics.h"
#include "cg.h"
#include "bmp.h"

/*
 * static methods
*/
static bmp_header *extract_header(BYTE *b);
static void getpal(Pallet256 *pal, BYTE *b);
static void extract_8bit(bmp_header *bmp, BYTE *pic, BYTE *b);
static void extract_24bit(bmp_header *bmp, WORD *pic, BYTE *b);

/*
 * Get information from cg header
 *   b: raw data (pointer to header)
 *   return: acquired bmp information object
*/
static bmp_header *extract_header(BYTE *b) {
	bmp_header *bmp = g_new(bmp_header, 1);
	
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
 * Get pallet from raw data
 *   pal: pallet to be stored 
 *   b  : raw data (pointer to pallet)
*/
static void getpal(Pallet256 *pal, BYTE *b) {
	int i;
	
	for (i = 0; i < 256; i++) {
		pal->blue[i]  = *b++;
		pal->red[i]   = *b++;
		pal->green[i] = *b++;
	}
}

/*
 * Do extract 8bit bmp image
 *   bmps: bmp header information
 *   pic: pixel to be stored
 *   b  : raw data (pointer to pixel)
*/
static void extract_8bit(bmp_header *bmp, BYTE *pic, BYTE *b) {
	int i, j;
	int pos;
	int LineNeed = (bmp->bmpXW * bmp->bmpBpp) / 8;
	
	if (LineNeed % 4)
		LineNeed = (((LineNeed / 4) + 1) * 4);   /* DWORD整列化 */
	
	pos = LineNeed * (bmp->bmpYW);           /* 最上行の位置 */
	for (i = 0; i < bmp->bmpYW; i++) {
		const BYTE* p;
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
static void extract_24bit(bmp_header *bmp, WORD *pic, BYTE *b) {
	int i, j;
	int pos;
	int LineNeed = (bmp->bmpXW * bmp->bmpBpp) / 8;
	
	if (LineNeed % 4)
		LineNeed = (((LineNeed / 4) + 1) * 4);   /* DWORD整列化 */
	
	pos = LineNeed * (bmp->bmpYW); /* 最上行の位置 */
	for (i = 0; i < bmp->bmpYW; i++) {
		const BYTE* p;
		pos -= LineNeed;
		p = b + pos;
		
		for (j = 0; j < bmp->bmpXW; j++) {
			WORD r,g,b;
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
 *   return: TRUE if data is bmp
*/
boolean bmp256_checkfmt(BYTE *data) {
	int w, h, bpp;
	if (data[0] != 'B' || data[1] != 'M') return FALSE;
	
	w = LittleEndian_getDW(data, 18);
	h = LittleEndian_getDW(data, 22);
	bpp = LittleEndian_getW(data, 28);
	
	if (bpp != 8) return FALSE;
	if (w < 0 || h < 0) return FALSE;
	
	return TRUE;
}

/*
 * Extract 8bit bmp, header, pallet and pixel
 *   data: raw data (pointer to data top)
 *   return: extracted image data and information
*/
cgdata *bmp256_extract(BYTE *data) {
	bmp_header *bmp = extract_header(data);
	cgdata *cg = g_new0(cgdata, 1);
	
	cg->pal = g_new(Pallet256, 1);
	getpal(cg->pal, data + bmp->bmpPp);
	
	/* +10: margin for broken cg */
	cg->pic = g_new(BYTE, (bmp->bmpXW + 10) * (bmp->bmpYW + 10));
	extract_8bit(bmp, cg->pic, data + bmp->bmpDp);
	
	cg->type = ALCG_BMP8;
	cg->x = 0;
	cg->y = 0;
	cg->width  = bmp->bmpXW;
	cg->height = bmp->bmpYW;
	cg->alpha  = NULL;
	
	g_free(bmp);
	
	return cg;
}

/*
 * Check data is 24bit bmp format cg or not
 *   data: raw data (pointer to data top)
 *   return: TRUE if data is pms
*/
boolean bmp16m_checkfmt(BYTE *data) {
	int w, h, bpp;
	
	if (data[0] != 'B' || data[1] != 'M') return FALSE;
	
	w = LittleEndian_getDW(data, 18);
	h = LittleEndian_getDW(data, 22);
	bpp = LittleEndian_getW(data, 28);
	
	if (bpp != 24) return FALSE;
	if (w < 0 || h < 0) return FALSE;
	
	return TRUE;
}

/*
 * Extract 24bit bmp, header, pallet and pixel
 *   data: raw data (pointer to data top)
 *   return: extracted image data and information
*/
cgdata *bmp16m_extract(BYTE *data) {
	cgdata *cg = g_new0(cgdata, 1);
	bmp_header *bmp = extract_header(data);
	
	/* +10: margin for broken cg */
	cg->pic = (BYTE *)g_new(WORD, (bmp->bmpXW + 10) * (bmp->bmpYW + 10));
	extract_24bit(bmp, (WORD *)cg->pic, data + bmp->bmpDp);
	
	cg->type = ALCG_BMP24;
	cg->x = 0;
	cg->y = 0;
	cg->width  = bmp->bmpXW;
	cg->height = bmp->bmpYW;
	cg->alpha = NULL;
	cg->pal   = NULL;
	
	g_free(bmp);
	
	return cg;
}

/*
 * Extract bmp pallet only
 *   data: raw data (pointer to data top)
 *   return: extracted pallet data
*/
cgdata *bmp_getpal(BYTE *data) {
	cgdata *cg = g_new0(cgdata, 1);
	bmp_header *bmp = extract_header(data);
	
	cg->pal = g_new(Pallet256, 1);
	getpal(cg->pal, data + bmp->bmpPp);
	
	cg->type  = ALCG_BMP8;
	cg->pic   = NULL;
	cg->alpha = NULL;
	
	g_free(bmp);
	
	return cg;
}
