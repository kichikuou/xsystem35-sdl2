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

EM_JS(int, muspcm_init, (int audio_buffer_size), {
	return xsystem35.Status.OK;
});

EM_JS(int, muspcm_exit, (void), {
	return xsystem35.Status.OK;
});

EM_JS(int, muspcm_reset, (void), {
	return xsystem35.audio.pcm_reset();;
});

EM_JS(int, muspcm_load_no, (int slot, int no), {  // async
	return xsystem35.audio.pcm_load(slot, no);
});

EM_JS(int, muspcm_load_mixlr, (int slot, int noL, int noR), {  // async
	return xsystem35.audio.pcm_load_mixlr(slot, noL, noR);
});

EM_JS(int, muspcm_unload, (int slot), {
	return xsystem35.audio.pcm_unload(slot);
});

EM_JS(int, muspcm_start, (int slot, int loop), {
	return xsystem35.audio.pcm_start(slot, loop);
});

EM_JS(int, muspcm_stop, (int slot), {
	return xsystem35.audio.pcm_stop(slot);
});

EM_JS(int, muspcm_fadeout, (int slot, int msec), {
	return xsystem35.audio.pcm_fadeout(slot, msec);
});

EM_JS(int, muspcm_pause, (int slot), {
	console.log('muspcm_pause: not implemented');
	return xsystem35.Status.NG;
});

EM_JS(int, muspcm_unpause, (int slot), {
	console.log('muspcm_unpause: not implemented');
	return xsystem35.Status.NG;
});

EM_JS(int, muspcm_getpos, (int slot), {
	return xsystem35.audio.pcm_getpos(slot);
});

EM_JS(int, muspcm_setvol, (int dev, int slot, int lv), {
	return xsystem35.audio.pcm_setvol(slot, lv);
});

EM_JS(int, muspcm_getwavelen, (int slot), {
	var len = xsystem35.audio.pcm_getwavelen(slot);
	return len > 65535 ? 65535 : len;
});

EM_JS(boolean, muspcm_isplaying, (int slot), {
	return xsystem35.audio.pcm_isplaying(slot);
});

EM_JS(int, muspcm_waitend, (int slot), {  // async
	return xsystem35.audio.pcm_waitend(slot);
});
