/*
 * NightDemonDemo.c: 
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
/* $Id: NightDemonDemo.c,v 1.2 2003/09/05 14:15:49 chikama Exp $ */

#include "config.h"

#include <stdio.h>

#include "portab.h"
#include "system.h"
#include "xsystem35.h"
#include "modules.h"
#include "nact.h"
#include "alk.h"
#include "music.h"
#include "sdl_core.h"
#include "input.h"
#include "ngraph.h"
#include "cg.h"
#ifdef HAVE_JPEG
#include "jpeg.h"
#endif

#ifdef HAVE_JPEG

#define FPS 30

static const int ndemo_mus[3] = {0, 1, 85};

static void ndd_run(int demonum) {
	const char *alk_path = nact->files.alk[demonum + 1];
	alk_t *alk = alk_new(alk_path);
	if (alk == NULL) return;

	while (sys_getInputInfo());  // wait for key up

	int mus = ndemo_mus[demonum];
	if (mus)
		mus_bgm_play(mus, 0, 100);

	uint32_t start = sdl_getTicks();
	while (!nact->is_quit) {
		int i = (sdl_getTicks() - start) / (1000 / FPS);
		if (i >= alk->datanum)
			break;

		cgdata *cg = jpeg_extract(alk->entries[i].data, alk->entries[i].size);
		if (cg) {
			gr_drawimage24(sf0, cg, 0, 0);
			ags_updateFull();
			cgdata_free(cg);
		} else {
			WARNING("Cannot decode CG %d in %s\n", i, alk_path);
		}

		int wait_ms = (i + 1) * (1000 / FPS) - (sdl_getTicks() - start);
		if (wait_ms > 0) {
			if (sys_keywait(wait_ms, KEYWAIT_CANCELABLE))
				break;
		} else {
			if (sys_getInputInfo())
				break;
		}
	}

	if (mus)
		mus_bgm_stop(ndemo_mus[demonum], 0);

	alk_free(alk);
}

#endif

static void Init() {
	int p1 = getCaliValue(); /* ISys3x */
	int p2 = getCaliValue(); /* IWinMsg */
	int p3 = getCaliValue(); /* ITimer */
	int *var = getCaliVariable();
	
	*var = 1;
	
	DEBUG_COMMAND("NightDemonDemo.Init %d,%d,%d,%p:\n", p1, p2, p3, var);
}

static void Run() {
	int p1 = getCaliValue(); // デモ番号 0,1,2
	int p2 = getCaliValue();
	
#ifdef HAVE_JPEG
	ndd_run(p1);
#endif
	
	DEBUG_COMMAND("NightDemonDemo.Run %d,%d:\n", p1, p2);
}

static const ModuleFunc functions[] = {
	{"Init", Init},
	{"Run", Run},
};

const Module module_NightDemonDemo = {"NightDemonDemo", functions, sizeof(functions) / sizeof(ModuleFunc)};
