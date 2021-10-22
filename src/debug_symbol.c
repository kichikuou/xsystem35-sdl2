/*
 * Copyright (C) 2021 kichikuou <KichikuouChrome@gmail.com>
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

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "LittleEndian.h"
#include "debug_symbol.h"
#include "system.h"

// All strings are in UTF-8.

typedef struct {
	int line;
	int addr;
} Mapping;

typedef struct {
	char *filename;
	int nr_lines;
	char **lines;
	int nr_mappings;
	Mapping *mappings;
} SrcInfo;

typedef struct {
	char *name;
	int page;
	int addr;
	boolean is_local;
} FuncInfo;

struct debug_symbols {
	int version;
	int nr_srcs;
	SrcInfo *srcs;
	int nr_vars;
	char **variables;
	int nr_funcs;
	FuncInfo *functions;
};

const char *fget4cc(FILE *fp) {
	static char buf[5];
	if (fread(buf, 4, 1, fp) != 1) {
		WARNING("fread: %s\n", strerror(errno));
		buf[0] = '\0';
	}
	return buf;
}

int fgetdw(FILE *fp) {
	BYTE buf[4];
	if (fread(buf, 4, 1, fp) != 1) {
		WARNING("fread: %s\n", strerror(errno));
		return 0;
	}
	return LittleEndian_getDW(buf, 0);
}

static boolean ensure_srcs(struct debug_symbols *dsym, int nr_srcs) {
	if (!dsym->srcs) {
		dsym->nr_srcs = nr_srcs;
		dsym->srcs = calloc(nr_srcs, sizeof(SrcInfo));
	} else if (nr_srcs != dsym->nr_srcs) {
		WARNING("malformed debug information");
		return false;
	}
	return true;
}

static boolean load_srcs(struct debug_symbols *dsym, char *buf) {
	int nr_srcs = LittleEndian_getDW(buf, 0);
	if (!ensure_srcs(dsym, nr_srcs))
		return false;

	char *p = buf + 4;
	for (int i = 0; i < nr_srcs; i++) {
		dsym->srcs[i].filename = p;
		p += strlen(p) + 1;
	}
	return true;
}

static void store_src_lines(SrcInfo *info, char *src) {
	int line = 0;
	int cap = 0;
	char **lines = NULL;
	while (src) {
		char *eol = strchr(src, '\n');
		if (eol) {
			if (src < eol && eol[-1] == '\r')
				eol[-1] = '\0';
			else
				eol[0] = '\0';
			eol++;
		}

		if (line >= cap) {
			cap = cap ? cap * 2 : 128;
			lines = realloc(lines, cap * sizeof(char *));
		}
		lines[line++] = src;
		src = eol;
	}
	info->nr_lines = line;
	info->lines = lines;
}

static boolean load_scnt(struct debug_symbols *dsym, char *buf) {
	int nr_srcs = LittleEndian_getDW(buf, 0);
	if (!ensure_srcs(dsym, nr_srcs))
		return false;

	char *p = buf + 4;
	for (int i = 0; i < nr_srcs; i++) {
		int len = strlen(p);
		store_src_lines(&dsym->srcs[i], p);
		p += len + 1;
	}
	return true;
}

static boolean load_line(struct debug_symbols *dsym, unsigned char *buf) {
	int nr_srcs = LittleEndian_getDW(buf, 0);
	if (!ensure_srcs(dsym, nr_srcs))
		return false;

	int ofs = 4;
	for (int i = 0; i < nr_srcs; i++) {
		SrcInfo *info = &dsym->srcs[i];
		info->nr_mappings = LittleEndian_getDW(buf, ofs);
		ofs += 4;
		info->mappings = calloc(info->nr_mappings, sizeof(Mapping));
		for (int i = 0; i < info->nr_mappings; i++) {
			info->mappings[i].line = LittleEndian_getDW(buf, ofs);
			info->mappings[i].addr = LittleEndian_getDW(buf, ofs + 4);
			ofs += 8;
		}
	}
	return true;
}

static boolean load_vari(struct debug_symbols *dsym, char *buf) {
	dsym->nr_vars = LittleEndian_getDW(buf, 0);
	dsym->variables = calloc(dsym->nr_vars, sizeof(char *));
	char *p = buf + 4;
	for (int i = 0; i < dsym->nr_vars; i++) {
		dsym->variables[i] = p;
		p += strlen(p) + 1;
	}
	return true;
}

static boolean load_func(struct debug_symbols *dsym, char *buf) {
	dsym->nr_funcs = LittleEndian_getDW(buf, 0);
	dsym->functions = calloc(dsym->nr_funcs, sizeof(FuncInfo));
	char *p = buf + 4;
	for (int i = 0; i < dsym->nr_funcs; i++) {
		dsym->functions[i].name = p;
		p += strlen(p) + 1;
		dsym->functions[i].page = LittleEndian_getW(p, 0);
		dsym->functions[i].addr = LittleEndian_getDW(p, 2);
		dsym->functions[i].is_local = p[6];
		p += 7;
	}
	return true;
}

struct debug_symbols *dsym_load(const char *path) {
	FILE *fp = fopen(path, "rb");
	if (!fp) {
		WARNING("Cannot open %s: %s\n", path, strerror(errno));
		return NULL;
	}
	if (strcmp("DSYM", fget4cc(fp))) {
		WARNING("%s: wrong signature\n", path);
		fclose(fp);
		return NULL;
	}
	int version = fgetdw(fp);
	if (version != 0) {
		WARNING("%s: unsupported debug info version\n", path);
		fclose(fp);
		return NULL;
	}
	struct debug_symbols *dsym = calloc(1, sizeof(struct debug_symbols));
	dsym->version = version;

	int nr_sections = fgetdw(fp);
	for (int i = 0; i < nr_sections; i++) {
		const char *tag = fget4cc(fp);
		int section_size = fgetdw(fp);
		void *section_content = malloc(section_size);
		if (!section_content) {
			WARNING("Memory allocation failure while loading %s\n", path);
			fclose(fp);
			return NULL;
		}
		if (fread(section_content, section_size - 8, 1, fp) != 1) {
			WARNING("%s: I/O error\n", path, strerror(errno));
			fclose(fp);
			return NULL;
		}

		boolean ok = true;
		if (!strcmp(tag, "SRCS")) {
			ok = load_srcs(dsym, section_content);
		} else if (!strcmp(tag, "SCNT")) {
			ok = load_scnt(dsym, section_content);
		} else if (!strcmp(tag, "LINE")) {
			ok = load_line(dsym, section_content);
		} else if (!strcmp(tag, "VARI")) {
			ok = load_vari(dsym, section_content);
		} else if (!strcmp(tag, "FUNC")) {
			ok = load_func(dsym, section_content);
		} else {
			WARNING("%s: unrecognized section %s\n", path, tag);
		}
		if (!ok) {
			fclose(fp);
			return NULL;
		}
	}
	if (fgetc(fp) != EOF)
		WARNING("%s: broken debug information structure\n", path);
	fclose(fp);
	return dsym;
}

int dsym_src2page(struct debug_symbols *dsym, const char *fname) {
	if (!dsym)
		return -1;
	for (int i = 0; i < dsym->nr_srcs; i++) {
		if (!strcasecmp(dsym->srcs[i].filename, fname))
			return i;
	}
	return -1;
}

const char *dsym_page2src(struct debug_symbols *dsym, int page) {
	if (!dsym || page < 0 || page >= dsym->nr_srcs)
		return NULL;
	return dsym->srcs[page].filename;
}

int dsym_line2addr(struct debug_symbols *dsym, int page, int line) {
	if (!dsym || page < 0 || page >= dsym->nr_srcs)
		return -1;
	SrcInfo *srci = &dsym->srcs[page];

	// Binary search.
	int left = 0, right = srci->nr_mappings;
	while (left < right) {
		int mid = (left + right) / 2;
		if (srci->mappings[mid].line < line)
			left = mid + 1;
		else
			right = mid;
	}
	if (left == srci->nr_mappings) {
		assert(srci->nr_mappings == 0 || srci->mappings[srci->nr_mappings - 1].line < line);
		return -1;
	}

	assert(left == 0 || srci->mappings[left - 1].line < line);
	assert(line <= srci->mappings[left].line);

	return srci->mappings[left].addr;
}

int dsym_addr2line(struct debug_symbols *dsym, int page, int addr) {
	if (!dsym || page < 0 || page >= dsym->nr_srcs)
		return -1;
	SrcInfo *srci = &dsym->srcs[page];

	// Binary search.
	int left = 0, right = srci->nr_mappings;
	while (left < right) {
		int mid = (left + right) / 2;
		if (srci->mappings[mid].addr <= addr)
			left = mid + 1;
		else
			right = mid;
	}
	if (left == 0) {
		assert(srci->nr_mappings == 0 || addr < srci->mappings[0].addr);
		return -1;
	}

	assert(srci->mappings[left - 1].addr <= addr);
	assert(left == srci->nr_mappings || addr < srci->mappings[left].addr);

	return srci->mappings[left - 1].line;
}

const char *dsym_source_line(struct debug_symbols *dsym, int page, int line) {
	if (!dsym || page < 0 || page >= dsym->nr_srcs)
		return NULL;
	SrcInfo *info = &dsym->srcs[page];

	line--; // 1-based to 0-based index
	if (line < 0 || line >= info->nr_lines)
		return NULL;
	return info->lines[line];
}

int dsym_num_variables(struct debug_symbols *dsym) {
	if (!dsym)
		return 0;
	return dsym->nr_vars;
}

const char *dsym_variable_name(struct debug_symbols *dsym, int i) {
	if (!dsym || (unsigned)i >= dsym->nr_vars)
		return NULL;
	return dsym->variables[i];
}

int dsym_lookup_variable(struct debug_symbols *dsym, const char *name) {
	if (!dsym || !dsym->variables)
		return -1;

	for (int i = 0; i < dsym->nr_vars; i++) {
		if (!strcmp(name, dsym->variables[i]))
			return i;
	}
	return -1;
}

const char *dsym_addr2func(struct debug_symbols *dsym, int page, int addr) {
	if (!dsym)
		return NULL;

	// Binary search.
	int left = 0, right = dsym->nr_funcs;
	while (left < right) {
		int mid = (left + right) / 2;
		FuncInfo *f = &dsym->functions[mid];
		if (f->page < page || (f->page == page && f->addr <= addr))
			left = mid + 1;
		else
			right = mid;
	}
	if (left == 0)
		return NULL;
	FuncInfo *f = &dsym->functions[left - 1];
	if (f->page != page)
		return NULL;
	assert(f->addr <= addr);
	return f->name;
}

boolean dsym_func2addr(struct debug_symbols *dsym, const char *name, int *inout_page, int *out_addr) {
	if (!dsym)
		return false;

	for (int i = 0; i < dsym->nr_funcs; i++) {
		if (dsym->functions[i].is_local && dsym->functions[i].page != *inout_page)
			continue;
		if (!strcmp(dsym->functions[i].name, name)) {
			*inout_page = dsym->functions[i].page;
			*out_addr = dsym->functions[i].addr;
			return true;
		}
	}
	return false;
}
