/*
 * profile.c  rcfile analizer
 *
 * Original GICQ Copyright 1998 Sean Gabriel <gabriel@montana.com>
 * ja extension Patch Copyright Takashi Mizuhiki <mizuhiki@cclub.cc.tut.ac.jp>
 * modified for xsystem35 Masaki Chikama (Wren) <masaki-c@is.aist-nata.ac.jp>
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
/* $Id: profile.c,v 1.6 2001/04/02 21:00:44 chikama Exp $ */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#ifdef _WIN32
#include <windows.h>
#include "win/resources.h"
#undef min
#undef max
#endif

#include "portab.h"
#include "profile.h"
#include "system.h"

#define RC_NAME ".xsys35rc"
#define RC_LINE_CHARS_MAX 256
#define REGKEY_PROFILE XSYSTEM35_REGKEY "\\profile"

struct profile_kv {
	char *name;
	char *value;
};

static struct profile_kv *profile = NULL;
static int profile_values = 0;

static struct profile_kv *check_profile(const char *name)
{
	for (int i = 0; i < profile_values; i++) {
		if (!strcmp(name, profile[i].name))
			return &profile[i];
	}
	return NULL;
}

static boolean insert_profile(const char *name, const char *value)
{
	struct profile_kv *kv;

	if ((kv = check_profile(name))) {
		free(kv->value);
		kv->value = strdup(value);
		return TRUE;
	}

	profile = realloc(profile, sizeof(struct profile_kv) * (profile_values + 1));
	if (!profile)
		NOMEMERR();

	profile[profile_values].name = strdup(name);
	profile[profile_values].value = strdup(value);
	profile_values++;
	return TRUE;
}

static int load_rc_file(const char *profile_path)
{
	FILE *fp;
	char rc_line[RC_LINE_CHARS_MAX];
	char *p, *q;
	int is_flag, line = 0;

	if (!(fp = fopen(profile_path, "r")))
		return 1;

	while (1) {
		if (!fgets(rc_line, sizeof(rc_line), fp)) {
			if (feof(fp))
				break;
			else {
				WARNING("%s: %s", profile_path, strerror(errno));
				return 1;
			}
		}
		line++;

		/* 読み込んだ行をパースする */
		p = q = rc_line;
		is_flag = FALSE;

		while (*q) {
			if (*q == ':')
				is_flag = TRUE;

			if (iscntrl(*q) || (!is_flag && isspace(*q)))
				q++;
			else
				*p++ = *q++;
		}

		*p = '\0';

		/* 行頭が # だった場合と、空行の場合は記憶しない */
		if (rc_line[0] == '#' || rc_line[0] == '\0')
			continue;

		if (!(p = strchr(rc_line, ':'))) {
			WARNING("Syntax Error in '%s' line %d.", profile_path, line);
			return 1;
		}

		*p = '\0';

		while (*++p)
			if (!isspace(*p) && !iscntrl(*p))
				break;

		if (!insert_profile(rc_line, p))
			return 1;
	}
	return 0;
}

#ifdef _WIN32
static void load_profile_from_registry(HKEY rootKey)
{
	HKEY hKey;
	LSTATUS err = RegOpenKeyEx(rootKey, REGKEY_PROFILE, 0, KEY_QUERY_VALUE, &hKey);
	if (err != ERROR_SUCCESS)
		return;

	for (DWORD index = 0;; index++) {
		char name[64];
		BYTE value[RC_LINE_CHARS_MAX];
		DWORD name_size = sizeof(name);
		DWORD value_size = sizeof(value);
		DWORD type;
		err = RegEnumValue(hKey, index, name, &name_size, NULL, &type, value, &value_size);
		if (err != ERROR_SUCCESS)
			break;
		if (type != REG_SZ)
			continue;
		insert_profile(name, value);
	}
}
#endif // _WIN32

int load_profile(void)
{
#ifdef _WIN32
	load_profile_from_registry(HKEY_LOCAL_MACHINE);
	load_profile_from_registry(HKEY_CURRENT_USER);
#endif // _WIN32

	char *home_dir;
	if ((home_dir = getenv("HOME"))) {
		char profile_path[PATH_MAX];
		sprintf(profile_path, "%s/%s", home_dir, RC_NAME);
		load_rc_file(profile_path);
	}
	return load_rc_file(RC_NAME);
}

char *get_profile(const char *name)
{
	struct profile_kv *kv;

	if (!(kv = check_profile(name)))
		return NULL;
	return kv->value;
}
