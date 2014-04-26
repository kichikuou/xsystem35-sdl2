/*
 * RandMT.c  王子さま Lv1: (おそらく Mersenne Twister使用の)乱数生成
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
/* $Id: RandMT.c,v 1.3 2002/09/01 11:54:51 chikama Exp $ */

#include "config.h"

#include <stdio.h>

#include "portab.h"
#include "system.h"
#include "xsystem35.h"
#include "nact.h"
#include "randMT.h"

void Init() {
	/*
	  乱数初期化

	   p1: 初期化用 seed
	*/
	int p1 = getCaliValue(); /* ITimer */
	
	DEBUG_COMMAND_YET("RandMT.Init %p:\n", p1);
}

void Get() {
	/*
	  1 から num までの乱数を生成
	  
	  num: 最大値
	  var: 結果を返す変数
	*/
	int num  = getCaliValue();
	int *var = getCaliVariable();
	
	if (num == 0 || num == 1) {
		*var = num;
	} else {
		*var = (int)(genrand() * num) + 1;
	}
	
	DEBUG_COMMAND_YET("RandMT.Get %d,%p:\n", num, var);
}

void GetNoOverlap() { /* not used ? */
	int p1    = getCaliValue();
	int p2    = getCaliValue();
	int *var1 = getCaliVariable();
	
	DEBUG_COMMAND_YET("RandMT.GetNoOverlap %p:\n", p1);
}
