// ∆©ªÎ≈Í±∆ —¥π§Œ§¶§¡°¢Xº¥§»Yº¥º˛§Í§Œ≤Û≈æ§Œ§ﬂ§ÚπÕŒ∏
/*
  Xº¥≤Û≈æπ‘ŒÛ  1    0   0  0 
               0  cos sin  0
               0 -sin cos  0
	       0    0   0  1

  Yº¥≤Û≈æπ‘ŒÛ   cos 0 sin 0
                  0 1   0 0
	       -sin 0 cos 0
	          0 0   0 1

  ªÎ≈¿∫¬…∏ —¥ππ‘ŒÛ (G)
               1 0  0 0
	       0 1  0 0
               0 0 -1 0
	       0 0  t 1   , t = 1
 
  ∆©ªÎ —¥ππ‘ŒÛ (H)
              1 0   0   0 
	      0 1   0   0 
	      0 0 1/s 1/s
	      0 0  -1   0 , s = 1

 Xº¥≤Û≈æ (ys = height/2)
	k[1] = -sin(rx* M_PI / 180) / (float)ys;
	k[2] = cos(rx* M_PI / 180);
	k[3] = cos(rx* M_PI / 180);
	k[7] = 1;
 Yº¥≤Û≈æ (xs = width/2)
	k[0] = sin(ry * M_PI / 180) / (float)xs;
	k[2] = cos(ry * M_PI / 180);
	k[3] = 1;
	k[7] = cos(ry * M_PI / 180);


*/
#include <math.h>

static void do_per(surface_t *in, surface_t *out, float *k) {
	int i, j, m, n;
	float x, y, w;
	int xs = in->width  /2;
	int ys = in->height /2;
	
	switch(in->depth) {
	case 15:
	case 16: {
		WORD *src, *dst;
		
		for (i = -ys; i < ys; i++) {
			for (j = -xs; j < xs; j++) {
				w = k[0] * j + k[1]*i + k[2];
				x = k[3] * j + k[4]*i + k[5];
				y = k[6] * j + k[7]*i + k[8];
				x = x / w;
				y = y / w;
				if (y > 0) m = (int)y;
				else       m = (int)(y -1);
				if (x > 0) n = (int)x;
				else       n = (int)(x - 1);
				
				src = (WORD *)GETOFFSET_PIXEL( in, n+xs, m+ys);
				dst = (WORD *)GETOFFSET_PIXEL(out, j+xs, i+ys);
				if (( m >= -ys) && (m < ys) && (n >= -xs) && (n < xs)) {
					*dst = *src;
				} else {
					*dst = 0;
				}
			}
		}
		break;
	}
	case 24:
	case 32: {
		DWORD *src, *dst;
		
		for (i = -ys; i < ys; i++) {
			for (j = -xs; j < xs; j++) {
				w = k[0] * j + k[1]*i + k[2];
				x = k[3] * j + k[4]*i + k[5];
				y = k[6] * j + k[7]*i + k[8];
				x = x / w;
				y = y / w;
				if (y > 0) m = (int)y;
				else       m = (int)(y -1);
				if (x > 0) n = (int)x;
				else       n = (int)(x - 1);
				
				src = (DWORD *)GETOFFSET_PIXEL( in, n+xs, m+ys);
				dst = (DWORD *)GETOFFSET_PIXEL(out, j+xs, i+ys);
				if (( m >= -ys) && (m < ys) && (n >= -xs) && (n < xs)) {
					*dst = *src;
				} else {
					*dst = 0;
				}
			}
		}
		break;
	}}
	
}


static void perspect_rotatex(surface_t *in, surface_t *out, double rx) {
	float k[9] = {0.0};
	
	k[1] = -sin(rx) / (in->height / 2.0);
	k[2] = cos(rx);
	k[3] = cos(rx);
	k[7] = 1.0;
	
	do_per(in, out, k);
}

static void perspect_rotatey(surface_t *in, surface_t *out, double ry) {
	float k[9] = {0.0};
	
	k[0] = sin(ry) / (in->width / 2.0);
	k[2] = cos(ry);
	k[3] = 1.0;
	k[7] = cos(ry);
	
	do_per(in, out, k);
}

// •›•Í•¥•Û≤Û≈æ(Yº¥°¶»æ∑◊≤Û§Í)
static void ec25_cb(surface_t *src, surface_t *dst) {
	int curstep, maxstep;
	maxstep = 180;
	curstep = maxstep * (ecp.curtime - ecp.sttime)/ (ecp.edtime - ecp.sttime);
	if (curstep < 90) {
		perspect_rotatey(src, sf0, -curstep * M_PI / 180);
	} else {
		perspect_rotatey(dst, sf0, (180 - curstep) * M_PI / 180);
	}
	ags_updateFull();
}

// •›•Í•¥•Û≤Û≈æ(Yº¥°¶∑◊≤Û§Í)
static void ec26_cb(surface_t *src, surface_t *dst) {
	int curstep, maxstep;
	maxstep = 180;
	curstep = maxstep * (ecp.curtime - ecp.sttime)/ (ecp.edtime - ecp.sttime);
	if (curstep < 90) {
		perspect_rotatey(src, sf0, curstep * M_PI / 180);
	} else {
		perspect_rotatey(dst, sf0, (curstep - 180) * M_PI / 180);
	}
	ags_updateFull();
}

// •›•Í•¥•Û≤Û≈æ(Xº¥°¶ª˛∑◊≤Û§Í)
static void ec28_cb(surface_t *src, surface_t *dst) {
	int curstep, maxstep;
	maxstep = 180;
	curstep = maxstep * (ecp.curtime - ecp.sttime)/ (ecp.edtime - ecp.sttime);
	if (curstep < 90) {
		perspect_rotatex(src, sf0, -curstep * M_PI / 180);
	} else {
		perspect_rotatex(dst, sf0, (180 - curstep) * M_PI / 180);
	}
	ags_updateFull();
}

// •›•Í•¥•Û≤Û≈æ(Xº¥)
static void ec29_cb(surface_t *src, surface_t *dst) {
	int curstep, maxstep;
	maxstep = 180;
	curstep = maxstep * (ecp.curtime - ecp.sttime)/ (ecp.edtime - ecp.sttime);
	if (curstep < 90) {
		perspect_rotatex(src, sf0, curstep * M_PI / 180);
	} else {
		perspect_rotatex(dst, sf0, (curstep - 180) * M_PI / 180);
	}
	ags_updateFull();
}
