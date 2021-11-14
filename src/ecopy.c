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

/* saved parameter */
struct ecopyparam {
	int sx;
	int sy;
	int w;
	int h;
	int dx;
	int dy;
	int sw;
	int opt;
	boolean cancel;
	int spCol;
	int extp[10];
};
typedef struct ecopyparam ecopyparam_t;
static ecopyparam_t ecp;

#define EC_WAIT \
	if ((key |= sys_getInputInfo()) && ecp.cancel) break; \
	do {												  \
		int wait_ms = cnt - sdl_getTicks();				  \
		if (wait_ms >= 16)								  \
			key = sys_keywait(wait_ms, ecp.cancel ? KEYWAIT_CANCELABLE : KEYWAIT_NONCANCELABLE); \
	} while (0)


static void eCopyUpdateArea(int sx, int sy, int w, int h, int dx, int dy) {
	MyRectangle src = {sx, sy, w, h};
	MyPoint dst = {
		dx - nact->sys_view_area.x,
		dy - nact->sys_view_area.y
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

#define E7_8X 24
static int eCopyArea7(int dx, int dy, int w, int h, int opt) {
	int i, key = 0, cnt;
	int waitcnt = opt == 0 ? 40 : opt;

	cnt = sdl_getTicks();
	for (i = 0; i < (w/2 - E7_8X+1); i+=E7_8X) {
		cnt += waitcnt;
		eCopyUpdateArea(dx, dy+i*h/w, w, E7_8X*h/w, dx, dy+i*h/w);
		eCopyUpdateArea(dx, dy+(h-(i+E7_8X)*h/w), w, E7_8X*h/w, dx, dy+(h-(i+E7_8X)*h/w));
		eCopyUpdateArea(dx+i, dy, E7_8X, h, dx+i, dy);
		eCopyUpdateArea(dx+(w-i-E7_8X), dy, E7_8X, h, dx+(w-i-E7_8X), dy);
		EC_WAIT;
	}
	eCopyUpdateArea(dx, dy, w, h, dx, dy);
	return key;
}

static int eCopyArea8(int dx, int dy, int w, int h, int opt) {
	int i, key = 0, cnt;
	int waitcnt = opt == 0 ? 40 : opt;

	cnt = sdl_getTicks();
	for (i = E7_8X; i < (w/2 -E7_8X+1); i+=E7_8X) {
		cnt += waitcnt;
		eCopyUpdateArea(dx+ w/2 - i , dy + h/2 - i*h/w, 2*i, 2*i*h/w ,dx+ w/2-i, dy + h/2 - i*h/w);
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

static void eCopyArea10(int step) {
	if (step == 0) {
		return;
	}
	if (step == 64) {
		sdl_copyArea(ecp.sx, ecp.sy, ecp.w, ecp.h, ecp.dx, ecp.dy);
		ags_updateArea(ecp.dx, ecp.dy, ecp.w, ecp.h);
		return;
	}
	ags_scaledCopyArea(ecp.sx, ecp.sy, ecp.w, ecp.h,
			   ecp.dx + ecp.w * (64 - step) / 128, ecp.dy + ecp.h * (64 - step) / 128,
			   ecp.w * step / 64, ecp.h * step / 64, 0);
	ags_updateArea(ecp.dx + ecp.w * (64 - step) / 128, ecp.dy + ecp.h * (64 - step) / 128,
		       ecp.w * step / 64, ecp.h * step / 64);
}

static int eCopyArea11(int dx, int dy, int w, int h, int opt) {
	int i, j, y, key = 0, cnt;
	int waitcnt = opt == 0 ? 40 : opt;

#define ECA11_SLICE 16
	cnt = sdl_getTicks();
	for (i = 0; i < ECA11_SLICE + h / ECA11_SLICE -1; i++) {
		cnt += waitcnt;
		for (j = 0; j < min(i+1, ECA11_SLICE); j++) {
			y = j + ECA11_SLICE*(i-j);
			if (y < 0 || y >= h) continue;
			eCopyUpdateArea(dx, dy + y, w, 1, dx, dy + y);
		}
		EC_WAIT;
	}
	
	eCopyUpdateArea(dx, dy, w, h, dx, dy);
	return key;
}

static int eCopyArea12(int dx, int dy, int w, int h, int opt) {
	int x, key = 0, cnt;
	int waitcnt = opt == 0 ? 20 : opt;
	
	cnt = sdl_getTicks();
	for (x = 0; x < (w -7); x += 8) {
		cnt += waitcnt;
		eCopyUpdateArea(dx + x, dy, 8, h, dx + x, dy);
		EC_WAIT;
	}
	eCopyUpdateArea(dx, dy, w, h, dx, dy);
	return key;
}

static int eCopyArea13(int dx, int dy, int w, int h, int opt) {
	int x, key = 0, cnt;
	int waitcnt = opt == 0 ? 20 : opt;
	
	cnt = sdl_getTicks();
	for (x = (w -8); x > 8; x -= 8) {
		cnt +=waitcnt;
		eCopyUpdateArea(dx + x, dy, 8, h, dx + x, dy);
		EC_WAIT;
	}
	eCopyUpdateArea(dx, dy, w, h, dx, dy);
	return key;
}

static int eCopyArea14(int dx, int dy, int w, int h, int opt) {
	int y, key = 0, cnt;
	int waitcnt = opt == 0 ? 20 : opt;
	
	cnt = sdl_getTicks();
	for (y = 0; y < (h -7); y += 8) {
		cnt += waitcnt;
		eCopyUpdateArea(dx, dy + y, w, 8, dx, dy + y);
		EC_WAIT;
	}
	eCopyUpdateArea(dx, dy, w, h, dx, dy);
	return key;
}

static int eCopyArea15(int dx, int dy, int w, int h, int opt) {
	int y, key = 0, cnt;
	int waitcnt = opt == 0 ? 20 : opt;
	
	cnt = sdl_getTicks();
	for (y = (h -8); y > 8; y -= 8) {
		cnt += waitcnt;
		eCopyUpdateArea(dx, dy + y, w, 8, dx, dy + y);
		EC_WAIT;
	}
	eCopyUpdateArea(dx, dy, w, h, dx, dy);
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

static int eCopyArea18(int dx, int dy, int w, int h, int opt) {
	int x, key = 0, cnt;
	int waitcnt = opt == 0 ? 30 : opt;

	cnt = sdl_getTicks();
	for (x = 0; x < (w/2 -7); x += 8) {
		cnt += waitcnt;
		eCopyUpdateArea(dx + w/2 - x - 8, dy, 8, h, dx + w/2 - x - 8, dy);
		eCopyUpdateArea(dx + w/2 + x    , dy, 8, h, dx + w/2 + x    , dy);
		EC_WAIT;
	}
	eCopyUpdateArea(dx, dy, w, h, dx, dy);
	return key;
}

static int eCopyArea19(int dx, int dy, int w, int h, int opt) {
	int x, key = 0, cnt;
	int waitcnt = opt == 0 ? 30 : opt;

	cnt = sdl_getTicks();
	for (x = 0; x < (w/2 -7); x += 8) {
		cnt += waitcnt;
		eCopyUpdateArea(dx + x,         dy, 8, h, dx + x,         dy);
		eCopyUpdateArea(dx + w - x - 8, dy, 8, h, dx + w - x - 8, dy);
		EC_WAIT;
	}
	eCopyUpdateArea(dx, dy, w, h, dx, dy);
	return key;
}

static int eCopyArea20(int dx, int dy, int w, int h, int opt) {
	int y, key = 0, cnt;
	int waitcnt = opt == 0 ? 30 : opt;

	cnt = sdl_getTicks();
	for (y = 0; y < (h/2 -7); y += 8) {
		cnt += waitcnt;
		eCopyUpdateArea(dx, dy + h/2 - y - 8, w, 8, dx, dy + h/2 - y - 8);
		eCopyUpdateArea(dx, dy + h/2 + y    , w, 8, dx, dy + h/2 + y    );
		EC_WAIT;
	}
	eCopyUpdateArea(dx, dy, w, h, dx, dy);
	return key;
}

static int eCopyArea21(int dx, int dy, int w, int h, int opt) {
	int y, key = 0, cnt;
	int waitcnt = opt == 0 ? 30 : opt;

	cnt = sdl_getTicks();
	for (y = 0; y < (h/2 -7); y += 8) {
		cnt += waitcnt;
		eCopyUpdateArea(dx, dy + y        , w, 8, dx, dy + y);
		eCopyUpdateArea(dx, dy + h - y - 8, w, 8, dx, dy + h - y - 8);
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

static int eCopyArea24(int sx, int sy, int w, int h, int dx, int dy, int opt) {
	int i, key = 0, cnt;
	int waitcnt = opt == 0 ? 100 : opt;
	static int slices[8]={80,70,60,48,32,16,8,4};

	cnt = sdl_getTicks();
	for (i = 0; i < 8; i++ ) {
		cnt += waitcnt;
		sdl_Mosaic(sx, sy, w, h, dx, dy, slices[i]);
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

static void eCopyArea27(int step) {
	if (step == 0) {
		return;
	}
	if (step == 64) {
		ags_copyArea_alphaLevel(ecp.sx, ecp.sy, ecp.w, ecp.h, ecp.dx, ecp.dy, 255);
		ags_updateArea(ecp.dx, ecp.dy, ecp.w, ecp.h);
		return;
	}
	ags_copyArea_alphaLevel(ecp.sx, ecp.sy, ecp.w, ecp.h, ecp.dx, ecp.dy, step*4);
	ags_updateArea(ecp.dx, ecp.dy, ecp.w, ecp.h);
}

static void eCopyArea28(int step) {
	if (step == 0) {
		return;
	}
	if (step == 64) {
		ags_copyArea_whiteLevel(ecp.sx, ecp.sy, ecp.w, ecp.h, ecp.dx, ecp.dy, 0);
		ags_updateArea(ecp.dx, ecp.dy, ecp.w, ecp.h);
		return;
	}
	ags_copyArea_whiteLevel(ecp.sx, ecp.sy, ecp.w, ecp.h, ecp.dx, ecp.dy, (64-step)*4);
	ags_updateArea(ecp.dx, ecp.dy, ecp.w, ecp.h);
}

static void eCopyArea29(int step) {
	if (step == 0) {
		return;
	}
	if (step == 64) {
		ags_copyArea_alphaLevel(ecp.sx, ecp.sy, ecp.w, ecp.h, ecp.dx, ecp.dy, 0);
		ags_updateArea(ecp.dx, ecp.dy, ecp.w, ecp.h);
		return;
	}
	ags_copyArea_alphaLevel(ecp.sx, ecp.sy, ecp.w, ecp.h, ecp.dx, ecp.dy, (64-step)*4);
	ags_updateArea(ecp.dx, ecp.dy, ecp.w, ecp.h);
}

static void eCopyArea30(int step) {
	if (step == 0) {
		return;
	}
	if (step == 64) {
		ags_copyArea_whiteLevel(ecp.sx, ecp.sy, ecp.w, ecp.h, ecp.dx, ecp.dy, 255);
		ags_updateArea(ecp.dx, ecp.dy, ecp.w, ecp.h);
		return;
	}
	ags_copyArea_whiteLevel(ecp.sx, ecp.sy, ecp.w, ecp.h, ecp.dx, ecp.dy, step*4);
	ags_updateArea(ecp.dx, ecp.dy, ecp.w, ecp.h);
}

static void eCopyArea31(int step) {
	static void *save;
	if (step == 0) {
		save = ags_saveRegion(ecp.dx, ecp.dy, ecp.w, ecp.h);
		return;
	}
	if (step == 64) {
		ags_copyArea(ecp.sx, ecp.sy, ecp.w, ecp.h, ecp.dx, ecp.dy);
		ags_updateArea(ecp.dx, ecp.dy, ecp.w, ecp.h);
		ags_delRegion(save);
		return;
	}
	ags_putRegion(save, ecp.dx, ecp.dy);
	ags_copyArea_alphaBlend(ecp.sx, ecp.sy, ecp.w, ecp.h, ecp.dx, ecp.dy, step*4);
	ags_updateArea(ecp.dx, ecp.dy, ecp.w, ecp.h);
}

static void eCopyArea32(int step) {
	static void *save;
	static int slices[32]={4,8,12,16,20,28,36,40,44,48,56,64,72,80,88,96,
			       88,80,72,64,56,48,44,40,36,28,24,20,16,12,8,4};
	
	if (step == 0) {
		save = ags_saveRegion(ecp.dx, ecp.dy, ecp.w, ecp.h);
		return;
	}
	if (step == 64) {
		ags_copyArea(ecp.sx, ecp.sy, ecp.w, ecp.h, ecp.dx, ecp.dy);
		ags_updateArea(ecp.dx, ecp.dy, ecp.w, ecp.h);
		ags_delRegion(save);
		return;
	}
	ags_putRegion(save, ecp.dx, ecp.dy);
	ags_copyArea_alphaBlend(ecp.sx, ecp.sy, ecp.w, ecp.h, ecp.dx, ecp.dy, step*4);
	sdl_Mosaic(ecp.sx, ecp.sy, ecp.w, ecp.h, ecp.dx, ecp.dy, slices[step >> 1]);
	ags_updateArea(ecp.dx, ecp.dy, ecp.w, ecp.h);
}

static int eCopyArea33(int dx, int dy, int w, int h, int opt) {
	int i, j, y, key = 0, cnt,dyy=dy+h-1;
	int waitcnt = opt == 0 ? 40 : opt;

	cnt = sdl_getTicks();
	for (i = 0; i < ECA11_SLICE + h / ECA11_SLICE -1; i++) {
		cnt += waitcnt;
		for (j = 0; j < min(i+1, ECA11_SLICE); j++) {
			y = j + ECA11_SLICE*(i-j);
			if (y < 0 || y >= h) continue;
			eCopyUpdateArea(dx, dyy - y, w, 1, dx, dyy - y);
		}
		EC_WAIT;
	}
	
	eCopyUpdateArea(dx, dy, w, h, dx, dy);
	return key;
}

static int eCopyArea34(int dx, int dy, int w, int h, int opt) {
	int i, j, y, key = 0, cnt,dyy=dy+h-1,h2=h/2;
	int waitcnt = opt == 0 ? 40 : opt;

	cnt = sdl_getTicks();
	for (i = 0; i < ECA11_SLICE + h / ECA11_SLICE/2 -1; i++) {
		cnt += waitcnt;
		for (j = 0; j < min(i+1, ECA11_SLICE); j++) {
			y = j + ECA11_SLICE*(i-j);
			if (y < 0 || y >= h2 ) continue;
			eCopyUpdateArea(dx, dy + y, w, 1, dx, dy + y);
			eCopyUpdateArea(dx, dyy - y, w, 1, dx, dyy - y);
		}
		EC_WAIT;
	}
	
	eCopyUpdateArea(dx, dy, w, h, dx, dy);
	return key;
}

#define ECA35_D 256
static void eCopyArea35(int step) {
	int j, l;
	int st_i, ed_i;
	static int last_i = 0;
	static void *save = NULL;
	
	if (step == 0) {
		save = ags_saveRegion(ecp.dx, ecp.dy, ecp.w,ecp.h);
		return;
	}
	
	if (step == ECA35_D + ecp.h) {
		ags_copyArea(ecp.sx, ecp.sy, ecp.w, ecp.h, ecp.dx, ecp.dy);
		ags_updateArea(ecp.dx, ecp.dy, ecp.w, ecp.h);
		ags_delRegion(save);
		return;
	}
	
	st_i = max(0, step - ECA35_D + 1);
	ed_i = min(ecp.h - 1, step);
	l = ed_i - st_i + 1;
	
	ags_copyRegion(save, 0, st_i, ecp.w, l, ecp.dx, ecp.dy+st_i);
	ags_copyArea(ecp.sx, ecp.sy + st_i, ecp.w, 1, ecp.dx, ecp.dy + st_i);
	for (j = st_i; j < ed_i; j++) {
		ags_copyArea_alphaBlend(ecp.sx, ecp.sy + j, ecp.w, 1, ecp.dx, ecp.dy + j, step - j);
	}
	
	if ((st_i - last_i) > 1) {
		ags_copyArea(ecp.sx, ecp.sy + last_i, ecp.w, st_i - last_i, ecp.dx, ecp.dy + last_i);
		ags_updateArea(ecp.dx, ecp.dy + last_i, ecp.w, st_i - last_i);
	}
	ags_updateArea(ecp.dx, ecp.dy+st_i, ecp.w, l);
	
	// if (st_i > 0) last_i = st_i;
	last_i = st_i;
}

static void eCopyArea36(int step) {
	int j, l;
	int st_i,ed_i;
	static int last_i=0;
	static void *save;
	
	int syy=ecp.sy+ecp.h-1,dyy=ecp.dy+ecp.h-1;

	if (step == 0) {
		save = ags_saveRegion(ecp.dx, ecp.dy, ecp.w,ecp.h);
		return;
	}
	if (step == ECA35_D + ecp.h) {
		ags_copyArea(ecp.sx, ecp.sy, ecp.w, ecp.h, ecp.dx, ecp.dy);
		ags_updateArea(ecp.dx, ecp.dy, ecp.w, ecp.h);
		ags_delRegion(save);
		return;
	}
	
	st_i = max(0, step-ECA35_D+1);
	ed_i = min(ecp.h -1, step);
	l = ed_i - st_i+1;
	ags_copyRegion(save,0,ecp.h -1 -ed_i,ecp.w,l,ecp.dx,dyy-ed_i);
	
	for (j = st_i; j <= ed_i; j++) {
		ags_copyArea_alphaBlend(ecp.sx, syy-j, ecp.w, 1, ecp.dx, dyy-j, step-j);
	}
	
	if ((st_i - last_i) > 1) {
		ags_copyArea(ecp.sx,syy-st_i+1,ecp.w,st_i-last_i,ecp.dx,dyy-st_i+1);
		ags_updateArea(ecp.dx, dyy-st_i+1, ecp.w, st_i-last_i);
	}
	ags_updateArea(ecp.dx, dyy-ed_i, ecp.w, l);

	// if (st_i > 0) last_i = st_i;
	last_i = st_i;
}

static void eCopyArea37(int step) {
	int j, l;
	int st_i,ed_i;
	static int last_i=0;
	static void *save = NULL;
	
	if (step == 0) {
		save = ags_saveRegion(ecp.dx,ecp.dy,ecp.w,ecp.h);
		return;
	}
	if (step == ECA35_D + ecp.w) {
		ags_copyArea(ecp.sx, ecp.sy, ecp.w, ecp.h, ecp.dx, ecp.dy);
		ags_updateArea(ecp.dx, ecp.dy, ecp.w, ecp.h);
		ags_delRegion(save);
		return;
	}
	
	st_i=max(0,step-ECA35_D+1);
	ed_i=min(ecp.w -1,step);
	l=ed_i-st_i+1;
	ags_copyRegion(save,st_i,0,l,ecp.h,ecp.dx+st_i,ecp.dy);
	
	for (j = st_i; j <= ed_i; j++) {
		ags_copyArea_alphaBlend(ecp.sx+j, ecp.sy, 1, ecp.h, ecp.dx+j, ecp.dy, step-j);
	}
	
	if ((st_i - last_i) > 1) {
		ags_copyArea(ecp.sx+last_i, ecp.sy, st_i-last_i, ecp.h,ecp.dx+last_i, ecp.dy);
		ags_updateArea(ecp.dx+last_i, ecp.dy, st_i-last_i, ecp.h);
	}
	ags_updateArea(ecp.dx+st_i, ecp.dy, l, ecp.h);

	//if (st_i > 0) last_i = st_i;
	last_i = st_i;
}

static void eCopyArea38(int step) {
	int j, l;
	int st_i, ed_i;
	static int last_i=0;
	static void *save;
	int sxx=ecp.sx+ecp.w - 1,dxx=ecp.dx+ecp.w-1;
	
	if (step == 0) {
		save = ags_saveRegion(ecp.dx, ecp.dy, ecp.w,ecp.h);
		return;
	}
	if (step == ECA35_D + ecp.w) {
		ags_copyArea(ecp.sx, ecp.sy, ecp.w, ecp.h, ecp.dx, ecp.dy);
		ags_updateArea(ecp.dx, ecp.dy, ecp.w, ecp.h);
		ags_delRegion(save);
		return;
	}
	
	st_i=max(0,step-ECA35_D+1);
	ed_i=min(ecp.w - 1,step);
	l=ed_i-st_i+1;
	ags_copyRegion(save,ecp.w-1-ed_i,0,l,ecp.h,dxx-ed_i,ecp.dy);
	
	for (j = st_i; j<=ed_i; j++) {
		ags_copyArea_alphaBlend(sxx-j, ecp.sy, 1, ecp.h, dxx-j, ecp.dy, step-j);
	}
	
	if ((st_i - last_i) > 1) {
		ags_copyArea(sxx-st_i+1, ecp.sy,st_i-last_i, ecp.h, dxx-st_i+1, ecp.dy);
		ags_updateArea(dxx-st_i+1,ecp.dy, st_i-last_i, ecp.h);
	}
	ags_updateArea(dxx-ed_i, ecp.dy, l, ecp.h);
	// if (st_i > 0) last_i=st_i;
	last_i=st_i;
}

static int eCopyArea39(int sx, int sy, int w, int h, int dx, int dy, int opt) {
	int i,dy1,dy2,sy1,sy2, key = 0, cnt;
	int waitcnt = opt == 0 ? 3 : opt;
	int h2=h/2;
	
	dy1 = dy;
	dy2 = dy+h-1;
	sy1 = sy;
	sy2 = sy+h-1;
	cnt=sdl_getTicks();
	for (i = 0; i < h2; i++) {
		cnt+=waitcnt;
		ags_copyArea_alphaBlend(sx, sy1, w, 1, dx, dy1, 128);
		ags_copyArea_alphaBlend(sx, sy2, w, 1, dx, dy2, 128);
		ags_updateArea(dx, dy1, w, 1);
		ags_updateArea(dx, dy2, w, 1);
		dy1++;
		sy1++;
		dy2--;
		sy2--;
		EC_WAIT;
	}
	if (h & 1) {
		eCopyUpdateArea(sx, sy1, w, 1, dx, dy1);
		dy1++;
		sy1++;
		dy2--;
		sy2--;
	}
	for (i = 0; i < h2; i++) {
		cnt+=waitcnt;
		eCopyUpdateArea(sx, sy1, w, 1, dx, dy1);
		eCopyUpdateArea(sx, sy2, w, 1, dx, dy2);
		dy1++;
		sy1++;
		dy2--;
		sy2--;
		EC_WAIT;
	}
	ags_copyArea(sx, sy, w, h, dx, dy);
	ags_updateArea(dx, dy, w, h);
	return key;
}

static int eCopyArea40(int sx, int sy, int w, int h, int dx, int dy, int opt) {
	int i,dx1,dx2,sx1,sx2, key = 0, cnt;
	int waitcnt = opt == 0 ? 3 : opt;
	int w2=w/2;
	
	dx1 = dx;
	dx2 = dx+w-1;
	sx1 = sx;
	sx2 = sx+w-1;
	cnt=sdl_getTicks();
	for (i = 0; i < w2; i++) {
		cnt+=waitcnt;
		ags_copyArea_alphaBlend(sx1, sy, 1, h, dx1, dy, 128);
		ags_copyArea_alphaBlend(sx2, sy, 1, h, dx2, dy, 128);
		ags_updateArea( dx1, dy, 1, h);
		ags_updateArea( dx2, dy, 1, h);
		dx1++;
		sx1++;
		dx2--;
		sx2--;
		EC_WAIT;
	}
	if (w & 1) {
		eCopyUpdateArea(sx1, sy, 1, h, dx1, dy);
		dx1++;
		sx1++;
		dx2--;
		sx2--;
	}
	for (i = 0; i < w2; i++) {
		cnt+=waitcnt;
		eCopyUpdateArea( sx1, sy, 1, h, dx1, dy);
		eCopyUpdateArea( sx2, sy, 1, h, dx2, dy);
		dx1++;
		sx1++;
		dx2--;
		sx2--;
		EC_WAIT;
	}
	ags_copyArea(sx, sy, w, h, dx, dy);
	ags_updateArea(dx, dy, w, h);
	return key;
}

#define ECA41_D 128
static void eCopyArea41(int step) {
	int j, k, l;
	int st_i,ed_i,w2=ecp.w/2,w21=ecp.w/2-1;
	int sxx1=ecp.sx+w2,dxx1=ecp.dx+w2;
	int sxx2=sxx1-1,dxx2=dxx1-1;
	static int last_i = -1;
	
	static void *save= NULL;

	if (step == 0) {
		save = ags_saveRegion(ecp.dx, ecp.dy, ecp.w, ecp.h);
		return;
	}
	
	if (step == ECA41_D + w21) {
		ags_copyArea(ecp.sx, ecp.sy, ecp.w, ecp.h, ecp.dx, ecp.dy);
		ags_updateArea(ecp.dx, ecp.dy, ecp.w, ecp.h);
		ags_delRegion(save);
		return;
	}
	
	if (ecp.w & 1) {
		sxx2++;
		dxx2++;
	}
	
	st_i=max(0,step-ECA41_D+1);
	ed_i=min(w21,step);
	l=ed_i-st_i+1;
	ags_copyRegion(save,w2 +st_i, 0, l, ecp.h, dxx1+st_i, ecp.dy);
	ags_copyRegion(save,w21-ed_i, 0, l, ecp.h, dxx2-ed_i, ecp.dy);
	for (j = st_i; j <= ed_i; j++) {
		k=(step-j+1)*2-1;
		ags_copyArea_alphaBlend(sxx1+j, ecp.sy, 1, ecp.h, dxx1+j, ecp.dy, k);
		ags_copyArea_alphaBlend(sxx2-j, ecp.sy, 1, ecp.h, dxx2-j, ecp.dy, k);
	}
	if ((st_i - last_i) > 1) {
		if( last_i<0 ) last_i=0;
		ags_copyArea(sxx1+last_i, ecp.sy, st_i-last_i, ecp.h,dxx1+last_i, ecp.dy);
		ags_copyArea(sxx2-st_i+1, ecp.sy, st_i-last_i, ecp.h,dxx2-st_i+1, ecp.dy);
		ags_updateArea( dxx1+last_i, ecp.dy, st_i-last_i, ecp.h);
		ags_updateArea( dxx2-st_i+1, ecp.dy, st_i-last_i, ecp.h);
	}
	ags_updateArea(dxx1+st_i, ecp.dy, l, ecp.h);
	ags_updateArea(dxx2-ed_i, ecp.dy, l, ecp.h);
	if (st_i > 0) last_i=st_i;
}

static void eCopyArea42(int step) {
	int j, k, l;
	int st_i,ed_i,h2=ecp.h/2,h21=ecp.h/2-1;
	int syy1=ecp.sy+h2,dyy1=ecp.dy+h2;
	int syy2=syy1-1,dyy2=dyy1-1;
	static void *save;
	static int last_i = -1;

	if (step == 0) {
		save = ags_saveRegion(ecp.dx, ecp.dy, ecp.w, ecp.h);
		return;
	}
	
	if (step == ECA41_D + h21) {
		ags_copyArea(ecp.sx, ecp.sy, ecp.w, ecp.h, ecp.dx, ecp.dy);
		ags_updateArea(ecp.dx, ecp.dy, ecp.w, ecp.h);
		ags_delRegion(save);
		return;
	}

	if( ecp.h&1 ) {
		syy2++;
		dyy2++;
	}
	
	st_i=max(0,step-ECA41_D+1);
	ed_i=min(h21,step);
	l=ed_i-st_i+1;
	ags_copyRegion(save, 0,h2 +st_i, ecp.w, l, ecp.dx, dyy1+st_i);
	ags_copyRegion(save, 0,h21-ed_i, ecp.w, l, ecp.dx, dyy2-ed_i);
	for( j=st_i ; j<=ed_i ; j++ ) {
		k=(step-j+1)*2-1;
		ags_copyArea_alphaBlend(ecp.sx, syy1+j, ecp.w, 1, ecp.dx, dyy1+j, k);
		ags_copyArea_alphaBlend(ecp.sx, syy2-j, ecp.w, 1, ecp.dx, dyy2-j, k);
	}
	if( (st_i - last_i)>1 ) {
		if( last_i<0 ) last_i=0;
		ags_copyArea(ecp.sx, syy1+last_i, ecp.w, st_i-last_i, ecp.dx, dyy1+last_i);
		ags_copyArea(ecp.sx, syy2-st_i+1, ecp.w, st_i-last_i, ecp.dx, dyy2-st_i+1);
		ags_updateArea( ecp.dx, dyy1+last_i, ecp.w, st_i-last_i);
		ags_updateArea( ecp.dx, dyy2-st_i+1, ecp.w, st_i-last_i);
	}
	ags_updateArea( ecp.dx, dyy1+st_i, ecp.w, l);
	ags_updateArea( ecp.dx, dyy2-ed_i, ecp.w, l);
	if( st_i > 0 ) last_i=st_i;
}

static void eCopyArea43(int step) {
	int deltax, deltay, deltaw, deltah;
	int slice = max(ecp.w, ecp.h);
	
	if (step == 0) {
		return;
	}
	if (step == slice) {
		ags_scaledCopyArea(ecp.sx, ecp.sy, ecp.w, ecp.h,
				   nact->sys_view_area.x, nact->sys_view_area.y,
				   nact->sys_view_area.w, nact->sys_view_area.h, 0);
		ags_updateFull();
		return;
	}
	
	deltax = (ecp.sx - nact->sys_view_area.x) * step / slice;
	deltay = (ecp.sy - nact->sys_view_area.y) * step / slice;
	deltaw = (nact->sys_view_area.w - ecp.w) * step / slice;
	deltah = (nact->sys_view_area.h - ecp.h) * step / slice;
	ags_zoom(nact->sys_view_area.x + deltax,
		 nact->sys_view_area.y + deltay,
		 nact->sys_view_area.w - deltaw,
		 nact->sys_view_area.h - deltah);
}

static void eCopyArea44(int step) {
	sdl_maskupdate(ecp.sx, ecp.sy, ecp.w, ecp.h, ecp.dx, ecp.dy, 44, step); 
}

static void eCopyArea45(int step) {
	sdl_maskupdate(ecp.sx, ecp.sy, ecp.w, ecp.h, ecp.dx, ecp.dy, 45, step); 
}

static void eCopyArea46(int step) {
	sdl_maskupdate(ecp.sx, ecp.sy, ecp.w, ecp.h, ecp.dx, ecp.dy, 46, step); 
}

static void eCopyArea47(int step) {
	sdl_maskupdate(ecp.sx, ecp.sy, ecp.w, ecp.h, ecp.dx, ecp.dy, 47, step); 
}

static int eCopyArea48(int dx, int dy, int w, int h, int opt) {
	int i, j, x, key = 0, cnt;
	int waitcnt = opt == 0 ? 40 : opt;

	cnt = sdl_getTicks();
	for (i = 0; i < ECA11_SLICE + w / ECA11_SLICE -1; i++) {
		cnt += waitcnt;
		for (j = 0; j < min(i+1, ECA11_SLICE); j++) {
			x = j + ECA11_SLICE*(i-j);
			if (x < 0 || x >= w) continue;
			eCopyUpdateArea( dx+x, dy, 1, h, dx+x, dy);
		}
		EC_WAIT;
	}
	
	eCopyUpdateArea(dx, dy, w, h, dx, dy);
	return key;
}

static int eCopyArea49(int dx, int dy, int w, int h, int opt) {
	int i, j, x, key = 0, cnt, dxx = dx + w - 1;
	int waitcnt = opt == 0 ? 40 : opt;
	
	cnt = sdl_getTicks();
	for (i = 0; i < ECA11_SLICE + w / ECA11_SLICE -1; i++) {
		cnt += waitcnt;
		for (j = 0; j < min(i+1, ECA11_SLICE); j++) {
			x = j + ECA11_SLICE*(i-j);
			if (x < 0 || x >= w) continue;
			eCopyUpdateArea(dxx - x, dy, 1, h, dxx - x, dy);
		}
		EC_WAIT;
	}
	
	eCopyUpdateArea(dx, dy, w, h, dx, dy);
	return key;
}

static void eCopyArea50(int step) {
	sdl_maskupdate(ecp.sx, ecp.sy, ecp.w, ecp.h, ecp.dx, ecp.dy, 50, step); 
}

static void eCopyArea51(int step) {
	sdl_maskupdate(ecp.sx, ecp.sy, ecp.w, ecp.h, ecp.dx, ecp.dy, 51, step); 
}

static void eCopyArea52(int step) {
	sdl_maskupdate(ecp.sx, ecp.sy, ecp.w, ecp.h, ecp.dx, ecp.dy, 52, step); 
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
	sdl_fillRectangle(dx, dy, w, h, 0);
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

void ags_eCopyArea(int sx, int sy, int w, int h, int dx, int dy, int sw, int opt, boolean cancel, int spCol) {
	int ret = 0;
	ags_faderinfo_t i;

#if 0
	NOTICE("ec_area sx %d sy %d w %d h %d dx %d dy %d sw %d opt %d spc %d cancel %s\n",
	       sx, sy, w, h, dx, dy, sw, opt, spCol, cancel ? "True" : "False");
#endif
	if (!ags_check_param(&sx, &sy, &w, &h)) return;
	if (!ags_check_param(&dx, &dy, &w, &h)) return;
	
	nact->waitcancel_key = 0;
	
	ecp.sx = sx;
	ecp.sy = sy;
	ecp.w  = w;
	ecp.h  = h;
	ecp.dx = dx;
	ecp.dy = dy;
	ecp.sw = sw;
	ecp.opt    = opt;
	ecp.cancel = cancel;
	ecp.spCol  = spCol;
	
	switch(sw) {
	case 1:
	case 2:
	case 3:
	case 4:
	case 6:
	case 7:
	case 8:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
	case 17:
	case 18:
	case 19:
	case 20:
	case 21:
	case 33:
	case 34:
	case 48:
	case 49:
	case 53:
	case 54:
		if (spCol == -1) {
			ags_copyArea(sx, sy, w, h, dx, dy);
		} else {
			ags_copyAreaSP(sx, sy, w, h, dx, dy, spCol);
		}
		{
			MyRectangle r = {dx, dy, w, h}, update;
			SDL_IntersectRect(&nact->sys_view_area, &r, &update);
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
	case 7:
		ret = eCopyArea7(dx, dy, w, h, opt);
		break;
	case 8:
		ret = eCopyArea8(dx, dy, w, h, opt);
		break;
	case 9:
		ret = eCopyArea9(sx, sy, w, h, dx, dy, opt);
		break;
	case 10:
		i.step_max = 64;
		i.effect_time = opt == 0 ? 500 : opt * 64;
		i.cancel = cancel;
		i.callback = eCopyArea10;
		ags_fader(&i);
		return;
	case 11:
		ret = eCopyArea11(dx, dy, w, h, opt);
		break;
	case 12:
		ret = eCopyArea12(dx, dy, w, h, opt);
		break;
	case 13:
		ret = eCopyArea13(dx, dy, w, h, opt);
		break;
	case 14:
		ret = eCopyArea14(dx, dy, w, h, opt);
		break;
	case 15:
		ret = eCopyArea15(dx, dy, w, h, opt);
		break;
	case 16:
		ret = eCopyArea16(dx, dy, w, h, opt);
		break;
	case 17:
		ret = eCopyArea17(dx, dy, w, h, opt);
		break;
	case 18:
		ret = eCopyArea18(dx, dy, w, h, opt);
		break;
 	case 19:
		ret = eCopyArea19(dx, dy, w, h, opt);
		break;
 	case 20:
		ret = eCopyArea20(dx, dy, w, h, opt);
		break;
 	case 21:
		ret = eCopyArea21(dx, dy, w, h, opt);
		break;
	case 22:
		ret = eCopyArea22(sx, sy, w, h, dx, dy, opt);
		break;
	case 23:
		ret = eCopyArea23(sx, sy, w, h, dx, dy, opt);
		break;
	case 24:
		ret = eCopyArea24(sx, sy, w, h, dx, dy, opt);
		break;
	case 25:
		ret = eCopyArea25(sx, sy, w, h, dx, dy, opt);
		break;
	case 26:
		ret = eCopyArea26(sx, sy, w, h, dx, dy, opt);
		break;
	case 27:
		i.step_max = 64;
		i.effect_time = opt == 0 ? 1700 : opt * 32;
		i.cancel = cancel;
		i.callback = eCopyArea27;
		ags_fader(&i);
		return;
	case 28:
		i.step_max = 64;
		i.effect_time = opt == 0 ? 1700 : opt * 32;
		i.cancel = cancel;
		i.callback = eCopyArea28;
		ags_fader(&i);
		return;
	case 29:
		i.step_max = 64;
		i.effect_time = opt == 0 ? 1700 : opt * 32;
		i.cancel = cancel;
		i.callback = eCopyArea29;
		ags_fader(&i);
		return;
	case 30:
		i.step_max = 64;
		i.effect_time = opt == 0 ? 1700 : opt * 32;
		i.cancel = cancel;
		i.callback = eCopyArea30;
		ags_fader(&i);
		return;
	case 31:
		i.step_max = 64;
		i.effect_time = opt == 0 ? 2700 : opt * 256;
		i.cancel = cancel;
		i.callback = eCopyArea31;
		ags_fader(&i);
		return;
	case 32:
		i.step_max = 64;
		i.effect_time = opt == 0 ? 1400 : opt * 256;
		i.cancel = cancel;
		i.callback = eCopyArea32;
		ags_fader(&i);
		return;
	case 33:
		ret = eCopyArea33(dx, dy, w, h, opt);
		break;
	case 34:
		ret = eCopyArea34(dx, dy, w, h, opt);
		break;
	case 35:
		i.step_max = ECA35_D + h;
		i.effect_time = opt == 0 ? 1150 : opt * i.step_max;
		i.cancel = cancel;
		i.callback = eCopyArea35;
		ags_fader(&i);
		return;
	case 36:
		i.step_max = ECA35_D + h;
		i.effect_time = opt == 0 ? 1150 : opt * i.step_max;
		i.cancel = cancel;
		i.callback = eCopyArea36;
		ags_fader(&i);
		return;
	case 37:
		i.step_max = ECA35_D + w;
		i.effect_time = opt == 0 ? 1150 : opt * i.step_max;
		i.cancel = cancel;
		i.callback = eCopyArea37;
		ags_fader(&i);
		return;
	case 38:
		i.step_max = ECA35_D + w;
		i.effect_time = opt == 0 ? 1150 : opt * i.step_max;
		i.cancel = cancel;
		i.callback = eCopyArea38;
		ags_fader(&i);
		return;
	case 39:
		ret = eCopyArea39(sx, sy, w, h, dx, dy, opt);
		break;
	case 40:
		ret = eCopyArea40(sx, sy, w, h, dx, dy, opt);
		break;
	case 41:
		i.step_max = ECA41_D + w/2 -1;
		i.effect_time = opt == 0 ? 1300 : opt * i.step_max;
		i.cancel = cancel;
		i.callback = eCopyArea41;
		ags_fader(&i);
		return;
	case 42:
		i.step_max = ECA41_D + h/2 -1;
		i.effect_time = opt == 0 ? 1300 : opt * i.step_max;
		i.cancel = cancel;
		i.callback = eCopyArea42;
		ags_fader(&i);
		return;
	case 43:
		i.step_max = max(w, h);
		i.effect_time = opt == 0 ? 2000 : opt + 300;
		i.cancel = cancel;
		i.callback = eCopyArea43;
		ags_fader(&i);
		return;
		
	case 44:
		i.step_max = 256;
		i.effect_time = opt == 0 ? 1000 : opt;
		i.cancel = cancel;
		i.callback = eCopyArea44;
		ags_fader(&i);
		return;

	case 45:
		i.step_max = 256;
		i.effect_time = opt == 0 ? 1000 : opt;
		i.cancel = cancel;
		i.callback = eCopyArea45;
		ags_fader(&i);
		return;
		
	case 46:
		i.step_max = 256;
		i.effect_time = opt == 0 ? 1000 : opt;
		i.cancel = cancel;
		i.callback = eCopyArea46;
		ags_fader(&i);
		return;
		
	case 47:
		i.step_max = 256;
		i.effect_time = opt == 0 ? 1000 : opt;
		i.cancel = cancel;
		i.callback = eCopyArea47;
		ags_fader(&i);
		return;
		
	case 48:
		ret = eCopyArea48(dx, dy, w, h, opt);
		break;
	case 49:
	        ret = eCopyArea49(dx, dy, w, h, opt);
		break;
		
	case 50:
		i.step_max = 256;
		i.effect_time = opt == 0 ? 1000 : opt;
		i.cancel = cancel;
		i.callback = eCopyArea50;
		ags_fader(&i);
		return;
		
	case 51:
		i.step_max = 256;
		i.effect_time = opt == 0 ? 1000 : opt;
		i.cancel = cancel;
		i.callback = eCopyArea51;
		ags_fader(&i);
		return;
		
	case 52:
		i.step_max = 256;
		i.effect_time = opt == 0 ? 1000 : opt;
		i.cancel = cancel;
		i.callback = eCopyArea52;
		ags_fader(&i);
		return;
		
	case 1000:
		if (nact->sys_world_depth != 8) return;
		ret = eCopyArea1000(sx, sy, w, h, dx, dy, opt, spCol);
		break;
	case 1001:
		if (nact->sys_world_depth != 8) return;
		ret = eCopyArea1001(sx, sy, w, h, dx, dy, opt, spCol);
		break;
	case 2000:
		if (nact->sys_world_depth == 8) return;
		ret = eCopyArea2000(sx, sy, w, h, dx, dy, opt);
		break;
	case 2001:
		if (nact->sys_world_depth == 8) return;
		ret = eCopyArea2001(sx, sy, w, h, dx, dy, opt);
		break;
	default:
		eCopyUpdateArea(dx, dy, w, h, dx, dy);
		WARNING("effect %d is not presented.\n", sw);
		break;
	}
	nact->waitcancel_key = ret;
}
