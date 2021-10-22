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

#ifndef __DEBUG_SYMBOL_H__
#define __DEBUG_SYMBOL_H__

#include "portab.h"

struct debug_symbols;

struct debug_symbols *dsym_load(const char *path);
int dsym_src2page(struct debug_symbols *dsym, const char *fname);
const char *dsym_page2src(struct debug_symbols *dsym, int page);
int dsym_line2addr(struct debug_symbols *dsym, int page, int line);
int dsym_addr2line(struct debug_symbols *dsym, int page, int addr);
const char *dsym_source_line(struct debug_symbols *dsym, int page, int line);
int dsym_num_variables(struct debug_symbols *dsym);
const char *dsym_variable_name(struct debug_symbols *dsym, int i);
int dsym_lookup_variable(struct debug_symbols *dsym, const char *name);
const char *dsym_addr2func(struct debug_symbols *dsym, int page, int addr);
boolean dsym_func2addr(struct debug_symbols *dsym, const char *name, int *inout_page, int *out_addr);

#endif // __DEBUG_SYMBOL_H__
