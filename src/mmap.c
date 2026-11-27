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
#include "config.h"

#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#else // _WIN32
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#ifdef HAVE_MMAP
#include <sys/mman.h>
#endif // HAVE_MMAP
#endif // _WIN32
#include "system.h"
#include "mmap.h"

#ifdef _WIN32

mmap_t *map_file(const char *path) {
	HANDLE hFile = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
							  FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);
	if (!hFile)
		return NULL;
	HANDLE hMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
	if (!hMapping) {
		CloseHandle(hFile);
		return NULL;
	}
	void *addr = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
	CloseHandle(hMapping);
	CloseHandle(hFile);
	if (addr == NULL)
		return NULL;

	mmap_t *m = malloc(sizeof(mmap_t));
	m->addr = addr;
	return m;
}

mmap_t *map_file_readwrite(const char *path, size_t size) {
	HANDLE hFile = CreateFile(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS,
							  FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);
	if (!hFile)
		return NULL;
	LARGE_INTEGER filesize;
	if (!GetFileSizeEx(hFile, &filesize)) {
		CloseHandle(hFile);
		return NULL;
	}
	if (filesize.QuadPart != size) {
		if (SetFilePointer(hFile, size, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER ||
			!SetEndOfFile(hFile)) {
			CloseHandle(hFile);
			return NULL;
		}
	}
	HANDLE hMapping = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, size, NULL);
	if (!hMapping) {
		CloseHandle(hFile);
		return NULL;
	}
	void *addr = MapViewOfFile(hMapping, FILE_MAP_WRITE, 0, 0, size);
	CloseHandle(hMapping);
	CloseHandle(hFile);
	if (addr == NULL)
		return NULL;

	if (filesize.QuadPart < size)
		memset(addr, 0, size);

	mmap_t *m = malloc(sizeof(mmap_t));
	m->addr = addr;
	return m;
}

int sync_mapped_file(mmap_t *m) {
	if (!m || !m->addr)
		return 0;
	return FlushViewOfFile(m->addr, 0) ? 0 : -1;
}

int unmap_file(mmap_t *m) {
	if (!m)
		return 0;
	UnmapViewOfFile(m->addr);
	free(m);
	return 0;
}

#else // _WIN32

mmap_t *map_file(const char *path) {
	int fd = open(path, O_RDONLY);
	if (fd < 0) {
		WARNING("open: %s", strerror(errno));
		return NULL;
	}

	struct stat sbuf;
	if (fstat(fd, &sbuf) < 0) {
		WARNING("fstat: %s", strerror(errno));
		close(fd);
		return NULL;
	}

#ifdef HAVE_MMAP
	void *addr = mmap(0, sbuf.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (addr == MAP_FAILED) {
		WARNING("mmap: %s", strerror(errno));
		close(fd);
		return NULL;
	}
#else
	void *addr = malloc(sbuf.st_size > 0 ? (size_t)sbuf.st_size : 1);
	if (!addr) {
		close(fd);
		return NULL;
	}
	size_t bytes = 0;
	while (bytes < (size_t)sbuf.st_size) {
		ssize_t ret = read(fd, (uint8_t *)addr + bytes, (size_t)sbuf.st_size - bytes);
		if (ret < 0 && errno == EINTR)
			continue;
		if (ret <= 0) {
			WARNING("read: %s", strerror(errno));
			close(fd);
			free(addr);
			return NULL;
		}
		bytes += (size_t)ret;
	}
#endif

	mmap_t *m = malloc(sizeof(mmap_t));
	m->addr = addr;
	m->length = (size_t)sbuf.st_size;
	m->fd = fd;
	m->writable = false;
	return m;
}

mmap_t *map_file_readwrite(const char *path, size_t size) {
	int fd = open(path, O_RDWR | O_CREAT, 0644);
	if (fd < 0) {
		WARNING("open: %s", strerror(errno));
		return NULL;
	}

#ifndef HAVE_MMAP
	void *addr = calloc(1, size > 0 ? size : 1);
	if (!addr) {
		close(fd);
		return NULL;
	}

	struct stat sbuf;
	if (fstat(fd, &sbuf) == 0 && sbuf.st_size > 0) {
		size_t to_read = (size_t)sbuf.st_size < size ? (size_t)sbuf.st_size : size;
		size_t bytes = 0;
		while (bytes < to_read) {
			ssize_t ret = read(fd, (uint8_t *)addr + bytes, to_read - bytes);
			if (ret < 0 && errno == EINTR)
				continue;
			if (ret <= 0) {
				WARNING("read: %s", strerror(errno));
				close(fd);
				free(addr);
				return NULL;
			}
			bytes += (size_t)ret;
		}
	}

	if (ftruncate(fd, (off_t)size) < 0) {
		WARNING("ftruncate: %s", strerror(errno));
	}

	mmap_t *m = malloc(sizeof(mmap_t));
	m->addr = addr;
	m->length = size;
	m->fd = fd;
	m->writable = true;
	return m;
#else
	if (ftruncate(fd, size) < 0) {
		WARNING("ftruncate: %s", strerror(errno));
		close(fd);
		return NULL;
	}

	void *addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (addr == MAP_FAILED) {
		WARNING("mmap: %s", strerror(errno));
		close(fd);
		return NULL;
	}

	mmap_t *m = malloc(sizeof(mmap_t));
	m->addr = addr;
	m->length = size;
	m->fd = fd;
	m->writable = true;
	return m;
#endif
}

int sync_mapped_file(mmap_t *m) {
	if (!m || !m->addr || !m->writable)
		return 0;
#ifdef HAVE_MMAP
	return msync(m->addr, m->length, MS_ASYNC);
#else
	if (m->fd < 0)
		return -1;
	if (lseek(m->fd, 0, SEEK_SET) < 0) {
		WARNING("lseek: %s", strerror(errno));
		return -1;
	}
	size_t written = 0;
	while (written < m->length) {
		ssize_t ret = write(m->fd, (const uint8_t *)m->addr + written, m->length - written);
		if (ret < 0 && errno == EINTR)
			continue;
		if (ret <= 0) {
			WARNING("write: %s", strerror(errno));
			return -1;
		}
		written += (size_t)ret;
	}
	return 0;
#endif
}

int unmap_file(mmap_t *m) {
	if (!m)
		return 0;
#ifdef HAVE_MMAP
	if (m->writable)
		msync(m->addr, m->length, MS_ASYNC);
	int rv = munmap(m->addr, m->length);
	close(m->fd);
	free(m);
	return rv;
#else
	int rv = 0;
	if (m->writable)
		rv = sync_mapped_file(m);
	if (m->fd >= 0)
		close(m->fd);
	free(m->addr);
	free(m);
	return rv;
#endif
}

#endif // _WIN32
