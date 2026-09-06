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
 *
*/
#include "cache.h"
#include "unittest.h"

#include <stdlib.h>

typedef struct {
	int value;
	bool pinned;
} TestValue;

static int destroyed;

static uint32_t int_hash(const void *key) {
	return (uint32_t)*(const int *)key;
}

static bool int_equal(const void *a, const void *b) {
	return *(const int *)a == *(const int *)b;
}

static void destroy_value(void *data) {
	destroyed++;
	free(data);
}

static bool value_is_pinned(const void *data) {
	return ((const TestValue *)data)->pinned;
}

static TestValue *new_value(int value) {
	TestValue *data = malloc(sizeof(TestValue));
	ASSERT_TRUE(data);
	data->value = value;
	data->pinned = false;
	return data;
}

void cache_test(void) {
	CacheOps ops = {
		.key_size = sizeof(int),
		.hash = int_hash,
		.equal = int_equal,
		.destroy = destroy_value,
		.is_pinned = value_is_pinned,
	};
	Cache *cache = cache_new(2, &ops);
	ASSERT_TRUE(cache);
	destroyed = 0;

	// A lookup makes k1 most-recently used, so inserting k3 evicts k2.
	int k1 = 1, k2 = 2, k3 = 3, k4 = 4, k5 = 5;
	ASSERT_EQUAL(cache_insert(cache, &k1, new_value(1), 1), CACHE_INSERT_OK);
	ASSERT_EQUAL(cache_insert(cache, &k2, new_value(2), 1), CACHE_INSERT_OK);
	ASSERT_EQUAL(((TestValue *)cache_get(cache, &k1))->value, 1);
	ASSERT_EQUAL(cache_insert(cache, &k3, new_value(3), 1), CACHE_INSERT_OK);
	ASSERT_NULL(cache_get(cache, &k2));

	// Pinned entries survive both capacity eviction and cache_clear().
	((TestValue *)cache_get(cache, &k1))->pinned = true;
	ASSERT_EQUAL(cache_insert(cache, &k4, new_value(4), 1), CACHE_INSERT_OK);
	ASSERT_NULL(cache_get(cache, &k3));
	((TestValue *)cache_get(cache, &k4))->pinned = true;

	// Insertion fails when every remaining entry is pinned.
	TestValue *rejected = new_value(5);
	ASSERT_EQUAL(cache_insert(cache, &k5, rejected, 1), CACHE_INSERT_FULL);
	ASSERT_EQUAL(((TestValue *)cache_get(cache, &k1))->value, 1);
	ASSERT_EQUAL(((TestValue *)cache_get(cache, &k4))->value, 4);
	ASSERT_EQUAL(cache_clear(cache), 2);
	ASSERT_EQUAL(destroyed, 2);
	free(rejected);

	// Once unpinned, cache_clear() destroys the remaining entries.
	((TestValue *)cache_get(cache, &k1))->pinned = false;
	((TestValue *)cache_get(cache, &k4))->pinned = false;
	ASSERT_EQUAL(cache_clear(cache), 0);
	ASSERT_EQUAL(destroyed, 4);
	cache_destroy(cache);
}
