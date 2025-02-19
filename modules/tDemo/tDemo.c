/*
 * Copyright (C) 1997-1998 Masaki Chikama (Wren) <chikama@kasumi.ipl.mech.nagoya-u.ac.jp>
 *               1998-                           <masaki-c@is.aist-nara.ac.jp>
 *               2024      kichikuou             <KichikuouChrome@gmail.com>
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
#include "config.h"

#include <stdio.h>

#include "portab.h"
#include "system.h"
#include "xsystem35.h"
#include "modules.h"
#include "nact.h"
#include "alk.h"
#include "input.h"
#include "bgm.h"
#include "sdl_core.h"
#include "cg.h"
#include "jpeg.h"

#define TDEMO_MUSIC_NO 1
#define FPS 30

static void Init() {
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int *var = getCaliVariable();
	
	*var = 1;
	
	TRACE("tDemo.Init %d,%d,%d,%p:", p1, p2, p3, var);
}

static void SetKeyCancelFlag() {
	int cancelflag = getCaliValue();
	
	TRACE_UNIMPLEMENTED("tDemo.SetKeyCancelFlag %d:", cancelflag);
}

static void SetLoopFlag() {
	/* Loop Flag */
	int loopflag = getCaliValue(); /* 0 なら無限繰り返し */
	
	TRACE_UNIMPLEMENTED("tDemo.SetLoopFlag %d:", loopflag);
}

static void Run() {
	alk_t *alk = alk_new("tDEMO.alk");
	if (!alk) {
		WARNING("Cannot open tDEMO.alk");
		return;
	}

	while (sys_getInputInfo());  // wait for key up

	musbgm_play(TDEMO_MUSIC_NO, 0, 100, 0);

	uint32_t start = sdl_getTicks();
	while (!nact->is_quit) {
		int i = (sdl_getTicks() - start) / (1000 / FPS);
		if (i >= alk->datanum)
			break;

		cgdata *cg = jpeg_extract(alk->entries[i].data, alk->entries[i].size);
		if (cg) {
			SDL_Surface *sf = SDL_CreateRGBSurfaceWithFormatFrom(cg->pic, cg->width, cg->height, 24, cg->width * 3, SDL_PIXELFORMAT_RGB24);
			SDL_BlitSurface(sf, NULL, main_surface, NULL);
			ags_updateFull();
			SDL_FreeSurface(sf);
			cgdata_free(cg);
		} else {
			WARNING("Cannot decode CG %d", i);
		}

		int wait_ms = (i + 1) * (1000 / FPS) - (sdl_getTicks() - start);
		if (wait_ms > 0) {
			if (sys_keywait(wait_ms, KEYWAIT_CANCELABLE))
				break;
		} else {
			sdl_updateScreen();
			if (sys_getInputInfo())
				break;
		}
	}

	musbgm_stop(TDEMO_MUSIC_NO, 0);

	alk_free(alk);

	TRACE("tDemo.Run:");
}

static const ModuleFunc functions[] = {
	{"Init", Init},
	{"Run", Run},
	{"SetKeyCancelFlag", SetKeyCancelFlag},
	{"SetLoopFlag", SetLoopFlag},
};

const Module module_tDemo = {"tDemo", functions, sizeof(functions) / sizeof(ModuleFunc)};
