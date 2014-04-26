/*
 * ags.c  system35のグラフィックブリッジ
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
/* $Id: ags.c,v 1.34 2004/10/31 04:18:05 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <glib.h>

#include "portab.h"
#include "xsystem35.h"
#include "ags.h"
#include "graphicsdevice.h"
#include "alpha_plane.h"
#include "counter.h"
#include "eucsjis.h"
#include "imput.h"
#include "flood.h"
#include "font.h"
#include "cursor.h"

#define check_param    ags_check_param
#define check_param_xy ags_check_param_xy
#define intersection   ags_intersection

static void    initPal(Pallet256 *sys_pal);
static boolean intersects(MyRectangle *r1, MyRectangle *r2);

static Pallet256 pal_256;
static boolean need_update = TRUE;
static boolean fade_outed = FALSE;
static int cursor_move_time = 50; /* カーソル移動にかかる時間(ms) */

static void initPal(Pallet256 *pal) {
	int i;
	for (i = 0; i < 256; i++) {
		pal->red[i]   =   0; pal->green[i]   =   0; pal->blue[i]   =   0;
	}
	pal->red[0]   =   0; pal->green[0]   =   0; pal->blue[0]   =   0;
	pal->red[7]   = 255; pal->green[7]   = 255; pal->blue[7]   = 255;
	pal->red[15]  = 255; pal->green[15]  = 255; pal->blue[15]  = 255;
	pal->red[255] = 255; pal->green[255] = 255; pal->blue[255] = 255;
	SetPallet(pal, 0, 256);
	nact->sys_pal_changed = TRUE;
}

boolean ags_regionContains(MyRectangle *r, int x, int y) {
	return x >= r->x && x < r->x + r->width && y >= r->y && y < r->y + r->height;
}

static boolean intersects(MyRectangle *r1, MyRectangle *r2) {
        return !((r2->x + r2->width  <= r1->x) ||
                 (r2->y + r2->height <= r1->y) ||
                 (r2->x >= r1->x + r1->width)  ||
                 (r2->y >= r1->y + r1->height));
}

void ags_intersection(MyRectangle *r1, MyRectangle *r2, MyRectangle *rst) {
        int x1 = max(r1->x, r2->x);
        int x2 = min(r1->x + r1->width, r2->x + r2->width);
        int y1 = max(r1->y, r2->y);
        int y2 = min(r1->y + r1->height, r2->y + r2->height);
        rst->x = x1;
	rst->y = y1;
	rst->width  = x2 - x1;
	rst->height = y2 - y1;
}

boolean ags_check_param(int *x, int *y, int *w, int *h) {
	if (*x >= nact->sys_world_size.width) {
		WARNING("Illegal Param x = %d (max=%d)(@%03x:%05x)\n", *x, nact->sys_world_size.width, sl_getPage(), sl_getIndex());
		return FALSE;
	}
	if (*y >= nact->sys_world_size.height) {
		WARNING("Illegal Param y = %d (max=%d)\n", *y, nact->sys_world_size.height);
		return FALSE;
	}
	
	if (*x < 0) { *w += *x; *x = 0; }
	if (*y < 0) { *h += *y; *y = 0; }
	
	if ((*x + *w) > nact->sys_world_size.width)  { *w = nact->sys_world_size.width  - *x;}
	if ((*y + *h) > nact->sys_world_size.height) { *h = nact->sys_world_size.height - *y;}
	
	if (*w <= 0) return FALSE;
	if (*h <= 0) return FALSE;
	
	return TRUE;
}

boolean ags_check_param_xy(int *x, int *y) {
	if (*x >= nact->sys_world_size.width) {
		WARNING("Illegal Param x = %d\n", *x);
		return FALSE;
	}
	if (*y >= nact->sys_world_size.height) {
		WARNING("Illegal Param y = %d\n", *y);
		return FALSE;
	}
	
	if (*x < 0) { *x = 0; }
	if (*y < 0) { *y = 0; }
	
	return TRUE;
}

void ags_init() {
	nact->sys_mouse_movesw = 2; /* 0:IZを無視, 1: 直接指定場所へ, 2: スムーズに指定場所に */
	nact->sys_pal = &pal_256;
	nact->sys_world_size.width  =  SYS35_DEFAULT_WIDTH;
	nact->sys_world_size.height =  SYS35_DEFAULT_HEIGHT;
	nact->sys_world_depth =  SYS35_DEFAULT_DEPTH;
	nact->sys_view_area.x = 0;
	nact->sys_view_area.y = 0;
	nact->sys_view_area.width  = SYS35_DEFAULT_WIDTH;
	nact->sys_view_area.height = SYS35_DEFAULT_HEIGHT;
	
	GraphicsInitilize();
	
	font_init(nact->fontdev);
	SetFontDevice(nact->ags.font);
	
	initPal(&pal_256);
	cg_init();
}

void ags_remove() {
	ags_autorepeat(TRUE);
	GraphicsRemove();
}

void ags_setWorldSize(int width, int height, int depth) {
	nact->sys_world_size.width  = width;
	nact->sys_world_size.height = height;
	nact->sys_world_depth       = depth;
	SetWorldSize(width, height, depth);
	
	nact->ags.dib = GetDIB();
	nact->ags.dib->has_alpha = FALSE;
	nact->ags.dib->has_pixel = TRUE;
	
	/* DIBが8以上の場合は、alpha plane を用意 */
	if (depth > 8) {
		if (nact->ags.dib->alpha != NULL) {
			g_free(nact->ags.dib->alpha);
		}
		nact->ags.dib->alpha = g_new0(BYTE, width * height);
		nact->ags.dib->has_alpha = TRUE;
	}
	
	fade_outed = FALSE;  /* thanx tajiri@wizard */
	
	nact->sys_pal_changed = TRUE;
}

void ags_setViewArea(int x, int y, int width, int height) {
	nact->sys_view_area.x = x;
	nact->sys_view_area.y = y;
	
	nact->sys_view_area.width  = width;
	nact->sys_view_area.height = height;
	SetWindowSize(x, y, width, height);
}

void ags_setWindowTitle(char *src) {
#define TITLEHEAD "XSystem35 Version "VERSION":"
	BYTE *dst, *d;

	dst = sjis2lang(src);
	if (NULL == (d = malloc(strlen(dst) + strlen(TITLEHEAD) + 1))) {
		NOMEMERR();
	}
	strcpy(d, TITLEHEAD);
	strcat(d, dst);
	SetWindowTitle(d);
	free(dst);
	free(d);
}

void ags_getDIBInfo(DispInfo *info) {
	info->width  = nact->sys_world_size.width;
	info->height = nact->sys_world_size.height;
	info->depth  = nact->sys_world_depth;
}

void ags_getViewAreaInfo(DispInfo *info) {
	GetWindowInfo(info);
	info->width  = nact->sys_view_area.width;
	info->height = nact->sys_view_area.height;
}

void ags_getWindowInfo(DispInfo *info) {
	GetWindowInfo(info);
}

void ags_setExposeSwitch(boolean bool) {
	need_update = bool;
}

void ags_updateArea(int x, int y, int w, int h) {
	MyRectangle r, update;
	MyPoint p;
	
	if (fade_outed) return;
	
	if (need_update) {
		r.x = x; r.y = y; r.width = w; r.height = h;
		if (intersects(&nact->sys_view_area, &r)) {
			intersection(&nact->sys_view_area, &r, &update);
			p.x = update.x - nact->sys_view_area.x;
			p.y = update.y - nact->sys_view_area.y;
			UpdateArea(&update, &p);
		}
	}
}

void ags_updateFull() {
	MyPoint p = {0, 0};
	MyRectangle r;
	
	if (fade_outed) return;
	
	if (need_update) {
		r.x = nact->sys_view_area.x;
		r.y = nact->sys_view_area.y;
		r.width  = min(nact->sys_view_area.width,  nact->sys_world_size.width);
		r.height = min(nact->sys_view_area.height, nact->sys_world_size.height);
		UpdateArea(&r, &p);
	}
}

void ags_setPallets(Pallet256 *src_pal, int src, int dst, int cnt) {
	int i;
	for (i = 0; i < cnt; i++) {
		nact->sys_pal->red  [dst + i] = src_pal->red  [src + i];
		nact->sys_pal->green[dst + i] = src_pal->green[src + i];
		nact->sys_pal->blue [dst + i] = src_pal->blue [src + i];
	}
	nact->sys_pal_changed = TRUE;
}

void ags_setPallet(int no, int red, int green, int blue) {
	nact->sys_pal->red[no]   = red;
	nact->sys_pal->green[no] = green;
	nact->sys_pal->blue[no]  = blue;
	nact->sys_pal_changed = TRUE;
}

void ags_setPalletToSystem(int src, int cnt) {
	if (!fade_outed) 
		SetPallet(nact->sys_pal, src, cnt);
}

void ags_drawRectangle(int x, int y, int w, int h, int col) {
	if (!check_param(&x, &y, &w, &h)) return;
	
	DrawRectangle(x, y, w, h, col);
}

void ags_fillRectangle(int x, int y, int w, int h, int col) {
	if (!check_param(&x, &y, &w, &h)) return;

	FillRectangle(x, y, w, h, col);
}

void ags_drawLine(int x0, int y0, int x1, int y1, int col) {
	if (!check_param_xy(&x0, &y0)) return;
	if (!check_param_xy(&x1, &y1)) return;

	DrawLine(x0, y0, x1, y1, col);
}

void ags_copyArea(int sx, int sy, int w, int h, int dx, int dy) {
	if (!check_param(&sx, &sy, &w, &h)) return;
	if (!check_param(&dx, &dy, &w, &h)) return;
	
	CopyArea(sx, sy, w, h, dx, dy);
}

void ags_scaledCopyArea(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, int mirror_sw) {
	if (!check_param(&sx, &sy, &sw, &sh)) return;
	if (!check_param(&dx, &dy, &dw, &dh)) return;
	
	DspDeviceSync(); /* Device依存の sync (ex. XSync()) */
	ScaledCopyArea(sx, sy, sw, sh, dx, dy, dw, dh, mirror_sw);
}

void ags_copyAreaSP(int sx, int sy, int w, int h, int dx, int dy, int col) {
	if (!check_param(&sx, &sy, &w, &h)) return;
	if (!check_param(&dx, &dy, &w, &h)) return;

	DspDeviceSync();
	CopyAreaSP(sx, sy, w, h, dx, dy, col);
}

void ags_wrapColor(int x, int y, int w, int h, int p1, int p2) {
	if (nact->sys_world_depth == 8) return;
	
	if (!check_param(&x, &y, &w, &h)) return;
	
	DspDeviceSync();
	WrapColor(x, y, w, h, p1, p2);
}

void ags_getPixel(int x, int y, Pallet *cell) {
	if (!check_param_xy(&x, &y)) return;

	DspDeviceSync();
	GetPixel(x, y, cell);
}

void ags_changeColorArea(int sx, int sy, int w, int h, int dst, int src, int cnt) {
	if (nact->sys_world_depth != 8) return;
	
	if (!check_param(&sx, &sy, &w, &h)) return;
	
	DspDeviceSync();
	{
		agsurface_t *dib = nact->ags.dib;
		int   x, y;
		int   src_last = src + cnt,dif = dst - src;
		BYTE *yl;
		BYTE *sdata = GETOFFSET_PIXEL(dib, sx, sy);
		
		for (y = 0; y < h; y++) {
			yl = sdata + y * dib->bytes_per_line;
			for (x = 0; x < w; x++) {
				if (*yl >= src && *yl < src_last) *yl += dif;
				yl++;
			}
		}
	}
}

void* ags_saveRegion(int x, int y, int w, int h) {
	if (!check_param(&x, &y, &w, &h)) return NULL;

	DspDeviceSync();
	return (void *)SaveRegion(x, y, w, h);
}

void ags_restoreRegion(void *region, int x, int y) {
	if (region == NULL) return;
	
	if (!check_param_xy(&x, &y)) return;
	
	DspDeviceSync();
	RestoreRegion(region, x, y);
}

void ags_putRegion(void *region, int x, int y) {
	if (region == NULL) return;
	
	if (!check_param_xy(&x, &y)) return;
	
	DspDeviceSync();
	PutRegion(region, x, y);
}

void ags_copyRegion(void *region, int sx, int sy , int w, int h, int dx, int dy) {
	if (region == NULL) return;
	
	if (!check_param_xy(&dx, &dy)) return;
	if (!check_param(&dx, &dy, &w, &h)) return;
	
	DspDeviceSync();
	CopyRegion(region, sx, sy, w, h, dx, dy);
}

void ags_delRegion(void *region) {
	if (region == NULL) return;
	
	DspDeviceSync();
	DelRegion(region);
}

int ags_drawString(int x, int y, char *src, int col) {
	int w;
	
	if (!check_param_xy(&x, &y)) return 0;
	
	DspDeviceSync();
	w = DrawString(x, y, src, col);
	
	return w;
}

void ags_drawCg8bit(cgdata *cg, int x, int y) {
	int sx, sy, w, h;
	
	sx = x;
	sy = y;
	w = cg->width;
	h = cg->height;
	
	if (!check_param(&x, &y, &w, &h)) return;
	
	cg->data_offset = abs(sy - y) * cg->width + abs(sx - x);
	DspDeviceSync();
	DrawImage8_fromData(cg, x, y, w, h);
}

void ags_drawCg16bit(cgdata *cg, int x, int y) {
	int sx, sy, w, h;
	
	sx = x;
	sy = y;
	w = cg->width;
	h = cg->height;

	if (!check_param(&x, &y, &w, &h)) return;
	
	cg->data_offset = abs(sy - y) * cg->width + abs(sx - x);
	DspDeviceSync();
	DrawImage16_fromData(cg, x, y, w, h);
}

void ags_copyArea_shadow(int sx, int sy, int w, int h, int dx, int dy) {
	if (nact->sys_world_depth == 8) return;
	
	if (!check_param(&sx, &sy, &w, &h)) return;
	if (!check_param(&dx, &dy, &w, &h)) return;
	
	DspDeviceSync();
	CopyAreaSP16_shadow(sx, sy, w, h, dx, dy);
}

void ags_copyArea_transparent(int sx, int sy, int w, int h, int dx, int dy, int col) {
	if (nact->sys_world_depth == 8) return;

	if (!check_param(&sx, &sy, &w, &h)) return;
	if (!check_param(&dx, &dy, &w, &h)) return;
	
	DspDeviceSync();
	CopyAreaSP(sx, sy, w, h, dx, dy, col);
}

void ags_copyArea_alphaLevel(int sx, int sy, int w, int h, int dx, int dy, int lv) {
	if (nact->sys_world_depth == 8) return;

	if (!check_param(&sx, &sy, &w, &h)) return;
	if (!check_param(&dx, &dy, &w, &h)) return;
	
	DspDeviceSync();
	CopyAreaSP16_alphaLevel(sx, sy, w, h, dx, dy, lv);
}

void ags_copyArea_alphaBlend(int sx, int sy, int w, int h, int dx, int dy, int lv) {
	if (nact->sys_world_depth == 8) return;

	if (!check_param(&sx, &sy, &w, &h)) return;
	if (!check_param(&dx, &dy, &w, &h)) return;
	
	DspDeviceSync();
	CopyAreaSP16_alphaBlend(sx, sy, w, h, dx, dy, lv);
}

void ags_copyArea_whiteLevel(int sx, int sy, int w, int h, int dx, int dy, int lv) {
	if (nact->sys_world_depth == 8) return;

	if (!check_param(&sx, &sy, &w, &h)) return;
	if (!check_param(&dx, &dy, &w, &h)) return;
	
	DspDeviceSync();
	CopyAreaSP16_whiteLevel(sx, sy, w, h, dx, dy, lv);
}


/*******************************************************
 *
 * special thanks to tajiri@wizard.elec.waseda.ac.jpさん
 *
 *******************************************************/
/* CP コマンドの実装用. 同じ色で出来た領域を指定された
   色で塗り変える。
*/
static int floodColor;
static int changeColor;
static agsurface_t *__img;
/*この操作のあとにアップデートする領域
  (updatePointTop と updatePointEndで囲まれた長方形)
 */
static MyPoint updatePointTop, updatePointEnd;

static int pixcel(int x, int y) {
	int pixval;
	
	if ((y >= 0) && (y <= __img->height) && (x >= 0) && (x <= __img->width)) {
		BYTE *dst = (BYTE *)(__img->pixel + y * __img->bytes_per_line + x);
		pixval = *dst;
		
		if (pixval == floodColor){
		/* if(pixval <= floodColor+2 && pixval >= floodColor-2){ */
			*dst = changeColor;
			if (x < updatePointTop.x) updatePointTop.x = x;
			if (x > updatePointEnd.x) updatePointEnd.x = x;
			if (y < updatePointTop.y) updatePointTop.y = y;
			if (y > updatePointEnd.y) updatePointEnd.y = y;
			return TRUE;
		}
	}
	return FALSE;
}

MyRectangle* ags_imageFlood(int x, int y, int c) {
	if (nact->sys_world_depth != 8) return NULL;
	
	if (!check_param_xy(&x, &y)) return NULL;

	DspDeviceSync();

{
	agsurface_t *dib = nact->ags.dib;
	BYTE *dst = GETOFFSET_PIXEL(dib, x, y);
	static MyRectangle rec;
	__img = dib;
	updatePointTop.x = x;
	updatePointTop.y = y;
	updatePointEnd.x = x;
	updatePointEnd.y = y;
	/*直線はぬりなおしたりしない！！*/
	if ((x <= 0 || (*(dst - 1) != *(dst))) && ((x >= dib->width) || (*(dst + 1) != *dst)))
		return NULL;
	if ((y <= 0 || (*(dst - dib->bytes_per_line) != *(dst)))
	    && ((y >= dib->height) || (*(dst + dib->bytes_per_line) != *dst)))
		return NULL;
	floodColor = *dst;
	
	changeColor = c;
	flood(x, y, pixcel);
	rec.x = updatePointTop.x;
	rec.y = updatePointTop.y;
	rec.width =  updatePointEnd.x - updatePointTop.x + 1;
	rec.height = updatePointEnd.y - updatePointTop.y + 1;
	return &rec;
}
}

void ags_copyFromAlpha(int sx, int sy, int w, int h, int dx, int dy, ALPHA_DIB_COPY_TYPE flg) {
	if (nact->sys_world_depth == 8) return;

	if (!check_param(&sx, &sy, &w, &h)) return;
	if (!check_param(&dx, &dy, &w, &h)) return;
	
	DspDeviceSync();
	Copy_from_alpha(sx, sy, w, h, dx, dy, flg);
}

void ags_copyToAlpha(int sx, int sy, int w, int h, int dx, int dy, ALPHA_DIB_COPY_TYPE flg) {
	if (nact->sys_world_depth == 8) return;

	if (!check_param(&sx, &sy, &w, &h)) return;
	if (!check_param(&dx, &dy, &w, &h)) return;
	
	DspDeviceSync();
	Copy_to_alpha(sx, sy, w, h, dx, dy, flg);
}

void ags_alpha_uppercut(int sx, int sy, int w, int h, int s, int d) {
	if (nact->sys_world_depth == 8) return;
	
	if (!check_param(&sx, &sy, &w, &h)) return;
	
	alpha_uppercut(nact->ags.dib, sx, sy, w, h, s, d);
}

void ags_alpha_lowercut(int sx, int sy, int w, int h, int s, int d) {
	if (nact->sys_world_depth == 8) return;
	
	if (!check_param(&sx, &sy, &w, &h)) return;

	alpha_lowercut(nact->ags.dib, sx, sy, w, h, s, d);
}

void ags_alpha_setLevel(int x, int y, int w, int h, int lv) {
	if (nact->sys_world_depth == 8) return;
	
	if (!check_param(&x, &y, &w, &h)) return;

	alpha_set_level(nact->ags.dib, x, y, w, h, lv);
}

void ags_alpha_copyArea(int sx, int sy, int w, int h, int dx, int dy) {
	if (nact->sys_world_depth == 8) return;
	
	if (!check_param(&sx, &sy, &w, &h)) return;
	if (!check_param(&dx, &dy, &w, &h)) return;
	
	alpha_copy_area(nact->ags.dib, sx, sy, w, h, dx, dy);
}

void ags_alpha_getPixel(int x, int y, int *pic) {
	if (nact->sys_world_depth == 8) return;
	
	if (!check_param_xy(&x, &y)) {
		*pic = 0;
	} else {
		alpha_get_pixel(nact->ags.dib, x, y, (BYTE *)pic);
	}
}

void ags_alpha_setPixel(int x, int y, int w, int h, BYTE *b) {
	int savex, savey, savew, saveh, offset;
	
	savex = x;
	savey = y;
	savew = w;
	saveh = h;
	
	if (!check_param(&x, &y, &w, &h)) return;	
	
	offset = abs(savey - y) * savew + abs(savex - x);
	
	alpha_set_pixels(nact->ags.dib, x, y, w, h, b + offset, savew);
}

/*
 * fade in/out の wait 制御
 */
void ags_fader(ags_faderinfo_t *i) {
	int cnt_st, step, key = 0, canceled_key = 0;
	cnt_st = get_high_counter(SYSTEMCOUNTER_MSEC);
	
	i->callback(0);
	
	if (i->effect_time == 0) goto out;
	
	step = 1;
	while(step < i->step_max) {
		int lefttime, leftstep, mstime, cnt1, cnt2;
		
		cnt1 = get_high_counter(SYSTEMCOUNTER_MSEC);
		i->callback(step);
		key = sys_getInputInfo();
		/* 実際の fade にかかった時間 */
		usleep(0); /* It's a magic !!! */
		cnt2 = get_high_counter(SYSTEMCOUNTER_MSEC) - cnt1;
		
		lefttime = i->effect_time - (cnt1 + cnt2 - cnt_st); /* fade 残り時間 */
		leftstep = i->step_max - step;  /* fade 残りステップ数 */
		
		if (lefttime <= 0) break;  /* 時間切れ */
		if (leftstep <= 0) break;
		
		mstime = lefttime / leftstep; /* 1stepに許される時間 */
		if (mstime > cnt2) {
			/* wait をいれる余裕がある場合 */
			key = sys_keywait(mstime - cnt2, i->cancel);
			step++;
		} else if (mstime > 0) {
			/* wait をいれる余裕が無い場合 */
			step += ((cnt2+1) * leftstep / lefttime);
			nact->callback();
		} else {
			break;
		}
		/* wait cancel が有効の場合 */
		if (i->cancel) {
			if (key != 0) {
				canceled_key = key;
				break;
			}
		}
	}
 out:
	/* fader last step */
	i->callback(i->step_max);
	
	/* store canceled key */
	nact->waitcancel_key = canceled_key;
}

void ags_fadeIn(int rate, boolean flag) {
	ags_faderinfo_t i;

	if (need_update) {
		i.effect_time = (rate * 16 * 1000) / 60;
		i.cancel = flag;
	} else {
		i.effect_time = 0;
	}
	fade_outed = FALSE;

	nact->waitcancel_key = 0;

	i.callback = FadeIn;
	i.step_max = 255;
	ags_fader(&i);
}

void ags_fadeOut(int rate, boolean flag) {
	ags_faderinfo_t i;
	
	if (need_update && !fade_outed) {
		i.effect_time = (rate * 16 * 1000) / 60;
		i.cancel = flag;
	} else {
		i.effect_time = 0;
	}
	fade_outed = TRUE;

	nact->waitcancel_key = 0;
	
	i.callback = FadeOut;
	i.step_max = 255;
	ags_fader(&i);
}

void ags_whiteIn(int rate, boolean flag) {	
	ags_faderinfo_t i;
	if (need_update) {
		i.effect_time = (rate * 16 * 1000) / 60;
		i.cancel = flag;
	} else {
		i.effect_time = 0;
	}
	fade_outed = FALSE;

	nact->waitcancel_key = 0;

	i.callback = WhiteIn;
	i.step_max = 255;
	ags_fader(&i);
}

void ags_whiteOut(int rate, boolean flag) {
	ags_faderinfo_t i;
	if (need_update && !fade_outed) {
		i.effect_time = (rate * 16 * 1000) / 60;
		i.cancel = flag;
	} else {
		i.effect_time = 0;
	}		
	fade_outed = TRUE;
	
	nact->waitcancel_key = 0;
	
	i.callback = WhiteOut;
	i.step_max = 255;
	ags_fader(&i);
}

void ags_setFont(int type, int size) {
	nact->ags.font->sel_font(type, size);
}

void ags_setCursorType(int type) {
	if (nact->noimagecursor && type >= 100) return;
	SetCursorType(type);
}

void ags_loadCursor(int p1,int p2) {
	if (!nact->noimagecursor) {
		cursor_load(p1, p2);
	}
}

void ags_setCursorLocation(int x, int y, boolean is_dibgeo) {
	int dx[8], dy[8];
	int i, delx, dely;
	MyPoint p;
	if (!check_param_xy(&x, &y)) return;

	/* DIB 座表系か Window 座表系か */
	if (is_dibgeo) {
		x -= nact->sys_view_area.x;
		y -= nact->sys_view_area.y;
	}
	
	switch(nact->sys_mouse_movesw) {
	case 0:
		return;
	case 1:
		SetCursorLocation(x, y); break;
	case 2:
		sys_getMouseInfo(&p, is_dibgeo);
		delx = x - p.x;
		dely = y - p.y;
		
		for (i = 1; i < 8; i++) {
			dx[i-1] = ((delx*i*i*i) >> 9) - ((3*delx*i*i)>> 6) + ((3*delx*i) >> 3) + p.x;
			dy[i-1] = ((dely*i*i*i) >> 9) - ((3*dely*i*i)>> 6) + ((3*dely*i) >> 3) + p.y;
		}
		dx[7] = x; dy[7] = y;
		
		for (i = 0; i < 8; i++) {
			SetCursorLocation(dx[i], dy[i]);
			usleep(cursor_move_time * 1000 / 8);
		}
		break;
	default:
		return;
	}
}

void ags_setAntialiasedStringMode(boolean on) {
	if (!nact->noantialias) {
		nact->ags.font->antialiase_on = on;
	}
}

boolean ags_getAntialiasedStringMode() {
	return nact->ags.font->antialiase_on;
}

void ags_fullscreen(boolean on) {
	nact->sys_fullscreen_on = on;
	FullScreen(on);
}

void ags_copyArea_shadow_withrate(int sx, int sy, int w, int h, int dx, int dy, int lv) {
	if (nact->sys_world_depth == 8) return;
	
	if (lv == 0) return;
	
	if (!check_param(&sx, &sy, &w, &h)) return;
	if (!check_param(&dx, &dy, &w, &h)) return;
	
	DspDeviceSync();
	CopyAreaSP16_shadow_withRate(sx, sy, w, h, dx, dy, lv);
} 

void ags_setCursorMoveTime(int msec) {
	 cursor_move_time = msec;
}

int ags_getCursorMoveTime() {
	 return cursor_move_time;
}

/*
 * 指定の領域に全画面をZoom
 * 
 */
void ags_zoom(int x, int y, int w, int h) {
	if (!check_param(&x, &y, &w, &h)) return;

	DspDeviceSync();
	Zoom(x, y, w, h);
}

agsurface_t *ags_getDIB() {
	return nact->ags.dib;
}

void ags_sync() {
	DspDeviceSync();
}

void ags_fillRectangleNeg(int x, int y, int w, int h, int col) {
	if (!check_param(&x, &y, &w, &h)) return;
	
	DspDeviceSync();
	image_fillRectangleNeg(nact->ags.dib, x, y, w, h, col);
}

void ags_autorepeat(boolean bool) {
	SetAutoRepeat(bool);
}
