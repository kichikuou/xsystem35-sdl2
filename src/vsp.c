/*
 * vsp.c  extract VSP cg
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
 *          0.00    97/11/06 初版
 *          0.01    97/11/12 TYPO
 *          0.01-01 97/11/13 ヘッダ情報の返しかたの変更
 *          0.01-02 97/11/27 記憶子の変更
 *          0.01-03 97/12/06 ヘッダの誤認識の修正
 *          0.02    98/02/26 デバッグ済み
 *          0.03    98/03/20 ヘッダチェックのミス
 *          0.03    98/12/19 dalkで640x401なCGがあったのでその対策
 * @version 1.5     00/09/17 rewrite for changeing interface
*/
/* $Id: vsp.c,v 1.6 2000/11/25 18:31:49 chikama Exp $ */

#include <glib.h>
#include <string.h>
#include "portab.h"
#include "LittleEndian.h"
#include "graphics.h"
#include "cg.h"
#include "vsp.h"

/*
 * static methods
*/
static vsp_header *extract_header(BYTE *b);
static void getpal(Pallet256 *pal, BYTE *b);
static void extract(vsp_header *vsp, BYTE *pic, BYTE *b);

/*
 * extraction buffer
*/
static BYTE _bc[4][480];
static BYTE _bp[4][480];
static BYTE *bc[4];
static BYTE *bp[4];

/*
 * Get information from cg header
 *   b: raw data (pointer to header)
 *   return: acquired vsp information object
*/
static vsp_header *extract_header(BYTE *b) {
	vsp_header *vsp = g_new(vsp_header, 1);
	
	vsp->vspX0 = LittleEndian_getW(b, 0);
	vsp->vspY0 = LittleEndian_getW(b, 2);
	vsp->vspXW = LittleEndian_getW(b, 4) - vsp->vspX0;
	vsp->vspYW = LittleEndian_getW(b, 6) - vsp->vspY0;
	vsp->vspPb = b[9];
	vsp->vspPp = 0x0a;
	vsp->vspDp = 0x3a;
	
	return vsp;
}

/*
 * Get pallet from raw data
 *   pal: pallet to be stored 
 *   b  : raw data (pointer to pallet)
*/
static void getpal(Pallet256 *pal, BYTE *b) {
	int red, green, blue, i;
	
	for (i = 0; i < 16; i++) {
		blue  = b[i * 3 + 0];
		red   = b[i * 3 + 1];
		green = b[i * 3 + 2];
		pal->red[i]   = (red   << 4);
		pal->green[i] = (green << 4);
		pal->blue[i]  = (blue  << 4);
	}
}

/*
 * Do extract vsp image
 *   vsp: vsp header information
 *   pic: pixel to be stored
 *   b  : raw data (pointer to pixel)
*/
static void extract(vsp_header *vsp, BYTE *pic, BYTE *b) {
	int c0;
	BYTE b0, b1, b2, b3, mask = 0;
	BYTE *bt;
	int i, l, x, y, pl, loc;
	
	bp[0] = _bp[0]; bc[0] = _bc[0];
	bp[1] = _bp[1]; bc[1] = _bc[1];
	bp[2] = _bp[2]; bc[2] = _bc[2];
	bp[3] = _bp[3]; bc[3] = _bc[3];
	
	for (x = 0; x < vsp->vspXW; x++) {
		for (pl = 0; pl < 4; pl++) {
			y = 0;
			while(y < vsp->vspYW) {
				c0 = *b++;
				if (c0 >= 0x08) {
					*(bc[pl] + y) = c0; y++;
				} else if (c0 == 0x00) {
					l = (*b) + 1; b++;
					memcpy(bc[pl] + y, bp[pl] + y, l);
					y+=l;
				} else if (c0 == 0x01) {
					l = (*b) + 1; b++;
					b0 = *b++;
					memset(bc[pl] + y, b0, l);
					y+=l;
				} else if (c0 == 0x02) {
					l = (*b) + 1; b++;
					b0 = *b++; b1 = *b++;
					for (i = 0; i < l; i++) {
						*(bc[pl] + y) = b0; y++;
						*(bc[pl] + y) = b1; y++;
					}
				} else if (c0 == 0x03) {
					l = (*b) + 1; b++;
					for (i = 0; i < l; i++) {
						*(bc[pl] + y) = (*(bc[0] + y) ^ mask); y++;
					}
					mask = 0;
				} else if (c0 == 0x04) {
					l = (*b) + 1; b++;
					for (i = 0; i < l; i++) {
						*(bc[pl] + y) = (*(bc[1] + y) ^ mask); y++;
					}
					mask = 0;
				} else if (c0 == 0x05) {
					l = (*b) + 1; b++;
					for (i = 0; i < l; i++) {
						*(bc[pl] + y) = (*(bc[2] + y) ^ mask); y++;
					}
					mask = 0;
				} else if (c0 == 0x06) {
					mask = 0xff;
				} else if (c0 == 0x07) {
					*(bc[pl] + y) = *b++ ; y++;
				}
			}
		}
		/* plane -> packed 展開 */
		for (y = 0; y < vsp->vspYW; y++) {
			loc = (y * vsp->vspXW + x) * 8;
			b0 = bc[0][y]; b1 = bc[1][y];
			b2 = bc[2][y]; b3 = bc[3][y];
			*(pic + loc + 0) = ((b0>>7)&0x01)|((b1>>6)&0x02)|((b2>>5)&0x04)|((b3>>4)&0x08);
			*(pic + loc + 1) = ((b0>>6)&0x01)|((b1>>5)&0x02)|((b2>>4)&0x04)|((b3>>3)&0x08);
			*(pic + loc + 2) = ((b0>>5)&0x01)|((b1>>4)&0x02)|((b2>>3)&0x04)|((b3>>2)&0x08);
			*(pic + loc + 3) = ((b0>>4)&0x01)|((b1>>3)&0x02)|((b2>>2)&0x04)|((b3>>1)&0x08);
			*(pic + loc + 4) = ((b0>>3)&0x01)|((b1>>2)&0x02)|((b2>>1)&0x04)|((b3   )&0x08);
			*(pic + loc + 5) = ((b0>>2)&0x01)|((b1>>1)&0x02)|((b2   )&0x04)|((b3<<1)&0x08);
			*(pic + loc + 6) = ((b0>>1)&0x01)|((b1   )&0x02)|((b2<<1)&0x04)|((b3<<2)&0x08);
			*(pic + loc + 7) = ((b0   )&0x01)|((b1<<1)&0x02)|((b2<<2)&0x04)|((b3<<3)&0x08);
		}
		/* bc -> bpにコピー */
		bt = bp[0]; bp[0] = bc[0]; bc[0] = bt;
		bt = bp[1]; bp[1] = bc[1]; bc[1] = bt;
		bt = bp[2]; bp[2] = bc[2]; bc[2] = bt;
		bt = bp[3]; bp[3] = bc[3]; bc[3] = bt;
	}
}

/*
 * Check data is vsp format cg or not
 *   data: raw data (pointer to data top)
 *   return: TRUE if data is vsp
*/
boolean vsp_checkfmt(BYTE *data) {
	int x0 = LittleEndian_getW(data, 0);
	int y0 = LittleEndian_getW(data, 2);
	int w  = LittleEndian_getW(data, 4) - x0;
	int h  = LittleEndian_getW(data, 6) - y0;
	
	if (x0 < 0 || x0 > 80 || y0 < 0 || y0 > 400) return FALSE;
	/* 401: for dalk's broken cg */
	if (w  < 0 || w  > 80 || h  < 0 || h  > 401) return FALSE;
	
	return TRUE;
}

/*
 * Extract vsp, header, pallet and pixel
 *   data: raw data (pointer to data top)
 *   return: extracted image data and information
*/
cgdata *vsp_extract(BYTE *data) {
	cgdata *cg = g_new0(cgdata, 1);
	vsp_header *vsp = extract_header(data);
	
	cg->pal = g_new(Pallet256, 1);
	getpal(cg->pal, data + vsp->vspPp);
	
	/* +10: margin for broken cg */
	cg->pic = g_new(BYTE, (vsp->vspXW * 8 + 10) * (vsp->vspYW + 10));
	extract(vsp, cg->pic, data + vsp->vspDp);
	
	cg->type = ALCG_VSP;
	cg->x = vsp->vspX0 * 8;
	cg->y = vsp->vspY0;
	cg->width    = vsp->vspXW * 8;
	cg->height   = vsp->vspYW;
	cg->vsp_bank = vsp->vspPb;
	cg->alpha = NULL;
	
	g_free(vsp);
	
	return cg;
}

/*
 * Extract vsp pallet only
 *   data: raw data (pointer to data top)
 *   return: extracted pallet data
*/
cgdata *vsp_getpal(BYTE *data) {
	cgdata *cg = g_new0(cgdata, 1);
	vsp_header *vsp = extract_header(data);
	
	cg->pal = g_new(Pallet256, 1);
	getpal(cg->pal, data + vsp->vspPp);
	
	cg->type  = ALCG_VSP;
	cg->pic   = NULL;
	cg->alpha = NULL;
	
	g_free(vsp);
	
	return cg;
}
