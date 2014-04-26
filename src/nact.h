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
#include "graphics.h"
#include "scenario.h"
#include "font.h"
#include "s39ain.h"
#include "selection.h"
#include "message.h"
#include "ags.h"

/* コマンド解析時に参照する */
#define sys_getc            sl_getc
#define sys_getw            sl_getw
#define sys_getdw           sl_getdw
#define sys_getaddress      sl_getadr
#define sys_getCaliValue    getCaliValue
#define sys_getCaliVariable getCaliVariable
extern int getCaliValue();
extern int *getCaliVariable();
extern int *getVariable();
extern char *sys_getString(char term);
extern char *sys_getConvString(char term);
extern void sys_addMsg(char *str);
extern void sys_setHankakuMode(int mode);
extern char *sys_getConstString();

// extern boolean sys_nact_engine();
extern void nact_main();
extern void nact_init();

typedef struct {
	/* general */
	boolean   is_quit;             /* quit command */
	void     (*callback)(void);    /* main の callback */
	boolean   is_va_animation;     /* VA command working */
	boolean   is_cursor_animation; /* animation cursor working */
	boolean   is_message_locked;   /* pointer 等の event handler を呼び出さない */
	boolean   popupmenu_opened;    /* popup menu が 開いているか */
	boolean   mmx_is_ok;           /* MMX が有効かどうか */
	
	char      *tmpdir;
	char       game_title_name[31];
	int        scenario_version;

	
	/* variables */
	void *datatbl_addr; /* データテーブルのアドレス */
	int fnc_return_value; /* 関数の戻り値として返す値 (~0,cali:で渡す値) */
	
#if 1
	/* ags info */
	Pallet256  *sys_pal;
	boolean     sys_pal_changed;
	MyRectangle sys_view_area;
	MyDimension sys_world_size;
	int         sys_world_depth;
	int         sys_mouse_movesw; /* 0:IZを無視, 1: 直接指定場所へ, 2: スムーズに指定場所に */
	boolean     sys_fullscreen_capable;
	boolean     sys_fullscreen_on;
	
#endif

	/* for fader/ecopy */
	int     effect_rate;
	int     effect_step; /* 0 to 255 , 0 と 255 は必ず通る*/

	/* key wait */
	int     waittime;
	int     waitcancel_key;
	int     waitcancel_key_mask;
	boolean waitcancel_enabled;
	
	/* message wait */
	boolean messagewait_enable;
	boolean messagewait_enable_save;
	int     messagewait_time;
	boolean messagewait_cancel;
	
	
	/* ags */
	ags_t ags;
	boolean noantialias; /* antialias を使用しない */
	boolean noimagecursor; /* リソースファイルのカーソルを読みこまない */
	fontdev_t fontdev; // 選択された fontdevice
	
	/* メッセージ関連 */
	msg_t msg;
 	boolean   is_msg_out;          /* 通常メッセージを表示するか */
	void (*msgout)(char *msg);     // 通常以外(DLL等)のメッセージ表示関数
	
	/* 選択肢関連 */
	sel_t sel;
	
	/* patch 関連 */
	int patch_ec;   /* see patch_ec command   */
	int patch_emen; /* see patch_emen command */
	int patch_g0;   /* see patch g0 command */
	
	/* ain 関連 */
	S39AIN ain;

	/* データのファイル名 */
	struct {
		char *scenario[2];
		char *graphics[2];
		char *wav[2];
		char *midi[2];
		char *data[2];
		char *resource[2];
		char *bgm[2];
		char *save[27];
		char *savedir;
		char *init;
		char *bgi;
		char *wai;
		char *sact01;
		char *alk[10];
	} files;
	
} NACTINFO;

extern NACTINFO *nact;

#endif /* __NACT_H__ */
