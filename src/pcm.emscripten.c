/*
 * pcm.emscripten.c  PCM audio for emscripten
 *
 * Copyright (C) 2019 <KichikuouChrome@gmail.com>
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

#include <emscripten.h>

#include "portab.h"
#include "music_pcm.h"
#include "music_private.h"
#include "nact.h"
#include "audio_meta.h"

bool muspcm_init(int audio_buffer_size) {
	wai_load(nact->files.wai);
	return true;
}

// Volume-valancer channel for PCM file number `no`, based on the WAI file.
static int pcm_channel(int no) {
	if (!wai_loaded())
		return SE_VOLVAL_CH;
	int ch = wai_mixch(no);
	if ((unsigned)ch >= 16)  // guard against bad WAI data
		ch = 0;
	return ch;
}

void muspcm_exit(void) {
	// do nothing
}

EM_JS(void, muspcm_reset, (void), {
	xsystem35.audio.pcm_reset();
});

EM_ASYNC_JS(bool, pcm_load_no_js, (int slot, int no, int ch), {
	return await xsystem35.audio.pcm_load(slot, no, ch);
});

bool muspcm_load_no(int slot, int no) {
	return pcm_load_no_js(slot, no, pcm_channel(no));
}

EM_ASYNC_JS(bool, muspcm_load_bgm, (int slot, int no), {
	return await xsystem35.audio.pcm_load_bgm(slot, no, BGM_VOLVAL_CH);
});

EM_ASYNC_JS(bool, muspcm_load_data, (int slot, uint8_t *buf, uint32_t len), {
	return await xsystem35.audio.pcm_load_data(slot, buf, len, SE_VOLVAL_CH);
});

EM_ASYNC_JS(bool, muspcm_load_mixlr, (int slot, int noL, int noR), {
	return await xsystem35.audio.pcm_load_mixlr(slot, noL, noR, SE_VOLVAL_CH);
});

EM_JS(void, muspcm_unload, (int slot), {
	xsystem35.audio.pcm_unload(slot);
});

EM_JS(bool, muspcm_start, (int slot, int loop), {
	return xsystem35.audio.pcm_start(slot, loop);
});

EM_JS(void, muspcm_stop, (int slot), {
	xsystem35.audio.pcm_stop(slot);
});

EM_JS(void, muspcm_fadeout, (int slot, int msec), {
	xsystem35.audio.pcm_fadeout(slot, msec);
});

EM_JS(void, muspcm_pause, (int slot), {
	console.log('muspcm_pause: not implemented');
});

EM_JS(void, muspcm_unpause, (int slot), {
	console.log('muspcm_unpause: not implemented');
});

EM_JS(int, muspcm_getpos, (int slot), {
	return xsystem35.audio.pcm_getpos(slot);
});

EM_JS(void, muspcm_setvol, (int slot, int lv), {
	xsystem35.audio.pcm_setvol(slot, lv);
});

EM_JS(int, muspcm_getwavelen, (int slot), {
	var len = xsystem35.audio.pcm_getwavelen(slot);
	return len > 65535 ? 65535 : len;
});

EM_JS(bool, muspcm_isplaying, (int slot), {
	return xsystem35.audio.pcm_isplaying(slot);
});

EM_ASYNC_JS(void, muspcm_waitend, (int slot), {
	await xsystem35.audio.pcm_waitend(slot);
});

EM_JS(void, pcm_set_valance, (const int *vols, int num), {
	xsystem35.audio.setValance(HEAP32.subarray(vols >> 2, (vols >> 2) + num));
});

void muspcm_reapply_valance(void) {
	pcm_set_valance(prv.volval, 16);
}
