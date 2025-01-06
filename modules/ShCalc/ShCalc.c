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
#include <stdint.h>

#include "portab.h"
#include "nact.h"
#include "system.h"
#include "xsystem35.h"
#include "modules.h"
#include "randMT.h"

static int numbase;
static int64_t  accumulator;

static int64_t mul64(int64_t a1, int64_t a2) {
	return a1 * a2;
}

static int64_t div64(int64_t a1, int64_t a2) {
	if (a1 == 0 || a2 == 0) return 0;
	
	return a1 / a2;
}

static int64_t get32(int *var) {
	return var[0] + mul64(var[1], 0x10000);
}

static void SetIntNumBase(void) { /* 0 */
	/*
	  64 bit 演算のための倍数をセット
	  
	  base: 倍数
	*/
	int base = getCaliValue();
	
	numbase = base;
	
	TRACE("ShCalc.SetIntNumBase %d:", base);
	
}

static void SetIntNum16(void) { /* 1 */
	/*
	  64 bit 演算のための数値をセット

	  var: 数値の入った変数
	*/
	int *var = getCaliVariable();
	
	accumulator = mul64(*var, numbase);
	
	TRACE("ShCalc.SetIntNum16 %p:", var);
}

static void SetIntNum32(void) { /* 2 */
	int *var = getCaliVariable();

	accumulator = mul64(get32(var), numbase);
	
	TRACE("ShCalc.SetIntNum32: %p:", var);
}

static void GetIntNum16(void) { /* 3 */
	/*
	  64 bit 演算した結果を得る

	  var: 数値をいれる変数
	 */
	int *var = getCaliVariable();
	int64_t i;
	
	i = div64(accumulator, numbase);
	
	if (i > 65535) {
		i = 65535;
	}
	
	*var = i;
	
	TRACE("ShCalc.GetIntNum16 %d:", var);
}

static void GetIntNum32(void) { /* 4 */
	int *var = getCaliVariable();
	int64_t i = div64(accumulator, numbase);
	
	var[0] = i & 0xFFFF;
	i >>= 16;
	var[1] = i > 0xFFFF ? 0xFFFF : i;

	TRACE("ShCalc.GetIntNum32: %p:", var);
}

static void AddIntNum16(void) { /* 5 */
	/*
	  64 bit 加算

	  var: 足す数の入った変数
	 */
	int *var = getCaliVariable();
	
	accumulator += mul64(*var, numbase);
	
	TRACE("ShCalc.AddIntNum16 %p:", var);
}

static void AddIntNum32(void) { /* 6 */
	int *var = getCaliVariable();

	accumulator += mul64(get32(var), numbase);

	TRACE("ShCalc.AddIntNum32: %p:", var);
}

static void SubIntNum16(void) { /* 7 */
	int p1 = getCaliValue();
	
	TRACE_UNIMPLEMENTED("ShCalc.SubIntNum16: %d:", p1);
}

static void SubIntNum32(void) { /* 8 */
	int *var = getCaliVariable();

	accumulator -= mul64(get32(var), numbase);

	TRACE("ShCalc.SubIntNum32: %p:", var);
}

static void MulIntNum16(void) {  /* 9 */
	/*
	  64 bit 乗算
	  
	  var: 掛ける数の入った変数
	 */
	int *var = getCaliVariable();
	
	accumulator *= mul64(*var, numbase);
	
	TRACE("ShCalc.MulIntNum16: %p:", var);
}

static void MulIntNum32(void) {  /* 10 */
	int p1 = getCaliValue();
	
	TRACE_UNIMPLEMENTED("ShCalc.MulIntNum32: %d:", p1);
}

static void DivIntNum16(void) { /* 11 */
	/*
	  64 bit 除算

	   var: 足す数の入った変数
	 */
	int *var = getCaliVariable();
	int64_t i;
	
	i = mul64(*var, numbase);
	accumulator = div64(accumulator, i);
	
	TRACE("ShCalc.DivIntNum16 %p:", var);
}

static void DivIntNum32(void) { /* 12 */
	int p1 = getCaliValue();

	TRACE_UNIMPLEMENTED("ShCalc.DivIntNum32: %d:", p1);
}

static void CmpIntNum16(void) { /* 13 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();

	TRACE_UNIMPLEMENTED("ShCalc.CmpIntNum16: %d,%d,%d:", p1, p2, p3);
}

static void CmpIntNum32(void) { /* 14 */
	int *var = getCaliVariable();
	int op = getCaliValue();
	int *result = getCaliVariable();

	int64_t val = mul64(get32(var), numbase);

	switch (op) {
	case 1: *result = val == accumulator; break;
	case 2: *result = val > accumulator; break;
	case 3: *result = val < accumulator; break;
	case 4: *result = val >= accumulator; break;
	case 5: *result = val <= accumulator; break;
	default:
		WARNING("ShCalc.CmpIntNum32: unknown operator %d", op);
		break;
	}

	TRACE("ShCalc.CmpIntNum32: %p,%d,%p:", var, op, result);
}
	
static void GetLengthNum16(void) { /* 15 */
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
	
	TRACE("ShCalc.GetLengthNum16 %p,%p:", var, vResult);
}

static void GetLengthNum32(void) { /* 16 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	
	TRACE_UNIMPLEMENTED("ShCalc.GetLengthNum32: %d,%d:", p1, p2);
}

static void NumToRate(void) { /* 17 */
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
	
	TRACE("ShCalc.NumToRate %d,%d,%d,%d,%p:", p1, p2, p3, flag, vResult);
	
}

static void NumToRateNum(void) { /* 18 */
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
	
	TRACE("ShCalc.NumToRateNum %d,%d,%d,%d,%p:", p1, p2, p3, flag, vResult);
}

static void SetRandomSeed() {
	/*
	  乱数の種の設定
	  
	    seed: 種
	*/
	int seed = getCaliValue();

	sgenrand(seed);
	
	TRACE("ShCalc.SetRandomSeed %d:", seed);
}

static void GetRandomNumA() {
	int num  = getCaliValue();
	int *var = getCaliVariable();

	if (num == 0 || num == 1) {
		*var = num;
	} else {
		*var = (int)(genrand() * num) + 1;
	}
	
	TRACE("ShCalc.GetRandomNumA %d,%p:", num, var);
}

static void NumToBit() {
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

	TRACE("ShCalc.NumToBit %d,%p:", beki, var);
}

static void BitToNum() {
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
	
	TRACE("ShCalc.BitToNum %d,%p:", val, var);
	
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

static const ModuleFunc functions[] = {
	{"AddIntNum16", AddIntNum16},
	{"AddIntNum32", AddIntNum32},
	{"BitToNum", BitToNum},
	{"CmpIntNum16", CmpIntNum16},
	{"CmpIntNum32", CmpIntNum32},
	{"DivIntNum16", DivIntNum16},
	{"DivIntNum32", DivIntNum32},
	{"GetIntNum16", GetIntNum16},
	{"GetIntNum32", GetIntNum32},
	{"GetLengthNum16", GetLengthNum16},
	{"GetLengthNum32", GetLengthNum32},
	{"GetRandomNumA", GetRandomNumA},
	{"MulIntNum16", MulIntNum16},
	{"MulIntNum32", MulIntNum32},
	{"NumToBit", NumToBit},
	{"NumToRate", NumToRate},
	{"NumToRateNum", NumToRateNum},
	{"SetIntNum16", SetIntNum16},
	{"SetIntNum32", SetIntNum32},
	{"SetIntNumBase", SetIntNumBase},
	{"SetRandomSeed", SetRandomSeed},
	{"SubIntNum16", SubIntNum16},
	{"SubIntNum32", SubIntNum32},
};

const Module module_ShCalc = {"ShCalc", functions, sizeof(functions) / sizeof(ModuleFunc)};
