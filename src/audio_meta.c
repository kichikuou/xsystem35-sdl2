/*
 * audio_meta.c: BGI (BGM information) / WAI (wave information) parser
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
#include <stdlib.h>

#include "portab.h"
#include "system.h"
#include "audio_meta.h"
#include "LittleEndian.h"

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

bool bgi_read(const char *path) {
	if (!path)
		return false;

	FILE *fp = fopen(path, "rb");
	if (!fp) {
		WARNING("Could not open %s", path);
		return false;
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
	return true;
}

bgi_t *bgi_find(int no) {
	for (int i = 0; i < bgi_nfile; i++) {
		if (bgi_data[i].no == no)
			return &bgi_data[i];
	}
	return NULL;
}

static int *wai_channels;
static int wai_count;

static void wai_unload(void) {
	free(wai_channels);
	wai_channels = NULL;
	wai_count = 0;
}

bool wai_load(const char *path) {
	wai_unload();
	if (!path)
		return false;

	FILE *fp = fopen(path, "rb");
	if (!fp)
		return false;

	uint8_t header[24];
	if (fread(header, 1, sizeof(header), fp) != sizeof(header) ||
		header[0] != 'X' || header[1] != 'I' || header[2] != '2' || header[3] != '\0')
	{
		WARNING("not WAI file");
		fclose(fp);
		return false;
	}

	int count = LittleEndian_getDW(header, 8);
	if (count <= 0) {
		WARNING("invalid WAI record count: %d", count);
		fclose(fp);
		return false;
	}

	int version = LittleEndian_getDW(header, 12);
	if (version != 3) {
		WARNING("unsupported WAI version: %d", version);
		fclose(fp);
		return false;
	}

	int *channels = calloc(count, sizeof(*channels));
	if (!channels) {
		fclose(fp);
		return false;
	}

	uint8_t record[12];
	for (int i = 0; i < count; i++) {
		if (fread(record, 1, sizeof(record), fp) != sizeof(record)) {
			WARNING("truncated WAI file");
			free(channels);
			fclose(fp);
			return false;
		}
		channels[i] = LittleEndian_getDW(record, 8);
	}

	fclose(fp);
	wai_channels = channels;
	wai_count = count;
	return true;
}

bool wai_loaded(void) {
	return wai_channels != NULL;
}

int wai_mixch(int no) {
	if (!wai_channels || no < 0 || no >= wai_count)
		return -1;
	return wai_channels[no];
}
