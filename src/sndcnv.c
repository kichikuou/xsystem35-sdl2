/*
 * sndcnv.c  PCM フォーマット変換
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
/* $Id: sndcnv.c,v 1.10 2003/08/22 17:09:23 chikama Exp $ */

#include <stdio.h>
#include <string.h>
#include <glib.h>

#include "portab.h"
#include "music_pcm.h"
#include "music_server.h"

#include "sndcnv_rate.c"

#define RIGHT(datum, bits)      ((datum) >> bits)
#define LEFT(datum, bits)       ((datum) << bits)


static int sndcnv_convert(pcmobj_t *pcm, int lv, int outlen);
static int sndcnv_norateconvert(pcmobj_t *pcm, int lv, int outlen);

int sndcnv_prepare(pcmobj_t *pcm, int outlen) {
	pcm->conv.ifmt = pcm->fmt;
	pcm->conv.ofmt = prv.ofmt;
	
	if (pcm->conv.ifmt.rate != pcm->conv.ofmt.rate) {
		pcm->conv.convert = sndcnv_convert;
	} else {
		pcm->conv.convert = sndcnv_norateconvert;
	}		
	
	pcm->conv.buf = g_malloc(outlen);
	pcm->conv.isample = 
		(outlen * (pcm->fmt.bit/8) * pcm->fmt.ch * pcm->fmt.rate) /
		((prv.ofmt.bit/8) * prv.ofmt.ch * prv.ofmt.rate);
	
	st_rate_start(&pcm->conv);
	
	//printf("in rate = %d, bit = %d, ch = %d\n", pcm->fmt.rate,
	//       pcm->fmt.bit, pcm->fmt.ch);
	//printf("devbuflen = %d\n", prv.audiodev.buf.len);
	return OK;
}

static int sndcnv_norateconvert(pcmobj_t *pcm, int lv, int outlen) {
	void *ibuf, *buf0, *pd;
	int len, i;
	int isample;
	
	if (pcm->conv.ifmt.ch  == pcm->conv.ofmt.ch  &&
	    pcm->conv.ifmt.bit == pcm->conv.ofmt.bit &&
	    lv == 100) {
		// puts("norateconv");
		len = pcm->src->read(pcm->src, pcm->conv.buf, 1, pcm->conv.isample);
		return len;
	}
	
	ibuf = g_malloc(outlen*2);
	
	len = pcm->src->read(pcm->src, ibuf, 1, pcm->conv.isample);
	
	if (len == 0) {
		g_free(ibuf);
		return 0;
	}
	
	buf0  = g_malloc(outlen*2);
	
	// 8bit -> 16bit
	switch(pcm->fmt.bit) {
	case 8: // 8bit
	{
		unsigned char *src = ibuf;
		WORD *dst = buf0;
		
		isample = len;
		for (i = 0; i < isample; i++) {
			*dst = LEFT(*src-128, 7);
			src++; dst++;
		}
		pd = buf0;
		break;
	}
	case 16:
		isample = len / 2;
		pd = ibuf;
		break;
	default:
		isample  = 0;
		printf("no supported\n");
		g_free(ibuf);
		g_free(buf0);
		return 0;
	}
	
	// change volume
	if (lv != 100) {
		double v = lv / 100.0, y;
		short *p = pd;
		for (i = 0; i < isample; i++) {
			y = v * *p;
			if (y < -32767.0) {
				y = -32767.0;
			} else if (y > 32767.0) {
				y = 32767.0;
			}
			*p++ = y + 0.5;
		}
	}
	
	// stereo 化
	if (pcm->fmt.ch == 1) {
		unsigned short *src, *dst;
		src = pd; dst = pcm->conv.buf;
		for (i = 0; i < isample; i++) {
			*dst = *src; dst++;
			*dst = *src; dst++; src++;
		}
		isample *= 4;
	} else {
		memcpy(pcm->conv.buf, pd, isample * 2);
		isample *= 2;
	}
	
	g_free(buf0);
	g_free(ibuf);

	return isample;
}

static int sndcnv_convert(pcmobj_t *pcm, int lv, int outlen) {
	void *ibuf, *buf0, *bufl, *bufr, *bufrw, *buflw;
	int len, i;
	void *pr, *pl;
	LONG isample = 0, osample = 0;
	LONG osampler = LONG_MAX, osamplel = LONG_MAX;

	ibuf = g_malloc(outlen*2);
	
	// printf("pcm->src = %p, sample = %d, outlen = %d\n", pcm->src, pcm->conv.isample, outlen);
	len = pcm->src->read(pcm->src, ibuf, 1, pcm->conv.isample);
	
	if (len == 0) {
		g_free(ibuf);
		return 0;
	}
	
	buf0  = g_malloc(outlen*2);
	bufr  = g_malloc(outlen*2);
	bufl  = g_malloc(outlen*2);
	bufrw = g_malloc(outlen*2);
	buflw = g_malloc(outlen*2);
	
	// 8|16 bit -> LONG 変換
	switch(pcm->fmt.bit) {
	case 8: // 8bit
	{
		unsigned char *src = ibuf;
		LONG *dst = buf0;
		isample = len;
		for (i = 0; i < isample; i++) {
			*dst = LEFT(*src-128, 23);
			src++; dst++;
		}
		break;
	}
	case 16: // 16 bit
	{
		unsigned short *src = ibuf;
		LONG  *dst = buf0;
		
		isample = len / 2;
		for (i = 0; i < isample; i++) {
			*dst = LEFT(*src, 16);
			src++; dst++;
		}
		break;
	}
	}
	
	// change volume
	if (lv != 100) {
		double v = lv / 100.0, y;
		LONG *p = buf0;
		// puts("change vol");
		for (i = 0; i < isample; i++) {
			y = v * *p;
			if (y < -2147483647.0) {
				y = -2147483647.0;
			} else if (y > 2147483647.0) {
				y = 2147483647.0;
			}
			*p++ = y + 0.5;
		}
	}
	
	// stereo 分解
	if (pcm->fmt.ch == 2) {
		LONG *src = buf0, *dstl = bufl, *dstr = bufr;
		isample /= 2;
		for (i = 0; i < isample; i++) {
			*dstl = *src; dstl++; src++;
			*dstr = *src; dstr++; src++;
		}
		pl = bufl;
		pr = bufr;
	} else {
		pl = buf0;
		pr = NULL;
	}
	
	// rate 変換 left
	i = isample;
	osamplel = outlen;
	//printf("insample = %d, osample = %d\n", isample, osamplel);
	st_rate_flow(&pcm->conv, pl, buflw, &isample, &osamplel);
	//printf("insample = %d, osample = %d\n", isample, osamplel);

	// rate 変換 right
	if (pcm->fmt.ch == 2) {
		isample  = i;
		osampler = outlen;
		//printf("insample = %d, osample = %d\n", isample, osampler);
		st_rate_flow(&pcm->conv, pr, bufrw, &isample, &osampler);
		//printf("insample = %d, osample = %d\n", isample, osampler);
	}
	osample = MIN(osampler, osamplel);
	
	// LONG -> 16bit 変換
	if (pcm->fmt.ch == 2) {
		LONG *srcl, *srcr;
		unsigned short *dstl, *dstr;
		
		srcl = buflw; srcr = bufrw;
		dstl = bufl;  dstr = bufr;
		
		for (i = 0; i < osample; i++) {
			*dstl = RIGHT(*srcl, 16); dstl++; srcl++;
			*dstr = RIGHT(*srcr, 16); dstr++; srcr++;
		}
	} else {
		LONG *src = buflw;
		unsigned short *dst = bufl;
		
		for (i = 0; i < osample; i++) {
			*dst = RIGHT(*src, 16); dst++; src++;
		}
	}
	
	// stereo 化
	if (pcm->fmt.ch == 2) {
		unsigned short *srcl, *srcr, *dst;
		
		srcl = bufl; srcr = bufr;
		dst  = pcm->conv.buf;
		for (i = 0; i < osample; i++) {
			*dst = *srcl; dst++; srcl++;
			*dst = *srcr; dst++; srcr++;
		}
	} else {
		unsigned short *src, *dst;
		src = bufl; dst = pcm->conv.buf;
		
		for (i = 0; i < osample; i++) {
			*dst = *src; dst++;
			*dst = *src; dst++; src++;
		}
	}

	g_free(buflw);
	g_free(bufrw);
	g_free(bufl);
	g_free(bufr);
	g_free(buf0);
	g_free(ibuf);

	//printf("outlen = %d\n", osample * 4);
	return osample * 4;  // 変換後の長さ(byte数)
}

int sndcnv_drain(pcmobj_t *pcm) {
	g_free(pcm->conv.buf);
	return OK;
}
