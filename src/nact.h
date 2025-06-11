/*
 * nact.h  NACT SYSTEM
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
/* $Id: nact.h,v 1.25 2003/11/09 15:06:13 chikama Exp $ */

#ifndef __NACT_H__
#define __NACT_H__

#include "portab.h"
#include "s39ain.h"
#include "gameresource.h"
#include "selection.h"
#include "message.h"
#include "ags.h"
#include "utfsjis.h"
#include "hacks.h"

#define toUTF8(s)   codeconv(UTF8, nact->encoding, s)
#define toSJIS(s)   codeconv(SHIFT_JIS, nact->encoding, s)
#define fromUTF8(s) codeconv(nact->encoding, UTF8, s)
#define fromSJIS(s) codeconv(nact->encoding, SHIFT_JIS, s)

extern void sys_addMsg(const char *str);
extern void sys_setHankakuMode(int mode);
extern void sys_setCharacterEncoding(CharacterEncoding encoding);

extern void nact_main();
extern void nact_init();
extern void nact_reset(void);
extern void nact_quit(bool restart);

// cali.c
struct VarRef;
int getCaliValue(void);
int *getCaliVariable(void);
bool getCaliArray(struct VarRef *ref);
int *getVariable(void);

// cmd_check.c
extern void exec_command(void);

// cmdv.c
void va_animation(void);
void va_reset(void);
// cmdz.c
void cmdz_reset(void);
// cmd2F.c
void cmd2F_reset(void);

typedef struct {
	/* general */
	bool   is_quit;             /* quit command */
	bool   restart;
	void     (*callback)(void);    /* main の callback */
	bool   is_va_animation;     /* VA command working */
	bool   is_cursor_animation; /* animation cursor working */
	bool   popupmenu_opened;    /* popup menu が 開いているか */
	CharacterEncoding encoding;
	
	char      *game_title_utf8;
	int        scenario_version;

	
	/* variables */
	void *datatbl_addr; /* データテーブルのアドレス */
	int fnc_return_value; /* 関数の戻り値として返す値 (~0,cali:で渡す値) */
	
	/* key wait */
	int     waitcancel_key;  // TODO: remove this
	
	/* message wait */
	bool messagewait_enable;
	bool messagewait_cancelled;
	int     messagewait_time;
	bool messagewait_cancel;
	
	
	/* ags */
	ags_t ags;
	
	/* メッセージ関連 */
	msg_t msg;
 	bool   is_msg_out;          /* 通常メッセージを表示するか */
	void (*msgout)(const char *msg);     // 通常以外(DLL等)のメッセージ表示関数

	/* 選択肢関連 */
	sel_t sel;
	
	/* patch 関連 */
	int patch_ec;   /* see patch_ec command   */
	int patch_emen; /* see patch_emen command */
	int patch_g0;   /* see patch g0 command */

	/* ain 関連 */
	S39AIN ain;

	/* data file names */
	GameResource files;

	/* start address of the command currently being executed */
	int current_page;
	int current_addr;

} NACTINFO;

extern NACTINFO *nact;

#endif /* __NACT_H__ */
