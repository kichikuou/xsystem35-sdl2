/*
 * Copyright (C) 2026 <KichikuouChrome@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "module_test.h"
#include "unittest.h"

static void call_with_value(const char *name, int value) {
	const int values[] = {value};
	module_test_call(&module_ShCalc, name, values, 1, NULL, 0);
}

static void call_with_variable(const char *name, vmvar_t *variable) {
	vmvar_t *variables[] = {variable};
	module_test_call(&module_ShCalc, name, NULL, 0, variables, 1);
}

static void word_array_round_trip_test(void) {
	vmvar_t zero[2] = {0, 0};
	vmvar_t one[2] = {1, 0};
	vmvar_t result[2] = {0, 0};
	vmvar_t round_trip[2] = {0, 0};

	call_with_value("SetIntNumBase", 1);
	call_with_variable("SetIntNum32", zero);
	call_with_variable("SubIntNum32", one);
	call_with_variable("GetIntNum32", result);
	ASSERT_EQUAL(result[0], 65535);
	ASSERT_EQUAL(result[1], 65535);

	call_with_variable("SetIntNum32", result);
	call_with_variable("GetIntNum32", round_trip);
	ASSERT_EQUAL(round_trip[0], 65535);
	ASSERT_EQUAL(round_trip[1], 65535);
}

static void word_result_test(void) {
	vmvar_t zero[2] = {0, 0};
	vmvar_t one[2] = {1, 0};
	vmvar_t result = 0;

	call_with_value("SetIntNumBase", 1);
	call_with_variable("SetIntNum32", zero);
	call_with_variable("SubIntNum32", one);
	call_with_variable("GetIntNum16", &result);
	ASSERT_EQUAL(result, 65535);
}

static void bit_to_num_test(void) {
	vmvar_t result = 0;
	vmvar_t *variables[] = {&result};
	const int power_values[] = {32768};
	const int non_power_values[] = {65535};

	module_test_call(&module_ShCalc, "BitToNum",
			 power_values, 1, variables, 1);
	ASSERT_EQUAL(result, 16);
	module_test_call(&module_ShCalc, "BitToNum",
			 non_power_values, 1, variables, 1);
	ASSERT_EQUAL(result, 0);
}

void shcalc_test(void) {
	word_array_round_trip_test();
	word_result_test();
	bit_to_num_test();
}
