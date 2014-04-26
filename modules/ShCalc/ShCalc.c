/*
 * ShCalc.c  ６４ビット演算 module
 *
 *      かえるにょ国にょアリス
 *      大悪司
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
/* $Id: ShCalc.c,v 1.4 2002/08/23 14:56:40 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>

#include "portab.h"
#include "nact.h"
#include "system.h"
#include "xsystem35.h"
#include "randMT.h"

static int numbase;
static gint64  l_1000A0C8;
static gint64  l_1000A0D0;
static gint64  l_1000A0D8;

static gint64 mul64(gint64 a1, gint64 a2) {
	return a1 * a2;
}

static gint64 div64(gint64 a1, gint64 a2) {
	if (a1 == 0 || a2 == 0) return 0;
	
	return a1 / a2;
}

void SetIntNumBase(void) { /* 0 */
	/*
	  64 bit 演算のための倍数をセット
	  
	  base: 倍数
	*/
	int base = getCaliValue();
	
	numbase = base;
	
	DEBUG_COMMAND("ShCalc.SetIntNumBase %d:\n", base);
	
}

void SetIntNum16(void) { /* 1 */
	/*
	  64 bit 演算のための数値をセット

	  var: 数値の入った変数
	*/
	int *var = getCaliVariable();
	
	l_1000A0D8 = mul64(*var, numbase);
	
	DEBUG_COMMAND("ShCalc.SetIntNum16 %p:\n", var);
}

void SetIntNum32(void) { /* 2 */
	int p1 = getCaliValue();
	
	DEBUG_COMMAND_YET("ShCalc.SetIntNum32: %d:\n", p1);
}

void GetIntNum16(void) { /* 3 */
	/*
	  64 bit 演算した結果を得る

	  var: 数値をいれる変数
	 */
	int *var = getCaliVariable();
	gint64 i;
	
	i = div64(l_1000A0D8, numbase);
	
	if (i > 65535) {
		i = l_1000A0C8 = 65535;
	}
	
	*var = i;
	
	DEBUG_COMMAND("ShCalc.GetIntNum16 %d:\n", var);
}

void GetIntNum32(void) { /* 4 */
	int p1 = getCaliValue();
	
	DEBUG_COMMAND_YET("ShCalc.GetIntNum32: %d:\n", p1);
}

void AddIntNum16(void) { /* 5 */
	/*
	  64 bit 加算

	  var: 足す数の入った変数
	 */
	int *var = getCaliVariable();
	
	l_1000A0D8 += mul64(*var, numbase);
	
	DEBUG_COMMAND("ShCalc.AddIntNum16 %p:\n", var);
}

void AddIntNum32(void) { /* 6 */
	int p1 = getCaliValue();
	
	DEBUG_COMMAND_YET("ShCalc.AddIntNum32: %d:\n", p1);
}

void SubIntNum16(void) { /* 7 */
	int p1 = getCaliValue();
	
	DEBUG_COMMAND_YET("ShCalc.SubIntNum16: %d:\n", p1);
}

void SubIntNum32(void) { /* 8 */
	int p1 = getCaliValue();
	
	DEBUG_COMMAND_YET("ShCalc.SubIntNum32: %d:\n", p1);
}

void MulIntNum16(void) {  /* 9 */
	/*
	  64 bit 乗算
	  
	  var: 掛ける数の入った変数
	 */
	int *var = getCaliVariable();
	
	l_1000A0D8 *= mul64(*var, numbase);
	
	DEBUG_COMMAND("ShCalc.MulIntNum16: %p:\n", var);
}

void MulIntNum32(void) {  /* 10 */
	int p1 = getCaliValue();
	
	DEBUG_COMMAND_YET("ShCalc.MulIntNum32: %d:\n", p1);
}

void DivIntNum16(void) { /* 11 */
	/*
	  64 bit 除算

	   var: 足す数の入った変数
	 */
	int *var = getCaliVariable();
	gint64 i;
	
	l_1000A0C8 = *var;
	
	i = mul64(l_1000A0C8, numbase);
	l_1000A0D8 = div64(l_1000A0D8, i);
	
	DEBUG_COMMAND("ShCalc.DivIntNum16 %p:\n", var);
}

void DivIntNum32(void) { /* 12 */ 
	int p1 = getCaliValue();

	DEBUG_COMMAND_YET("ShCalc.DivIntNum32: %d:\n", p1);
}

void CmpIntNum16(void) { /* 13 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();

	DEBUG_COMMAND_YET("ShCalc.CmpIntNum16: %d,%d,%d:\n", p1, p2, p3);
}

void CmpIntNum32(void) { /* 14 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();

	DEBUG_COMMAND_YET("ShCalc.CmpIntNum32: %d,%d,%d:\n", p1, p2, p3);
}
	
void GetLengthNum16(void) { /* 15 */
	/*
	  数値の桁数を返す
	   var: 数値
	   vResult: 数値の桁数を返す変数
	 */
	int *var     = getCaliVariable();
	int *vResult = getCaliVariable();
	
	if (*var >= 10000) {
		*vResult = 5;
	} else if (*var >= 1000) {
		*vResult = 4;
	} else if (*var >= 100) {
		*vResult = 3;
	} else if (*var >= 10) {
		*vResult = 2;
	} else {
		*vResult = 1;
	}
	
	DEBUG_COMMAND("ShCalc.GetLengthNum16 %p,%p:\n", var, vResult);
}

void GetLengthNum32(void) { /* 16 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	
	DEBUG_COMMAND_YET("ShCalc.GetLengthNum32: %d,%d:\n", p1, p2);
}

void NumToRate(void) { /* 17 */
	/*
	  p1 * (p3/p2) の演算結果を返す。

	   p1: 数値１
	   p2: 数値２
	   p3: 数値３
	   flag: 0  切捨て
	         1  切り上げ
	   vResult: 結果を返す変数 
	 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int flag = getCaliValue();
	int *vResult = getCaliVariable();
	int i;
	
	i = (p1 * p3) / p2;
	
	if (flag != 0) {
		if ((i * p2) != (p1 * p3)) {
			i ++;
		}
	}
	
	*vResult = i;
	
	DEBUG_COMMAND("ShCalc.NumToRate %d,%d,%d,%d,%p:\n", p1, p2, p3, flag, vResult);
	
}

void NumToRateNum(void) { /* 18 */
	/*
	  p1 * (p2/p3) の演算結果を返す。

	   p1: 数値１
	   p2: 数値２
	   p3: 数値３
	   flag: 0  切捨て
	         1  切り上げ
	   vResult: 結果を返す変数 
	*/
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int flag = getCaliValue();
	int *vResult = getCaliVariable();
	int i;
	
	i = (p1 * p2) / p3;
	
	if (flag != 0) {
		if ((i * p3) != (p1 * p2)) {
			i ++;
		}
	}
	
	*vResult = i;
	
	DEBUG_COMMAND("ShCalc.NumToRateNum %d,%d,%d,%d,%p:\n", p1, p2, p3, flag, vResult);
}

void SetRandomSeed() {
	/*
	  乱数の種の設定
	  
	    seed: 種
	*/
	int seed = getCaliValue();

	sgenrand(seed);
	
	DEBUG_COMMAND("ShCalc.SetRandomSeed %d:\n", seed);
}

void GetRandomNumA() {
	int num  = getCaliValue();
	int *var = getCaliVariable();

	if (num == 0 || num == 1) {
		*var = num;
	} else {
		*var = (int)(genrand() * num) + 1;
	}
	
	DEBUG_COMMAND("ShCalc.GetRandomNumA %d,%p:\n", num, var);
}

void NumToBit() {
	/*
	  2^(beki-1)
	  
	  var: 値を返す変数
	*/
	int beki = getCaliValue();
	int *var = getCaliVariable();
	int i, j = 1;
	
	if (beki < 17) {
		for (i = 1; i < beki; i++) {
			j <<= 1;
		}
		*var = j;
	} else {
		*var = 0;
	}

	DEBUG_COMMAND("ShCalc.NumToBit %d,%p:\n", beki, var);
}

void BitToNum() {
	/*
	  val -> var
	  1   -> 1
	  2   -> 2
	  4   -> 3
	  8   -> 4
          16  -> 5 ....
	  others -> 0
	  
	  var: 値を返す変数
	*/
	int val = getCaliValue();
	int *var = getCaliVariable();
	int i;
	
	DEBUG_COMMAND("ShCalc.BitToNum %d,%p:\n", val, var);
	
	if (val == 0) {
		*var = 0;
		return;
	}
	
	*var = 1;
	while(val != 1) {
		if (val % 2) {
			*var = 0;
			break;
		}
		val /= 2;
		(*var)++;
	}
}
