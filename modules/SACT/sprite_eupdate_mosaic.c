


// クロスフェードモザイク
static void ec6_cb(surface_t *src, surface_t *dst) {
	int curstep, maxstep = 32;
	surface_t *st, *dt;
        static int slices[32]={4,8,12,16,20,28,36,40,44,48,56,64,72,80,88,96,
                               88,80,72,64,56,48,44,40,36,28,24,20,16,12,8,4};
	

	curstep = maxstep * (ecp.curtime - ecp.sttime)/ (ecp.edtime - ecp.sttime);
	if (ecp.oldstep == curstep) {
		usleep(0);
		return;
	}
	st = sf_dup(src);
	dt = sf_dup(dst);

	image_Mosaic(st, 0, 0, st->width, st->height, 0, 0, slices[curstep]);
	image_Mosaic(dt, 0, 0, dt->width, dt->height, 0, 0, slices[curstep]);
	
	gre_Blend(sf0, 0, 0, st, 0, 0, dt, 0, 0, st->width, st->height, curstep*8);
	ags_updateFull();
	
	sf_free(st);
	sf_free(dt);
	
	ecp.oldstep = curstep;
}

