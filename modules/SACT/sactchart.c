/*
 * sactchart.c: SACT.ChartPos 処理
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
/* $Id: sactchart.c,v 1.2 2003/04/27 11:00:36 chikama Exp $ */

#include <stdio.h>
#include "portab.h"

// グラフ用チャート作成
int schart_pos(int *pos, int pos1, int pos2, int val1, int val2, int val) {
	if (val1 == val2) {
		*pos = 0;
	} else {
		*pos = (((pos2 - pos1) * (val - val1)) / (val2 - val1)) + pos1;
	}
	return OK;
}

/*
37,103f: SACT.ChartPos 0x8b627648,0,135,0,99,66:   ->90
37,103f: SACT.ChartPos 0x8b627648,0,135,0,99,99:   ->135
37,103f: SACT.ChartPos 0x8b627648,0,135,0,300,26:  ->11
37,103f: SACT.ChartPos 0x8b627648,0,135,0,300,300: ->135
37,103f: SACT.ChartPos 0x8b627648,0,135,0,40,40:   ->135
37,103f: SACT.ChartPos 0x8b627648,0,135,0,40,40:
37,103f: SACT.ChartPos 0x8b627648,0,70,0,25,1:     ->2
37,103f: SACT.ChartPos 0x8b627648,0,70,0,25,25:    ->70
37,103f: SACT.ChartPos 0x8b627648,0,70,0,25,25:
37,65d: SACT.ChartPos 0x8b627510,0,100,0,99,66:    ->66
37,65d: SACT.ChartPos 0x8b627510,0,100,0,300,26:   ->8
37,65d: SACT.ChartPos 0x8b627510,0,70,0,300,20:    ->4
78,97a0: SACT.ChartPos 0x5812e638,10000,9520,0,240,1: ->9998
78,97a0: SACT.ChartPos 0x5812e638,10000,9520,0,240,240: ->9520
*/
