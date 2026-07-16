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
#include "unittest.h"
#include "variable.h"

static void allocate_page_test(void) {
	ASSERT_FALSE(v_allocatePage(-1, 1, true));
	ASSERT_FALSE(v_allocatePage(256, 1, true));
	ASSERT_FALSE(v_allocatePage(1, 0, true));
	ASSERT_FALSE(v_allocatePage(1, 65537, true));

	ASSERT_TRUE(v_allocatePage(0, 65536, true));
	ASSERT_EQUAL_PTR(varPage[0].value, sysVar);

	ASSERT_TRUE(v_allocatePage(1, 1, false));
	ASSERT_TRUE(v_allocatePage(255, 65536, true));
}

static void array_reference_test(void) {
	struct VarRef ref;
	int index = 2;

	ASSERT_TRUE(v_allocatePage(1, 5, true));
	ASSERT_FALSE(v_bindArray(-1, &index, 0, 1));
	ASSERT_FALSE(v_bindArray(65537, &index, 0, 1));
	ASSERT_FALSE(v_bindArray(10, &index, 0, 0));
	ASSERT_FALSE(v_bindArray(10, &index, 0, 256));
	ASSERT_FALSE(v_bindArray(10, &index, -1, 1));
	ASSERT_FALSE(v_bindArray(10, &index, 5, 1));

	ASSERT_TRUE(v_bindArray(10, &index, 1, 1));
	ASSERT_EQUAL_PTR(v_ref_indexed(10, 0, &ref), varPage[1].value + 1);
	ASSERT_EQUAL(ref.var, 10);
	ASSERT_EQUAL(ref.page, 1);
	ASSERT_EQUAL(ref.index, 1);
	ASSERT_EQUAL_PTR(v_ref(10, &ref), varPage[1].value + 3);
	ASSERT_EQUAL(ref.index, 3);
	ASSERT_NULL(v_ref_indexed(10, 4, NULL));

	ASSERT_EQUAL_PTR(v_ref_indexed(20, 3, &ref), sysVar + 23);
	ASSERT_EQUAL(ref.var, 20);
	ASSERT_EQUAL(ref.page, 0);
	ASSERT_EQUAL(ref.index, 23);
	ASSERT_EQUAL_PTR(v_ref_indexed(65536, 0, NULL), sysVar + 65536);
	ASSERT_NULL(v_ref_indexed(65536, 1, NULL));

	ASSERT_TRUE(v_unbindArray(10));
	ASSERT_EQUAL_PTR(v_ref(10, &ref), sysVar + 10);
	ASSERT_EQUAL(ref.page, 0);
}

static void string_variable_conversion_test(void) {
	const int input[] = {
		0x41,    // 'A'
		0xa082,  // Shift-JIS 0x82 0xa0 ('あ')
		0x42,    // 'B'
		0,       // terminator
	};
	int output[4] = {-1, -1, -1, -1};

	svar_fromVars(0, input);
	ASSERT_STRCMP(svar_get(0), "A\x82\xa0" "B");
	ASSERT_EQUAL(svar_toVars(0, output), 4);
	for (int i = 0; i < 4; i++)
		ASSERT_EQUAL(output[i], input[i]);
}

void variable_test(void) {
	v_init();
	allocate_page_test();
	v_reset();
	array_reference_test();
	v_reset();
	string_variable_conversion_test();
	v_reset();
}
