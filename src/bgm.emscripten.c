/*
 * bgm.emscripten.c  BGM (*BA.ALD) emscripten implementation
 *
 * Copyright (C) 2021 <KichikuouChrome@gmail.com>
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
#include <stdio.h>
#include <emscripten.h>
#include "portab.h"
#include "nact.h"
#include "bgm.h"
#include "bgi.h"
#include "system.h"

bool musbgm_init(DRIFILETYPE type, int base_no) {
	if (type == DRIFILE_BGM)
		return bgi_read(nact->files.bgi);
	else
		EM_ASM({ xsystem35.cdPlayer.setBGMLoader($0, $1); }, type, base_no);
	return true;
}

void musbgm_exit(void) {
	musbgm_stopall(0);
}

void musbgm_reset(void) {
	musbgm_stopall(0);
}

EM_JS(bool, musbgm_play, (int no, int time, int vol, int loop_count), {
	xsystem35.cdPlayer.fade(time * 10, vol / 100);
	xsystem35.cdPlayer.play(no, loop_count == 0);
	return 1;
});

EM_JS(void, musbgm_stop, (int no, int time), {
	xsystem35.cdPlayer.stop(time * 10);
});

EM_JS(void, musbgm_fade, (int no, int time, int vol), {
	xsystem35.cdPlayer.fade(time * 10, vol / 100);
});

int musbgm_getpos(int no) {
	int t = EM_ASM_INT_V( return xsystem35.cdPlayer.getPosition(); );
	if (!t || (t & 0xff) != no)
		return 0;
	return (t >> 8) * 100 / 75;  // frame -> 10ms
}

int musbgm_getlen(int no) {
	bgi_t *bgi = bgi_find(no);
	if (!bgi) return 0;

	// FIXME: This assumes 44.1kHz
	return bgi->len / 441;
}

bool musbgm_isplaying(int no) {
	int t = EM_ASM_INT_V( return xsystem35.cdPlayer.getPosition(); );
	return ((t & 0xff) == no);
}

void musbgm_stopall(int time) {
	musbgm_stop(0, time);
}

void musbgm_wait(int no, int timeout) {
	for (int i = 0; i * 16 < timeout * 10; i++) {
		if (!musbgm_isplaying(no))
			break;
		sys_wait_vsync();
	}
}
