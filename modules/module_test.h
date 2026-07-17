/*
 * Copyright (C) 2026 <KichikuouChrome@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __MODULE_TEST_H__
#define __MODULE_TEST_H__

#include "modules.h"

void module_test_call(const Module *module, const char *name,
		      const int *values, int nr_values,
		      vmvar_t **variables, int nr_variables);

#endif /* __MODULE_TEST_H__ */
