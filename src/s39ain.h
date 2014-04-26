/*
 * s39ain.h  System39.ain read
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
/* $Id: s39ain.h,v 1.1 2001/09/16 15:59:11 chikama Exp $ */

#ifndef __S39AIN_H__
#define __S39AIN_H__

#include <ltdl.h>
#include "portab.h"

/* DLL 内関数情報 */
typedef struct {
	char *name; /* 関数名 */
	int   argc; /* 関数の引数の数 */
	int  *argv; /* 関数の引数のそれぞれの種類 */
} S39AIN_DLLFN;

/* DLL 情報 */
typedef struct {
	lt_dlhandle *handle;       /* DLL handler */
	char        *name;         /* DLL 名      */
	int          function_num; /* 関数の数    */
	S39AIN_DLLFN       *function; /* 関数本体 */
} S39AIN_DLLINF;

/* シナリオ関数情報 */
typedef struct {
	char *name; /* シナリオ関数名 */
	int page;   /* シナリオ上の位置 (ページ番号) */
	int index;  /* シナリオ上の位置 (アドレス)   */
} S39AIN_FUNCNAME;

/* System39.ain 全体の情報 */
typedef struct {
	char *path_to_ain; /* system39.ain へのパス  */
	char *path_to_dll; /* DLL モジュールへのパス */
	
	int   dllnum; /* DLL  の数 */
	int   fncnum; /* FUNC の数 */
	int   varnum; /* VARI の数 */
	int   msgnum; /* MSGI の数 */
	
	S39AIN_DLLINF   *dll; /* DLL  に関数る情報 */
	S39AIN_FUNCNAME *fnc; /* FUNC に関する情報 */
	char **var;           /* VARI に関する情報 */
	char **msg;           /* MSGI に関する情報 */
} S39AIN;

extern int s39ain_init(void);

#endif /* __S39AIN_H__ */
