// 透視投影変換のうち、X軸とY軸周りの回転のみを考慮
/*
  X軸回転行列  1    0   0  0 
               0  cos sin  0
               0 -sin cos  0
	       0    0   0  1

  Y軸回転行列   cos 0 sin 0
                  0 1   0 0
	       -sin 0 cos 0
	          0 0   0 1

  視点座標変換行列 (G)
               1 0  0 0
	       0 1  0 0
               0 0 -1 0
	       0 0  t 1   , t = 1
 
  透視変換行列 (H)
              1 0   0   0 
	      0 1   0   0 
	      0 0 1/s 1/s
	      0 0  -1   0 , s = 1

 X軸回転 (ys = height/2)
	k[1] = -sin(rx* M_PI / 180) / (float)ys;
	k[2] = cos(rx* M_PI / 180);
	k[3] = cos(rx* M_PI / 180);
	k[7] = 1;
 Y軸回転 (xs = width/2)
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

// ポリゴン回転(Y軸・半計回り)
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

// ポリゴン回転(Y軸・計回り)
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

// ポリゴン回転(X軸・時計回り)
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

// ポリゴン回転(X軸)
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
