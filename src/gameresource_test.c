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
#include <unistd.h>

static void initGameResource_test(void) {
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
	for (int i = 1; i <= 24; i++)
		assert(gr.save_fname[i] == NULL);
	assert(strcmp(gr.save_fname[25], "/tmp/foo_sz.asd") == 0);
	// save_path is determined based on save_fname[0].
	assert(strcmp(gr.save_path, "/home/kichikuou/save") == 0);
}

static struct dirent *mockReaddir(DIR *dir) {
	static struct dirent ent;
	const char ***entries = (const char ***)dir;
	if (**entries) {
		strcpy(ent.d_name, **entries);
		(*entries)++;
		return &ent;
	}
	return NULL;
}

static const char *joinPath(const char *dir, const char *fname) {
	static char buf[512];
	sprintf(buf, "%s/%s", dir, fname);
	return buf;
}

static void initGameResourceFromDir_test(void) {
	const char *files[] = {
		"ADISK.ALD",
		"FOOSB.ALD",
		"foogz.ald",
		"WA.ALD",
		"unknownXA.ald",
		"a.ald",
		".ald",
		"a",
		"SYSTEM39.AIN",
		"foo_WA.WAI",
		"foo_BA.BGI",
		"SACTEFAM.KLD",
		"System39.ini",
		"foo1.alk",
		"0.alk",
		".alk",
		NULL
	};
	const char **dir = files;
	static char cwd[256];
	getcwd(cwd, sizeof(cwd));
	GameResource gr;
	assert(initGameResourceFromDir(&gr, (DIR *)&dir, mockReaddir));
	assert(gr.cnt[DRIFILE_SCO] == 2);
	assert(strcmp(gr.game_fname[DRIFILE_SCO][0], joinPath(cwd, "ADISK.ALD")) == 0);
	assert(strcmp(gr.game_fname[DRIFILE_SCO][1], joinPath(cwd, "FOOSB.ALD")) == 0);
	assert(gr.cnt[DRIFILE_CG] == 26);
	assert(strcmp(gr.game_fname[DRIFILE_CG][25], joinPath(cwd, "foogz.ald")) == 0);
	assert(gr.cnt[DRIFILE_WAVE] == 1);
	assert(strcmp(gr.game_fname[DRIFILE_WAVE][0], joinPath(cwd, "WA.ALD")) == 0);
	for (int i = 0; i < 26; i++) {
		char buf[16];
		sprintf(buf, "%csleep.asd", 'a' + i);
		assert(strcmp(gr.save_fname[i], joinPath(cwd, buf)) == 0);
	}
	assert(strcmp(gr.save_path, cwd) == 0);
	assert(strcmp(gr.ain, joinPath(cwd, "SYSTEM39.AIN")) == 0);
	assert(strcmp(gr.wai, joinPath(cwd, "foo_WA.WAI")) == 0);
	assert(strcmp(gr.bgi, joinPath(cwd, "foo_BA.BGI")) == 0);
	assert(strcmp(gr.sact01, joinPath(cwd, "SACTEFAM.KLD")) == 0);
	assert(strcmp(gr.init, joinPath(cwd, "System39.ini")) == 0);
	assert(strcmp(gr.alk[0], joinPath(cwd, "0.alk")) == 0);
	assert(strcmp(gr.alk[1], joinPath(cwd, "foo1.alk")) == 0);
	for (int i = 2; i < 10; i++)
		assert(gr.alk[i] == NULL);
}

void gameresource_test(void) {
#ifdef _WIN32
	_putenv("HOME=/home/kichikuou");
#else
	setenv("HOME", "/home/kichikuou", 1);
#endif

	initGameResource_test();
	initGameResourceFromDir_test();
}
