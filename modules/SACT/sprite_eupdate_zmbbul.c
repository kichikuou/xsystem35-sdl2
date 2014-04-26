#include "ngraph.h"
#include "graph.h"

static surface_t *ec10_sf[6];


// ズームブレンドブラー
static void ec10_cb(surface_t *sfsrc, surface_t *sfdst) {
	int curstep, maxstep, turstep;
	surface_t *sf;
	int i, sx, sy, sw, sh;
	
	maxstep = sqrt((sfsrc->width - (sfsrc->width/10))*(sfsrc->width - (sfsrc->width/10))+
		       (sfsrc->height - (sfsrc->height/10))*(sfsrc->height - (sfsrc->height/10)));
	
	curstep = maxstep * (ecp.curtime - ecp.sttime)/ (ecp.edtime - ecp.sttime);
	turstep = maxstep / 2;
	WARNING("step = %d/%d\n", curstep, maxstep);
	
	if (curstep > turstep) {
		// 帰り
		sx = (maxstep - curstep) * (sfsrc->width - sfsrc->width/10) / maxstep;
		sy = (maxstep - curstep) * (sfsrc->height - sfsrc->height/10) / maxstep;
		sw = sfsrc->width - sx*2;
		sh = sfsrc->height - sy*2;
	} else {
		// 行き
		sx = curstep * (sfsrc->width - sfsrc->width/10) / maxstep;
		sy = curstep * (sfsrc->height - sfsrc->height/10) / maxstep;
		sw = sfsrc->width - sx*2;
		sh = sfsrc->height - sy*2;
	}

	sf = ec10_sf[0];
	ec10_sf[0] = ec10_sf[1];
	ec10_sf[1] = ec10_sf[2];
	ec10_sf[2] = ec10_sf[3];
	ec10_sf[3] = ec10_sf[4];
	ec10_sf[4] = ec10_sf[5];
	ec10_sf[5] = sf;

	if (ec10_sf[0] == NULL) {
		ec10_sf[0] = sf_dup2(sfsrc, FALSE, FALSE);
		sf = ec10_sf[0];
		sf->has_alpha = FALSE;
		gr_copy_stretch(sf, 0, 0, sf->width, sf->height, sfsrc, sx, sy, sw, sh);
		gr_bright_dst_only(sf, 0, 0, sf->width, sf->height, 255/6);
		return;
	}
	
	if (curstep > turstep) {
		gr_copy_stretch(sf, 0, 0, sf->width, sf->height, sfdst, sx, sy, sw, sh);
	} else {
		gr_copy_stretch(sf, 0, 0, sf->width, sf->height, sfsrc, sx, sy, sw, sh);
	}
	
	gr_bright_dst_only(sf, 0, 0, sf->width, sf->height, 255/6);
	
	sf_copyall(sf0, sf);

	for (i = 0; i < 5; i++) {
		gre_BlendScreen(sf0, 0, 0, sf0, 0, 0, ec10_sf[i], 0, 0,
				sf0->width, sf0->height);
	}
	
	ags_updateFull();
}


static void ec10_prepare(surface_t *src, surface_t *dst) {
	int i;
	
	for (i = 0; i < 6; i++) {
		ec10_sf[i] = NULL;
	}
}

static void ec10_drain(surface_t *src, surface_t *dst) {
	int i;
	
	for (i = 0; i < 6; i++) {
		sf_free(ec10_sf[i]);
	}
}
