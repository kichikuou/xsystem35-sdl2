// 11 と 19 で共有
static surface_t *ec11_ss[6];
static surface_t *ec11_sd[6];

// 線形ブラー
static void ec11_cb(surface_t *sfsrc, surface_t *sfdst) {
	int curstep, maxstep = 6; 
	curstep = maxstep * (ecp.curtime - ecp.sttime)/ (ecp.edtime - ecp.sttime);
	if (curstep < 6) {
		gre_Blend(sf0, 0, 0, ec11_ss[curstep], 0, 0, ec11_sd[5-curstep], 0, 0, sfsrc->width, sfsrc->height, (curstep+1)*256/7);
		ags_updateFull();
	}
}

static void ec11_prepare(surface_t *src, surface_t *dst) {
	int i;
	
	ec11_ss[0] = sf_create_surface(src->width, src->height, src->depth);
	ec11_sd[0] = sf_create_surface(src->width, src->height, src->depth);
	gr_buller(ec11_ss[0], 0, 0, src, 0, 0, src->width, src->height, 1 << 2);
	gr_buller(ec11_sd[0], 0, 0, dst, 0, 0, dst->width, dst->height, 1 << 2);
	for (i = 1; i < 6; i++) {
		ec11_ss[i] = sf_create_surface(src->width, src->height, src->depth);
		ec11_sd[i] = sf_create_surface(dst->width, dst->height, dst->depth);
		gr_buller(ec11_ss[i], 0, 0, ec11_ss[i-1], 0, 0, src->width, src->height, 1 << (i+2));
		gr_buller(ec11_sd[i], 0, 0, ec11_sd[i-1], 0, 0, dst->width, dst->height, 1 << (i+2));
	}
}

static void ec11_drain(surface_t *src, surface_t *dst) {
	int i;
	
	for (i = 0; i < 6; i++) {
		sf_free(ec11_ss[i]);
		sf_free(ec11_sd[i]);
	}
}

// 縦線形ブラー
static void ec19_cb(surface_t *sfsrc, surface_t *sfdst) {
	ec11_cb(sfsrc, sfdst);
}

static void ec19_prepare(surface_t *src, surface_t *dst) {
	int i;
	
	ec11_ss[0] = sf_create_surface(src->width, src->height, src->depth);
	ec11_sd[0] = sf_create_surface(src->width, src->height, src->depth);
	gr_buller_v(ec11_ss[0], 0, 0, src, 0, 0, src->width, src->height, 1 << 2);
	gr_buller_v(ec11_sd[0], 0, 0, dst, 0, 0, dst->width, dst->height, 1 << 2);
	for (i = 1; i < 6; i++) {
		ec11_ss[i] = sf_create_surface(src->width, src->height, src->depth);
		ec11_sd[i] = sf_create_surface(dst->width, dst->height, dst->depth);
		gr_buller_v(ec11_ss[i], 0, 0, ec11_ss[i-1], 0, 0, src->width, src->height, 1 << (i+2));
		gr_buller_v(ec11_sd[i], 0, 0, ec11_sd[i-1], 0, 0, dst->width, dst->height, 1 << (i+2));
	}
}

static void ec19_drain(surface_t *src, surface_t *dst) {
	int i;
	
	for (i = 0; i < 6; i++) {
		sf_free(ec11_ss[i]);
		sf_free(ec11_sd[i]);
	}
}
