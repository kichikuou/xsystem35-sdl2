// アフィン変換 (回転)

static void do_aff(surface_t *in, surface_t *out, int deg, double zx, double zy) {
	int xs = in->width  / 2;
	int ys = in->height / 2;
	double r = deg * M_PI / 180;
	double c = cos(r);
	double s = sin(r);
	double x, y;
	int i, j, m, n;
	
	switch(in->depth) {
	case 15:
	case 16: {
		WORD *src, *dst;
		
		for (i = -ys; i < ys; i++) {
			for (j = -xs; j < xs; j++) {
				y = (j * s + i * c) / zy;
				x = (j * c - i * s) / zx;
				if (y > 0) m = (int)y;
				else       m = (int)(y -1);
				if (x > 0) n = (int)x;
				else       n = (int)(x -1);
				src = (WORD *)GETOFFSET_PIXEL(in,  n+xs, m+ys);
				dst = (WORD *)GETOFFSET_PIXEL(out, j+xs, i+ys);
				if ((m >= -ys) && (m < ys) && (n >= -xs) && (n < xs)) {
					*dst = *src;
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
				y = (j * s + i * c) / zy;
				x = (j * c - i * s) / zx;
				if (y > 0) m = (int)y;
				else       m = (int)(y -1);
				if (x > 0) n = (int)x;
				else       n = (int)(x -1);
				src = (DWORD *)GETOFFSET_PIXEL(in,  n+xs, m+ys);
				dst = (DWORD *)GETOFFSET_PIXEL(out, j+xs, i+ys);
				if ((m >= -ys) && (m < ys) && (n >= -xs) && (n < xs)) {
					*dst = *src;
				}
			}
		}
		break;
	}}
}



//回転アウト
static void ec20_cb(surface_t *sfsrc, surface_t *sfdst) {
	//１回転だけ
	int maxstep, curstep;
	maxstep = 360;
	curstep = maxstep * (ecp.curtime - ecp.sttime)/ (ecp.edtime - ecp.sttime);
	sf_copyall(sf0, sfdst);
	do_aff(sfsrc, sf0, curstep, 1- (double)curstep / maxstep, 1- (double)curstep / maxstep);
	ags_updateFull();
	ecp.oldstep = curstep;
}

//回転イン
static void ec21_cb(surface_t *sfsrc, surface_t *sfdst) {
	int maxstep, curstep;
	maxstep = 360;
	curstep = maxstep * (ecp.curtime - ecp.sttime)/ (ecp.edtime - ecp.sttime);
	sf_copyall(sf0, sfsrc);
	do_aff(sfdst, sf0, curstep, (double)curstep / maxstep, (double)curstep / maxstep);
	ags_updateFull();
	ecp.oldstep = curstep;
}

//回転アウト(時計回り)
static void ec22_cb(surface_t *sfsrc, surface_t *sfdst) {
	int maxstep, curstep;
	maxstep = 360;
	curstep = maxstep * (ecp.curtime - ecp.sttime)/ (ecp.edtime - ecp.sttime);
	sf_copyall(sf0, sfdst);
	do_aff(sfsrc, sf0, -curstep, 1- (double)curstep / maxstep, 1- (double)curstep / maxstep);
	ags_updateFull();
	ecp.oldstep = curstep;
}

//回転イン(時計回り)
static void ec23_cb(surface_t *sfsrc, surface_t *sfdst) {
	int maxstep, curstep;
	maxstep = 360;
	curstep = maxstep * (ecp.curtime - ecp.sttime)/ (ecp.edtime - ecp.sttime);
	sf_copyall(sf0, sfsrc);
	do_aff(sfdst, sf0, -curstep, (double)curstep / maxstep, (double)curstep / maxstep);
	ags_updateFull();
	ecp.oldstep = curstep;
}

