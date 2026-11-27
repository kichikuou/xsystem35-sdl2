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
	vmvar_t index = 2;

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
	const vmvar_t input[] = {
		0x41,    // 'A'
		0xa082,  // Shift-JIS 0x82 0xa0 ('あ')
		0x42,    // 'B'
		0,       // terminator
	};
	vmvar_t output[4] = {-1, -1, -1, -1};

	svar_fromVars(0, input);
	ASSERT_STRCMP(svar_get(0), "A\x82\xa0" "B");
	ASSERT_EQUAL(svar_toVars(0, output), 4);
	for (int i = 0; i < 4; i++)
		ASSERT_EQUAL(output[i], input[i]);
}

static void bound_slice_size_test(void) {
	struct VarRef ref;
	vmvar_t index = 0;

	ASSERT_TRUE(v_allocatePage(1, 16, true));
	ASSERT_TRUE(v_bindArray(10, &index, 0, 1));
	ASSERT_TRUE(v_bindArray(11, &index, 4, 1));
	ASSERT_TRUE(v_bindArray(12, &index, 10, 1));
	ASSERT_TRUE(v_bindArray(13, &index, 4, 1));  // alias at same offset

	ASSERT_TRUE(v_ref_indexed(10, 0, &ref) != NULL);
	ASSERT_EQUAL(v_boundSliceSize(&ref), 4);
	ASSERT_TRUE(v_ref_indexed(11, 0, &ref) != NULL);
	ASSERT_EQUAL(v_boundSliceSize(&ref), 6);
	ASSERT_TRUE(v_ref_indexed(12, 0, &ref) != NULL);
	ASSERT_EQUAL(v_boundSliceSize(&ref), 6);

	ASSERT_TRUE(v_unbindArray(11));
	ASSERT_TRUE(v_unbindArray(13));
	ASSERT_TRUE(v_ref_indexed(10, 0, &ref) != NULL);
	ASSERT_EQUAL(v_boundSliceSize(&ref), 10);

	// Unbound / page 0 refs return 0
	ASSERT_TRUE(v_ref_indexed(20, 0, &ref) != NULL);
	ASSERT_EQUAL(v_boundSliceSize(&ref), 0);
}

static void string_variable_char_type_test(void) {
	// GBK: 'A' (1 byte), "大" 0xb4 0xf3 (2 bytes), "司" 0xcb 0xbe (2 bytes), plus dangling lead byte 0x81
	v_set_encoding(GBK);
	svar_set(0, "A\xb4\xf3\xcb\xbe" "B");
	ASSERT_EQUAL(svar_getCharType(0, 0), 1);
	ASSERT_EQUAL(svar_getCharType(0, 1), 2);
	ASSERT_EQUAL(svar_getCharType(0, 2), 2);
	ASSERT_EQUAL(svar_getCharType(0, 3), 1);
	ASSERT_EQUAL(svar_getCharType(0, 4), 0);

	// Truncated trailing GBK lead byte must not advance past NUL.
	svar_set(1, "A\x81");
	ASSERT_EQUAL(svar_getCharType(1, 0), 1);
	ASSERT_EQUAL(svar_getCharType(1, 1), 1);
	ASSERT_EQUAL(svar_getCharType(1, 2), 0);

	// UTF-8: 'A' (1 byte), "大" (3 bytes)
	v_set_encoding(UTF8);
	svar_set(2, "A\xe5\xa4\xa7");
	ASSERT_EQUAL(svar_getCharType(2, 0), 1);
	ASSERT_EQUAL(svar_getCharType(2, 1), 2);
	ASSERT_EQUAL(svar_getCharType(2, 2), 0);

	v_set_encoding(SHIFT_JIS);
}

void variable_test(void) {
	v_init();
	allocate_page_test();
	v_reset();
	array_reference_test();
	v_reset();
	bound_slice_size_test();
	v_reset();
	string_variable_conversion_test();
	v_reset();
	string_variable_char_type_test();
	v_reset();
}
