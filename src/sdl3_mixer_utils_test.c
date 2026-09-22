/*
 * Copyright (C) 2026 <KichikuouChrome@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <stdint.h>

#include "sdl3_mixer_utils.h"
#include "unittest.h"

static int gain_basis_points(int volume, int balance)
{
	return sdl3_mixer_gain(volume, balance) * 10000.0f + 0.5f;
}

void sdl3_mixer_utils_test(void)
{
	ASSERT_EQUAL(gain_basis_points(100, 100), 10000);
	ASSERT_EQUAL(gain_basis_points(50, 50), 2500);
	ASSERT_EQUAL(gain_basis_points(33, 77), 2541);
	ASSERT_EQUAL(gain_basis_points(-1, 100), 0);
	ASSERT_EQUAL(gain_basis_points(100, -1), 0);
	ASSERT_EQUAL(gain_basis_points(101, 200), 10000);

	ASSERT_EQUAL(sdl3_mixer_10ms_to_ms(0), 0);
	ASSERT_EQUAL(sdl3_mixer_10ms_to_ms(1), 10);
	ASSERT_EQUAL(sdl3_mixer_10ms_to_ms(123), 1230);

	ASSERT_EQUAL(sdl3_mixer_elapsed_10ms(1000, 1000), 0);
	ASSERT_EQUAL(sdl3_mixer_elapsed_10ms(1000, 1009), 0);
	ASSERT_EQUAL(sdl3_mixer_elapsed_10ms(1000, 1010), 1);
	ASSERT_EQUAL(sdl3_mixer_elapsed_10ms(1000, 1123), 12);
	ASSERT_EQUAL(sdl3_mixer_elapsed_10ms(UINT32_MAX - 4, 5), 1);
}
