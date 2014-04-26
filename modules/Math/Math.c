/*
 * Math.c  汎用数学関数？ OnlyYou -リ・クスル他
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
/* $Id: Math.c,v 1.3 2001/11/29 11:21:44 chikama Exp $ */

#include "config.h"

#include <stdio.h>

#include "portab.h"
#include "system.h"
#include "xsystem35.h"
#include "nact.h"
#include "randMT.h"

static int numtblmax;

void RandMTInit() {
	/*
	  (おそらく Mersenne Twister使用の) 乱数初期化

	   p1: 初期化用 seed
	*/
	int p1 = getCaliValue(); /* ITimer */
	
	DEBUG_COMMAND("Math.RandMTInit %d:\n", p1);
}

void RandMTGet() {
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
	
	DEBUG_COMMAND("Math.RandMTGet %d,%p:\n", num, var);
}

void RandMTMakeNumTable() {
	/*
	  乱数テーブルの最大値を設定

	  p1: 最大値
	*/
	int p1 = getCaliValue();
	
	numtblmax = p1;
	
	DEBUG_COMMAND("Math.RandMTMakeNumTable %d:\n", p1);
}

void RandMTGetNumTable() {
	/*
	  乱数テーブルから値を取得

	  var: 乱数を格納する変数
	*/
	int *var = getCaliVariable();
	
	*var = (int)(genrand() * numtblmax) + 1;
	
	DEBUG_COMMAND("Math.RandMTGetNumTable %d:\n", *var);
}
