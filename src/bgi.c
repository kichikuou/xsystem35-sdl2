/*
 * bgi.c: BGI (BGM information) parser
 *
 * Copyright (C) 1997-1998 Masaki Chikama (Wren) <chikama@kasumi.ipl.mech.nagoya-u.ac.jp>
 *               1998-                           <masaki-c@is.aist-nara.ac.jp>
 *               2021 kichikuou <KichikuouChrome@gmail.com>
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
#include "config.h"

#include <stdio.h>

#include "portab.h"
#include "system.h"
#include "bgi.h"

#define BGI_MAX 100

static int bgi_nfile;
static bgi_t bgi_data[BGI_MAX];

static char *bgi_gets(char *buf, int n, FILE *fp) {
	char *s = buf;
	int c;
	while (--n > 0 && (c = fgetc(fp)) != EOF) {
		c = c >> 4 | (c & 0xf) << 4;  // decrypt
		*s++ = c;
		if (c == '\n')
			break;
	}
	if (s == buf && c == EOF)
		return NULL;
	*s = '\0';
	return buf;
}

int bgi_read(const char *path) {
	if (!path)
		return NG;

	FILE *fp = fopen(path, "rb");
	if (!fp) {
		WARNING("Could not open %s\n", path);
		return NG;
	}

	char buf[100];
	while (bgi_nfile < BGI_MAX && bgi_gets(buf, sizeof(buf), fp)) {
		int terminator;
		if (sscanf(buf, " %d, %d, %d, %d, %d",
				   &bgi_data[bgi_nfile].no,
				   &bgi_data[bgi_nfile].loopno,
				   &bgi_data[bgi_nfile].looptop,
				   &bgi_data[bgi_nfile].len,
				   &terminator) != 5
			|| terminator != -1) {
			continue;
		}
		bgi_nfile++;
	}
	return OK;
}

bgi_t *bgi_find(int no) {
	for (int i = 0; i < bgi_nfile; i++) {
		if (bgi_data[i].no == no)
			return &bgi_data[i];
	}
	return NULL;
}
