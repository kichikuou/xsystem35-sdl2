/*
 * savedata.h  セーブデータの管理
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
/* $Id: savedata.h,v 1.13 2001/05/08 05:36:08 chikama Exp $ */

#ifndef __SAVEDATA__
#define __SAVEDATA__

#include "portab.h"
#include "cg.h"
#include "windowframe.h"

#define SAVE_MAXNUMBER    (26)

#define SAVE_SAVEERR      (255)
#define SAVE_LOADERR      (255)
#define SAVE_LOADSHORTAGE (254)
#define SAVE_OTHERERR     (201)
#define SAVE_LOADOK       (0)
#define SAVE_SAVEOK1      (1)
#define SAVE_SAVEOK0      (0)

#define SAVE_DATAID "System3.5 SavaData(c)ALICE-SOFT"
#define SAVE_DATAVERSION 0x350200

typedef struct {
        WORD x;
        WORD y;
        WORD width;
        WORD height;
} RectangleW;

typedef struct {
        char ID[32];
	int version;
	char gameName[28];
	BYTE selMsgSize;
	BYTE selMsgColor;
	BYTE selBackColor;
	BYTE selFrameColor;
	BYTE msgMsgSize;
	BYTE msgMsgColor;
	BYTE msgBackColor;
	BYTE msgFrameColor;
	BYTE rsvB1;
	BYTE rsvB2;
	BYTE rsvB3;
	BYTE rsvB4;
	BYTE rsvB5;
	BYTE rsvB6;
	BYTE rsvB7;
	BYTE rsvB8;
	int  scoPage;
	int  scoIndex;
	int  rsvI1;
	int  rsvI2;
	RectangleW selWinInfo[SELWINMAX];
	RectangleW msgWinInfo[MSGWINMAX];
	int  stackinfo;
	int  varStr;
	int  rsvI3;
	int  rsvI4;
	int  varSys[256];
	int  rsvI[228];
} Ald_baseHdr;

typedef struct {
	int size;
	int count;
	int maxlen;
	int rsv1;
} Ald_strVarHdr;

typedef struct {
	int size;
	int rsv1;
	int rsv2;
	int rsv3;
} Ald_stackHdr;

typedef struct {
	int size;
	int pageNo;
	int rsv1;
	int rsv2;
} Ald_sysVarHdr;

/* defined by cmdb.c */
extern Bcom_WindowInfo msgWinInfo[];
extern Bcom_WindowInfo selWinInfo[];
/* defined by variable.c */
extern int  strvar_cnt;
extern int  strvar_len;

extern int save_loadAll(int no);
extern int save_saveAll(int no);
extern int save_loadPartial(int no, int page, int offset, int cnt);
extern int save_savePartial(int no, int page, int offset, int cnt);
extern int save_copyAll(int dstno, int srcno);
extern int save_save_var_with_file(char *filename, int *start, int cnt);
extern int save_load_var_with_file(char *filename, int *start, int cnt);
extern int save_save_str_with_file(char *filename, int start, int cnt);
extern int save_load_str_with_file(char *filename, int start, int cnt);
extern BYTE* load_cg_with_file(char *file,int *status);
extern void save_set_path(char *path);
extern void save_register_file(char *name, int index);
extern char* save_get_file(int index);
extern int save_delete_file(int index);

#endif /* __SAVEDATA__ */
