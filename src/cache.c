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

#include <stdlib.h>
#include <string.h>

#define CACHE_BUCKETS 1024

typedef struct CacheEntry {
	struct CacheEntry *hash_next;
	struct CacheEntry *lru_prev;
	struct CacheEntry *lru_next;
	uint32_t hash;
	size_t cost;
	void *data;
	unsigned char key[];
} CacheEntry;

struct Cache {
	CacheOps ops;
	CacheEntry *buckets[CACHE_BUCKETS];
	CacheEntry *lru_head;
	CacheEntry *lru_tail;
	size_t capacity;
	size_t size;
	size_t count;
	size_t hits;
	size_t misses;
};

static bool is_pinned(const Cache *cache, const CacheEntry *entry) {
	return cache->ops.is_pinned && cache->ops.is_pinned(entry->data);
}

static void lru_remove(Cache *cache, CacheEntry *entry) {
	if (entry->lru_prev)
		entry->lru_prev->lru_next = entry->lru_next;
	else
		cache->lru_head = entry->lru_next;
	if (entry->lru_next)
		entry->lru_next->lru_prev = entry->lru_prev;
	else
		cache->lru_tail = entry->lru_prev;
}

static void lru_prepend(Cache *cache, CacheEntry *entry) {
	entry->lru_prev = NULL;
	entry->lru_next = cache->lru_head;
	if (cache->lru_head)
		cache->lru_head->lru_prev = entry;
	else
		cache->lru_tail = entry;
	cache->lru_head = entry;
}

static void remove_entry(Cache *cache, CacheEntry *entry) {
	CacheEntry **link = &cache->buckets[entry->hash % CACHE_BUCKETS];
	while (*link != entry)
		link = &(*link)->hash_next;
	*link = entry->hash_next;
	lru_remove(cache, entry);
	cache->size -= entry->cost;
	cache->count--;
	cache->ops.destroy(entry->data);
	free(entry);
}

static CacheEntry *find_entry(Cache *cache, const void *key, uint32_t hash) {
	for (CacheEntry *entry = cache->buckets[hash % CACHE_BUCKETS]; entry;
	     entry = entry->hash_next) {
		if (entry->hash == hash && cache->ops.equal(entry->key, key))
			return entry;
	}
	return NULL;
}

Cache *cache_new(size_t capacity, const CacheOps *ops) {
	if (!ops || !ops->key_size || !ops->hash || !ops->equal || !ops->destroy)
		return NULL;
	Cache *cache = calloc(1, sizeof(Cache));
	if (!cache)
		return NULL;
	cache->ops = *ops;
	cache->capacity = capacity;
	return cache;
}

void cache_destroy(Cache *cache) {
	if (!cache)
		return;
	while (cache->lru_tail)
		remove_entry(cache, cache->lru_tail);
	free(cache);
}

void *cache_get(Cache *cache, const void *key) {
	if (!cache || !key)
		return NULL;
	CacheEntry *entry = find_entry(cache, key, cache->ops.hash(key));
	if (!entry) {
		cache->misses++;
		return NULL;
	}
	cache->hits++;
	lru_remove(cache, entry);
	lru_prepend(cache, entry);
	return entry->data;
}

CacheInsertResult cache_insert(Cache *cache, const void *key, void *data, size_t cost) {
	if (!cache || !key || !data)
		return CACHE_INSERT_NOMEM;
	uint32_t hash = cache->ops.hash(key);
	if (find_entry(cache, key, hash))
		return CACHE_INSERT_EXISTS;
	if (cost > cache->capacity)
		return CACHE_INSERT_FULL;

	CacheEntry *entry = malloc(sizeof(CacheEntry) + cache->ops.key_size);
	if (!entry)
		return CACHE_INSERT_NOMEM;

	for (CacheEntry *old = cache->lru_tail, *prev;
	     old && cache->size > cache->capacity - cost; old = prev) {
		prev = old->lru_prev;
		if (!is_pinned(cache, old))
			remove_entry(cache, old);
	}
	if (cache->size > cache->capacity - cost) {
		free(entry);
		return CACHE_INSERT_FULL;
	}

	entry->hash = hash;
	entry->cost = cost;
	entry->data = data;
	memcpy(entry->key, key, cache->ops.key_size);
	unsigned bucket = hash % CACHE_BUCKETS;
	entry->hash_next = cache->buckets[bucket];
	cache->buckets[bucket] = entry;
	lru_prepend(cache, entry);
	cache->size += cost;
	cache->count++;
	return CACHE_INSERT_OK;
}

bool cache_remove(Cache *cache, const void *key) {
	if (!cache || !key)
		return false;
	CacheEntry *entry = find_entry(cache, key, cache->ops.hash(key));
	if (!entry || is_pinned(cache, entry))
		return false;
	remove_entry(cache, entry);
	return true;
}

size_t cache_clear(Cache *cache) {
	if (!cache)
		return 0;
	for (CacheEntry *entry = cache->lru_tail, *prev; entry; entry = prev) {
		prev = entry->lru_prev;
		if (!is_pinned(cache, entry))
			remove_entry(cache, entry);
	}
	return cache->count;
}

CacheStats cache_get_stats(const Cache *cache) {
	if (!cache)
		return (CacheStats){0};
	return (CacheStats){
		.count = cache->count,
		.size = cache->size,
		.capacity = cache->capacity,
		.hits = cache->hits,
		.misses = cache->misses,
	};
}
