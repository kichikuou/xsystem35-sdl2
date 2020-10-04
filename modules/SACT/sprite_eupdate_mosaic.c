


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

	SDL_Surface *stt = com2surface(st);
	SDL_Surface *dtt = com2surface(dt);
	image_Mosaic(stt, 0, 0, stt->w, stt->h, 0, 0, slices[curstep]);
	image_Mosaic(dtt, 0, 0, dtt->w, dtt->h, 0, 0, slices[curstep]);
	SDL_FreeSurface(stt);
	SDL_FreeSurface(dtt);

	gre_Blend(sf0, 0, 0, st, 0, 0, dt, 0, 0, st->width, st->height, curstep*8);
	ags_updateFull();
	
	sf_free(st);
	sf_free(dt);
	
	ecp.oldstep = curstep;
}

