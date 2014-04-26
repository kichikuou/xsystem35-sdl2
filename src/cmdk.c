/*
 * cmdk.c  SYSTEM36 K command
 *
 * Copyright (C) 1999-     Masaki Chikama (Wren) <masaki-c@is.aist-nara.ac.jp>
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
/* $Id: cmdk.c,v 1.3 2000/09/10 10:39:48 chikama Exp $ */

#include <stdio.h>
#include "portab.h"
#include "xsystem35.h"
#include "network.h"

void commandKI() {
	/* ネットワークのチャンネルを新規作成する／ネットワークのチャンネルに接続する */
	int *var     = getCaliVariable();
	int port_num = getCaliValue();
	int user_max = getCaliValue();
	
	*var = network_create_channel(port_num, user_max);
	
	DEBUG_COMMAND_YET("KI %d,%d,%d:\n", *var, port_num, user_max);
	return;
}

void commandKK() {
	/* user_num番のユーザーを切断する */
	int use_num = getCaliValue();

	network_close(use_num);
	
	DEBUG_COMMAND_YET("KK %d:\n", use_num);
	return;
}

void commandKN() {
	/* 自分自身の接続番号を取得する */
	int *var = getCaliVariable();

	*var = network_get_channel();
	
	DEBUG_COMMAND_YET("KN %d:\n", *var);
	return;
}

void commandKP() {
	/* データ受信バッファにデータがあるか調べる */
	int *var = getCaliVariable();
	
	*var = network_check_buffer();
	
	DEBUG_COMMAND_YET("KP %d:\n", *var);
	return;
}

void commandKQ() {
	/* ユーザーが接続されているかどうか確認する */
	int *var = getCaliVariable();
	int user_num = getCaliValue();
	
	*var = network_get_user_state(user_num);
	
	DEBUG_COMMAND_YET("KQ %d,%d:\n", *var, user_num);
	return;
}

void commandKR() {
	/* データ受信バッファからデータを取得する */
	int *var = getCaliVariable();
	
	network_read_buffer(var);
	
	DEBUG_COMMAND_YET("KR %d:\n", *var);
	return;
}

void commandKW() {
	/* データを送信する */
	int *var = getCaliVariable();
	int num  = getCaliValue();

	network_write_buffer(var, num);
	
	DEBUG_COMMAND_YET("KR %d,%d:\n", *var, num);
	return;
}
