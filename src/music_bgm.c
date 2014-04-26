/*
 * music_bgm.c: BGMファイル(*BA.ALD)による再生
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
/* $Id: music_bgm.c,v 1.9 2003/08/30 21:29:16 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "portab.h"
#include "system.h"
#include "nact.h"
#include "music_bgm.h"
#include "music_server.h"
#include "music_fader.h"
#include "musstream.h"
#include "audio.h"
#include "ald_manager.h"
#include "wavfile.h"
#include "counter.h"
#include "pcmlib.h"


/**
 * BGI のデータ構造
 *   0xD0, 0xA0 を終端とするデータチャンクの集合で構成
 *
 *   最初の11個または12個のチャンクの内容は不明
 *      9個目のチャンクが 0xF2 で始まっていれば 12個、そうでなければ11個。
 *
 *   その後は１つのチャンク内に曲毎の番号や長さなどが格納
 *   全曲数分あり、曲チャンクの先頭が 0xD7 の場合、終端
 *
 *   曲データチャンクは 0xC2 を終端とする複数のフィールドで構成
 *     曲番号 , 0xC2, ループ回数, 0xC2, ループ時先頭戻りアドレス, 0xC2, 曲長さ,
 *     0xC2, 不明, 0xD0, 0xA0
 *
 *     それぞれのフィールドは 上位４ビットを１０進数字とみなし、任意桁の数値を
 *     構成(0xC2が終端)。下位４ビットは上位４ビットが1-9の時は3、0の時は2である
 *     (Rance5Dに例外あり)
 *     
 *     曲番号はシナリオ上での番号
 *     ループ回数は０の場合は無限
 *     ループ時先頭戻りアドレスは、ループする場合に、ここにもどってくる
 *       ループ時先頭戻りアドレスと曲長さはサンプル数(バイト長さではない)
 *
 */

struct _bgii {
	int no;      // シナリオ上での番号
	int loopno;  // 繰り返し数
	int looptop; // 巻き戻し位置
	int len;     // 曲長さ
};

struct _bgi {
	int nfile;
	struct _bgii *i;
};
typedef struct _bgi bgi_t;

// １回の cb で audio buffer の何分の１を処理するか
#define SLICE 4

#define SLOTOFFSET 100
static bgi_t *bgmindex;
static int bgmslot[10];

/*
  0xD0, 0xA0 を終端としてデータの塊があるようなので、次の先頭を探す
*/
static BYTE *findterm(BYTE *b) {
	int c1 = *b++, c2 = *b++;
	
	while (c1 != 0xd0 && c2 != 0xa0) {
		c1 = c2; c2 = *b++;
	}
	
	return b;
}

/*
  0xC2 を終端とするデータフィールドをデコード
*/
static int decode(BYTE *b, int *len) {
	int c, d = 0;
	*len = 1;
	
	while (*b != 0xc2) {
		if (*b == 0x90) { b++; (*len)++; } // r5d だけこの処理がいる
		c = *b++; c >>= 4;
		d = d * 10 + c;
		(*len)++;
	}
	return d;
}

/** 
 * BGI ファイルの解析
 */
static bgi_t *bgi_read() {
	FILE *fp;
	BYTE *b, *_b, *blast;
	long len;
	int i, skip, cnt = 0;
	bgi_t *bgi;
	
	if (nact->files.bgi == NULL) return NULL;

	if (NULL == (fp = fopen(nact->files.bgi, "r"))) {
		WARNING("%s file read fail\n", nact->files.bgi);
		return NULL;
	}
	
	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	_b = b = g_malloc(len);
	fread(b, len, 1, fp);
	fclose(fp);

	blast = b + len;
	
	// XXXX (先頭部分は不明)
	for (i = 0; i < 8; i++) {
		b = findterm(b);
	}
	if (b[0] == 0xf2) {
		skip = 4;
	} else {
		skip = 3;
	}
	for (i = 0; i < skip; i++) {
		b = findterm(b);
	}
	
	bgi = g_new(bgi_t, 1);
	bgi->nfile = 1 + (len - (b - _b)) / 29; // だいたいこれくらい?
	bgi->i = g_new(struct _bgii, bgi->nfile);
	
	while (b < blast) {
		int len;
		//printf("adr = %x\n", b - _b);
		if (*b == 0xd7) break;
		bgi->i[cnt].no = decode(b, &len); b += len;
		//printf("cnt = %d, no = %d\n", cnt, bgi->i[cnt].no);
		
		bgi->i[cnt].loopno = decode(b, &len); b += len;
		//printf("cnt = %d, loopno = %d\n", cnt, bgi->i[cnt].loopno);
		
		bgi->i[cnt].looptop = decode(b, &len); b += len;
		//printf("cnt = %d, looptop = %d\n", cnt, bgi->i[cnt].looptop);
		
		bgi->i[cnt].len = decode(b, &len); b += len;
		//printf("cnt = %d, len = %d\n", cnt, bgi->i[cnt].len);
		cnt++;
		
		b = findterm(b);
	}
	
	g_free(_b);
	return bgi;
}

static WAVFILE *bgm_load(int no) {
	dridata *dfile;
	WAVFILE *wfile;
	
	dfile = ald_getdata(DRIFILE_BGM, no -1);
	if (dfile == NULL) {
		WARNING("DRIFILE_BGM fail to open %d\n", no -1);
		return NULL;
	}
	
	wfile = wav_getinfo(dfile->data);
	if (wfile == NULL) {
		WARNING("not .wav file\n");
		ald_freedata(dfile);
		return NULL;
	}
	
	wfile->dfile = (void *)dfile;

	return wfile;
}

// とりあえず８曲くらい同時に鳴らせるようにslotを8つ使う (100-107) (^^;;;
static int slt_getnext(int no) {
	int i, j = -1;
	
	for (i = 0; i < 8; i++) {
		if (bgmslot[i] == 0) {
			bgmslot[i] = no;
			return i + SLOTOFFSET;
		} else {
			if (!prv.pcm[i + SLOTOFFSET]->playing) {
				bgmslot[i] = 0;
				j = i;
			}
		}
	}
	
	if (j != -1) {
		bgmslot[j] = no;
		return j + SLOTOFFSET;
	}		
	
	return -1;
}

// 指定の曲がどのスロットで演奏中かを調べる
static int slt_find(int no) {
	int i;
	
	for (i = 0; i < 8; i++) {
		if (bgmslot[i] == no) {
			if (!prv.pcm[i + SLOTOFFSET]->playing) {
				bgmslot[i] = 0;
				continue;
			}
			return i + SLOTOFFSET;
		}
	}
	return -1;
}

// シナリオ番号からデータ番号へ
static int senario2file(int no) {
	int i;
	
	for (i = 0; i < bgmindex->nfile; i++) {
		if (bgmindex->i[i].no == no) {
			return i;
		}
	}

	return 0; // 適当に
}

int musbgm_init() {
	bgmindex = bgi_read();
	if (bgmindex == NULL) return NG;
	return OK;
}

int musbgm_play(int no, int time, int vol) {
	pcmobj_t *obj;
	musstream_t *s;
	WAVFILE *wfile;
	int slot, dno;

	if (bgmindex == NULL) return NG;
	
	if (no <= 0) return NG;
	
	if (-1 != slt_find(no)) {
		musbgm_stop(no, 0);
	}
	slot = slt_getnext(no);
	dno = senario2file(no);
	
	wfile = bgm_load(no);
	if (wfile == NULL) {
		return NG;
	}
	
	obj = g_new0(pcmobj_t, 1);
	obj->sdata = (void *)wfile;
	obj->stype = OBJSRC_FILE;
	if (bgmindex->i[dno].len == 0) {
		s = ms_wav2(wfile, wfile->bytes, bgmindex->i[dno].looptop*4);
	} else {
		s = ms_wav2(wfile, bgmindex->i[dno].len*4, bgmindex->i[dno].looptop*4);
	}
	obj->src = s;
	obj->fmt.rate = wfile->rate;
	obj->fmt.bit  = wfile->bits;
	obj->fmt.ch   = wfile->ch;
	
	obj->data_len = (wfile->bytes * 1000) / (obj->fmt.rate * (obj->fmt.bit/8) * obj->fmt.ch);
	
	sndcnv_prepare(obj, prv.audiodev.buf.len / SLICE);
	
	obj->slot = slot;
	
	prv.pcm[slot] = obj;
	prv.vol_pcm_sub[slot] = 0; // volume valancer は channel 0 を使用
	
	obj->loop = bgmindex->i[dno].loopno;
	obj->cnt  = 0;
	obj->vollv = 0;
	obj->written_len = 0;
	obj->playing = TRUE;
	
	if (-1 == g_list_index(prv.pcmplist, obj)) {
		prv.pcmplist = g_list_append(prv.pcmplist, (gpointer)obj);
	}
	
	musfade_new(MIX_PCM, slot, time, vol, 0);
	WARNING("no=%d,time=%d,slot=%d\n", no, time, slot);
	
	return OK;
}

int musbgm_stop(int no, int time) {
	int slot;
	
	if (bgmindex == NULL) return NG;
	
	if (no <= 0) return NG;
	
	slot = slt_find(no);
	WARNING("no=%d,time=%d,slot=%d\n", no, time, slot);
	if (slot == -1) return OK;
	
	musfade_new(MIX_PCM, slot, time, 0, 1);
	
	return OK;
}

int musbgm_fade(int no, int time, int vol) {
	int slot;
	if (bgmindex == NULL) return NG;
	
	if (no <= 0) return NG;
	
	slot = slt_find(no);
	WARNING("no=%d,time=%d,slot=%d\n", no, time, slot);
	if (slot == -1) return OK;
	
	musfade_new(MIX_PCM, slot, time, vol, 0);
	
	return OK;
}

int musbgm_getpos(int no) {
	int slot;
	pcmobj_t *obj;
	guint64 len;

	if (bgmindex == NULL) return 0;
	
	if (no <= 0) return 0;
	
	slot = slt_find(no);
	if (slot == -1) return 0;
	
	obj = prv.pcm[slot];
	len = obj->src->hidden.mem.cur - obj->src->hidden.mem.base;
	
	return (int)(((guint64)len * 100) / (obj->fmt.rate * (obj->fmt.bit/8) * obj->fmt.ch));
}

int musbgm_getlen(int no) {
	int slot;
	pcmobj_t *obj;
	guint64 len;
	int ilen;
	
	if (bgmindex == NULL) return 0;
	
	if (no <= 0) return 0;
	
	slot = slt_find(no);
	if (slot == -1) {
		WAVFILE *wfile;
		// load and get
		wfile = bgm_load(no);
		len = wfile->bytes;
		ilen = (int)(((guint64)len * 100) / (wfile->rate * (wfile->bits/8) * wfile->ch));
		pcmlib_free(wfile);
	} else {
		obj = prv.pcm[slot];
		len = obj->src->hidden.mem.end - obj->src->hidden.mem.base;
		ilen = (int)(((guint64)len * 100) / (obj->fmt.rate * (obj->fmt.bit/8) * obj->fmt.ch));
	}
	
	return ilen;
}

int musbgm_isplaying(int no) {
	int slot;

	if (bgmindex == NULL) return 0;
	
	if (no <= 0) return 0;
	
	slot = slt_find(no);
	
	if (slot == -1) goto notplaying;
	
	if (prv.pcm[slot] == NULL) goto notplaying;
	if (!prv.pcm[slot]->playing) goto notplaying;
	
	return 1;

 notplaying:
	if (slot != -1) {
		bgmslot[slot - SLOTOFFSET] = 0;
	}
	return 0;
}

int musbgm_stopall(int time) {
	int i, slot;
	
	if (bgmindex == NULL) return NG;

	for (i = 0; i < 8; i++) {
		slot = bgmslot[i];
		if (slot == -1) continue;
		musfade_new(MIX_PCM, i + SLOTOFFSET, time, 0, 1);
		bgmslot[i] = 0;
	}

	return 0;
}
