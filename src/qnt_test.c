/*
 * Copyright (C) 2026 <KichikuouChrome@gmail.com>
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
 */
#include "cg.h"
#include "qnt.h"
#include "unittest.h"

#include <stdio.h>
#include <stdlib.h>

static void odd_size_test(void) {
	FILE *fp = fopen("testdata/odd-size.qnt", "rb");
	ASSERT_TRUE(fp);

	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	rewind(fp);
	uint8_t *data = malloc(size);
	ASSERT_EQUAL(fread(data, 1, size, fp), size);
	fclose(fp);

	cgdata *cg = qnt_extract(data);
	ASSERT_EQUAL(cg->width, 3);
	ASSERT_EQUAL(cg->height, 3);
	ASSERT_TRUE(cg->alpha);

	const uint8_t expected[] = {
		0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90,
		0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0, 0x11, 0x22, 0x33,
		0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc,
	};
	ASSERT_TRUE(!memcmp(cg->pic, expected, sizeof(expected)));

	const uint8_t expected_alpha[] = {
		0xff, 0x80, 0x40,
		0xff, 0x20, 0xc0,
		0x7f, 0xfe, 0x10,
	};
	ASSERT_TRUE(!memcmp(cg->alpha, expected_alpha, sizeof(expected_alpha)));

	free(cg->pic);
	free(cg->alpha);
	free(cg);
	free(data);
}

void qnt_test(void) {
	odd_size_test();
}
