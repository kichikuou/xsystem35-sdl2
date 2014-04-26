/*
 * ShArray.c  各種配列演算 module
 *
 *    かえるにょ国にょアリス
 *    大悪司
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
/* $Id: ShArray.c,v 1.4 2002/08/18 09:35:29 chikama Exp $ */

#include "config.h"

#include <stdio.h>

#include "portab.h"
#include "system.h"
#include "xsystem35.h"
#include "nact.h"

void GetAtArray(void) { /* 0 */
	/*
	  配列から演算しながら値を取り出す
	  
	  vAry: 配列
	  cnt : 個数
	  type: 演算の種類
	  vResult: 演算結果を返す変数
	*/
	int *vAry    = getCaliVariable();
	int cnt      = getCaliValue();
	int type     = getCaliValue();
	int *vResult = getCaliVariable();
	int i, j;

	DEBUG_COMMAND("ShArray.GetAtArray %p,%d,%d,%p:\n", vAry, cnt, type, vResult);
	
	j = *vAry; vAry++;
	for (i = 1; i < cnt; i++) {
		switch(type) {
		case 1:
			j += *vAry;
			break;
		case 2:
			j *= *vAry;
			break;
		case 3:
			j &= *vAry;
			break;
		case 4:
			j |= *vAry;
			break;
		case 5:
			j ^= *vAry;
			break;
		}
		vAry++;
	}
	
	if (j > 65535) {
		j = 65535;
	}
	
	*vResult = j;
}

void AddAtArray(void) { /* 1 */
	/*
	  配列１に配列２を足す。65535 を超えたら 65535 に。
	  
	  vAry1: 配列１
	  vAry2: 配列２
	  cnt  : 個数
	*/
	int *vAry1 = getCaliVariable();
	int *vAry2 = getCaliVariable();
	int cnt    = getCaliValue();
	int i;
	
	DEBUG_COMMAND("ShArray.AddAtArray %p,%p,%d:\n", vAry1, vAry2, cnt);
	
	for (i = 0; i < cnt; i++) {
		int result = (*vAry1) + (*vAry2);
		if (result > 65535) {
			*vAry1 = 65535;
		} else {
			*vAry1 = result;
		}
		vAry1++; vAry2++;
	}
}

void SubAtArray(void) { /* 2 */
	/*
	  配列１から配列２を引く。負になったら０をかく
	  
 	  vAry1: 配列１
	  vAry2: 配列２
	  cnt  : 個数
	*/
	int *vAry1 = getCaliVariable();
	int *vAry2 = getCaliVariable();
	int cnt    = getCaliValue();
	int i;
	
	DEBUG_COMMAND("ShArray.SubAtArray %p,%p,%d:\n", vAry1, vAry2, cnt);
	
	for (i = 0; i < cnt; i++) {
		int result = (*vAry1) - (*vAry2);
		if (result < 0) {
			*vAry1 = 0;
		} else {
			*vAry1 = result;
		}
		vAry1++; vAry2++;
	}
}

void MulAtArray(void) { /* 3 */
	/*
	  配列１に配列２をかけて、配列１に格納、65535まで。

 	  vAry1: 配列１
	  vAry2: 配列２
	  cnt  : 個数
	*/
	int *vAry1 = getCaliVariable();
	int *vAry2 = getCaliVariable();
	int cnt    = getCaliValue();
	int i;
	
	DEBUG_COMMAND("ShArray.MulAtArray %p,%p,%d:\n", vAry1, vAry2, cnt);
	
	for (i = 0; i < cnt; i++) {
		int result = (*vAry1) * (*vAry2);
		if (result > 65535) {
			*vAry1 = 65535;
		} else {
			*vAry1 = result;
		}
		vAry1++; vAry2++;
	}
}

void DivAtArray(void) { /* 4 */
	/*
	  配列１を配列２で割って、配列１に格納、65535まで。
	  
 	  vAry1: 配列１
	  vAry2: 配列２
	  cnt  : 個数
	*/
	int *vAry1 = getCaliVariable();
	int *vAry2 = getCaliVariable();
	int cnt    = getCaliValue();
	int i;
	
	DEBUG_COMMAND("ShArray.DivAtArray: %d,%d,%d:\n", vAry1, vAry2, cnt);
	
	for (i = 0; i < cnt; i++) {
		if (*vAry2 == 0) {
			*vAry1 = 0;
		} else {
			int result = (*vAry1) / (*vAry2);
			if (result > 65535) {
				*vAry1 = 65535;
			} else {
				*vAry1 = result;
			}
		}
		vAry1++; vAry2++;
	}
}

void MinAtArray(void) { /* 5 */
	/*
	  配列 vAry1 の中身を配列 vAry2 で下限に設定する

 	  vAry1: 配列１
	  vAry2: 配列２
	  cnt  : 個数
	*/
	int *vAry1 = getCaliVariable();
	int *vAry2 = getCaliVariable();
	int cnt    = getCaliValue();
	int i;
	
	DEBUG_COMMAND("ShArray.MinAtArray: %d,%d,%d:\n", vAry1, vAry2, cnt);
	
	for (i = 0; i < cnt; i++) {
		if (*vAry1 < *vAry2) {
			*vAry1 = *vAry2;
		}
		vAry1++; vAry2++;
	}
}

void MaxAtArray(void) { /* 6 */
	/*
	  配列 vAry1 の中身を配列 vAry2 で上限に設定する
	  
 	  vAry1: 配列１
	  vAry2: 配列２
	  cnt  : 個数
	*/
	int *vAry1 = getCaliVariable();
	int *vAry2 = getCaliVariable();
	int cnt    = getCaliValue();
	int i;
	
	DEBUG_COMMAND("ShArray.MaxAtArray: %d,%d,%d:\n", vAry1, vAry2, cnt);
	
	for (i = 0; i < cnt; i++) {
		if (*vAry1 > *vAry2) {
			*vAry1 = *vAry2;
		}
		vAry1++; vAry2++;
	}
}

void AndNumArray(void) { /* 7 */
	/*
	  配列のデータと val で AND をとる

	  vAry: 配列
	  cnt : 個数
	  val : ANDをとる値
	*/
	int *vAry = getCaliVariable();
	int cnt   = getCaliValue();
	int val   = getCaliValue();
	int i;
	
	DEBUG_COMMAND("ShArray.AndNumArray: %p,%d,%d:\n", vAry, cnt, val);
	
	for (i = 0; i < cnt; i++) {
		(*vAry) &= val;
		vAry++;
	}
}

void OrNumArray(void) { /* 8 */
	/*
	  配列のデータと val で OR をとる

	  vAry: 配列
	  cnt : 個数
	  val : ORをとる値
	*/
	int *vAry = getCaliVariable();
	int cnt   = getCaliValue();
	int val   = getCaliValue();
	int i;
	
	DEBUG_COMMAND_YET("ShArray.OrNumArray: %p,%d,%d:\n", vAry, cnt, val);
	
	for (i = 0; i < cnt; i++) {
		(*vAry) |= val;
		vAry++;
	}
}

void XorNumArray(void) { /* 9 */
	/*
	  配列のデータと val で XOR をとる

	  vAry: 配列
	  cnt : 個数
	  val : XORをとる値
	*/
	int *vAry = getCaliVariable();
	int cnt   = getCaliValue();
	int val   = getCaliValue();
	int i;
	
	DEBUG_COMMAND("ShArray.XorNumArray %p,%d,%d:\n", vAry, cnt, val);
	
	for (i = 0; i < cnt; i++) {
		(*vAry) ^= val;
		vAry++;
	}
}

void SetEquArray(void) { /* 10 */
	/*
	  配列が val と等しければ 配列 vResults に 1 を、そうでなければ
	  0 を代入
	  
	  vAry: 配列
	  cnt : 個数
	  val : 比較する値
	  vResults  : 結果を格納する配列
	*/
	int *vAry = getCaliVariable();
	int cnt   = getCaliValue();
	int val   = getCaliValue();
	int *vResults = getCaliVariable();
	int i;
	
	DEBUG_COMMAND("ShArray.SetEquArray %p,%d,%d,%p:\n", vAry, cnt, val, vResults);
	
	for (i = 0; i < cnt; i++) {
		*vResults = (*vAry == val) ? 1 : 0;
		vResults++; vAry++;
	}
}

void SetNotArray(void) { /* 11 */
	/*
	  配列が val と等しくなければ配列 vResults に 1 を、そうでなければ
	  0 を代入
	  
	  vAry: 配列
	  cnt : 個数
	  val : 比較する値
	  vResults  : 結果を格納する配列
	*/
	int *vAry = getCaliVariable();
	int cnt   = getCaliValue();
	int val   = getCaliValue();
	int *vResults = getCaliVariable();
	int i;
	
	DEBUG_COMMAND("ShArray.SetNotArray %p,%d,%d,%p:\n", vAry, cnt, val, vResults);

	for (i = 0; i < cnt; i++) {
		*vResults = (*vAry != val) ? 1 : 0;
		vResults++; vAry++;
	}
}

void SetLowArray(void) { /* 12 */
	/*
	  配列データが val よりも小さければ vResult に 1 をセット
	  
	  vAry: 配列
	  cnt : 個数
	  val : 閾値
	  vResult: 結果を返す変数
	*/
	int *vAry = getCaliVariable();
	int cnt   = getCaliValue();
	int val   = getCaliValue();
	int *vResults = getCaliVariable();
	int i;
	
	DEBUG_COMMAND("ShArray.SetLowArray %p,%d,%d,%p:\n", vAry, cnt, val, vResults);
	
	for (i = 0; i < cnt; i++) {
		(*vResults) = ((*vAry < val) ? 1 : 0);
		vResults++; vAry++;
	}
}

void SetHighArray(void) { /* 13 */
	/*
	  配列データが val よりも大きければ vResult に 1 をセット
	  
	  vAry: 配列
	  cnt : 個数
	  val : 閾値
	  vResults: 結果を返す変数
	*/
	int *vAry = getCaliVariable();
	int cnt   = getCaliValue();
	int val   = getCaliValue();
	int *vResults = getCaliVariable();
	int i;
	
	DEBUG_COMMAND("ShArray.SetHighArray %p,%d,%d,%p:\n", vAry, cnt, val, vResults);
	
	for (i = 0; i < cnt; i++) {
		(*vResults) = ((*vAry > val) ? 1 : 0);
		vResults++; vAry++;
	}
}

void SetRangeArray(void) { /* 14 */
	/* 
	   配列データがある範囲(min〜max)にあるかチェック
	   
	   vAry: 配列
	   cnt : 個数
	   min : 最小値
	   max : 最大値
	   vResults: 結果を返す変数
	     min < vAry < max の時 vResults = 1;
             それ以外              vResults = 0;
	 */
	int *vAry = getCaliVariable();
	int cnt   = getCaliValue();
	int min   = getCaliValue();
	int max   = getCaliValue();
	int *vResults = getCaliVariable();
	int i;
	
	DEBUG_COMMAND("ShArray.SetRangeArray %p,%d,%d,%d,%p:\n", vAry, cnt, min, max, vResults);
	
	for (i = 0; i < cnt; i++) {
		*vResults = ((*vAry > min) && (*vAry < max)) ? 1 : 0;
		vResults++; vAry++;
	}
}

void SetAndEquArray(void) { /* 15 */
	/*
	  配列 vAry と mask との AND をとって val に等しければ
	  配列 vResults に１を、等しくなければ 0 を書く

	   vAry: 配列
	   mask: 配列にかけるマスク
	   cnt : 個数
	   val : 比較する値
	   vResults  : 結果を代入する配列

	*/
	int *vAry = getCaliVariable();
	int mask   = getCaliValue();
	int cnt    = getCaliValue();
	int val    = getCaliValue();
	int *vResults = getCaliVariable();
	int i;
	
	DEBUG_COMMAND("ShArray.SetAndEquArray: %p,%d,%d,%d,%p:\n", vAry, mask, cnt, val, vResults);
	
	for (i = 0; i < cnt; i++) {
		*vResults = ((*vAry & mask) == val) ? 1 : 0;
		vResults++; vAry++;
	}
}

void AndEquArray(void) { /* 16 */
	/*
	  配列 vAry 中で val と同じならば、vResult と 1 の AND を
	  違うならば 0 をかく。
	   
	   vAry: 配列
	   cnt : 個数
	   val : 比較する値
	   vResults  : 結果を代入する配列
	*/
	int *vAry = getCaliVariable();
	int cnt   = getCaliValue();
	int val   = getCaliValue();
	int *vResults = getCaliVariable();
	int i;
	
	DEBUG_COMMAND("ShArray.AndEquArray %p,%d,%d,%p:\n", vAry, cnt, val, vResults);
	
	for (i = 0; i < cnt; i++) {
		*vResults &= ((*vAry == val) ? 1 : 0);
		vResults++; vAry++;
	}
}

void AndNotArray(void) { /* 17 */
	/*
	  配列 vAry 中で val と等しくないならば、vResult と 1 の AND を
	  違う場合は 0 をかく。
	  
	   vAry: 配列
	   cnt : 個数
	   val : 比較する値
	   vResults  : 結果を代入する配列
	*/
	int *vAry = getCaliVariable();
	int cnt   = getCaliValue();
	int val   = getCaliValue();
	int *vResults = getCaliVariable();
	int i;
	
	DEBUG_COMMAND("ShArray.AndNotArray %p,%d,%d,%p:\n", vAry, cnt, val, vResults);
	
	for (i = 0; i < cnt; i++) {
		*vResults &= ((*vAry != val) ? 1 : 0);
		vResults++; vAry++;
	}
}

void AndLowArray(void) { /* 18 */
	/*
	  配列 vAry が min よりも小さいならば、vResult と 1 の AND を
	  そうでないならば 0 をかく。
	  
	   vAry: 配列
	   cnt : 個数
	   min : 最小値
	   vResults  : 結果を代入する配列
	*/
	int *vAry = getCaliVariable();
	int cnt   = getCaliValue();
	int min   = getCaliValue();
	int *vResults = getCaliVariable();
	int i;
	
	DEBUG_COMMAND("ShArray.AndLowArray: %d,%d,%d,%d:\n", vAry, cnt, min, vResults);
	
	for (i = 0; i < cnt; i++) {
		*vResults &= ((*vAry < min) ? 1 : 0);
		vResults++; vAry++;
	}
}

void AndHighArray(void) { /* 19 */
	/*
	  配列 vAry が min よりも大きいならば、vResult と 1 の AND を
	  そうでないならば 0 をかく。
	  
	   vAry: 配列
	   cnt : 個数
	   max : 最小値
	   vResults  : 結果を代入する配列
	*/
	int *vAry     = getCaliVariable();
	int cnt       = getCaliValue();
	int max       = getCaliValue();
	int *vResults = getCaliVariable();
	int i;
	
	DEBUG_COMMAND("ShArray.AndHighArray: %p,%d,%d,%p:\n", vAry, cnt, max, vResults);
	
	for (i = 0; i < cnt; i++) {
		*vResults &= ((*vAry > max) ? 1 : 0);
		vResults++; vAry++;
	}
}

void AndRangeArray(void) { /* 20 */
	/*
	  vAry が min から max にある場合、vResults と 1 の AND を
	  そうでなければ 0 をかく。
	  
	  vAry: 配列
	  cnt:  個数
	  min:  最小値
	  max:  最大値
	  vResults:   結果を返す配列
	*/
	int *vAry = getCaliVariable();
	int cnt   = getCaliValue();
	int min   = getCaliValue();
	int max   = getCaliValue();
	int *vResults = getCaliVariable();
	int i;
	
	DEBUG_COMMAND("ShArray.AndRangeArray %d,%d,%d,%d,%d:\n", vAry, cnt, min, max, vResults);
	
	for (i = 0; i < cnt; i++) {
		*vResults &= (((*vAry > min) && (*vAry < max)) ? 1 : 0);
		vResults++; vAry++;
	}
}

void AndAndEquArray(void) { /* 21 */
	/*
	  配列 vAry と mask との AND をとって val に等しければ
	  配列 vResults と１の AND、等しくなければ 0 を書く

	   vAry: 配列
	   mask: 配列にかけるマスク
	   cnt : 個数
	   val : 比較する値
	   vResults  : 結果を代入する配列

	*/
	int *vAry = getCaliVariable();
	int mask  = getCaliValue();
	int cnt   = getCaliValue();
	int val   = getCaliValue();
	int *vResults = getCaliVariable();
	int i;
	
	DEBUG_COMMAND("ShArray.AndAndEquArray: %d,%d,%d,%d,%d:\n", vAry, mask, cnt, val, vResults);
	
	for (i = 0; i < cnt; i++) {
		(*vResults) &= (((*vAry & mask) == val) ? 1 : 0);
		vResults++; vAry++;
	}
}

void OrEquArray(void) { /* 22 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();

	DEBUG_COMMAND_YET("ShArray.OrEquArray: %d,%d,%d,%d:\n", p1, p2, p3, p4);
}

void OrNotArray(void) { /* 23 */
	/*
	  配列の値が val と等しくなければ vResult に 1 を書き込む
	  
	  vAry: 配列
	  cnt : 個数
	  val : 比較する値
	  vResults: 結果を書き込む変数
	*/
	int *vAry = getCaliVariable();
	int cnt   = getCaliValue();
	int val   = getCaliValue();
	int *vResults = getCaliVariable();
	int i;
	
	DEBUG_COMMAND("ShArray.OrNotArray %p,%d,%d,%p:\n", vAry, cnt, val, vResults);
	
	for (i = 0; i < cnt; i++) {
		// if (*vAry != val) *vResults = 1;
		(*vResults) |= ((*vAry != val) ? 1 : 0);
		vResults++; vAry++;
	}
}

void OrLowArray(void) { /* 24 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();

	DEBUG_COMMAND_YET("ShArray.OrLowArray: %d,%d,%d,%d:\n", p1, p2, p3, p4);
}

void OrHighArray(void) { /* 25 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();

	DEBUG_COMMAND_YET("ShArray.OrHighArray: %d,%d,%d,%d:\n", p1, p2, p3, p4);
}

void OrRangeArray(void) { /* 26 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();

	DEBUG_COMMAND_YET("ShArray.OrRangeArray: %d,%d,%d,%d,%d:\n", p1, p2, p3, p4,p5);
}

void OrAndEquArray(void) { /* 27 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();

	DEBUG_COMMAND_YET("ShArray.OrAndEquArray: %d,%d,%d,%d,%d:\n", p1, p2, p3, p4,p5);
}

void EnumEquArray(void) { /* 28 */
	/*
	  配列のデータ中に val と同じデータを数を vResult に返す
	  
	  vAry: 配列
	  cnt : 個数
	  val : 比較する値
	  vResult: 一致する個数を返す変数
	*/
	int *vAry = getCaliVariable();
	int cnt   = getCaliValue();
	int val   = getCaliValue();
	int *vResult = getCaliVariable();
	int i;
	
	DEBUG_COMMAND("ShArray.EnumEquArray %p,%d,%d,%p:\n", vAry, cnt, val, vResult);
	
	*vResult = 0;
	
	for (i = 0; i < cnt; i++) {
		if (*vAry == val) (*vResult)++;
		vAry++;
	}
}

void EnumEquArray2(void) { /* 29 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();

	DEBUG_COMMAND_YET("ShArray.EnumEquArray2: %d,%d,%d,%d,%d,%d:\n", p1, p2, p3, p4, p5, p6);
}

void EnumEquNotArray2(void) { /* 30 */
	/*
	  配列１が val1 に等しく、配列２がval2 に等しくないものの数を
	  vResult に返す。

	  vAry1: 配列１
	  vAry2: 配列２
	  cnt:   個数
	  val1: 配列１と比較する値
	  val2: 配列２と比較する値
	  vResult: 条件に一致する数を返す変数
	*/
	int *vAry1 = getCaliVariable();
	int *vAry2 = getCaliVariable();
	int cnt    = getCaliValue();
	int val1   = getCaliValue();
	int val2   = getCaliValue();
	int *vResult = getCaliVariable();
	int i;
	
	DEBUG_COMMAND("ShArray.EnumEquNotArray2 %p,%p,%d,%d,%d,%p:\n", vAry1, vAry2, cnt, val1, val2, vResult);
	
	*vResult = 0;
	
	for (i = 0; i < cnt; i++) {
		if ((*vAry1 == val1) && (*vAry2 != val2)) {
			(*vResult)++;
		}
		vAry1++; vAry2++;
	}
}

void EnumNotArray(void) { /* 31 */
	/*
	  配列のなかで val と等しくないものの個数を返す
	  
	  vAry: 配列
	  cnt: 個数
	  val: 比較する値
	  vResult: 等しくないものの数
	*/
	int *vAry  = getCaliVariable();
	int cnt    = getCaliValue();
	int val    = getCaliValue();
	int *vResult = getCaliVariable();
	int i;

	DEBUG_COMMAND("ShArray.EnumNotArray %p, %d, %d, %p:\n", vAry, cnt, val, vResult);
	
	*vResult = 0;
	
	for (i = 0; i < cnt; i++) {
		if (*vAry != val) {
			(*vResult)++;
		}
		vAry++;
	}
}

void EnumNotArray2(void) { /* 32 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();

	DEBUG_COMMAND_YET("ShArray.EnumNotArray2: %d,%d,%d,%d,%d,%d:\n", p1, p2, p3, p4, p5, p6);
}

void EnumLowArray(void) { /* 33 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	
	DEBUG_COMMAND_YET("ShArray.EnumLowArray: %d,%d,%d,%d:\n", p1, p2, p3, p4);
}

void EnumHighArray(void) { /* 34 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	
	DEBUG_COMMAND_YET("ShArray.EnumHighArray: %d,%d,%d,%d:\n", p1, p2, p3, p4);
}

void EnumRangeArray(void) { /* 35 */
	/*
	  配列の値のうちが min と max の間あるものの数を vResult に返す
	  
	  vAry: 配列
	  cnt : 個数
	  min : 最小値
	  max : 最大値
	  vResult: 一致した数を返す変数
	*/
	int *vAry  = getCaliVariable();
	int cnt    = getCaliValue();
	int min    = getCaliValue();
	int max    = getCaliValue();
	int *vResult = getCaliVariable();
	int i;
	
	DEBUG_COMMAND("ShArray.EnumRangeArray %d,%d,%d,%d,%d:\n", vAry, cnt, min, max, vResult);
	
	*vResult = 0;
	
	for (i = 0; i < cnt; i++) {
		if ((*vAry > min) && (*vAry < max)) {
			(*vResult)++;
		}
		vAry++;
	}

}

void GrepEquArray(void) { /* 36 */
	/*
	  配列の値が val と等しければ vLastMatch に一致した index を返し
	  vResult に 1 を返す
	  
	  vAry: 配列
	  cnt : 個数
	  val : 比較する値
	  vMatch: 一致したインデックス
	  vResult: 一つでも val と同じ値があれば 1
	*/
	int *vAry  = getCaliVariable();
	int cnt    = getCaliValue();
	int val    = getCaliValue();
	int *vMatch  = getCaliVariable();
	int *vResult = getCaliVariable();
	int i;

	DEBUG_COMMAND("ShArray.GrepEquArray  %p,%d,%d,%p,%p:\n", vAry, cnt, val, vMatch, vResult);
	
	*vResult = 0;
	
	for (i = 0; i < cnt; i++) {
		if (*vAry == val) {
			*vMatch  = i;
			*vResult = 1;
			return;
		}
		vAry++;
	}
}

void GrepNotArray(void) { /* 37 */
	/*
	  配列の値が val と等しくなければ vLastMatch にその index を返し
	  vResult に 1 を返す
	  
	  vAry: 配列
	  cnt : 個数
	  val : 比較する値
	  vMatch: 一致するindex
	  vResult: 一つでも val と同じ値があれば 1
	*/
	int *vAry  = getCaliVariable();
	int cnt    = getCaliValue();
	int val    = getCaliValue();
	int *vMatch  = getCaliVariable();
	int *vResult = getCaliVariable();
	int i;
	
	DEBUG_COMMAND("ShArray.GrepNotArray %p,%d,%d,%p,%p:\n", vAry, cnt, val, vMatch, vResult);
	
	*vResult = 0;
	
	for (i = 0; i < cnt; i++) {
		if (*vAry != val) {
			*vMatch  = i;
			*vResult = 1;
			return;
		}
		vAry++;
	}
}

void GrepEquArray2(void) { /* 38 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();

	DEBUG_COMMAND_YET("ShArray.GrepEquArray2: %d,%d,%d,%d,%d,%d,%d:\n", p1, p2, p3, p4, p5, p6, p7);
}

void GrepNotArray2(void) { /* 39 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();

	DEBUG_COMMAND_YET("ShArray.GrepNotArray2: %d,%d,%d,%d,%d,%d,%d:\n", p1, p2, p3, p4, p5, p6, p7);
}

void GrepEquNotArray2(void) { /* 40 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();

	DEBUG_COMMAND_YET("ShArray.GrepEquNotArray2: %d,%d,%d,%d,%d,%d,%d:\n", p1, p2, p3, p4, p5, p6, p7);
}

void GrepLowArray(void) { /* 41 */
	/*
	  配列の値が min よりも小さければ vMatch に一致した index 
	  を返し vResult に 1 を返す
	  
	  vAry: 配列
	  cnt : 個数
	  min : 最小値
	  vMatch: 一致したインデックス
	  vResult: 一つでも val と同じ値があれば 1
	*/
	int *vAry = getCaliVariable();
	int cnt   = getCaliValue();
	int min   = getCaliValue();
	int *vMatch  = getCaliVariable();
	int *vResult = getCaliVariable();
	int i;
	
	DEBUG_COMMAND("ShArray.GrepLowArray: %p,%d,%d,%p,%p:\n", vAry, cnt, min, vMatch, vResult);
	
	*vResult = 0;
	
	for (i = 0; i < cnt; i++) {
		if (*vAry < min) {
			*vMatch = i;
			*vResult = 1;
			return;
		}
		vAry++;
	}
}

void GrepHighArray(void) { /* 42 */
	/*
	  配列の値が min よりも大きければ vMatch に一致した index 
	  を返し vResult に 1 を返す
	  
	  vAry: 配列
	  cnt : 個数
	  max : 最大値
	  vMatch: 一致したインデックス
	  vResult: 一つでも val と同じ値があれば 1
	*/
	int *vAry = getCaliVariable();
	int cnt   = getCaliValue();
	int max   = getCaliValue();
	int *vMatch  = getCaliVariable();
	int *vResult = getCaliVariable();
	int i;
	
	DEBUG_COMMAND("ShArray.GrepHighArray: %p,%d,%d,%p,%p:\n", vAry, cnt, max, vMatch, vResult);
	
	*vResult = 0;
	
	for (i = 0; i < cnt; i++) {
		if (*vAry > max) {
			*vMatch = i;
			*vResult = 1;
			return;
		}
		vAry++;
	}
}

void GrepRangeArray(void) { /* 43 */
	/*
	  配列の値が max と min の間にあれば vMatch に一致した index 
	  を返し vResult に 1 を返す
	  
	  vAry: 配列
	  cnt : 個数
	  min : 最小値
	  max : 最大値
	  vMatch: 一致したインデックス
	  vResult: 一つでも val と同じ値があれば 1
	*/
	int *vAry  = getCaliVariable();
	int cnt    = getCaliValue();
	int min    = getCaliValue();
	int max    = getCaliValue();
	int *vMatch = getCaliVariable();
	int *vResult    = getCaliVariable();
	int i;
	
	DEBUG_COMMAND("ShArray.GrepRangeArray %p,%d,%d,%d,%p,%p:\n", vAry, cnt, max, min, vMatch, vResult);
	
	*vResult = 0;
	
	for (i = 0; i < cnt; i++) {
		if ((*vAry > min) && (*vAry < max)) {
			*vMatch = i;
			*vResult = 1;
			return;
		}
		vAry++;
	}
}

void GrepLowOrderArray(void) { /* 44 */
	/*
	  配列 vAry の中で minよりも大きく、 max よりも小さいもの
	  のうち、最も小さいものの index を vLastMatch に返す。
	  ただし。v1[index] は 0 である必要がある。
	  
	  vAry: 配列
	  cnt : 個数
	  min : 最小値
	  max : 最大値
	  v1  : 結果を返す配列(0 の場所しか比較せず、最小の場所に 1 を書く
	  vLastMatch: 最小値を示す配列のindex
	  vResult: 最小値が見つかれば 1, 見つからなければ 0
	*/
	int *vAry  = getCaliVariable();
	int cnt    = getCaliValue();
	int min    = getCaliValue();
	int max    = getCaliValue();
	int *v1    = getCaliVariable();
	int *vLastMatch = getCaliVariable();
	int *vResult    = getCaliVariable();
	int i, j, k = 0;
	
	DEBUG_COMMAND("ShArray.GrepLowOrderArray %p,%d,%d,%d,%p,%p,%p:\n", vAry, cnt, min, max, v1, vLastMatch, vResult);
	
	*vResult = 0;
	for (i = 0; i < cnt; i++) {
		if((*(vAry + i) == min) && (*(v1 + i) == 0)) {
			*vResult    = 1;
			*vLastMatch = i;
			*(v1 + i)   = 1;
			return;
		}
	}
	
	j = 65536;
	for (i = 0; i < cnt; i++) {
		if ((*(vAry + i) > min) && (*(vAry + i) < max) &&
		    (*(v1 + i) == 0) && (*(vAry + i) < j)) {
			j = *(vAry + i);
			k = i;
		}
	}
	if (j < 65536) {
		*vResult    = 1;
		*vLastMatch = k;
		*(v1 + k)   = 1;
	}

}

void GrepHighOrderArray(void) { /* 45 */
	/*
	  配列 vAry の中で minよりも大きく、 max よりも小さいもの
	  のうち、最も大きいものの index を vLastMatch に返す。
	  ただし。v1[index] は 0 である必要がある。
	  
	  vAry: 配列
	  cnt : 個数
	  min : 最小値
	  max : 最大値
	  v1  : 結果を返す配列(0 の場所しか比較せず、最小の場所に 1 を書く
	  vLastMatch: 最大値を示す配列のindex
	  vResult: 最大値が見つかれば 1, 見つからなければ 0
	*/
	int *vAry  = getCaliVariable();
	int cnt    = getCaliValue();
	int min    = getCaliValue();
	int max    = getCaliValue();
	int *v1    = getCaliVariable();
	int *vLastMatch = getCaliVariable();
	int *vResult    = getCaliVariable();
	int i, j, k = 0;
	
	DEBUG_COMMAND("ShArray.GrepHighOrderArray %p,%d,%d,%d,%p,%p,%p:\n", vAry, cnt, min, max, v1, vLastMatch, vResult);
	
	*vResult = 0;
	for (i = 0; i < cnt; i++) {
		if ((*(vAry + i) == max) && (*(v1 + i) == 0)) {
			*vResult    = 1;
			*vLastMatch = i;
			*(v1 + i)   = 1;
			return;
		}
	}
	
	j = -1;
	for (i = 0; i < cnt; i++) {
		if ((*(vAry + i) >= min) && (*(vAry + i) < max) &&
		    (*(v1 + i) == 0) && (*(vAry + i) > j)) {
			j = *(vAry + i);
			k = i;
		}
	}
	if (j >= 0) {
		*vResult    = 1;
		*vLastMatch = k;
		*(v1 + k)   = 1;
	}
}

void ChangeEquArray(void) { /* 46 */
	int *vAry = getCaliVariable();
	int cnt = getCaliValue();
	int src = getCaliValue();
	int dst = getCaliValue();
	int i;
	
	DEBUG_COMMAND("ShArray.ChangeEquArray: %d,%d,%d,%d:\n", vAry, cnt, src, dst);
	
	for (i = 0; i < cnt; i++) {
		if (*vAry == src) {
			*vAry = dst;
		}
		vAry++;
	}
}

void ChangeNotArray(void) { /* 47 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	
	DEBUG_COMMAND_YET("ShArray.ChangeNotArray: %d,%d,%d,%d:\n", p1, p2, p3, p4);
}

void ChangeLowArray(void) { /* 48 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	
	DEBUG_COMMAND_YET("ShArray.ChangeLowArray: %d,%d,%d,%d:\n", p1, p2, p3, p4);
}

void ChangeHighArray(void) { /* 49 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	
	DEBUG_COMMAND_YET("ShArray.ChangeHighArray: %d,%d,%d,%d:\n", p1, p2, p3, p4);
}

void ChangeRangeArray(void) { /* 50 */
	/*
	  配列内データが min から max の間にあるときは val で置き換え
	  
	  vAry: 配列
	  cnt : 個数
	  min : 最小値
	  max : 最大値
	  val : 置き換える値
	*/
	int *vAry = getCaliVariable();
	int cnt   = getCaliValue();
	int min   = getCaliValue();
	int max   = getCaliValue();
	int val   = getCaliValue();
	int i;
	
	DEBUG_COMMAND("ShArray.ChangeRangeArray %p,%d,%d,%d,%d:\n", vAry, cnt, min, max, val);
	
	for (i = 0; i < cnt; i++) {
		if ((*vAry > min) && (*vAry < max)) {
			*vAry = val;
		}
		vAry++;
	}
}

void CopyArrayToRect(void) { /* 51 */
	/*
	  vSrc の sw * sh の領域を vDst の dx,dy の位置にコピー 
	  
	  vSrc: コピー元配列
	  sw  : コピー元 width
	  sh  : コピー元 height
	  sx  : コピー元 x
	  sy  : コピー元 y
	  vDst: コピー先配列
	  dw  : コピー先 width
	  dh  : コピー先 height
	*/
	int *vSrc = getCaliVariable();
	int sw    = getCaliValue();
	int sh    = getCaliValue();
	int sx    = getCaliValue();
	int sy    = getCaliValue();
	int *vDst = getCaliVariable();
	int dw    = getCaliValue();
	int dh    = getCaliValue();
	int x, y;

	DEBUG_COMMAND("ShArray.CopyArrayToRect %p,%d,%d,%d,%d,%p,%d,%d:\n", vSrc, sw, sh, sx, sy, vDst, dw, dh);
	
	vSrc += (sy * sw + sx);
	for (y = 0; y < dh; y++) {
		for (x = 0; x < dw; x++) {
			*(vDst + x) = *(vSrc + x);
		}
		vSrc += sw; vDst += dw; 
	}
}

void CopyRectToArray(void) { /* 52 */
	/*
	  vSrc の sw * sh の領域を vDst の dx,dy の位置にコピー 
	  
	  vSrc: コピー元配列
	  sw  : コピー元 width
	  sh  : コピー元 height
	  vDst: コピー先配列
	  dw  : コピー先 width
	  dh  : コピー先 height
	  dx  : コピー先 x
	  dy  : コピー先 y
	*/
	int *vSrc = getCaliVariable();
	int sw    = getCaliValue();
	int sh    = getCaliValue();
	int *vDst = getCaliVariable();
	int dw    = getCaliValue();
	int dh    = getCaliValue();
	int dx    = getCaliValue();
	int dy    = getCaliValue();
	int x, y;
	
	DEBUG_COMMAND("ShArray.CopyRectToArray %p,%d,%d,%p,%d,%d,%d,%d:\n", vSrc, sw, sh, vDst, dw, dh, dx, dy);
	
	vDst += (dy * dw + dx);
	for (y = 0; y < sh; y++) {
		for (x = 0; x < sw; x++) {
			*(vDst + x) = *(vSrc + x);
		}
		vSrc += sw; vDst += dw;
	}
}

void ChangeSecretArray(void) { /* 53 */
	/*
	  良く分からないが、データをコード化しているようだ
	  
	  vAry: 配列
	  cnt : 個数
	  type: 機能番号
	  vResult: 結果を返す変数
	*/
	int *vAry    = getCaliVariable();
	int cnt      = getCaliValue();
	int type     = getCaliValue();
	int *vResult = getCaliVariable();
	static WORD key[4] = { 0x7A7A, 0xADAD, 0xBCBC, 0xCECE }; /* key */
	
	DEBUG_COMMAND("ShArray.ChangeSecretArray %p,%d,%d,%p:\n", vAry, cnt, type, vResult);
	
	*vResult = 0;
	
	switch(type) {
	case 0:
		/*
		  cnt -1 番目のキーを vAry に取り出す
		*/
		if (cnt > 0 && cnt < 5) {
			*vAry = key[cnt -1];
			*vResult = 1;
		}
		break;
	case 1:
		/*
		  vAry を cnt -1 番目のキーにセットする
		*/
		if (cnt > 0 && cnt < 5) {
			key[cnt -1] = *vAry;
			*vResult = 1;
		}
		break;
	case 2:
		{
			/*
			  エンコードその１
			*/
			int i, j = 0;
			WORD ax = key[3] ^ 0x5a5a;
			for (i = 0; i < cnt; i++) {
				(*vAry) ^= ax; ax = (key[i&3] ^ *vAry);
				j ^= ax;
				if (i & 2) {
					ax = !ax ^ (i*3);
				}
				if (i & 4) {
					ax = (ax >> 4) | (ax << 12);
				}
				vAry++;
			}
			*vResult = j;
		}
		break;
	case 3:
		{
			/*
			  エンコードその２
			*/
			int i, j = 0, k;
			WORD ax = key[3] ^ 0x5a5a;
			for (i = 0; i < cnt; i++) {
				k = *vAry; 
				*vAry ^= ax; ax = (key[i&3] ^ k);
				j ^= ax;
				if (i & 2) {
					ax = !ax ^ (i*3);
				}
				if (i & 4) {
					ax = (ax >> 4) | (ax << 12);
				}
				vAry++;
				
			}
			*vResult = j;
		}
		break;
	}
}
