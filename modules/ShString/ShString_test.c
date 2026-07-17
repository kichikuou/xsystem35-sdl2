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
#include "variable.h"

static void numeric_result_to_string_test(void) {
	vmvar_t result[2] = {0, 0};
	vmvar_t *variables[] = {result};
	const int values[] = {0};

	v_init();
	svar_set(0, "65601");
	module_test_call(&module_ShString, "SetStringNum16",
			 values, 1, variables, 1);
	svar_fromVars(1, result);
	ASSERT_STRCMP(svar_get(1), "A");
	v_reset();
}

void shstring_test(void) {
	numeric_result_to_string_test();
}
