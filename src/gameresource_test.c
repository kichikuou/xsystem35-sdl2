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
#include "gameresource.h"
#include "unittest.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void initGameResource_test(void) {
	GameResource gr;
	ASSERT_TRUE(initGameResource(&gr, "testdata/test.gr"));

	ASSERT_EQUAL(gr.cnt[DRIFILE_SCO], 1);
	ASSERT_STRCMP(gr.game_fname[DRIFILE_SCO][0], "fooSA.ALD");
	ASSERT_EQUAL(gr.cnt[DRIFILE_CG], 2);
	ASSERT_STRCMP(gr.game_fname[DRIFILE_CG][0], "fooGA.ALD");
	ASSERT_STRCMP(gr.game_fname[DRIFILE_CG][1], "fooGB.ALD");
	ASSERT_EQUAL(gr.cnt[DRIFILE_WAVE], 26);
	ASSERT_STRCMP(gr.game_fname[DRIFILE_WAVE][0], "fooWA.ALD");
	for (int i = 1; i <= 24; i++)
		ASSERT_NULL(gr.game_fname[DRIFILE_WAVE][i]);
	ASSERT_STRCMP(gr.game_fname[DRIFILE_WAVE][25], "fooWZ.ALD");
	ASSERT_EQUAL(gr.cnt[DRIFILE_MIDI], 3);
	ASSERT_STRCMP(gr.game_fname[DRIFILE_MIDI][2], "fooMC.ALD");
	ASSERT_STRCMP(gr.game_fname[DRIFILE_DATA][0], "file name with whitespaces");
	ASSERT_STRCMP(gr.game_fname[DRIFILE_RSC][0], "this line has whitespaces at end of line");
	ASSERT_STRCMP(gr.game_fname[DRIFILE_BGM][0], "/path/to/fooBA.ALD");

	ASSERT_STRCMP(gr.ain, "System39.ain");
	ASSERT_STRCMP(gr.wai, "foo_WA.WAI");
	ASSERT_STRCMP(gr.bgi, "foo_BA.BGI");
	ASSERT_STRCMP(gr.sact01, "SACTEFAM.KLD");
	ASSERT_STRCMP(gr.init, "System39.ini");
	ASSERT_STRCMP(gr.alk[0], "0.alk");
	for (int i = 1; i <= 8; i++)
		ASSERT_NULL(gr.alk[i]);
	ASSERT_STRCMP(gr.alk[9], "9.alk");

	ASSERT_STRCMP(gr.save_fname[0], "/home/kichikuou/save/foo_sa.asd");
	for (int i = 1; i <= 24; i++)
		ASSERT_NULL(gr.save_fname[i]);
	ASSERT_STRCMP(gr.save_fname[25], "/tmp/foo_sz.asd");
	// save_path is determined based on save_fname[0].
	ASSERT_STRCMP(gr.save_path, "/home/kichikuou/save");
	ASSERT_STRCMP(gr.msgskip, "msgskip.asd");
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
	GameResource gr;
	ASSERT_TRUE(initGameResourceFromDir(&gr, (DIR *)&dir, mockReaddir));
	ASSERT_EQUAL(gr.cnt[DRIFILE_SCO], 2);
	ASSERT_STRCMP(gr.game_fname[DRIFILE_SCO][0], "ADISK.ALD");
	ASSERT_STRCMP(gr.game_fname[DRIFILE_SCO][1], "FOOSB.ALD");
	ASSERT_EQUAL(gr.cnt[DRIFILE_CG], 26);
	ASSERT_STRCMP(gr.game_fname[DRIFILE_CG][25], "foogz.ald");
	ASSERT_EQUAL(gr.cnt[DRIFILE_WAVE], 1);
	ASSERT_STRCMP(gr.game_fname[DRIFILE_WAVE][0], "WA.ALD");
	for (int i = 0; i < 26; i++) {
		char buf[16];
		sprintf(buf, "%csleep.asd", 'a' + i);
		ASSERT_STRCMP(gr.save_fname[i], buf);
	}
	ASSERT_STRCMP(gr.save_path, ".");
	ASSERT_STRCMP(gr.ain, "SYSTEM39.AIN");
	ASSERT_STRCMP(gr.wai, "foo_WA.WAI");
	ASSERT_STRCMP(gr.bgi, "foo_BA.BGI");
	ASSERT_STRCMP(gr.sact01, "SACTEFAM.KLD");
	ASSERT_STRCMP(gr.init, "System39.ini");
	ASSERT_STRCMP(gr.alk[0], "0.alk");
	ASSERT_STRCMP(gr.alk[1], "foo1.alk");
	for (int i = 2; i < 10; i++)
		ASSERT_NULL(gr.alk[i]);
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
