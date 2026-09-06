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
#ifndef XSYSTEM35_CACHE_H
#define XSYSTEM35_CACHE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct Cache Cache;

typedef struct {
	size_t key_size;
	uint32_t (*hash)(const void *key);
	bool (*equal)(const void *a, const void *b);
	void (*destroy)(void *data);
	bool (*is_pinned)(const void *data);
} CacheOps;

typedef enum {
	CACHE_INSERT_OK,
	CACHE_INSERT_EXISTS,
	CACHE_INSERT_FULL,
	CACHE_INSERT_NOMEM,
} CacheInsertResult;

typedef struct {
	size_t count;
	size_t size;
	size_t capacity;
	size_t hits;
	size_t misses;
} CacheStats;

Cache *cache_new(size_t capacity, const CacheOps *ops);
void cache_destroy(Cache *cache);

/* The returned pointer is owned by the cache and is invalidated by removal. */
void *cache_get(Cache *cache, const void *key);

/* Ownership of data is transferred only when CACHE_INSERT_OK is returned. */
CacheInsertResult cache_insert(Cache *cache, const void *key, void *data, size_t cost);

/* Pinned entries cannot be removed. */
bool cache_remove(Cache *cache, const void *key);

/* Removes all unpinned entries and returns the number of entries left. */
size_t cache_clear(Cache *cache);

/* Returns zero-filled statistics when cache is NULL. */
CacheStats cache_get_stats(const Cache *cache);

#endif /* XSYSTEM35_CACHE_H */
