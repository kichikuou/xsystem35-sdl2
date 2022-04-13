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
#ifndef __MMAP_H__
#define __MMAP_H__

#include <stddef.h>

#if defined(HAVE_MMAP) || defined(_WIN32)
#define HAVE_MEMORY_MAPPED_FILE
#endif

typedef struct {
	void *addr;
#ifndef _WIN32
	size_t length;
	int fd;	 // Emscripten needs fd for mmap kept open.
#endif
} mmap_t;

mmap_t *map_file(const char *path);
mmap_t *map_file_readwrite(const char *path, size_t size);
int unmap_file(mmap_t *m);

#endif /* __MMAP_H__ */
