/*
 * alk.h  ALK archive manager
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
 * @version 1.0     01/11/29 initial version
*/
/* $Id: alk.h,v 1.2 2003/11/09 15:06:13 chikama Exp $ */

#ifndef __ALK_H__
#define __ALK_H__

#include <unistd.h>
#include <sys/types.h>

/* 
   .ALK データ構造
   
   char[4] sig = "ALK0"; シグネチャ
   int32   num = ファイル数
     int32 offset; 個々のファイルのファイルの先頭からのオフセット
     int32 size;   個々のファイルの大きさ(in bytes)
*/     

typedef struct {
	int   fd;      /* .alk ファイルの file discpriter   */
	char *mapadr;  /* mmap したファイルの先頭アドレス   */
	off_t size;    /* mmap したバイト数(ファイルサイズ) */
	int   datanum; /* .alk ファイル中のファイル数       */
	int  *offset;  /* 各ファイルへのオフセット          */
} alk_t;

extern alk_t *alk_new(char *path);
extern int    alk_free(alk_t *alk);
extern char  *alk_get(alk_t *alk, int no);

#endif /* __ALK_H__ */
