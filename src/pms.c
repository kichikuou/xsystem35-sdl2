/*
 * pms.c  extract 8/16 bit PMS cg
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
 *  pms256.c
 *          0.00    97/11/06 初版
 *          0.01    97/11/13 ヘッダ情報の返しかたの変更
 *          0.01-01 97/11/27 記憶子の変更
 *          0.02    98/02/26 デバッグ完了
 *  pms64k.c
 *          0.00    97/11/06 初版
 *          0.01    97/11/13 ヘッダ情報の返しかたの変更
 *          0.02    98/02/26 デバッグ完了
 *          0.03    98/07/27 うそぷ〜。透明色の展開まずってた。
 * @version 1.1     00/09/17 rewrite for changeing interface
*/
/* $Id: pms.c,v 1.2 2000/11/25 18:31:49 chikama Exp $ */

#include <glib.h>
#include <string.h>
#include "portab.h"
#include "LittleEndian.h"
#include "graphics.h"
#include "cg.h"
#include "pms.h"

/*
 * static methods
*/
static pms_header *extract_header(BYTE *b);
static void getpal(Pallet256 *pal, BYTE *b);
static void extract_8bit(pms_header *pms, BYTE *pic, BYTE *b);
static void extract_16bit(pms_header *pms, WORD *pic, BYTE *b);

/*
 * Get information from cg header
 *   b: raw data (pointer to header)
 *   return: acquired pms information object
*/
static pms_header *extract_header(BYTE *b) {
	pms_header *pms = g_new(pms_header, 1);
	
	pms->pmsVer = LittleEndian_getW(b, 2);
	pms->pmsHdrSize = LittleEndian_getW(b, 4);
	pms->pmsBpp = b[6];
	pms->pmsBppS = b[7];
	pms->pmsSf = b[8];
	pms->pmsBf = LittleEndian_getW(b, 10);
	pms->pmsX0 = LittleEndian_getDW(b, 16);
	pms->pmsY0 = LittleEndian_getDW(b, 20);
	pms->pmsXW = LittleEndian_getDW(b, 24);
	pms->pmsYW = LittleEndian_getDW(b, 28);
	pms->pmsDp = LittleEndian_getDW(b, 32);
	pms->pmsPp = LittleEndian_getDW(b, 36);
	pms->pmsCp = LittleEndian_getDW(b, 40);
	
	return pms;
}

/*
 * Get pallet from raw data
 *   pal: pallet to be stored 
 *   b  : raw data (pointer to pallet)
*/
static void getpal(Pallet256 *pal, BYTE *b) {
	int i;
	
	for (i = 0; i < 256; i++) {
		pal->red[i]   = *b++;
		pal->green[i] = *b++;
		pal->blue[i]  = *b++;
	}
}

/*
 * Do extract 8bit pms image
 *   pms: pms header information
 *   pic: pixel to be stored
 *   b  : raw data (pointer to pixel)
*/
static void extract_8bit(pms_header *pms, BYTE *pic, BYTE *b) {
	int c0, c1;
	int x, y, loc, l, i;
	int scanline = pms->pmsXW;
	
	for (y = 0; y < pms->pmsYW; y ++) {
		for (x = 0; x < pms->pmsXW; ) {
			loc = y * scanline + x;
			c0 = *b++;
			if (c0 <= 0xf7) {
				*(pic + loc) = c0; x++;
			} else if (c0 == 0xff) {
				l = (*b) + 3; x+=l; b++;
				memcpy(pic + loc, pic + loc - scanline, l);
			} else if (c0 == 0xfe) {
				l = (*b) + 3; x+=l; b++;
				memcpy(pic + loc, pic + loc - scanline * 2, l);
			} else if (c0 == 0xfd) {
				l = (*b) + 4; x+=l; b++;
				c0 = *b++;
				memset(pic + loc, c0, l);
			} else if (c0 == 0xfc) {
				l = ((*b) + 3)  * 2; x+=l; b++;
				c0 = *b++; c1 = *b++;
				for (i = 0; i < l; i+=2) {
					*(pic + loc + i    ) = c0;
					*(pic + loc + i + 1) = c1;
				}
			} else {
				*(pic + loc) = *b++; x++;
			}
		}
	}
}

/*
 * Do extract 16bit pms image
 *   pms: pms header information
 *   pic: pixel to be stored
 *   b  : raw data (pointer to pixel)
*/
static void extract_16bit(pms_header *pms, WORD *pic, BYTE *b) {
	int c0, c1, pc0, pc1;
	int x, y, i, l, loc;
	int scanline = pms->pmsXW;
	
	for (y = 0; y < pms->pmsYW; y++) {
		for (x = 0; x < pms->pmsXW;) {
			loc = y * scanline + x;
			c0 = *b++;
			if (c0 <= 0xf7) {
				c1 = *b++; x++;
				*(pic + loc) = c0 | (c1 << 8);
			} else if (c0 == 0xff) {
				l = (*b) + 2; x+=l; b++;
				for (i = 0; i < l; i++) {
					*(pic + loc + i) = *(pic + loc + i - scanline);
				}
			} else if (c0 == 0xfe) {
				l = (*b) + 2; x+=l; b++;
				for (i = 0; i < l; i++) {
					*(pic + loc + i) = *(pic + loc + i - scanline * 2);
				}
			} else if (c0 == 0xfd) {
				l = (*b) + 3; x+=l; b++;
				c0 = *b++; c1 = *b++;
				pc0 = c0 | (c1 << 8);
				for (i = 0; i < l; i++) {
					*(pic + loc + i) = pc0;
				}
			} else if (c0 == 0xfc) {
				l = ((*b) + 2) * 2; x+=l; b++;
				c0 = *b++; c1 = *b++; pc0 = c0 | (c1 << 8);
				c0 = *b++; c1 = *b++; pc1 = c0 | (c1 << 8);
				for (i = 0; i < l; i+=2) {
					*(pic + loc + i    ) = pc0;
					*(pic + loc + i + 1) = pc1;
				}
			} else if (c0 == 0xfb) {
				x++;
				*(pic + loc) = *(pic + loc - scanline - 1);
			} else if (c0 == 0xfa) {
				x++;
				*(pic + loc) = *(pic + loc - scanline + 1);
			} else if (c0 == 0xf9) {
				l = (*b) + 1; x+=l; b++;
				c0 = *b++; c1 = *b++;
				pc0 = ((c0 & 0xe0) << 8) + ((c0 & 0x18) << 6) + ((c0 & 0x07) << 2);
				pc1 = ((c1 & 0xc0) << 5) + ((c1 & 0x3c) << 3) + (c1 & 0x03);
				*(pic + loc) = pc0 + pc1;
				for (i = 1; i < l; i++) {
					c1 = *b++;
					pc1 = ((c1 & 0xc0) << 5) + ((c1 & 0x3c) << 3) + (c1 & 0x03);
					*(pic + loc + i) = pc0 | pc1;
				}
			} else {
				c0 = *b++; c1 = *b++; x++;
				*(pic + loc) = c0 | (c1 << 8);
			}
		}
	}
}

/*
 * Check data is 8bit pms format cg or not
 *   data: raw data (pointer to data top)
 *   return: TRUE if data is pms
*/
boolean pms256_checkfmt(BYTE *data) {
	int x, y, w, h;
	
	if (data[0] != 0x50 || data[1] != 0x4d) return FALSE;
	if (data[6] != 8) return FALSE;
	
	x = LittleEndian_getDW(data, 16);
	y = LittleEndian_getDW(data, 20);
	w = LittleEndian_getDW(data, 24);
	h = LittleEndian_getDW(data, 28);
	
	if (x < 0 || y < 0 || w < 0 || h < 0) return FALSE;
	
	return TRUE;
}

/*
 * Extract 8bit pms, header, pallet and pixel
 *   data: raw data (pointer to data top)
 *   return: extracted image data and information
*/
cgdata *pms256_extract(BYTE *data) {
	cgdata *cg = g_new0(cgdata, 1);
	pms_header *pms = extract_header(data);
	
	cg->pal = g_new(Pallet256, 1);
	getpal(cg->pal, data + pms->pmsPp);
	
	/* +10: margin for broken cg */
	cg->pic = g_new(BYTE, (pms->pmsXW + 10) * (pms->pmsYW + 10));
	extract_8bit(pms, cg->pic, data + pms->pmsDp);
	
	cg->type = ALCG_PMS8;
	cg->x = pms->pmsX0;
	cg->y = pms->pmsY0;
	cg->width    = pms->pmsXW;
	cg->height   = pms->pmsYW;
	cg->pms_bank = pms->pmsBf;
	cg->alpha = NULL;
	
	g_free(pms);
	
	return cg;
}

/*
 * Check data is 16bit pms format cg or not
 *   data: raw data (pointer to data top)
 *   return: TRUE if data is pms
*/
boolean pms64k_checkfmt(BYTE *data) {
	int x, y, w, h;
	
	if (data[0] != 0x50 || data[1] != 0x4d) return FALSE;
	if (data[6] != 16) return FALSE;
	
	x = LittleEndian_getDW(data, 16);
	y = LittleEndian_getDW(data, 20);
	w = LittleEndian_getDW(data, 24);
	h = LittleEndian_getDW(data, 28);
	
	if (x < 0 || y < 0 || w < 0 || h < 0) return FALSE;
	
	return TRUE;
}

/*
 * Extract 16bit pms, header, pallet and pixel
 *   data: raw data (pointer to data top)
 *   return: extracted image data and information
*/
cgdata *pms64k_extract(BYTE *data) {
	cgdata *cg = g_new0(cgdata, 1);
	pms_header *pms = extract_header(data);
	
	/* +10: margin for broken cg */
	cg->pic = (BYTE *)g_new(WORD, (pms->pmsXW + 10) * (pms->pmsYW + 10));
	extract_16bit(pms, (WORD *)cg->pic, data + pms->pmsDp);
	
	cg->alpha = NULL;
	if (pms->pmsPp != 0) {
		cg->alpha = g_new(BYTE, (pms->pmsXW + 10) * (pms->pmsYW + 10));
		extract_8bit(pms, cg->alpha, data + pms->pmsPp);
	}
	
	cg->type = ALCG_PMS16;
	cg->x = pms->pmsX0;
	cg->y = pms->pmsY0;
	cg->width  = pms->pmsXW;
	cg->height = pms->pmsYW;
	cg->pal = NULL;
	
	g_free(pms);
	
	return cg;
}

/*
 * Extract pms pallet only
 *   data: raw data (pointer to data top)
 *   return: extracted pallet data
*/
cgdata *pms_getpal(BYTE *data) {
	cgdata *cg = g_new0(cgdata, 1);
	pms_header *pms = extract_header(data);
	
	cg->pal = g_new(Pallet256, 1);
	getpal(cg->pal, data + pms->pmsPp);
	
	cg->type  = ALCG_PMS8;
	cg->pic   = NULL;
	cg->alpha = NULL;
	
	g_free(pms);
	
	return cg;
}
