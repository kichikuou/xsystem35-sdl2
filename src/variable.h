/*
 * variable.h  変数管理
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
/* $Id: variable.h,v 1.11 2003/04/22 16:34:28 chikama Exp $ */

#ifndef __VARIABLE__
#define __VARIABLE__

#include <sys/types.h>
#include "portab.h"


// #define SYSVAR_MAX       1024
//#define SYSVAR_MAX       8192#
#define SYSVAR_MAX       65536
#define SYSVARLONG_MAX   128
#define STRVAR_MAX       5000
#define STRVAR_LEN       101
#define ARRAYVAR_PAGEMAX 256

typedef struct {
	int size;
	boolean saveflag;
	int *value;
} arrayVarBufferStruct;

typedef struct {
	int *pointvar;
	int page;
	int offset;
} arrayVarStruct;

extern int sysVar[];
extern arrayVarStruct sysVarAttribute[];
extern arrayVarBufferStruct arrayVarBuffer[];
extern double longVar[];
extern boolean v_allocateArrayBuffer(int page, int size, boolean saveflag);
extern boolean v_defineArrayVar(int datavar, int *pointvar, int offset, int page);
extern boolean v_releaseArrayVar(int datavar);
extern boolean v_getArrayBufferStatus(int page);
extern boolean v_initVars();

extern void svar_init(int cnt, int len);
extern const char *svar_get(int no);
extern void svar_set(int no, const char *str);
extern void svar_copy(int dstno, int dstpos, int srcno, int srcpos, int len);
extern void svar_append(int no, const char *str);
extern size_t svar_length(int no);
extern int svar_width(int no);
extern int svar_find(int no, int start, const char *str);
extern void svar_fromVars(int no, const int *vars);
extern int svar_toVars(int no, int *vars);
extern int svar_getCharType(int no, int pos);
extern void svar_replaceAll(int no, int pattern, int replacement);

#endif /* !__VARIABLE__ */
