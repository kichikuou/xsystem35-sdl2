/*
 * Copyright (C) 2026 <KichikuouChrome@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <stdlib.h>
#include <string.h>

#include "module_test.h"
#include "unittest.h"

static void get_at_array_test(void) {
	vmvar_t array[] = {40000, 30000};
	vmvar_t result = 0;
	vmvar_t *variables[] = {array, &result};
	const int values[] = {2, 1};

	module_test_call(&module_ShArray, "GetAtArray",
			 values, 2, variables, 2);
	ASSERT_EQUAL(result, 65535);
}

static void array_arithmetic_test(void) {
	vmvar_t lhs[] = {65535, 40000, 1};
	vmvar_t rhs[] = {1, 30000, 2};
	vmvar_t *variables[] = {lhs, rhs};
	const int values[] = {3};

	module_test_call(&module_ShArray, "AddAtArray",
			 values, 1, variables, 2);
	ASSERT_EQUAL(lhs[0], 65535);
	ASSERT_EQUAL(lhs[1], 65535);
	ASSERT_EQUAL(lhs[2], 3);

	lhs[0] = 0;
	lhs[1] = 1;
	lhs[2] = 65535;
	rhs[0] = 1;
	rhs[1] = 2;
	rhs[2] = 65535;
	module_test_call(&module_ShArray, "SubAtArray",
			 values, 1, variables, 2);
	ASSERT_EQUAL(lhs[0], 0);
	ASSERT_EQUAL(lhs[1], 0);
	ASSERT_EQUAL(lhs[2], 0);

	lhs[0] = 65535;
	lhs[1] = 300;
	lhs[2] = 7;
	rhs[0] = 2;
	rhs[1] = 300;
	rhs[2] = 8;
	module_test_call(&module_ShArray, "MulAtArray",
			 values, 1, variables, 2);
	ASSERT_EQUAL(lhs[0], 65535);
	ASSERT_EQUAL(lhs[1], 65535);
	ASSERT_EQUAL(lhs[2], 56);
}

static void enum_counter_test(void) {
	const int count = 65535;
	vmvar_t *array = malloc(count * sizeof(vmvar_t));
	vmvar_t result = 0;
	vmvar_t *variables[] = {array, &result};
	const int values[] = {count, 1};

	ASSERT_TRUE(array != NULL);
	for (int i = 0; i < count; i++)
		array[i] = 1;
	module_test_call(&module_ShArray, "EnumEquArray",
			 values, 2, variables, 2);
	ASSERT_EQUAL(result, 65535);
	free(array);
}

static void order_sentinel_test(void) {
	vmvar_t array[] = {65534, 42, 60000};
	vmvar_t used[] = {0, 0, 0};
	vmvar_t match = 0;
	vmvar_t result = 0;
	vmvar_t *variables[] = {array, used, &match, &result};
	const int values[] = {3, 0, 65535};

	module_test_call(&module_ShArray, "GrepLowOrderArray",
			 values, 3, variables, 4);
	ASSERT_EQUAL(result, 1);
	ASSERT_EQUAL(match, 1);
	ASSERT_EQUAL(used[1], 1);
}

static void secret_array_round_trip_test(void) {
	vmvar_t array[] = {0, 1, 0x1234, 0xffff, 0x8000, 42, 7, 60000};
	vmvar_t original[8];
	vmvar_t encode_result = 0;
	vmvar_t decode_result = 0;
	const int encode_values[] = {8, 2};
	const int decode_values[] = {8, 3};
	vmvar_t *encode_variables[] = {array, &encode_result};
	vmvar_t *decode_variables[] = {array, &decode_result};

	memcpy(original, array, sizeof(array));
	module_test_call(&module_ShArray, "ChangeSecretArray",
			 encode_values, 2, encode_variables, 2);
	ASSERT_TRUE(memcmp(array, original, sizeof(array)) != 0);
	module_test_call(&module_ShArray, "ChangeSecretArray",
			 decode_values, 2, decode_variables, 2);
	ASSERT_TRUE(memcmp(array, original, sizeof(array)) == 0);
}

void sharray_test(void) {
	get_at_array_test();
	array_arithmetic_test();
	enum_counter_test();
	order_sentinel_test();
	secret_array_round_trip_test();
}
