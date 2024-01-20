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

#include "portab.h"
#include "profile.h"
#include "system.h"

#define RC_NAME ".xsys35rc"
#define RC_LINE_CHARS_MAX 256

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

int load_profile(void)
{
	char *home_dir = getenv("HOME");
#ifdef _WIN32
	if (!home_dir)
		home_dir = getenv("USERPROFILE");
#endif
	if (home_dir) {
		char profile_path[PATH_MAX];
		sprintf(profile_path, "%s/%s", home_dir, RC_NAME);
		load_rc_file(profile_path);
	}
	return load_rc_file(RC_NAME);
}

const char *get_profile(const char *name) {
	struct profile_kv *kv;

	if (!(kv = check_profile(name)))
		return NULL;
	return kv->value;
}

bool get_boolean_profile(const char *name, bool *out) {
	const char *value = get_profile(name);
	if (!value)
		return false;
	const char *yes_values[] = {"yes", "true", "on", "1", NULL};
	const char *no_values[] = {"no", "false", "off", "0", NULL};
	for (int i = 0; yes_values[i]; i++) {
		if (!strcasecmp(value, yes_values[i])) {
			*out = true;
			return true;
		}
	}
	for (int i = 0; no_values[i]; i++) {
		if (!strcasecmp(value, no_values[i])) {
			*out = false;
			return true;
		}
	}
	WARNING("invalid profile value for %s: \"%s\"\n", name, value);
	return false;
}
