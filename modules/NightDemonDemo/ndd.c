/*
 * ndd.c: 
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
/* $Id: ndd.c,v 1.2 2003/11/09 15:06:12 chikama Exp $ */

#include <stdio.h>
#include <glib.h>

#include "portab.h"
#include "ags.h"
#include "alk.h"
#include "system.h"
#include "surface.h"
#include "music_client.h"
#include "counter.h"
#include "ngraph.h"
#include "imput.h"

// 0: AliceSoft
// 1: Opening
// 2: Meteo

struct nddemo {
	char **fn;
	alk_t *alk;
	surface_t *sf[10];
};

static struct nddemo ndd;
static int ndemo_alk[3] = {2, 3, 4};
static int ndemo_mus[3] = {0, 1, 85};
static int ndemo_nums[3] = {150, 603, 467};

extern surface_t *jpeg2surface(FILE *fp, int offset);

// ALKファイルの登録
void ndd_init(char *files[], int n) {
	int i;
	ndd.fn = g_new(char *, n);

	for (i = 1; i <= n; i++) {
		ndd.fn[i] = files[i];
	}
}

// デモ本体
void ndd_run(int demonum) {
	FILE *fp;
	alk_t *alk;
	int i, start, ct;
	
	// ファイルオフセットだけ alk からもらう
	alk = alk_new(ndd.fn[ndemo_alk[demonum]]);
	if (alk == NULL) return;
	
	// libjpeg には FILE* を渡す
	if (NULL == (fp = fopen(ndd.fn[ndemo_alk[demonum]], "rb"))) {
		WARNING("%s not found\n", ndd.fn[ndemo_alk[demonum]]);
		return;
	}
	
	// wait keyup
	while (sys_getInputInfo());
	
	mus_bgm_play(ndemo_mus[demonum], 0, 100);
	start = get_high_counter(SYSTEMCOUNTER_MSEC);
	ct = 0;
	for (i = 0; i <= ndemo_nums[demonum];) {
		int offset;
		int cur1, cur = get_high_counter(SYSTEMCOUNTER_MSEC);
		
		offset = alk->offset[i];
		
		jpeg2surface(fp, offset); ct++;
		ags_updateFull();
		
		cur1 = get_high_counter(SYSTEMCOUNTER_MSEC);
		if (cur1 - cur < 33) {
			if (sys_keywait(33 - (cur1 - cur), TRUE)) break;
		} else {
			if (sys_getInputInfo()) break;
		}
		
		i = (cur - start) / 33;
	}
	
	NOTICE("%d/%d processed\n", ct, ndemo_nums[demonum]);
	mus_bgm_stop(ndemo_mus[demonum], 0);
	
	fclose(fp);
	alk_free(alk);
}
