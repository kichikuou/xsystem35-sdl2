/*
 * pcm.dummy.c  Dummy PCM implementation
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
#include "portab.h"
#include "music_pcm.h"

int muspcm_init(int audio_buffer_size) { return NG; }
int muspcm_exit(void) { return NG; }
int muspcm_load_no(int slot, int no) { return NG; }
int muspcm_load_mixlr(int slot, int noL, int noR) { return NG; }
int muspcm_unload(int slot) { return NG; }
int muspcm_start(int slot, int loop) { return NG; }
int muspcm_stop(int slot) { return NG; }
int muspcm_fadeout(int slot, int msec) { return NG; }
int muspcm_pause(int slot) { return NG; }
int muspcm_unpause(int slot) { return NG; }
int muspcm_getpos(int slot) { return NG; }
int muspcm_setvol(int dev, int slot, int lv) { return NG; }
int muspcm_getwavelen(int slot) { return NG; }
boolean muspcm_isplaying(int slot) { return FALSE; }
int muspcm_waitend(int slot) { return NG; }
