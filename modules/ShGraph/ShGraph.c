/*
 * ShGraph.c  アニメーション関連 module
 *
 *    大悪司
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
/* $Id: ShGraph.c,v 1.7 2002/09/01 11:54:51 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "portab.h"
#include "system.h"
#include "xsystem35.h"
#include "nact.h"
#include "ags.h"
#include "counter.h"
#include "music_client.h"

#define SLOT 40

static void copy_sprite(int sx, int sy, int width, int height, int dx, int dy, int r, int g, int b);

static MyRectangle maprect;  /* アニメーション表示領域 */
static MyRectangle mapback;  /* 背景セーブ領域 */
static int mapback_p5;       /* 背景転送先 X */
static int mapback_p6;       /* 背景転送先 Y */

struct animsrc {
	int x;
	int y;
	int w;
	int h;
	int uw;
	int uh;
	int r;
	int g;
	int b;
};
static struct animsrc src[SLOT];  /* アニメーション各コマ転送元 */
	
struct _s0 {
	int *dst_p1;
	int *dst_p2;
	int dw_1000A188;
};
static struct _s0 s0[SLOT];

struct _s1 {
	int add_p2;
	int add_p3;
	int w_1000A378;
	int add_p4;
	int add_p6; // wavfile;
};
static struct _s1 s1[SLOT];

struct _s2 {
	int dst_p3;
	int dst_p4;
	int dst_p5;
	int dst_p6;
	int w_1000A8A0;
	int w_1000A8A2;
};
static struct _s2 s2[SLOT];

static int* add_p5[SLOT]; /* どこまでアニメーションのコマが進んだか */


void Init() {
	/*
	  モジュール初期化
	*/
	int p1 = getCaliValue(); /* ISurface */
	
	DEBUG_COMMAND("ShGraph.Init %d:\n", p1);
}

void GetSurfaceData() {
	int p1 = getCaliValue(); /* ISurface */
	
	DEBUG_COMMAND_YET("ShGraph.GetSurfaceData %d:\n", p1);
}

void ChangeEquColor() {
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int *p5 = getCaliVariable();
	int *p6 = getCaliVariable();
	int p7 = getCaliValue(); /* ISurface */
	
	DEBUG_COMMAND_YET("ShGraph.ChangeEquColor %d,%d,%d,%d,%p,%p,%d:\n", p1, p2, p3, p4, p5, p6, p7);
}

void ChangeNotColor() {
	/*
	  指定の領域が src と等しくなければ dst に塗りつぶす
	  
	  x0:     source x0
	  y0:     source y0
	  width:  source width
	  height: source height
	  *src:   塗りつぶし候補の色が入っている変数 src, src+1, src+2
	  *dst:   塗りつぶしの色が入っている変数 dst, dst+1, dst+2
	*/
	int x0 = getCaliValue();
	int y0 = getCaliValue();
	int width  = getCaliValue();
	int height = getCaliValue();
	int *src = getCaliVariable(); /* r, g, b */
	int *dst = getCaliVariable(); /* r, g, b */
	int p7 = getCaliValue(); /* ISurface */
	agsurface_t *dib;
	int x, y;
	BYTE *dp;
	
	DEBUG_COMMAND("ShGraph.ChangeNotColor %d,%d,%d,%d,%p,%p,%d:\n", x0, y0, width, height, src, dst, p7);
	
	ags_check_param(&x0, &y0, &width, &height);
	ags_sync();
	
	dib = nact->ags.dib;
	dp = GETOFFSET_PIXEL(dib, x0, y0);
	
	switch(dib->depth) {
	case 15:
	{
		WORD pic15s = PIX15(*src, *(src+1), *(src+2));
		WORD pic15d = PIX15(*dst, *(dst+1), *(dst+2));
		WORD *yl;
		
		for (y = 0; y < height; y++) {
			yl = (WORD *)(dp + y * dib->bytes_per_line);
			for (x = 0; x < width; x++) {
				if (*yl != pic15s) {
					*yl = pic15d;
				}
				yl++;
			}
		}
		break;
	}
	case 16:
	{
		WORD pic16s = PIX16(*src, *(src+1), *(src+2));
		WORD pic16d = PIX16(*dst, *(dst+1), *(dst+2));
		WORD *yl;
		
		for (y = 0; y < height; y++) {
			yl = (WORD *)(dp + y * dib->bytes_per_line);
			for (x = 0; x < width; x++) {
				if (*yl != pic16s) {
					*yl = pic16d;
				}
				yl++;
			}
		}
		break;
	}
	case 24:
	case 32:
	{
		DWORD pic24s = PIX24(*src, *(src+1), *(src+2)) & 0xf0f0f0;
		DWORD pic24d = PIX24(*dst, *(dst+1), *(dst+2)) & 0xf0f0f0;
		DWORD *yl;
		
		for (y = 0; y < height; y++) {
			yl = (DWORD *)(dp + y * dib->bytes_per_line);
			for (x = 0; x < width; x++) {
				if ((*yl & 0xf0f0f0) != pic24s) {
					*yl = pic24d;
				}
				yl++;
			}
		}
		break;
	}
	}
}

/*
  アニメーション使用手順
   1: まず、リセット (ResetAnimData 0:)
   2: アニメーションを表示する領域の設定 (SetAnimeRect x,y,w,h:)
   3: アニメーションの背景が書かれている領域の設定 
      (SetAnimeBack sx,sy,w,h,dx,dy:) dx, dy は転送先
   4: 各アニメーションのパターンの領域の設定
      (SetAnimeSrc slot,sx,sy,w,h,uw,uh,pal)
   5: 各アニメーションの表示座標の設定
      (SetAnimeDst slot,varx,vary,offx,offy,offw,offh:)
   6: アニメーションパターンの設定
      何番のスロットをどの順番で再生するかを指定
      (SetAnimeData pat,src,dst,num,varpat,waveno:)
      例
       ShGraph.AddAnimeData 1,11,11,5,var1,81:
       ShGraph.AddAnimeData 1,13,12,8,var2,84:
       ShGraph.AddAnimeData 1,14,12,2,var3,20736:
       
         アニメその１は パターン 11->13->14 のスロットを再生。それぞれ、5コマ,
       8コマ,2コマづつ使用する。表示位置は 11->12->12 のスロットを使用。
       
   7: 実際に再生するアニメを決定。(AddAnimeRemain slot:)
   8: 指定するスロットまでアニメ再生 (PlayAnimeData lastslot,wait:)
      lastslotが０の場合は 7 で指定したところまで。

*/
void ResetAnimeData() {
	/*
	  アニメーション用の各種データをクリア
	  
	  no: クリア対象スロット(0なら全スロットクリア)
	*/
	int no = getCaliValue();
	
	DEBUG_COMMAND("ShGraph.ResetAnimeData %d:\n", no);
	
	if (no > 0 && no <= SLOT) {
		memset(&src[no-1], 0, sizeof(struct animsrc));
		memset(&s0[no-1],  0, sizeof(struct _s2));
		memset(&s2[no-1],  0, sizeof(struct _s0));
	} else {
		memset(src, 0, sizeof(struct animsrc) * SLOT);
		memset(s0,  0, sizeof(struct _s0) * SLOT);
		memset(s2,  0, sizeof(struct _s2) * SLOT);
	}
	
	memset(s1, 0, sizeof(struct _s1) * SLOT);
	memset(add_p5, 0, sizeof(int *) * SLOT);
	
}

void SetAnimeSrc() {
	/*
	  アニメーションの各パターンのソース領域の設定

	  no: スロット番号
	  x0: src x0
	  y0: src y0
	  w : src width
	  h : src height
	  uw: パターンの横の並びの数
	  uh: パターンの縦の並びの数
	  *pal: 抜き色を格納してある変数の先頭 (pal, pal+1, pal+2)
	*/
	int no = getCaliValue();
	int x0 = getCaliValue();
	int y0 = getCaliValue();
	int w = getCaliValue();
	int h = getCaliValue();
	int uw = getCaliValue();
	int uh = getCaliValue();
	int *pal = getCaliVariable();
	int r, g, b;
	
	DEBUG_COMMAND("ShGraph.SetAnimeSrc %d,%d,%d,%d,%d,%d,%d,%p:\n", no, x0, y0, w, h, uw, uh, pal);

	if (no <= 0 || no > SLOT) return;
	
	r = *pal;
	g = *(pal + 1);
	b = *(pal + 2);
	
	no--;
	src[no].x = x0;
	src[no].y = y0;
	src[no].w = w;
	src[no].h = h;
	src[no].uw = uw;
	src[no].uh = uh;
	src[no].r = r;
	src[no].g = g;
	src[no].b = b;
}

void SetAnimeDst() {
	/*
	  アニメーションの描画先設定
	  
	  no: スロット番号
	  p1: 現在の描画先(x)を格納する変数
	  p2: 現在の描画先(y)を格納する変数
	  p3: 描画先オフセット (x) 10000が中央(0)
	  p4: 描画先オフセット (y) 10000が中央
	  p5: 描画先オフセット追加分 (w) 10000が中央
	  p6: 描画先オフセット追加分 (h) 10000が中央
	*/
	int no = getCaliValue();
	int *p1 = getCaliVariable();
	int *p2 = getCaliVariable();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	
	DEBUG_COMMAND("ShGraph.SetAnimeDst %d,%p,%p,%d,%d,%d,%d:\n", no, p1, p2, p3, p4, p5, p6);
	
	if (no <= 0 || no > SLOT) return;
	
	no--;
	s0[no].dst_p1 = p1;
	s0[no].dst_p2 = p2;
	s2[no].dst_p3 = p3;
	s2[no].dst_p4 = p4;
	s2[no].dst_p5 = p5;
	s2[no].dst_p6 = p6;
	s2[no].w_1000A8A0 = 0;
	s2[no].w_1000A8A2 = 0;
}

void AddAnimeData() {
	/*
	  アニメーション各種設定
	  
	  no: スロット番号
	  p2: アニメーションパターン格納先頭スロット
	  p3: アニメーションパターン表示先頭スロット
	  p4: アニメーションパターン個数
	  p5: 現在どの何番目のパターンを書いているかを保存しておく変数
	  p6: 効果音 WAV ファイル番号
	*/
	int no = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int *p5 = getCaliVariable();
	int p6 = getCaliValue();
	int i;
	
	DEBUG_COMMAND("ShGraph.AddAnimeData %d,%d,%d,%d,%p,%d:\n", no, p2, p3, p4, p5, p6);
	
	if (no <= 0 || no > SLOT) return;
	
	no--;
	for (i = 0; i < SLOT; i++) {
		if (s1[i].add_p4 == 0 && s1[i].add_p6 == 0) break;
	}
	
	if (i == SLOT) return;
	
	s1[i].w_1000A378  = s2[no].w_1000A8A0;
	s2[no].w_1000A8A0 += p4;
	if (p4 != 0) {
		s2[no].w_1000A8A2 = i;
	}
	
	s1[i].add_p2 = p2 -1;
	s1[i].add_p3 = p3 -1;
	s1[i].add_p4 = p4;
	s1[i].add_p6 = p6;
	
	add_p5[i] = p5;
}

void AddAnimeRemain() {
	/*
	  アニメーション各種設定
	  
	  no: スロット番号
	*/
	int no = getCaliValue();
	int i, _max = 0;
	
       	DEBUG_COMMAND("ShGraph.AddAnimeRemain %d:\n", no);
	
	if (no <= 0 || no > SLOT) return;
	
	for (i = 0; i < SLOT; i++) {
		_max = max(s2[i].w_1000A8A0, _max);
	}
	
	no--;
	
	if (s2[no].w_1000A8A0 < _max) {
		i = s2[no].w_1000A8A2;
		s1[i].add_p4     += (_max -  s2[no].w_1000A8A0);
		s2[no].w_1000A8A0 = _max;
	}
}

void SetAnimeRect() {
	/*
	  アニメーション描画領域設定

	  x: region x
	  y: region y
	  w: region width
	  h: region height
	*/
	int x = getCaliValue();
	int y = getCaliValue();
	int w = getCaliValue();
	int h = getCaliValue();
	
	DEBUG_COMMAND("ShGraph.SetAnimeRect %d,%d,%d,%d:\n", x, y, w, h);
	
	maprect.x = x;
	maprect.y = y;
	maprect.width  = w;
	maprect.height = h;
}

void SetAnimeBack() {
	/*
	  アニメーション背景領域設定
	  
	  sx: 背景領域 x
	  sy: 背景領域 y
	  w:  背景領域 width
	  h:  背景領域 height
	  p5: 背景領域 転送先 x オフセット
	  p6: 背景領域 転送先 y オフセット
	*/
	int sx = getCaliValue();
	int sy = getCaliValue();
	int w  = getCaliValue();
	int h  = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	
	DEBUG_COMMAND("ShGraph.SetAnimeBack %d,%d,%d,%d,%d,%d:\n", sx, sy, w, h, p5, p6);

	mapback.x = sx;
	mapback.y = sy;
	mapback.width  = w;
	mapback.height = h;
	mapback_p5 = p5;
	mapback_p6 = p6;
}

void PlayAnimeData() {
	/*
	  実際にアニメーションを実行
	  
	  p1: 最大再生スロット(0の場合は全部)
	  p2: アニメーションインターバル(10msec単位)
	*/
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue(); /* ISurface */
	int p4 = getCaliValue(); /* ISys3xDIB */
	int p5 = getCaliValue(); /* ITimer */
	int p6 = getCaliValue(); /* IWinMsg */
	int interval, loop;
	int i;
	boolean is_backcopied;
	agsurface_t *dib;
	
	DEBUG_COMMAND("ShGraph.PlayAnimeData %d,%d,%d,%d,%d,%d:\n", p1, p2, p3, p4, p5, p6);
	
	interval = p2 * 10;
	
	if (p1 == 0) {
		for (i = 0; i < SLOT; i++) {
			p1 = max(s2[i].w_1000A8A0, p1);
		}
	}
	if (p1 == 0) return;
	
	dib = ags_getDIB();
	
	for (loop = 0; loop < p1; loop++) {
		int cnt = get_high_counter(SYSTEMCOUNTER_MSEC);
		is_backcopied = FALSE;
		
		for (i = 0; i < SLOT; i++) {
			int srcno = s1[i].add_p2;
			int dstno = s1[i].add_p3;
			int wavno = s1[i].add_p6;
			
#if 0
			printf("loop=%d,i=%d,w_1000A378=%d,add_p4=%d,srcno=%d, dstno=%d,dst_p1=%d,dst_p2=%d\n",
			       loop, i, s1[i].w_1000A378, s1[i].add_p4,
			       srcno, dstno,
			       *s0[dstno].dst_p1, *s0[dstno].dst_p2);
#endif
			if (loop < s1[i].w_1000A378) continue;
			
			if (s1[i].add_p4 != 0) {
				int sx, sy, dx, dy;
				
				if (!is_backcopied) {
					is_backcopied = TRUE;
					ags_copyArea(mapback.x, mapback.y,
						     mapback.width, mapback.height,
						     mapback_p5, mapback_p6);
					ags_sync();
				}
				
				if (wavno != 0) {
					if (wavno > 255) {
						mus_pcm_stop(wavno % 255);
					}
					// printf("wavPlay %d\n", wavno % 256);
					mus_wav_play(wavno % 256, 1);
					s1[i].add_p6 = 0;
				}
				
				sx = src[srcno].x + (*add_p5[i] % src[srcno].uw) * src[srcno].w;
				sy = src[srcno].y + (*add_p5[i] / src[srcno].uw) * src[srcno].h;
				
				dx = *(s0[dstno].dst_p1) + s2[dstno].dst_p3 -10000;
				dy = *(s0[dstno].dst_p2) + s2[dstno].dst_p4 -10000;
				if (dx > 10000) dx = 0;
				if (dy > 10000) dy = 0;
				
				copy_sprite(sx, sy,
					    src[srcno].w, src[srcno].h, 
					    maprect.x + dx, maprect.y + dy,
					    src[srcno].r, src[srcno].g, src[srcno].b);
				
				*(s0[dstno].dst_p1) += (s2[dstno].dst_p5 - 10000);
				*(s0[dstno].dst_p2) += (s2[dstno].dst_p6 - 10000);
				if (*(s0[dstno].dst_p1) >= 10000) {
					*(s0[dstno].dst_p1) = 0;
				}
				if (*(s0[dstno].dst_p2) >= 10000) {
					*(s0[dstno].dst_p2) = 0;
				}
				
				(*(add_p5[i]))++;
				s1[i].add_p4--;
				
				if (*add_p5[i] >= (src[srcno].uw * src[srcno].uh)) {
					*add_p5[i] = 0;
				}
			} else {
				if (wavno == 0) continue;
				
				s1[i].add_p6 = 0;
				
				if (wavno > 255) {
					mus_wav_stop(wavno % 256);
				}
				mus_wav_play(wavno % 256, 1);
				// printf("wavPlay %d\n", wavno % 256);
			}
		}
		if (is_backcopied && maprect.width != 0 && maprect.height != 0) {
			ags_updateArea(maprect.x, maprect.y, maprect.width, maprect.height);
		}
		{
			int now = get_high_counter(SYSTEMCOUNTER_MSEC);
			if (now - cnt < interval) {
				usleep((interval - (now-cnt)) * 1000);
			}
		}
	}
}

static void copy_sprite(int sx, int sy, int width, int height, int dx, int dy, int r, int g, int b) {
	int x, y;
	BYTE *sp, *dp;
	agsurface_t *dib;
	
	if (dx < 0 || dy < 0) return;
	
	ags_check_param(&sx, &sy, &width, &height);
	ags_check_param(&dx, &dy, &width, &height);
	
	dib = nact->ags.dib;
	
	sp = GETOFFSET_PIXEL(dib, sx, sy);
	dp = GETOFFSET_PIXEL(dib, dx, dy);
	
	switch(dib->depth) {
	case 15:
	{
		WORD pic15 = PIX15(r, g, b);
		WORD *yls, *yld;
		
		for (y = 0; y < height; y++) {
			yls = (WORD *)(sp + y * dib->bytes_per_line);
			yld = (WORD *)(dp + y * dib->bytes_per_line);
			for (x = 0; x < width; x++) {
				if (*yls != pic15) {
					*yld = *yls;
				}
				yls++; yld++;
			}
		}
		break;
	}
	case 16:
	{
		WORD pic16 = PIX16(r, g, b);
		WORD *yls, *yld;
		
		for (y = 0; y < height; y++) {
			yls = (WORD *)(sp + y * dib->bytes_per_line);
			yld = (WORD *)(dp + y * dib->bytes_per_line);
			for (x = 0; x < width; x++) {
				if (*yls != pic16) {
					*yld = *yls;
				}
				yls++; yld++;
			}
		}
		break;
	}
	case 24:
	case 32:
	{
		DWORD pic24 = PIX24(r, g, b) & 0xf0f0f0;
		DWORD *yls, *yld;
		
		for (y = 0; y < height; y++) {
			yls = (DWORD *)(sp + y * dib->bytes_per_line);
			yld = (DWORD *)(dp + y * dib->bytes_per_line);
			for (x = 0; x < width; x++) {
				if ((*yls & 0xf0f0f0) != pic24) {
					*yld = *yls;
				}
				yls++; yld++;
			}
		}
		break;
	}
	}
}
