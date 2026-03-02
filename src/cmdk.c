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
#include "scenario.h"
#include "network.h"

void commandKI() {
	/* ネットワークのチャンネルを新規作成する／ネットワークのチャンネルに接続する */
	int *var     = getCaliVariable();
	int port_num = getCaliValue();
	int user_max = getCaliValue();
	
	*var = network_create_channel(port_num, user_max);
	
	TRACE_UNIMPLEMENTED("KI %d,%d,%d:", *var, port_num, user_max);
	return;
}

void commandKK() {
	/* user_num番のユーザーを切断する */
	int use_num = getCaliValue();

	network_close(use_num);
	
	TRACE_UNIMPLEMENTED("KK %d:", use_num);
	return;
}

void commandKN() {
	/* 自分自身の接続番号を取得する */
	int *var = getCaliVariable();

	*var = network_get_channel();
	
	TRACE_UNIMPLEMENTED("KN %d:", *var);
	return;
}

void commandKP() {
	/* データ受信バッファにデータがあるか調べる */
	int *var = getCaliVariable();
	
	*var = network_check_buffer();
	
	TRACE_UNIMPLEMENTED("KP %d:", *var);
	return;
}

void commandKQ() {
	/* ユーザーが接続されているかどうか確認する */
	int *var = getCaliVariable();
	int user_num = getCaliValue();
	
	*var = network_get_user_state(user_num);
	
	TRACE_UNIMPLEMENTED("KQ %d,%d:", *var, user_num);
	return;
}

void commandKR() {
	/* データ受信バッファからデータを取得する */
	int *var = getCaliVariable();
	
	network_read_buffer(var);
	
	TRACE_UNIMPLEMENTED("KR %d:", *var);
	return;
}

void commandKW() {
	/* データを送信する */
	int *var = getCaliVariable();
	int num  = getCaliValue();

	network_write_buffer(var, num);
	
	TRACE_UNIMPLEMENTED("KR %d,%d:", *var, num);
	return;
}
