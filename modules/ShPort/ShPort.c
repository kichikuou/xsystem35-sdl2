/*
 * ShPort.c  Widget 呼び出し？ module
 *
 *    かえるにょ国にょアリス(未使用)
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
/* $Id: ShPort.c,v 1.6 2003/01/25 01:34:50 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <glib.h>

#include "portab.h"
#include "nact.h"
#include "system.h"
#include "xsystem35.h"
#include "graphicsdevice.h"

// キー変換テーブル
#define KEYMAP_MAX 8
static BYTE *keymap[KEYMAP_MAX];

void OutputMessageBox(void) { /* 0 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();

	DEBUG_COMMAND_YET("ShPort.OutputMessageBox: %d,%d,%d,%d,%d,%d:\n", p1, p2, p3, p3, p4, p5, p6);
}

void InputListNum(void) { /* 1 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();

	DEBUG_COMMAND_YET("ShPortInputListNum: %d,%d,%d,%d,%d,%d,%d:\n", p1, p2, p3, p3, p4, p5, p6, p7);
}

/**
 * ShPort:Init
 *   ShPortサブシステム全体の初期化
 *   @param p1: ISys3x
 */
void Init(void) {
	int p1 = getCaliValue();
	
	DEBUG_COMMAND_YET("ShPort.Init: %d:\n", p1);
}

/**
 * ShPort:InitKeyStatus
 *   指定のキーコードマップの初期化
 *   @param no: キーコードマップの番号(1~)
 */
void InitKeyStatus(void) {
	int no = getCaliValue();
	
	if (no >= KEYMAP_MAX) {
		WARNING("Overflow keymap table(p1)\n", no);
		return;
	}
	
	if (keymap[no -1] == NULL) {
		keymap[no -1] = g_new0(BYTE, 256);
	} else {
		memset(keymap[no -1], 0, 256);
	}
	
	DEBUG_COMMAND("ShPort.InitKeyStatus: %d:\n", no);
}

/**
 * ShPort:SetKeyStatus
 *   指定マップのキーコードへの機能の割り付け。
 *   @param no: マップ番号
 *   @param key: キーコード
 *   @param func: 機能キーコード
 */
void SetKeyStatus(void) {
	int no   = getCaliValue();
	int key  = getCaliValue();
	int func = getCaliValue();
	
	if (no >= KEYMAP_MAX) {
		WARNING("Overflow keymap table(p1)\n", no);
		return;
	}
	
	keymap[no -1][key] = func;
	
	DEBUG_COMMAND("ShPort.SetKeyStatus: %d,%d,%d:\n", no, key, func);
}

/**
 * ShPort:GetKeyStatus
 *   指定マップのキーの押下状態の取得
 *   @param no:  マップ番号
 *   @param var: 押下キーに対応する機能コードを返す変数
 */
void GetKeyStatus(void) {
	int no   = getCaliValue();
	int *var = getCaliVariable();
	int i;
	
	if (no >= KEYMAP_MAX) {
		WARNING("Overflow keymap table(p1)\n", no);
		return;
	}
	
	*var = 0;
	for (i = 0; i < 256; i++) {
		*var |= (keymap[no -1][i] * RawKeyInfo[i]);
	}
	
	DEBUG_COMMAND("ShPort.GetKeyStatus: %d,%p:\n", no, var);
}

void InputListString(void) {
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();

	DEBUG_COMMAND_YET("ShPort.InputListString: %d,%d,%d,%d,%d,%d,%d:\n", p1, p2, p3, p3, p4, p5, p6, p7);
}

void InputOpenFile(void) {
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();

	DEBUG_COMMAND_YET("ShPort.InputOpenFile: %d,%d,%d,%d,%d,%d,%d:\n", p1, p2, p3, p3, p4, p5, p6, p7);
}

void InputSaveFile(void) {
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();

	DEBUG_COMMAND_YET("ShPort.InputSaveFile: %d,%d,%d,%d,%d,%d,%d:\n", p1, p2, p3, p3, p4, p5, p6, p7);
}

