/*
 * cmd_check.c  SYSTEM35のコマンド解析
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
/* $Id: cmd_check.c,v 1.35 2003/04/22 16:34:28 chikama Exp $ */

#include <stdio.h>
#include <stdlib.h>

#include "portab.h"
#include "cmd_check.h"
#include "scenario.h"
#include "xsystem35.h"
#include "selection.h"
#include "message.h"
#include "imput.h"

static void undeferr();

static void commandsINC() {
        int *var = getCaliVariable();
        if (*var < 65535) (*var)++;
}

static void commandsDEC() {
        int *var = getCaliVariable();
        if (*var > 0) (*var)--;
}

/* 変数の代入 */
static void letVar(int type) {
	int *varno = getVariable();
	int val    = getCaliValue();

	if (varno == NULL) {
		WARNING("varno is NULL\n");
		return;
	}
	
	switch(type) {
	case '!':
		*varno = val; break;
	case 0x10: /* += */
		*varno = (int)(WORD)(*varno + val); break;
	case 0x11: /* -= */
		*varno = max(0, *varno - val); break;
	case 0x12: /* *= */
		*varno = (int)(WORD)(*varno * val); break;
	case 0x13: /* /= */
		if (val == 0) *varno  = 0;
		else          *varno /= val;
		break;
	case 0x14: /* %= */
		if (val == 0) *varno  = 0;
		else          *varno %= val;
		break;
	case 0x15: /* &= */
		*varno &= val; break;
	case 0x16: /* |= */
		*varno |= val; break;
	case 0x17: /* ^= */
		*varno ^= val; break;
	}

	// printf("letvar %p=%d\n",varno, val);
}

/* データテーブルの設定 */
static void getDataTableAdr() {
	int index  = sys_getaddress();
	int offset = sys_getCaliValue();
	
	if (offset) {
		index = sl_getdAt(index + 4 * (offset - 1));
	}
	
	if (NULL == (nact->datatbl_addr = sl_setDataTable(sl_getPage(), index))) {
		WARNING("data table address set failed\n");
	}
}

/* < ループ開始 */
static void loopStart() {
	int p1 = sys_getc();
	int exitadr, limit, direction, step;
	int *var;
	
	if (p1 == 0) {
		sys_getc();
		sys_getc();
	} else if (p1 != 1) {
		undeferr();
	}
	
	exitadr   = sys_getaddress();
	var       = getCaliVariable();
	limit     = getCaliValue();
	direction = getCaliValue();
	step      = getCaliValue();
	
	if (direction == 0) {
		/* dec */
		if (p1 == 1) {
			*var -= step;
		}
		if (*var < limit) {
			if (p1 == 0) *var-= step;
			sl_jmpNear(exitadr);
			if (*var < 0) { *var = 0; }
			return;
		}
	} else if (direction == 1) {
		/* inc */
		if (p1 == 1) {
			*var += step;
		}
		if (*var > limit) {
			// if (p1 == 0) *var+= step;
			sl_jmpNear(exitadr);
			if (*var > 65535) { *var = 65535; }
			return;
		}
	} else {
		undeferr();
	}
	return;
}

static void undeferr() {
	SYSERROR("Undefined Command:@ %03d,%05x\n", sl_getPage(), sl_getIndex());
}

void check_command(int c0) {
	int page, index;
	int bool;
	
	switch(c0) {
	case 0:
		/* メッセージのゴミ？ */
		break;
	case '!':
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
	case 0x15:
	case 0x16:
	case 0x17:
		/* 変数代入 */
		letVar(c0);
		break;
	case '#':
		/* データテーブルアドレス指定 */
		getDataTableAdr();
		break;
	case '$':
		/* 選択肢の登録 */
		if (nact->sel.in_setting) {
			sel_fixElement();
			nact->sel.in_setting = FALSE;
		} else {
			sel_addRetValue(sys_getaddress());
			nact->sel.in_setting = TRUE;
		}
		break;
	case '%':
		/* ページコール */
		page = getCaliValue();
		if (page == 0) {
			sl_retFar();
		} else {
			sl_callFar(page);
		}
		break;
	case '&':
		/* ページジャンプ */
		sl_jmpFar(getCaliValue());
		break;
	case '@':
		/* puts("ラベルジャンプ") */
		sl_jmpNear(sys_getaddress());
		break;
	case '<':
		/* for loop */
		loopStart();
		break;
	case '>':
		/* loop end */
		sl_jmpNear(sys_getaddress());
		break;
	case '/':
		/* 小文字コマンド */
		switch(sl_getc()) {
		case 0x00:
			commands2F00(); break;
		case 0x01:
			commands2F01(); break;
		case 0x02:
			commands2F02(); break;
		case 0x03:
			commands2F03(); break;
		case 0x04:
			commands2F04(); break;
		case 0x05:
			commands2F05(); break;
		case 0x06:
			commandsINC(); break;
		case 0x07:
			commandsDEC(); break;
		case 0x08:
			commands2F08(); break;
		case 0x09:
			commands2F09(); break;
		case 0x0a:
			commands2F0A(); break;
		case 0x0b:
			commands2F0B(); break;
		case 0x0c:
			commands2F0C(); break;
		case 0x0d:
			commands2F0D(); break;
		case 0x0e:
			commands2F0E(); break;
		case 0x0f:
			commands2F0F(); break;
		case 0x10:
			commands2F10(); break;
		case 0x11:
			commands2F11(); break;
		case 0x12:
			commands2F12(); break;
		case 0x13:
			commands2F13(); break;
		case 0x14:
			commands2F14(); break;
		case 0x15:
			commands2F15(); break;
		case 0x16:
			commands2F16(); break;
		case 0x17:
			commands2F17(); break;
		case 0x18:
			commands2F18(); break;
		case 0x19:
			commands2F19(); break;
		case 0x1a:
			commands2F1A(); break;
		case 0x1b:
			commands2F1B(); break;
		case 0x1c:
			commands2F1C(); break;
		case 0x1d:
			commands2F1D(); break;
		case 0x1e:
			commands2F1E(); break;
		case 0x1f:
			commands2F1F(); break;
		case 0x20:
			commands2F20(); break;
		case 0x21:
			commands2F21(); break;
		case 0x22:
			commandHH(); break;
		case 0x23:
			commands2F23(); break;
		case 0x24:
			commands2F24(); break;
		case 0x25:
			commands2F25(); break;
		case 0x26:
			commands2F26(); break;
		case 0x27:
			commands2F27(); break;
		case 0x28:
			commands2F28(); break;
		case 0x29:
			commands2F29(); break;
		case 0x2a:
			commands2F2A(); break;
		case 0x2b:
			commands2F2B(); break;
		case 0x2c:
			commandF(); break;
 		case 0x2d:
			commands2F2D(); break;
		case 0x2e:
			commands2F2E(); break;
		case 0x2f:
			commands2F2F(); break;
		case 0x30:
			commands2F30(); break;
		case 0x31:
			commands2F31(); break;
		case 0x32:
			commands2F32(); break;
		case 0x33:
			commands2F33(); break;
		case 0x34:
			commands2F34(); break;
		case 0x35:
			commands2F35(); break;
		case 0x36:
			commands2F36(); break;
		case 0x37:
			commands2F37(); break;
		case 0x38:
			commands2F38(); break;
		case 0x39:
			commands2F39(); break;
		case 0x3a:
			commands2F3A(); break;
		case 0x3b:
			commands2F3B(); break;
		case 0x3c:
			commands2F3C(); break;
		case 0x3d:
			commands2F3D(); break;
		case 0x3e:
			commands2F3E(); break;
		case 0x3f:
			commands2F3F(); break;
		case 0x40:
			commands2F40(); break;
		case 0x41:
			commands2F41(); break;
		case 0x42:
			commands2F42(); break;
		case 0x43:
			commands2F43(); break;
		case 0x44:
			commands2F44(); break;
		case 0x45:
			commands2F45(); break;
		case 0x46:
			commands2F46(); break;
		case 0x47:
			commands2F47(); break;
		case 0x48:
			commands2F48(); break;
		case 0x49:
			commands2F49(); break;
		case 0x4a:
			commands2F4A(); break;
		case 0x4b:
			commands2F4B(); break;
		case 0x4c:
			commands2F4C(); break;
		case 0x4d:
			commands2F4D(); break;
		case 0x4e:
			commands2F4E(); break;
		case 0x4f:
			commands2F4F(); break;
		case 0x50:
			commands2F50(); break;
		case 0x51:
			commands2F51(); break;
		case 0x52:
			commands2F52(); break;
		case 0x53:
			commands2F53(); break;
		case 0x54:
			commands2F54(); break;
		case 0x55:
			commands2F55(); break;
		case 0x56:
			commands2F56(); break;
		case 0x57:
			commands2F57(); break;
		case 0x58:
			commands2F58(); break;
		case 0x59:
			commands2F59(); break;
		case 0x5a:
			commands2F5A(); break;
		case 0x5b:
			commands2F5B(); break;
		case 0x5c:
			commands2F5C(); break;
		case 0x5d:
			commands2F5D(); break;
		case 0x5e:
			commands2F5E(); break;
		case 0x5f:
			commands2F5F(); break;
		case 0x60:
			commands2F60(); break;
		case 0x61:
			commands2F61(); break;
		case 0x62:
			commands2F62(); break;
		case 0x63:
			commands2F63(); break;
		case 0x64:
			commands2F64(); break;
		case 0x65:
			commands2F65(); break;
		case 0x66:
			commands2F66(); break;
		case 0x67:
			commands2F67(); break;
		case 0x68:
			commands2F68(); break;
		case 0x69:
			commands2F69(); break;
		case 0x6a:
			commands2F6A(); break;
		case 0x6b:
			commands2F6B(); break;
		case 0x6c:
			commands2F6C(); break;
		case 0x6d:
			commands2F6D(); break;
		case 0x6e:
			commands2F6E(); break;
		case 0x6f:
			commands2F6F(); break;
		case 0x70:
			commands2F70(); break;
		case 0x71:
			commands2F71(); break;
		case 0x72:
			commands2F72(); break;
		case 0x73:
			commands2F73(); break;
		case 0x74:
			commands2F74(); break;
		case 0x75:
			commands2F75(); break;
		case 0x76:
			commands2F76(); break;
		case 0x77:
			commands2F77(); break;
		case 0x78:
			commands2F78(); break;
		case 0x79:
			commands2F79(); break;
		case 0x7A:
			commands2F7A(); break;
		case 0x7B:
			commands2F7B(); break;
		case 0x7C:
			commands2F7C(); break;
		case 0x7D:
			commands2F7D(); break;
		case 0x7E:
			commands2F7E(); break;
		case 0x7F:
			commands2F7F(); break;
		case 0x80:
			commands2F80(); break;
		case 0x81:
			commands2F81(); break;
		case 0x82:
			commands2F82(); break;
		case 0x83:
			commands2F83(); break;
		case 0x84:
			commands2F84(); break;
		case 0x85:
			commands2F85(); break;
		case 0x86:
			commands2F86(); break;
		case 0x87:
			commands2F87(); break;
		case 0x88:
			commands2F88(); break;
		case 0x89:
			commands2F89(); break;
		case 0x8A:
			commands2F8A(); break;
		case 0x8B:
			commands2F8B(); break;
		case 0x8C:
			commands2F8C(); break;
		default:
			undeferr();
		}
		break;
	case 'A':
		/* hit Any Key */
		sys_hit_any_key();
		msg_nextPage(TRUE);
		DEBUG_COMMAND("A\n");
		break;
	case 'B':
		switch(sl_getc()) {
		case 0:
			commandB0(); break;
		case 1:
			commandB1(); break;
		case 2:
			commandB2(); break;
		case 3:
			commandB3(); break;
		case 4:
			commandB4(); break;
		case 10:
			commandB10(); break;
		case 11:
			commandB11(); break;
		case 12:
			commandB12(); break;
		case 13:
			commandB13(); break;
		case 14:
			commandB14(); break;
		case 21:
			commandB21(); break;
		case 22:
			commandB22(); break;
		case 23:
			commandB23(); break;
		case 24:
			commandB24(); break;
		case 31:
			commandB31(); break;
		case 32:
			commandB32(); break;
		case 33:
			commandB33(); break;
		case 34:
			commandB34(); break;
		default:
			undeferr();
		}
		break;
	case 'C':
		switch(sl_getc()) {
		case 'B':
			commandCB(); break;
		case 'C':
			commandCC(); break;
		case 'D':
			commandCD(); break;
		case 'E':
			commandCE(); break;
		case 'F':
			commandCF(); break;
		case 'K':
			commandCK(); break;
		case 'L':
			commandCL(); break;
		case 'M':
			commandCM(); break;
		case 'P':
			commandCP(); break;
		case 'S':
			commandCS(); break;
		case 'T':
			commandCT(); break;
		case 'U':
			commandCU(); break;
		case 'V':
			commandCV(); break;
		case 'X':
			commandCX(); break;
		case 'Y':
			commandCY(); break;
		case 'Z':
			commandCZ(); break;
		default:
			undeferr();
		}
		break;
	case 'D':
		switch(sl_getc()) {
		case 'C':
			commandDC(); break;
		case 'I':
			commandDI(); break;
		case 'S':
			commandDS(); break;
		case 'R':
			commandDR(); break;
		case 'F':
			commandDF(); break;
		default:
			undeferr();
		}
		break;
	case 'E':
		switch(sl_getc()) {
		case 'S':
			commandES(); break;
		case 'C':
			commandEC(); break;
		case 'G':
			commandEG(); break;
		case 'M':
			commandEM(); break;
		case 'N':
			commandEN(); break;
		default:
			undeferr();
		}
		break;
	case 'F':
		commandF(); break;
	case 'G':
		switch(sl_getc()) {
		case 0:
			commandG0(); break;
		case 1:
			commandG1(); break;
		case 'S':
			commandGS(); break;
		case 'X':
			commandGX(); break;
		default:
			undeferr();
		}
		break;
	case 'H':
		commandH(); break;
	case 'I':
		switch(sl_getc()) {
		case 'K':
			commandIK(); break;
		case 'M':
			commandIM(); break;
		case 'C':
			commandIC(); break;
		case 'Z':
			commandIZ(); break;
		case 'X':
			commandIX(); break;
		case 'Y':
			commandIY(); break;
		case 'G':
			commandIG(); break;
		case 'E':
			commandIE(); break;
		default:
			undeferr();
		}
		break;
	case 'J':
		switch(sl_getc()) {
		case 0:
			commandJ0(); break;
		case 1:
			commandJ1(); break;
		case 2:
			commandJ2(); break;
		case 3:
			commandJ3(); break;
		case 4:
			commandJ4(); break;
		default:
			undeferr();
		}
		break;
	case 'K':
		switch(sl_getc()) {
		case 'I':
			commandKI(); break;
		case 'K':
			commandKK(); break;
		case 'N':
			commandKN(); break;
		case 'P':
			commandKP(); break;
		case 'Q':
			commandKQ(); break;
		case 'R':
			commandKR(); break;
		case 'W':
			commandKW(); break;
		default:
			undeferr();
		}
		break;
	case 'L':
		switch(sl_getc()) {
		case 'C':
			commandLC(); break;
		case 'D':
			commandLD(); break;
		case 'P':
			commandLP(); break;
		case 'T':
			commandLT(); break;
		case 'E':
			commandLE(); break;
		case 'L':
			commandLL(); break;
		case 'H':
			switch(sl_getc()) {
			case 'D':
				commandLHD(); break;
			case 'G':
				commandLHG(); break;
			case 'M':
				commandLHM(); break;
			case 'S':
				commandLHS(); break;
			case 'W':
				commandLHW(); break;
			default:
				undeferr();
			}
			break;
		case 'X':
			switch(sl_getc()) {
			case 'G':
				commandLXG(); break;
			case 'O':
				commandLXO(); break;
			case 'C':
				commandLXC(); break;
			case 'L':
				commandLXL(); break;
			case 'S':
				commandLXS(); break;
			case 'P':
				commandLXP(); break;
			case 'R':
				commandLXR(); break;
			case 'W':
				commandLXW(); break;
			default:
				undeferr();
			}
			break;
		default:
			undeferr();
		}
		break;
	case 'M':
		switch(sl_getc()) {
		case 'A':
			commandMA(); break;
		case 'C':
			commandMC(); break;
		case 'D':
			commandMD(); break;
		case 'E':
			commandME(); break;
		case 'F':
			commandMF(); break;
		case 'G':
			commandMG(); break;
		case 'H':
			commandMH(); break;
		case 'I':
			commandMI(); break;
		case 'J':
			commandMJ(); break;
		case 'L':
			commandML(); break;
		case 'M':
			commandMM(); break;
		case 'N':
			commandMN(); break;
		case 'P':
			commandMP(); break;
		case 'S':
			commandMS(); break;
		case 'T':
			commandMT(); break;
		case 'V':
			commandMV(); break;
		case 'Z':
			switch(sl_getc()) {
			case 0:
				commandMZ0(); break;
			default:
				undeferr();
			}
			break;
		default:
			undeferr();
		}
		break;
	case 'N':
		switch(sl_getc()) {
		case '+':
			commandN_ADD(); break;
		case '-':
			commandN_SUB(); break;
		case '*':
			commandN_MUL(); break;
		case '/':
			commandN_DIV(); break;
		case '>':
			commandN_GT(); break;
		case '<':
			commandN_LT(); break;
		case '=':
			commandN_EQ(); break;
		case '\\':
			commandN_NE(); break;
		case '&':
			commandN_AND(); break;
		case '|':
			commandN_OR(); break;
		case '^':
			commandN_XOR(); break;
		case '~':
			commandN_NOT(); break;
		case 'B':
			commandNB(); break;
		case 'C':
			commandNC(); break;
		case 'I':
			commandNI(); break;
		case 'P':
			commandNP(); break;
		case 'R':
			commandNR(); break;
		case 'O':
			commandNO(); break;
		case 'T':
			commandNT(); break;
		case 'D':
			switch(sl_getc()) {
			case 'C':
				commandNDC(); break;
			case 'D':
				commandNDD(); break;
			case 'M':
				commandNDM(); break;
			case 'A':
				commandNDA(); break;
			case 'H':
				commandNDH(); break;
			case '+':
				commandND_ADD(); break;
			case '-':
				commandND_SUB(); break;
			case '*':
				commandND_MUL(); break;
			case '/':
				commandND_DIV(); break;
			default:
				undeferr();
			}
			break;
		default:
			undeferr();
		}
		break;
	case 'O':
		commandO(); break;
	case 'P':
		switch(sl_getc()) {
		case 'C':
			commandPC(); break;
		case 'D':
			commandPD(); break;
		case 'F':
			commandPF(); break;
		case 'G':
			commandPG(); break;
		case 'N':
			commandPN(); break;
		case 'P':
			commandPP(); break;
		case 'S':
			commandPS(); break;
		case 'W':
			commandPW(); break;
		case 'T':
			switch(sl_getc()) {
			case 0:
				commandPT0(); break;
			case 1:
				commandPT1(); break;
			case 2:
				commandPT2(); break;
			default:
				undeferr();
			}
			break;
		default:
			undeferr();
		}
		break;
	case 'Q':
		switch(sl_getc()) {
		case 'C':
			commandQC(); break;
		case 'D':
			commandQD(); break;
		case 'E':
			commandQE(); break;
		case 'P':
			commandQP(); break;
		default:
			undeferr();
		}
		break;
	case 'R':
		/* 改行 */
		DEBUG_MESSAGE("\n");
		msg_nextLine();
		break;
	case 'S':
		switch(sl_getc()) {
		case 'C':
			commandSC(); break;
		case 'G':
			commandSG(); break;
		case 'L':
			commandSL(); break;
		case 'M':
			commandSM(); break;
		case 'O':
			commandSO(); break;
		case 'P':
			commandSP(); break;
		case 'Q':
			commandSQ(); break;
		case 'R':
			commandSR(); break;
		case 'S':
			commandSS(); break;
		case 'T':
			commandST(); break;
		case 'U':
			commandSU(); break;
		case 'W':
			commandSW(); break;
		case 'X':
			commandSX(); break;
		case 'I':
			commandSI(); break;
		default:
			undeferr();
		}
		break;
	case 'T':
		commandT(); break;
	case 'U':
		switch(sl_getc()) {
		case 'C':
			commandUC(); break;
		case 'D':
			commandUD(); break;
		case 'R':
			commandUR(); break;
		case 'S':
			commandUS(); break;
		case 'G':
			commandUG(); break;
		case 'P':
			switch(sl_getc()) {
			case 0:
				commandUP0(); break;
			case 1:
				commandUP1(); break;
			case 2:
			case 3:
				commandUP3(); break;
			default:
				undeferr();
			}
			break;
		default:
			undeferr();
		}
		break;
	case 'V':
		switch(sl_getc()) {
		case 'C':
			commandVC(); break;
		case 'P':
			commandVP(); break;
		case 'S':
			commandVS(); break;
		case 'G':
			commandVG(); break;
		case 'H':
			commandVH(); break;
		case 'F':
			commandVF(); break;
		case 'V':
			commandVV(); break;
		case 'R':
			commandVR(); break;
		case 'W':
			commandVW(); break;
		case 'E':
			commandVE(); break;
		case 'Z':
			commandVZ(); break;
		case 'X':
			commandVX(); break;
		case 'T':
			commandVT(); break;
		case 'B':
			commandVB(); break;
		case 'I':
			switch(sl_getc()) {
			case 'C':
				commandVIC(); break;
			case 'P':
				commandVIP(); break;
			default:
				undeferr();
			}
			break;
		case 'A':
			commandVA(); break;
		case 'J':
			commandVJ(); break;
		default:
			undeferr();
		}
		break;
	case 'W':
		switch(sl_getc()) {
		case 'W':
			commandWW(); break;
		case 'V':
			commandWV(); break;
		case 'X':
			commandWX(); break;
		case 'Z':
			commandWZ(); break;
		default:
			undeferr();
		}
		break;
	case 'X':
	{
		int num=getCaliValue();
		sys_addMsg(v_str(num - 1));
		DEBUG_COMMAND("X %d:\n", num);
	}
		break;
	case 'Y':
		commandY(); break;
	case 'Z':
		switch(sl_getc()) {
		case 'C':
			commandZC(); break;
		case 'M':
			commandZM(); break;
		case 'S':
			commandZS(); break;
		case 'B':
			commandZB(); break;
		case 'H':
			commandZH(); break;
		case 'W':
			commandZW(); break;
		case 'L':
			commandZL(); break;
		case 'E':
			commandZE(); break;
		case 'F':
			commandZF(); break;
		case 'D':
			commandZD(); break;
		case 'T':
			switch(sl_getc()) {
			case 0:
				commandZT0(); break;
			case 1:
				commandZT1(); break;
			case 2:
				commandZT2(); break;
			case 3:
				commandZT3(); break;
			case 4:
				commandZT4(); break;
			case 5:
				commandZT5(); break;
			case 10:
				commandZT10(); break;
			case 11:
				commandZT11(); break;
			case 20:
				commandZT20(); break;
			case 21:
				commandZT21(); break;
			default:
				undeferr();
			}
			 break;
		case 'Z':
			switch(sl_getc()) {
			case 0:
				commandZZ0(); break;
			case 1:
				commandZZ1(); break;
			case 2:
				commandZZ2(); break;
			case 3:
				commandZZ3(); break;
			case 4:
				commandZZ4(); break;
			case 5:
				commandZZ5(); break;
			case 7:
				commandZZ7(); break;
			case 8:
				commandZZ8(); break;
			case 9:
				commandZZ9(); break;
			case 10:
				commandZZ10(); break;
			case 13:
				commandZZ13(); break;
			case 14:
				commandZZ14(); break;
			default:
				undeferr();
			}
			break;
		case 'G':
			commandZG(); break;
		case 'I':
			commandZI(); break;
		case 'A':
			commandZA(); break;
		case 'K':
			commandZK(); break;
		case 'R':
			commandZR(); break;
		default:
			undeferr();
		}
		break;
	case '\\':
		/* label call */
		index = sys_getaddress();
		if (index == 0) {
			sl_retNear();
		} else {
			sl_callNear(index);
		}
		break;
	case ']':
		/* puts("選択"); */
		sel_select();
		break;
	case '{':
		/* puts("条件文"); */
		bool = getCaliValue();
		index = sys_getaddress();
		if (bool == 0) {
			sl_jmpNear(index);
		} 
		break;
	case '~':
		/* label far call */
		page = sys_getw();
		if (page == 0x0000) {
			// puts("~ cali:");
			nact->fnc_return_value = getCaliValue();
			sl_retFar();
		} else if (page == 0xffff) {
			// puts("~~ cali:");
			*getCaliVariable() = nact->fnc_return_value;
		} else {
			sl_callFar2(page - 1, sys_getaddress());
		}
		break;
	default:
		undeferr();
	}
}
