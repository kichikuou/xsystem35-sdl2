/*
 * filecheck.c  save/load file existance and kanjicode check 
 *
 * Copyright (C) 1997-1998 Masaki Chikama (Wren) <chikama@kasumi.ipl.mech.nagoya-u.ac.jp>
 *               1998-                           <masaki-c@is.aist-nara.ac.jp>
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
/* $Id: filecheck.c,v 1.5 2006/04/21 16:40:48 chikama Exp $ */

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#endif

#include "filecheck.h"

static char *saveDataPath;

static char *get_fullpath(const char* dir, const char *filename) {
	char *fn = malloc(strlen(filename) + strlen(dir) + 3);
	if (fn == NULL) {
		return NULL;
	}
	strcpy(fn, dir);
	strcat(fn, "/");
	strcat(fn, filename);
	return fn;
}

#ifdef _WIN32

static int make_dir(const char *path)
{
	wchar_t wpath[PATH_MAX + 1];
	if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1, wpath, PATH_MAX + 1)) {
		errno = EILSEQ;
		return -1;
	}
	return _wmkdir(wpath);
}

#else
#define make_dir(path) mkdir(path, S_IRWXU)
#endif

// Adapted from http://stackoverflow.com/a/2336245/119527
int mkdir_p(const char *path_utf8)
{
	const size_t len = strlen(path_utf8);
	char path[PATH_MAX];
	char *p;

	errno = 0;

	// Copy string so its mutable
	if (len > sizeof(path)-1) {
		errno = ENAMETOOLONG;
		return -1;
	}
	strcpy(path, path_utf8);

	// Iterate the string
	for (p = path + 1; *p; p++) {
		if (*p == '/') {
			// Temporarily truncate
			*p = '\0';

			if (make_dir(path) != 0) {
				if (errno != EEXIST)
					return -1;
			}

			*p = '/';
		}
	}

	if (make_dir(path) != 0) {
		if (errno != EEXIST)
			return -1;
	}

	return 0;
}

/* list up file in current directory */
/*   name : save/load directory      */
void fc_init(const char *name) {
	mkdir_p(name);
	free(saveDataPath);
	saveDataPath = strdup(name);
}

void fc_fini(void) {
	free(saveDataPath);
	saveDataPath = NULL;
}

#ifdef _WIN32

char *fc_resolve_existing_path(const char *path) {
	if (!path || !*path)
		return NULL;
	wchar_t wpath[PATH_MAX + 1];
	if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1, wpath, PATH_MAX + 1) &&
	    _waccess(wpath, F_OK) == 0) {
		return strdup(path);
	}
	return NULL;
}

int fc_remove_case_variants(const char *canonical_path, bool keep_canonical, const char *backup_first_to) {
	(void)backup_first_to;
	if (!canonical_path || !*canonical_path || keep_canonical)
		return 0;
	wchar_t wpath[PATH_MAX + 1];
	if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, canonical_path, -1, wpath, PATH_MAX + 1) &&
	    _wunlink(wpath) == 0) {
		return 1;
	}
	return 0;
}

FILE *fopen_utf8(const char *path_utf8, char type) {
	wchar_t wpath[PATH_MAX + 1];
	if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path_utf8, -1, wpath, PATH_MAX + 1))
		return NULL;
	return _wfopen(wpath, type == 'r' ? L"rb" : L"wb");
}

char *fc_get_path(const char *fname_utf8) {
	return get_fullpath(saveDataPath ? saveDataPath : ".", fname_utf8);
}

bool fc_exists(const char *fname_utf8)
{
	char *path = fc_get_path(fname_utf8);
	wchar_t wpath[PATH_MAX + 1];
	bool result = false;
	if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1, wpath, PATH_MAX + 1)) {
		result = _waccess(wpath, F_OK) != -1;
	}
	free(path);
	return result;
}

FILE *fc_open(const char *fname_utf8, char type) {
	char *path = fc_get_path(fname_utf8);
	if (type == 'w') {
		// Ensure the directory exists
		char *p = strrchr(path, '/');
		if (p) {
			*p = '\0';
			mkdir_p(path);
			*p = '/';
		}
	}
	FILE *fp = fopen_utf8(path, type);
	if (!fp && type == 'r') {
		fp = fopen_utf8(fname_utf8, type);
	}
	free(path);
	return fp;
}

#else // !_WIN32

static bool fc_split_dir_base(const char *path, const char **dir_out, char **dir_alloc_out, const char **base_out, bool *has_slash_out) {
	const char *slash = strrchr(path, '/');
	const char *base = slash ? slash + 1 : path;
	if (!*base)
		return false;

	char *dir_alloc = NULL;
	const char *dir = ".";
	if (slash) {
		if (slash == path) {
			dir = "/";
		} else {
			size_t dlen = (size_t)(slash - path);
			dir_alloc = malloc(dlen + 1);
			if (!dir_alloc)
				return false;
			memcpy(dir_alloc, path, dlen);
			dir_alloc[dlen] = '\0';
			dir = dir_alloc;
		}
	}
	*dir_out = dir;
	*dir_alloc_out = dir_alloc;
	*base_out = base;
	*has_slash_out = (slash != NULL);
	return true;
}

static char *fc_join_dir_entry(const char *dir, bool has_slash, const char *name) {
	if (!has_slash)
		return strdup(name);
	if (strcmp(dir, "/") == 0) {
		char *p = malloc(strlen(name) + 2);
		if (p)
			sprintf(p, "/%s", name);
		return p;
	}
	return get_fullpath(dir, name);
}

char *fc_resolve_existing_path(const char *path) {
	if (!path || !*path)
		return NULL;
	if (access(path, F_OK) == 0)
		return strdup(path);

	const char *dir = NULL;
	char *dir_alloc = NULL;
	const char *base = NULL;
	bool has_slash = false;
	if (!fc_split_dir_base(path, &dir, &dir_alloc, &base, &has_slash))
		return NULL;

	DIR *d = opendir(dir);
	if (!d) {
		free(dir_alloc);
		return NULL;
	}

	char *found = NULL;
	struct dirent *entry;
	while ((entry = readdir(d)) != NULL) {
		if (strcasecmp(base, entry->d_name) == 0) {
			found = fc_join_dir_entry(dir, has_slash, entry->d_name);
			break;
		}
	}
	closedir(d);
	free(dir_alloc);
	return found;
}

int fc_remove_case_variants(const char *canonical_path, bool keep_canonical, const char *backup_first_to) {
	if (!canonical_path || !*canonical_path)
		return 0;

	int removed = 0;
	if (!keep_canonical) {
		if (unlink(canonical_path) == 0)
			removed++;
	}

	const char *dir = NULL;
	char *dir_alloc = NULL;
	const char *base = NULL;
	bool has_slash = false;
	if (!fc_split_dir_base(canonical_path, &dir, &dir_alloc, &base, &has_slash))
		return removed;

	struct stat canon_st;
	bool has_canon = keep_canonical && (stat(canonical_path, &canon_st) == 0);

	DIR *d = opendir(dir);
	if (!d) {
		free(dir_alloc);
		return removed;
	}

	char **matches = NULL;
	size_t match_cnt = 0;
	size_t match_cap = 0;
	struct dirent *entry;
	while ((entry = readdir(d)) != NULL) {
		if (strcasecmp(base, entry->d_name) != 0)
			continue;
		if (keep_canonical && strcmp(base, entry->d_name) == 0)
			continue;
		char *full = fc_join_dir_entry(dir, has_slash, entry->d_name);
		if (!full)
			continue;
		if (match_cnt == match_cap) {
			size_t new_cap = match_cap ? match_cap * 2 : 4;
			char **tmp = realloc(matches, new_cap * sizeof(*matches));
			if (!tmp) {
				free(full);
				break;
			}
			matches = tmp;
			match_cap = new_cap;
		}
		matches[match_cnt++] = full;
	}
	closedir(d);

	bool backed_up = false;
	for (size_t i = 0; i < match_cnt; i++) {
		if (has_canon) {
			struct stat var_st;
			if (stat(matches[i], &var_st) == 0 &&
			    var_st.st_dev == canon_st.st_dev &&
			    var_st.st_ino == canon_st.st_ino) {
				free(matches[i]);
				continue;
			}
		}
		if (keep_canonical && backup_first_to && !backed_up) {
			if (rename(matches[i], backup_first_to) == 0) {
				backed_up = true;
				removed++;
				free(matches[i]);
				continue;
			}
		}
		if (unlink(matches[i]) == 0)
			removed++;
		free(matches[i]);
	}
	free(matches);
	free(dir_alloc);
	return removed;
}

static char *fc_search(const char *fname_utf8, const char *dir) {
	if (!dir)
		dir = ".";
	// If path contains no directory separator, do case-insensitive search.
	if (strchr(fname_utf8, '/') == NULL) {
		DIR *d = opendir(dir);
		if (d == NULL)
			return NULL;

		char *found = NULL;
		struct dirent *entry;
		while ((entry = readdir(d)) != NULL) {
			if (strcasecmp(fname_utf8, entry->d_name) == 0) {
				found = get_fullpath(dir, entry->d_name);
				break;
			}
		}
		closedir(d);
		return found;
	} else {
		// Check exact path first, then case-insensitive filename fallback.
		char *fullpath = get_fullpath(dir, fname_utf8);
		if (!fullpath)
			return NULL;
		char *resolved = fc_resolve_existing_path(fullpath);
		free(fullpath);
		return resolved;
	}
}

char *fc_get_path(const char *fname_utf8) {
	const char *dir = saveDataPath ? saveDataPath : ".";
	char *path = fc_search(fname_utf8, dir);
	if (path)
		return path;
	return get_fullpath(dir, fname_utf8);
}

bool fc_exists(const char *fname_utf8)
{
	char *path = fc_get_path(fname_utf8);
	bool result = access(path, F_OK) != -1;
	free(path);
	return result;
}

FILE *fc_open(const char *fname_utf8, char type) {
	const char *dir = saveDataPath ? saveDataPath : ".";
	char *fullpath = fc_search(fname_utf8, dir);
	if (!fullpath) {
		if (type == 'r') {
			fullpath = fc_search(fname_utf8, ".");
			if (!fullpath)
				return NULL;
		} else {
			fullpath = get_fullpath(dir, fname_utf8);
		}
	}

	FILE *fp;
	if (type == 'w') {
		// Ensure the directory exists
		char *p = strrchr(fullpath, '/');
		if (p) {
			*p = '\0';
			mkdir_p(fullpath);
			*p = '/';
		}
		fp = fopen(fullpath, "wb");
	} else {
		fp = fopen(fullpath, "rb");
	}
	free(fullpath);
	return fp;
}
#endif // _WIN32

void fc_backup_oldfile(const char *filename) {
#ifndef __EMSCRIPTEN__
	char *newname;
	
	if (!filename) return;
	newname = malloc(strlen(filename) + 3);
	
	strcpy(newname, filename);
	strcat(newname, ".");
	rename(filename, newname);
	
	free(newname);
#endif
}
