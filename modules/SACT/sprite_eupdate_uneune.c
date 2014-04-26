static void uneune(surface_t *in, surface_t *out, int st) {
	int r = 4 * st;
	double th = 4 * st * M_PI;
	int x, y, m, n;
	
	switch(in->depth) {
	case 15:
	case 16: {
		WORD *src, *dst;
		for (y = 0; y < in->height; y++) {
			for (x = 0; x < in->width; x++) {
				m = x + r * sin(y * th / in->height);
				n = y + r * cos(x * th / in->width);
				src = (WORD *)GETOFFSET_PIXEL(in, m, n);
				dst = (WORD *)GETOFFSET_PIXEL(out, x, y);
				if ((m > 0) && (m < in->width) && (n > 0) && (n < in->height)) {
					*dst = *src;
				} else {
					*dst = 0;
				}
			}
		}
	}
	case 24:
	case 32: {
		DWORD *src, *dst;
		for (y = 0; y < in->height; y++) {
			for (x = 0; x < in->width; x++) {
				m = x + r * sin(y * th / in->height);
				n = y + r * cos(x * th / in->width);
				src = (DWORD *)GETOFFSET_PIXEL(in, m, n);
				dst = (DWORD *)GETOFFSET_PIXEL(out, x, y);
				if ((m > 0) && (m < in->width) && (n > 0) && (n < in->height)) {
					*dst = *src;
				} else {
					*dst = 0;
				}
			}
		}
	}}
}



// うねうねクロスフェード
static void ec31_cb(surface_t *sfsrc, surface_t *sfdst) {
	int curstep, maxstep;
	surface_t *st, *dt;
	
	maxstep = 32;
	curstep = maxstep * (ecp.curtime - ecp.sttime)/ (ecp.edtime - ecp.sttime);
	st = sf_dup2(sfsrc, FALSE, FALSE);
	dt = sf_dup2(sfdst, FALSE, FALSE);
	
	if (curstep < 16) {
		uneune(sfsrc, st, curstep);
		uneune(sfdst, dt, curstep);
		gre_Blend(sf0, 0, 0, st, 0, 0, dt, 0, 0, st->width, st->height, curstep * 8);
	} else {
		uneune(sfsrc, st, 32-curstep);
		uneune(sfdst, dt, 32-curstep);
		gre_Blend(sf0, 0, 0, st, 0, 0, dt, 0, 0, st->width, st->height, curstep * 8);
	}
	sf_free(st);
	sf_free(dt);
	ags_updateFull();
}

