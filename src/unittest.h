/*
 * Copyright (C) 2020 <KichikuouChrome@gmail.com>
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
 *
 */
#ifndef __UNITTEST_H__
#define __UNITTEST_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ASSERT_TRUE(condition) assert_true(#condition, !!(condition), __FILE__, __LINE__)
#define ASSERT_FALSE(condition) assert_false(#condition, !!(condition), __FILE__, __LINE__)
#define ASSERT_NULL(actual) ASSERT_EQUAL_PTR(actual, NULL)
#define ASSERT_EQUAL(actual, expected) assert_equal_int(actual, expected, __FILE__, __LINE__)
#define ASSERT_EQUAL_PTR(actual, expected) assert_equal_ptr(actual, expected, __FILE__, __LINE__)
#define ASSERT_STRCMP(actual, expected) assert_strcmp(actual, expected, __FILE__, __LINE__)

static inline void assert_true(const char *condition, int actual, const char *file, int line) {
	if (!actual) {
		fprintf(stderr, "%s:%d assertion failed: %s\n", file, line, condition);
		exit(1);
	}
}

static inline void assert_false(const char *condition, int actual, const char *file, int line) {
	if (actual) {
		fprintf(stderr, "%s:%d expression unexpectedly evaluated to true: %s\n", file, line, condition);
		exit(1);
	}
}

static inline void assert_equal_int(int actual, int expected, const char *file, int line) {
	if (expected != actual) {
		fprintf(stderr, "%s:%d expected %d, got %d\n", file, line, expected, actual);
		exit(1);
	}
}

static inline void assert_equal_ptr(const void *actual, const void *expected, const char *file, int line) {
	if (expected != actual) {
		fprintf(stderr, "%s:%d expected %p, got %p\n", file, line, expected, actual);
		exit(1);
	}
}

static inline void assert_strcmp(const char *actual, const char *expected, const char *file, int line) {
	if (!actual || strcmp(expected, actual)) {
		fprintf(stderr, "%s:%d expected %s, got %s\n", file, line, expected, actual);
		exit(1);
	}
}

#endif /* __UNITTEST_H__ */
