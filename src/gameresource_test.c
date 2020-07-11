/*
 * Copyright (C) 2020 <KichikuouChrome@gmail.com>
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
#undef NDEBUG
#include "gameresource.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void gameresource_test(void) {
#ifdef _WIN32
	_putenv("HOME=/home/kichikuou");
#else
	setenv("HOME", "/home/kichikuou", 1);
#endif

	GameResource gr;
	assert(initGameResource(&gr, "testdata/test.gr"));

	assert(gr.cnt[DRIFILE_SCO] == 1);
	assert(strcmp(gr.game_fname[DRIFILE_SCO][0], "fooSA.ALD") == 0);
	assert(gr.cnt[DRIFILE_CG] == 2);
	assert(strcmp(gr.game_fname[DRIFILE_CG][0], "fooGA.ALD") == 0);
	assert(strcmp(gr.game_fname[DRIFILE_CG][1], "fooGB.ALD") == 0);
	assert(gr.cnt[DRIFILE_WAVE] == 26);
	assert(strcmp(gr.game_fname[DRIFILE_WAVE][0], "fooWA.ALD") == 0);
	for (int i = 1; i <= 24; i++)
		assert(gr.game_fname[DRIFILE_WAVE][i] == NULL);
	assert(strcmp(gr.game_fname[DRIFILE_WAVE][25], "fooWZ.ALD") == 0);
	assert(gr.cnt[DRIFILE_MIDI] == 3);
	assert(strcmp(gr.game_fname[DRIFILE_MIDI][2], "fooMC.ALD") == 0);
	assert(strcmp(gr.game_fname[DRIFILE_DATA][0], "file name with whitespaces") == 0);
	assert(strcmp(gr.game_fname[DRIFILE_RSC][0], "this line has whitespaces at end of line") == 0);
	assert(strcmp(gr.game_fname[DRIFILE_BGM][0], "/path/to/fooBA.ALD") == 0);

	assert(strcmp(gr.ain, "System39.ain") == 0);
	assert(strcmp(gr.wai, "foo_WA.WAI") == 0);
	assert(strcmp(gr.bgi, "foo_BA.BGI") == 0);
	assert(strcmp(gr.sact01, "SACTEFAM.KLD") == 0);
	assert(strcmp(gr.init, "System39.ini") == 0);
	assert(strcmp(gr.alk[0], "0.alk") == 0);
	for (int i = 1; i <= 8; i++)
		assert(gr.alk[i] == NULL);
	assert(strcmp(gr.alk[9], "9.alk") == 0);

	assert(strcmp(gr.save_fname[0], "/home/kichikuou/save/foo_sa.asd") == 0);
	assert(strcmp(gr.save_fname[25], "/tmp/foo_sz.asd") == 0);
	// save_path is determined based on save_fname[0].
	assert(strcmp(gr.save_path, "/home/kichikuou/save") == 0);
}
