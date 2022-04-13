/*
 * ecopy.c  copyarea with effect
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
/* $Id: ecopy.c,v 1.20 2001/08/11 20:05:55 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <math.h>

#include "portab.h"
#include "sdl_core.h"
#include "input.h"
#include "ags.h"
#include "system.h"
#include "nact.h"
#include "effect.h"

static boolean ecp_cancel;

#define EC_WAIT \
	if ((key |= sys_getInputInfo()) && ecp_cancel) break; \
	do {												  \
		int wait_ms = cnt - sdl_getTicks();				  \
		if (wait_ms >= 16)								  \
			key = sys_keywait(wait_ms, ecp_cancel ? KEYWAIT_CANCELABLE : KEYWAIT_NONCANCELABLE); \
	} while (0)


static void eCopyUpdateArea(int sx, int sy, int w, int h, int dx, int dy) {
	MyRectangle src = {sx, sy, w, h};
	MyPoint dst = {
		dx - nact->ags.view_area.x,
		dy - nact->ags.view_area.y
	};
	sdl_updateArea(&src, &dst);
}

static int eCopyArea1(int dx, int dy, int w, int h, int opt) {
	int y, key = 0, cnt;
	int waitcnt = opt == 0 ? 20 : opt;
	
	cnt = sdl_getTicks();
	for (y = 0; y < h - 24; y += 24) {
		cnt += waitcnt;
		eCopyUpdateArea(dx, dy + h - y, w, y, dx, dy);
		EC_WAIT;
	}

	eCopyUpdateArea(dx, dy, w, h, dx, dy);
	return key;
}

static int eCopyArea2(int dx, int dy, int w, int h, int opt) {
	int y, key = 0, cnt;
	int waitcnt = opt == 0 ? 20 : opt;
	
	cnt = sdl_getTicks();
	for (y = 0; y < h - 24; y += 24) {
		cnt += waitcnt;
		eCopyUpdateArea(dx, dy , w, y, dx, dy + h - y);
		EC_WAIT;
	}
	
	eCopyUpdateArea(dx, dy, w, h, dx, dy);
	return key;
}

static int eCopyArea3(int dx, int dy, int w, int h, int opt) {
	int y1, y2, key = 0, cnt;
	int waitcnt = opt == 0 ? 3 : opt;
	
	if (h % 2) {
		h--;
		eCopyUpdateArea(dx , dy + h, w, 1, dx, dy + h);
	}
	y1 = dy;
	y2 = dy + h - 1;
	cnt = sdl_getTicks();
	while (h > 0) {
		cnt += waitcnt;
		eCopyUpdateArea(dx, y1, w, 1, dx, y1);
		eCopyUpdateArea(dx, y2, w, 1, dx, y2);
		y1 += 2;
		y2 -= 2;
		h -= 2;
		EC_WAIT;
	}
	eCopyUpdateArea(dx, dy, w, h, dx, dy);
	return key;
}

static int eCopyArea4(int dx, int dy, int w, int h, int opt) {
	int x1,x2, key = 0, cnt;
	int waitcnt = opt == 0 ? 3 : opt;
	
	if (w % 2) {
		w--;
		eCopyUpdateArea(dx + w , dy, 1, h, dx + w, dy);
	}
	x1 = dx;
	x2 = dx + w - 1;
	cnt = sdl_getTicks();
	while (w > 0) {
		cnt += waitcnt;
		eCopyUpdateArea(x1, dy, 1, h, x1, dy);
		eCopyUpdateArea(x2, dy, 1, h, x2, dy);
		x1 += 2;
		x2 -= 2;
		w -= 2;
		EC_WAIT;
	}
	eCopyUpdateArea(dx, dy, w, h, dx, dy);
	return key;
}

static int eCopyArea5(int sx, int sy, int w, int h, int dx, int dy, int opt) {
	int i, x, y, key = 0, cnt;
	int waitcnt = opt == 0 ? 20 : opt;
	static int hx[16]={ 0,32,16,48, 0,32,16,48,16,48, 0,32,16,48, 0,32};
	static int hy[16]={ 0, 0,16,16,32,32,48,48, 0, 0,16,16,32,32,48,48};
	
	cnt = sdl_getTicks();
	for (i = 0; i < 16; i++) {
		cnt += waitcnt;
		for (y = 0; y < (h -63); y += 64) {
			for (x = 0; x < (w -63); x += 64) {
				ags_copyArea(sx + x + hx[i], sy + y + hy[i], 16, 16, 
					     dx + x + hx[i], dy + y + hy[i]);
			}
		}
		ags_updateArea(dx, dy, w, h);
		EC_WAIT;
	}
	ags_copyArea(sx, sy, w, h, dx, dy);
	ags_updateArea(dx, dy, w, h);
	return key;
}

static int eCopyArea6(int dx, int dy, int w, int h, int opt) {
	int i, x, y, key = 0, cnt;
	int waitcnt = opt == 0 ? 30 : opt;

	cnt = sdl_getTicks();
	for (i = 0; i < 7; i++) {
		cnt += waitcnt;
		for (x = 0; x < (w -63); x += 64) {
			eCopyUpdateArea(dx+x+i*4,     dy, 4, h, dx+x+i*4, dy);
			eCopyUpdateArea(dx+x+(60-i*4),dy, 4, h, dx+x+(60-i*4), dy);
		}
		for (y = 0; y < (h -63); y += 64) {
			eCopyUpdateArea(dx+4, dy+y+i*4,      w-8, 4, dx+4, dy+y+i*4);
			eCopyUpdateArea(dx+4, dy+y+(60-i*4), w-8, 4, dx+4, dy+y+(60-i*4));
		}
		EC_WAIT;
	}
	eCopyUpdateArea(dx, dy, w, h, dx, dy);
	return key;
}

static int eCopyArea9(int sx, int sy, int w, int h, int dx, int dy, int opt) {
	int i, x, y, key = 0, cnt;
	int waitcnt = opt == 0 ? 80 : opt;
	static int hintX[4] = {0,8,0,8};
	static int hintY[4] = {0,8,8,0};

	
	cnt = sdl_getTicks();
	for (i = 0; i < 4; i++) {
		cnt+=waitcnt;
		for (y = 0; y < h -15; y+=16) {
			for (x = 0; x < (w -7); x+=16) {
				sdl_copyArea(sx + x + hintX[i],sy + y + hintY[i], 8, 8, dx + x + hintX[i], dy + y + hintY[i]);
			}
		}
		ags_updateArea(dx, dy, w, h);
		EC_WAIT;
	}
	sdl_copyArea(sx, sy, w, h, dx, dy);
	ags_updateArea(dx, dy, w, h);
	return key;
}

static int eCopyArea16(int dx, int dy, int w, int h, int opt) {
	int i, x, key = 0, cnt;
	int waitcnt = opt == 0 ? 30 : opt;
	
	cnt = sdl_getTicks();
	for (i = 0; i < 8; i++) {
		cnt += waitcnt;
		for (x = 0; x < (w -15); x += 16) {
			eCopyUpdateArea(dx + x + (7-i), dy, 1, h, dx + x + (7-i), dy);
			eCopyUpdateArea(dx + x + (8+i), dy, 1, h, dx + x + (8+i), dy);
		}
		EC_WAIT;
	}
	eCopyUpdateArea(dx, dy, w, h, dx, dy);
	return key;
}

static int eCopyArea17(int dx, int dy, int w, int h, int opt) {
	int i, y, key = 0, cnt;
	int waitcnt = opt == 0 ? 30 : opt;

#define E17X 18
	cnt = sdl_getTicks();
	for (i = 0; i < E17X; i++) {
		cnt += waitcnt;
		for (y = 0; y < (h - E17X + 1); y += E17X) {
			eCopyUpdateArea(dx, dy + y + i, w, 1, dx, dy + y + i);
		}
		EC_WAIT;
	}
	eCopyUpdateArea(dx, dy, w, h, dx, dy);
	return key;
}

static int eCopyArea22(int sx, int sy, int w, int h, int dx, int dy, int opt) {
	int i, x, y, key = 0, cnt;
	int waitcnt = opt == 0 ? 80 : opt;
	
	cnt = sdl_getTicks();
	for (i = 0; i < 2; i++) {
		cnt += waitcnt;
		for (y = 0; y < (h -3); y+=4) {
			for (x = 0; x < (w -3); x+=4) {
				ags_copyArea(sx + x + (i == 0 ? 0 : 2), sy + y + (i == 0 ? 0 : 2), 2, 2,
					     dx + x + (i == 0 ? 0 : 2), dy + y + (i == 0 ? 0 : 2));
				ags_copyArea(sx + x + (i == 0 ? 2 : 0), sy + y + (i == 0 ? 0 : 2), 2, 2,
					     dx + x + (i == 0 ? 2 : 0), dy + y + (i == 0 ? 0 : 2));
			}
		}
		ags_updateArea(dx, dy, w, h);
		EC_WAIT;
	}
	ags_copyArea(sx, sy, w, h, dx, dy);
	ags_updateArea(dx, dy, w, h);
	return key;
}

static int eCopyArea23(int sx, int sy, int w, int h, int dx, int dy, int opt) {
	int i, x, y, key = 0, cnt;
	int waitcnt = opt == 0 ? 80 : opt;
	static int hintX[4] = {0,2,2,0};
	static int hintY[4] = {0,2,0,2};
	
	cnt = sdl_getTicks();
	for (i = 0; i < 4; i++) {
		cnt += waitcnt;
		for (y = 0; y < (h -3); y+=4) {
			for (x = 0; x < (w -3); x+=4) {
				sdl_copyArea(sx + x + hintX[i], sy + y + hintY[i], 2, 2,
					 dx + x + hintX[i], dy + y + hintY[i]);
			}
		}
		ags_updateArea(dx, dy, w, h);
		EC_WAIT;
	}
	ags_copyArea(sx, sy, w, h, dx, dy);
	ags_updateArea(dx, dy, w, h);
	return key;
}

#define SCA25_6_SLICE 10
static int eCopyArea25(int sx, int sy, int w, int h, int dx, int dy, int opt) {
	int r, rr, x, xx, y, yy, key = 0, cnt;
	int waitcnt = opt == 0 ? 20 : opt;
	int w2=w/2,h2=h/2,h1=h-1,mr=(int)(sqrt(w2*w2+h2*h2));
	int ux=0,uy=0,ux_y=h2-1,uw,uh;

	cnt = sdl_getTicks();
	for (r = SCA25_6_SLICE; r < mr; r += SCA25_6_SLICE ) {
		cnt += waitcnt;
		rr=r*r;
		uw=uh=0;
		for (y = 0; y < h2; y++) {
			yy=h2-y;
			xx=rr-yy*yy;
			if (xx > 0) {
				x=(int)(sqrt(xx));
				x=(w2>x)?x:w2;
				xx=w2-x;
				ux=dx+xx;
				uw=x*2;
				if (uh == 0) {
					uh=(h2-y)*2;
					uy=dy+y;
				}
				if (xx == 0) {
					ags_copyArea(sx+xx, sy+y,       uw, ux_y-y+1, ux, dy+y);
					ags_copyArea(sx+xx, sy+h1-ux_y, uw, ux_y-y+1, ux, dy+h1-ux_y);
					ux_y=y;
					break;
				} 
				ags_copyArea(sx+xx, sy+y   , uw, 1, ux, dy+y   );
				ags_copyArea(sx+xx, sy+h1-y, uw, 1, ux, dy+h1-y);
			}
		}
		ags_updateArea(ux,uy,uw,uh);
		EC_WAIT;
	}

	ags_copyArea(sx,sy,w,h,dx,dy);
	ags_updateArea(dx, dy, w, h);
	return key;
}

static int eCopyArea26(int sx, int sy, int w, int h, int dx, int dy, int opt) {
	int r, rr, x, xx=0, y, yy, key = 0, cnt;
	int waitcnt = opt == 0 ? 20 : opt;
	int w2=w/2,w1=w-1,h2=h/2,h1=h-1,mr=(int)(sqrt(w2*w2+h2*h2));
	int ux=0,uy=0,ux_y=0,uw,uh;

	cnt = sdl_getTicks();
	for (r = mr - SCA25_6_SLICE; r > 0; r -= SCA25_6_SLICE) {
		cnt += waitcnt;
		rr=r*r;
		uy=ux_y;
		uw=uh=0;
		for (y = ux_y; y < h2; y++) {
			yy=h2-y;
			xx=rr-yy*yy;
			if (xx < 0) {
				uy++;
				continue;
			}
			x=(int)(sqrt(xx));
			x=(w2>x)?x:w2;
			xx=w2-x;
			if (xx == 0) break;
			if ((xx-ux) > uw) uw=xx-ux;
			uh++;
			ags_copyArea(sx+ux,    sy+y, xx-ux, 1, dx+ux,    dy+y);
			ags_copyArea(sx+w1-xx, sy+y, xx-ux, 1, dx+w1-xx, dy+y);
			ags_copyArea(sx+ux,    sy+h1-y, xx-ux, 1, dx+ux,    dy+h1-y);
			ags_copyArea(sx+w1-xx, sy+h1-y, xx-ux, 1, dx+w1-xx, dy+h1-y);
		}
		if (uy != ux_y) {
			ags_copyArea(sx, sy+ux_y,w,uy-ux_y,dx,dy+ux_y);
			ags_updateArea(dx,dy+ux_y,w,uy-ux_y);
			ags_copyArea(sx, sy+h-uy,w,uy-ux_y,dx,dy+h-uy);
			ags_updateArea(dx,dy+h-uy,w,uy-ux_y);
			ux_y=uy;
		}
		ags_updateArea(dx+ux,       dy+uy, uw, uh);
		ags_updateArea(dx+w1-ux-uw, dy+uy, uw, uh);
		ags_updateArea(dx+ux,       dy+h-uy-uh, uw, uh);
		ags_updateArea(dx+w1-ux-uw, dy+h-uy-uh, uw, uh);
		ux=xx;
		EC_WAIT;
	}

	ags_copyArea(sx,sy,w,h,dx,dy);
	ags_updateArea(dx, dy, w, h);
	return key;
}

static int eCopyArea1000(int sx, int sy, int w, int h, int dx, int dy, int opt, int spCol) {
	/* XOR */
	return sys_getInputInfo();
}

static int eCopyArea1001(int sx, int sy, int w, int h, int dx, int dy, int opt, int spCol) {
	/* パレットシフト */
	return sys_getInputInfo();
}

static int eCopyArea2000(int sx, int sy, int w, int h, int dx, int dy, int opt) {
	sdl_fillRectangleRGB(dx, dy, w, h, 0, 0, 0);
	ags_copyArea_alphaLevel(sx, sy, w, h, dx, dy, opt);
	ags_updateArea(dx, dy, w, h);
	return sys_getInputInfo();
}

static int eCopyArea2001(int sx, int sy, int w, int h, int dx, int dy, int opt) {
	ags_copyArea_alphaBlend(sx, sy, w, h, dx, dy, opt);
	ags_updateArea(dx, dy, w, h);
	return sys_getInputInfo();
}

static int eCopyArea5sp(int sx, int sy, int w, int h, int dx, int dy, int opt) {
	int i, j, k, key = 0, cnt;
	int st_i, ed_i;
	int waitcnt = opt == 0 ? 50 : opt * 10;
	int step[200];
	void *save = ags_saveRegion(dx - 50, dy, w + 100, h);
	
	for (i = 0; i < 200; i++) {
		step[i] = 50 * sin((2 * M_PI * i) / 50);
	}
	
	cnt = sdl_getTicks();
	
	for (i = 150; i < h + 200; i++) {
		cnt += waitcnt;
		st_i = max(0, i - 200);
		ed_i = h;
		ags_copyRegion(save, 0, st_i, w + 100, ed_i - st_i, dx - 50, dy + st_i);
		for (j = st_i, k = 0; j < ed_i; j++, k++) {
			ags_copyArea_shadow(sx , sy + j, w, 1,
					    dx + step[(k + max(0, 200 - i)) % 200],
					    dy + j);
		}
		ags_updateArea(dx - 50, dy + st_i, w + 100, ed_i - st_i);
		EC_WAIT;
	}
	
	ags_delRegion(save);
	
	return key;
}

static int duration(enum nact_effect effect, int opt, SDL_Rect *rect) {
	switch (effect) {
	case NACT_EFFECT_ZOOM_IN:
		return opt ? opt * 64 : 500;
	case NACT_EFFECT_BLIND_DOWN:
	case NACT_EFFECT_BLIND_UP:
		return (opt ? opt : 40) * (rect->h / 16 + 16);
	case NACT_EFFECT_BLIND_UP_DOWN:
		return (opt ? opt : 40) * (rect->h / 16 + 16) / 2;
	case NACT_EFFECT_BLIND_LR:
	case NACT_EFFECT_BLIND_RL:
		return (opt ? opt : 40) * (rect->w / 16 + 16);
	case NACT_EFFECT_WIPE_IN:
	case NACT_EFFECT_WIPE_OUT:
		return (opt ? opt : 40) * (rect->w / 48);
	case NACT_EFFECT_WIPE_LR:
	case NACT_EFFECT_WIPE_RL:
		return (opt ? opt : 20) * rect->w / 8;
	case NACT_EFFECT_WIPE_DOWN:
	case NACT_EFFECT_WIPE_UP:
		return (opt ? opt : 20) * rect->h / 8;
	case NACT_EFFECT_WIPE_OUT_V:
	case NACT_EFFECT_WIPE_IN_V:
		return (opt ? opt : 30) * rect->w / 16;
	case NACT_EFFECT_WIPE_OUT_H:
	case NACT_EFFECT_WIPE_IN_H:
		return (opt ? opt : 30) * rect->h / 16;
	case NACT_EFFECT_MOSAIC:
		return (opt ? opt : 100) * 8;
	case NACT_EFFECT_FADEIN:
	case NACT_EFFECT_WHITEIN:
	case NACT_EFFECT_FADEOUT:
	case NACT_EFFECT_WHITEOUT:
		return opt ? opt * 32 : 1700;
	case NACT_EFFECT_CROSSFADE:
		return opt ? opt * 256 : 2700;
	case NACT_EFFECT_CROSSFADE_MOSAIC:
		return opt ? opt * 256 : 1400;
	case NACT_EFFECT_CROSSFADE_DOWN:
	case NACT_EFFECT_CROSSFADE_UP:
		return opt ? opt * (rect->h + 256) : 1150;
	case NACT_EFFECT_CROSSFADE_LR:
	case NACT_EFFECT_CROSSFADE_RL:
		return opt ? opt * (rect->w + 256) : 1150;
	case NACT_EFFECT_BLEND_UP_DOWN:
		return (opt ? opt : 3) * rect->h;
	case NACT_EFFECT_BLEND_LR_RL:
		return (opt ? opt : 3) * rect->w;
	case NACT_EFFECT_CROSSFADE_LR_RL:
		return opt ? opt * (127 + rect->w / 2) : 1300;
	case NACT_EFFECT_CROSSFADE_UP_DOWN:
		return opt ? opt * (127 + rect->h / 2) : 1300;
	case NACT_EFFECT_MAGNIFY:
		return opt ? opt + 300 : 2000;
	case NACT_EFFECT_PENTAGRAM_IN_OUT:
	case NACT_EFFECT_PENTAGRAM_OUT_IN:
	case NACT_EFFECT_HEXAGRAM_IN_OUT:
	case NACT_EFFECT_HEXAGRAM_OUT_IN:
	case NACT_EFFECT_WINDMILL:
	case NACT_EFFECT_WINDMILL_180:
	case NACT_EFFECT_WINDMILL_360:
		 return opt ? opt : 1000;
	case NACT_EFFECT_LINEAR_BLUR:
		return opt ? opt : 1700;
	}
	return 1000;
}

void ags_eCopyArea(int sx, int sy, int w, int h, int dx, int dy, int sw, int opt, boolean cancel, int spCol) {
	int ret = 0;

#if 0
	NOTICE("ec_area sx %d sy %d w %d h %d dx %d dy %d sw %d opt %d spc %d cancel %s\n",
	       sx, sy, w, h, dx, dy, sw, opt, spCol, cancel ? "True" : "False");
#endif
	if (!ags_check_param(&sx, &sy, &w, &h)) return;
	if (!ags_check_param(&dx, &dy, &w, &h)) return;
	
	nact->waitcancel_key = 0;

	if (sw == NACT_EFFECT_MAGNIFY) {
		SDL_Rect target_rect = { sx, sy, w, h };
		struct sdl_effect *eff = sdl_effect_magnify_init(sdl_getDIB(), &nact->ags.view_area, &target_rect);
		ags_runEffect(duration(sw, opt, NULL), cancel, (ags_EffectStepFunc)sdl_effect_step, eff);
		sdl_effect_finish(eff);
		// Actual copy.
		ags_scaledCopyArea(
			sx, sy, w, h,
			nact->ags.view_area.x, nact->ags.view_area.y, nact->ags.view_area.w, nact->ags.view_area.h, 0);
		ags_updateFull();
		return;
	}

	enum sdl_effect_type sdl_effect = from_nact_effect(sw);
	if (sdl_effect != EFFECT_INVALID) {
		SDL_Rect rect = { dx - nact->ags.view_area.x, dy - nact->ags.view_area.y, w, h };
		struct sdl_effect *eff = sdl_effect_init(&rect, sdl_getDIB(), dx, dy, sdl_getDIB(), sx, sy, from_nact_effect(sw));
		ags_runEffect(duration(sw, opt, &rect), cancel, (ags_EffectStepFunc)sdl_effect_step, eff);
		sdl_effect_finish(eff);

		// Actual copy.
		switch (sw) {
		case NACT_EFFECT_FADEOUT:
			sdl_fillRectangleRGB(dx, dy, w, h, 0, 0, 0);
			ags_updateArea(dx, dy, w, h);
			break;
		case NACT_EFFECT_WHITEOUT:
			sdl_fillRectangleRGB(dx, dy, w, h, 255, 255, 255);
			ags_updateArea(dx, dy, w, h);
			break;
		default:
			ags_copyArea(sx, sy, w, h, dx, dy);
			ags_updateArea(dx, dy, w, h);
			break;
		}
		return;
	}

	ecp_cancel = cancel;
	
	switch(sw) {
	case 1:
	case 2:
	case 3:
	case 4:
	case 6:
	case 16:
	case 17:
	case 54:
		if (spCol == -1) {
			ags_copyArea(sx, sy, w, h, dx, dy);
		} else {
			ags_copyAreaSP(sx, sy, w, h, dx, dy, spCol);
		}
		{
			MyRectangle r = {dx, dy, w, h}, update;
			SDL_IntersectRect(&nact->ags.view_area, &r, &update);
			w = update.w;
			h = update.h;
		}
	}
	
	switch(sw) {
	case 1:
		ret = eCopyArea1(dx, dy, w, h, opt);
		break;
	case 2:
		ret = eCopyArea2(dx, dy, w, h, opt);
		break;
	case 3:
		ret = eCopyArea3(dx, dy, w, h, opt);
		break;
	case 4:
		ret = eCopyArea4(dx, dy, w, h, opt);
		break;
	case 5:
		if (spCol == -1) {
			ret = eCopyArea5(sx, sy, w, h, dx, dy, opt);
		} else {
			ret = eCopyArea5sp(sx, sy, w, h, dx, dy, opt);
		}
		break;
	case 6:
		ret = eCopyArea6(dx, dy, w, h, opt);
		break;
	case 9:
		ret = eCopyArea9(sx, sy, w, h, dx, dy, opt);
		break;
	case 16:
		ret = eCopyArea16(dx, dy, w, h, opt);
		break;
	case 17:
		ret = eCopyArea17(dx, dy, w, h, opt);
		break;
	case 22:
		ret = eCopyArea22(sx, sy, w, h, dx, dy, opt);
		break;
	case 23:
		ret = eCopyArea23(sx, sy, w, h, dx, dy, opt);
		break;
	case 25:
		ret = eCopyArea25(sx, sy, w, h, dx, dy, opt);
		break;
	case 26:
		ret = eCopyArea26(sx, sy, w, h, dx, dy, opt);
		break;
		
	case 1000:
		if (nact->ags.world_depth != 8) return;
		ret = eCopyArea1000(sx, sy, w, h, dx, dy, opt, spCol);
		break;
	case 1001:
		if (nact->ags.world_depth != 8) return;
		ret = eCopyArea1001(sx, sy, w, h, dx, dy, opt, spCol);
		break;
	case 2000:
		if (nact->ags.world_depth == 8) return;
		ret = eCopyArea2000(sx, sy, w, h, dx, dy, opt);
		break;
	case 2001:
		if (nact->ags.world_depth == 8) return;
		ret = eCopyArea2001(sx, sy, w, h, dx, dy, opt);
		break;
	default:
		eCopyUpdateArea(dx, dy, w, h, dx, dy);
		WARNING("effect %d is not presented.\n", sw);
		break;
	}
	nact->waitcancel_key = ret;
}
