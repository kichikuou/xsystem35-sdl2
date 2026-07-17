/*
 * Copyright (C) 2026 <KichikuouChrome@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <stdarg.h>
#include <string.h>

#include "cali.h"
#include "module_test.h"
#include "system.h"
#include "unittest.h"

static const int *value_args;
static int nr_value_args;
static int value_index;
static vmvar_t **variable_args;
static int nr_variable_args;
static int variable_index;

int getCaliValue(void) {
	ASSERT_TRUE(value_index < nr_value_args);
	return value_args[value_index++];
}

vmvar_t *getCaliVariable(void) {
	ASSERT_TRUE(variable_index < nr_variable_args);
	return variable_args[variable_index++];
}

void sys_message(int level, char *format, ...) {
	(void)level;
	(void)format;
}

double genrand(void) {
	return 0;
}

void sgenrand(unsigned long seed) {
	(void)seed;
}

void module_test_call(const Module *module, const char *name,
		      const int *values, int nr_values,
		      vmvar_t **variables, int nr_variables) {
	const ModuleFunc *func = NULL;

	for (int i = 0; i < module->nfunc; i++) {
		if (!strcmp(module->funcs[i].name, name)) {
			func = &module->funcs[i];
			break;
		}
	}
	ASSERT_TRUE(func != NULL);

	value_args = values;
	nr_value_args = nr_values;
	value_index = 0;
	variable_args = variables;
	nr_variable_args = nr_variables;
	variable_index = 0;
	func->entrypoint();
	ASSERT_EQUAL(value_index, nr_value_args);
	ASSERT_EQUAL(variable_index, nr_variable_args);
}
