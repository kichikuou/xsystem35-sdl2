/*
 * Copyright (C) 2026 <KichikuouChrome@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "sactchart.h"
#include "unittest.h"

void sactchart_test(void) {
	vmvar_t pos = 0;

	schart_pos(&pos, 0, 1, 1, 2, 0);
	ASSERT_EQUAL(pos, 65535);
	schart_pos(&pos, 40000, 0, 1, 0, 2);
	ASSERT_EQUAL(pos, 14464);
}
