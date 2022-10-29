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

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
typedef int emscripten_align1_int;
#endif

#include "portab.h"
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
        uint16_t x;
        uint16_t y;
        uint16_t width;
        uint16_t height;
} RectangleW;

typedef struct {
        char ID[32];
	int version;
	char gameName[28];
	uint8_t selMsgSize;
	uint8_t selMsgColor;
	uint8_t selBackColor;
	uint8_t selFrameColor;
	uint8_t msgMsgSize;
	uint8_t msgMsgColor;
	uint8_t msgBackColor;
	uint8_t msgFrameColor;
	uint8_t rsvB1;
	uint8_t rsvB2;
	uint8_t rsvB3;
	uint8_t rsvB4;
	uint8_t rsvB5;
	uint8_t rsvB6;
	uint8_t rsvB7;
	uint8_t rsvB8;
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
	emscripten_align1_int size;
	emscripten_align1_int pageNo;
	emscripten_align1_int rsv1;
	emscripten_align1_int rsv2;
} Ald_sysVarHdr;

/* defined by variable.c */
extern int  strvar_len;
struct VarRef;

extern int save_loadAll(int no);
extern int save_saveAll(int no);
extern int save_loadPartial(int no, struct VarRef *vref, int cnt);
extern int save_savePartial(int no, struct VarRef *vref, int cnt);
extern int save_copyAll(int dstno, int srcno);
extern int save_vars_to_file(char *fname_utf8, struct VarRef *src, int cnt);
extern int load_vars_from_file(char *fname_utf8, struct VarRef *dest, int cnt);
extern int save_save_str_with_file(char *fname_utf8, int start, int cnt);
extern int save_load_str_with_file(char *fname_utf8, int start, int cnt);
extern uint8_t* load_cg_with_file(char *fname_utf8, int *status, long *filesize);
extern const char *save_get_file(int index);
extern int save_delete_file(int index);

#endif /* __SAVEDATA__ */
