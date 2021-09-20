/*
 * bgi.c: BGI (BGM information) parser
 *
 * Copyright (C) 1997-1998 Masaki Chikama (Wren) <chikama@kasumi.ipl.mech.nagoya-u.ac.jp>
 *               1998-                           <masaki-c@is.aist-nara.ac.jp>
 *               2021 kichikuou <KichikuouChrome@gmail.com>
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
#ifndef _BGI_H__
#define _BGI_H__

typedef struct {
	int no;      // シナリオ上での曲番号
	int loopno;  // ループ回数 (０の場合は無限)
	int looptop; // ループ時先頭戻り位置 (サンプル数)
	int len;     // 曲長さ (サンプル数)
} bgi_t;

extern int bgi_read(const char *path);
extern bgi_t *bgi_find(int no);

#endif /* _BGI_H__ */
