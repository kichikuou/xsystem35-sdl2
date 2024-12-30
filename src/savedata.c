/*
 * savedate.c  セーブデータの管理
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
/* $Id: savedata.c,v 1.36 2003/07/21 23:06:47 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
typedef int emscripten_align1_int;
#endif

#include "portab.h"
#include "savedata.h"
#include "scenario.h"
#include "xsystem35.h"
#include "LittleEndian.h"
#include "filecheck.h"
#include "windowframe.h"
#include "selection.h"
#include "message.h"

const char *save_signature[] = {
	[SAVEFMT_XSYS35] = "System3.5 SavaData(c)ALICE-SOFT",
	[SAVEFMT_SYS36]  = "System3.5 SaveData(c)ALICE-SOFT",
	[SAVEFMT_SYS38]  = "System3.8 SaveData(c)ALICE-SOFT",
};

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
} asd_baseHdr;

typedef struct {
	int size;
	int count;
	int maxlen;
	int rsv1;
} asd_strVarHdr;

typedef struct {
	int size;
	int rsv1;
	int rsv2;
	int rsv3;
} asd_stackHdr;

typedef struct {
	emscripten_align1_int size;
	emscripten_align1_int pageNo;
	emscripten_align1_int rsv1;
	emscripten_align1_int rsv2;
} asd_varPageHdr;

#ifdef __EMSCRIPTEN__
static enum save_format save_format = SAVEFMT_XSYS35;
#else
static enum save_format save_format = SAVEFMT_SYS38;
#endif

#ifdef __EMSCRIPTEN__
EM_JS(void, scheduleSync, (), {
	xsystem35.shell.syncfs();
});
#else
#define scheduleSync()
#endif

static void fputw(uint16_t n, FILE *fp) {
	fputc(n & 0xff, fp);
	fputc(n >> 8, fp);
}

static int saveStack(enum save_format format, FILE *fp) {
	int size;
	uint8_t *data = sl_saveStack(format, &size);

	asd_stackHdr head = { .size = size };
	fwrite(&head, sizeof(head), 1, fp);
	if (size > 0)
		fwrite(data, size, 1, fp);
	free(data);
	return sizeof(head) + size;
}

static void loadStack(enum save_format format, char *buf) {
	asd_stackHdr *head = (asd_stackHdr *)buf;
	uint8_t *data = buf + sizeof(asd_stackHdr);
	sl_loadStack(format, data, head->size);
}

static int saveStrVars(enum save_format format, FILE *fp) {
	const int base = format == SAVEFMT_XSYS35 ? 1 : 0;
	asd_strVarHdr head = {
		.size   = 0,
		.count  = svar_maxindex() + (1 - base),
		.maxlen = (format == SAVEFMT_XSYS35 ? 101 : 0)  // so that old versions of xystem35 can read this save file
	};
	fseek(fp, sizeof(head), SEEK_CUR);

	for (int i = base; i <= svar_maxindex(); i++) {
		const char *s = svar_get(i);
		int len = strlen(s) + 1;
		fwrite(s, len, 1, fp);
		head.size += len;
	}
	if (ferror(fp))
		return 0;
	fseek(fp, -(sizeof(head) + head.size), SEEK_CUR);
	fwrite(&head, sizeof(head), 1, fp);
	fseek(fp, head.size, SEEK_CUR);

	return sizeof(head) + head.size;
}

static void loadStrVars(enum save_format format, char *buf) {
	asd_strVarHdr *head = (asd_strVarHdr *)buf;
	const int base = format == SAVEFMT_XSYS35 ? 1 : 0;

	int cnt = head->count;
	if ((1 - base) + svar_maxindex() != cnt) {
		WARNING("Unexpected number of strings in savedata (%d, expected %d)", cnt, (1 - base) + svar_maxindex());
		svar_init(format == SAVEFMT_XSYS35 ? cnt : cnt - 1);
	}
	buf += sizeof(asd_strVarHdr);
	for (int i = 0; i < cnt; i++) {
		svar_set(i + base, buf);
		buf += strlen(buf) + 1;
	}
}

static int saveVarPage(int page, FILE *fp) {
	if (!varPage[page].saveflag)
		return 0;
	int cnt = varPage[page].size;
	int *var = varPage[page].value;
	if (!var)
		return 0;
	asd_varPageHdr head = {
		.size   = cnt * sizeof(uint16_t),
		.pageNo = page
	};
	fwrite(&head, sizeof(head), 1, fp);
	for (int i = 0; i < cnt; i++)
		fputw(var[i], fp);

	return sizeof(head) + head.size;
}

static int loadVarPage(char *buf) {
	asd_varPageHdr *head = (asd_varPageHdr *)buf;
	int page = head->pageNo;

	int cnt = head->size / sizeof(uint16_t);
	if (varPage[page].size < cnt || varPage[page].value == NULL) {
		if (!v_allocatePage(page, cnt, TRUE)) {
			WARNING("Array allocation failed: page=%d size=%d", page, cnt);
			return SAVE_LOADERR;
		}
	}
	int *var = varPage[page].value;
	uint16_t *data = (uint16_t *)(buf + sizeof(asd_varPageHdr));
	for (int i = 0; i < cnt; i++)
		*var++ = *data++;

	return SAVE_LOADOK;
}

static void writeAsdFooter(enum save_format format, FILE *fp) {
	if (format != SAVEFMT_XSYS35) {
		// Padding(?)
		for (int i = 0; i < 256; i++)
			fputc(0xff, fp);
		// Write a mark to indicate that this file was created by xsystem35.
		fputs("XS35", fp);
	}
	if (format == SAVEFMT_SYS38) {
		// Stack info
		struct stack_info sinfo;
		sl_getStackInfo(&sinfo);
		int size = sinfo.page_calls * 2 + sinfo.label_calls;
		int *buf = calloc(size, sizeof(int));
		int *ptr = buf + size;
		int *vars = NULL;
		int *label_calls = NULL;
		struct stack_frame_info *sfi = NULL;
		while ((sfi = sl_next_stack_frame(sfi)) != NULL) {
			switch (sfi->tag) {
			case STACK_NEARCALL:
				if (label_calls)
					(*label_calls)++;
				vars = --ptr;
				break;
			case STACK_FARCALL:
				vars = --ptr;
				label_calls = --ptr;
				break;
			case STACK_VARIABLE:
				if (vars)
					(*vars)++;
				break;
			}
		}
		// Note that only the first `size` byte of the `size` words data is
		// written, so information is lost.
		fwrite(ptr, size, 1, fp);
		fwrite(&size, 4, 1, fp);
		free(buf);
		fputs("INFS", fp);
	}
}

/* ゲームデータのロード

 no:    セーブファイル番号 0~
 *status:  ステータス
 *size: データの大きさを返すポインタ

あとで free(*buf)するのを忘れないように
*/
static void* loadGameData(int no, int *status, int *size) {
	void *buf = NULL;
	FILE *fp = fopen(nact->files.save_fname[no], "rb");
	if (!fp)
		goto errexit;
	fseek(fp, 0L, SEEK_END);
	long filesize = ftell(fp);
	if (filesize == 0)
		goto errexit;

	buf = malloc(filesize);
	if (buf == NULL)
		goto errexit;

	fseek(fp, 0L, SEEK_SET);
	if (fread(buf, filesize, 1, fp) != 1)
		goto errexit;
	fclose(fp);

	*size = (int)filesize;
	*status = SAVE_LOADOK;
	return buf;

 errexit:
	if (buf != NULL)
		free(buf);
	if (fp != NULL)
		fclose(fp);
	*status = SAVE_LOADERR;
	return NULL;
}

static int saveGameData(int no, char *buf, int size) {
	FILE *fp;
	int status = SAVE_SAVEOK1;

	fc_backup_oldfile(nact->files.save_fname[no]);
	fp = fopen(nact->files.save_fname[no], "wb");
	if (fp == NULL) {
		return SAVE_SAVEERR;
	}
	if (1 != fwrite(buf, size, 1, fp)) {
		status = SAVE_SAVEERR;
	}
	fclose(fp);
	scheduleSync();
	return status;
}

int save_setFormat(const char *format_name) {
	if (!strcmp(format_name, "xsystem35"))
		save_format = SAVEFMT_XSYS35;
	else if (!strcmp(format_name, "system36"))
		save_format = SAVEFMT_SYS36;
	else if (!strcmp(format_name, "system38"))
		save_format = SAVEFMT_SYS38;
	else if (!strcmp(format_name, "system39"))
		save_format = SAVEFMT_SYS38;
	else {
		WARNING("unknown save format %s", format_name);
		return NG;
	}
	return OK;
}

/* savefile を参照 */
const char *save_get_file(int index) {
	return nact->files.save_fname[index];
}

/* savefile を削除 */
int save_delete_file(int index) {
	int ret = unlink(nact->files.save_fname[index]);
	
	if (ret == 0) {
		return 1;
	}
	return 1; /* とりあえず */
}

// QE command
int save_vars_to_file(char *fname_utf8, struct VarRef *src, int cnt) {
	// FIXME: System39.exe does not truncate existing file.
	FILE *fp = fc_open(fname_utf8, 'w');
	if (!fp)
		return SAVE_SAVEERR;

	if (cnt > v_sliceSize(src)) {
		WARNING("QE: array size too small (size = %d, data count = %d)", v_sliceSize(src), cnt);
		cnt = v_sliceSize(src);
	}

	int *p = v_resolveRef(src);
	while (cnt--)
		fputw(*p++, fp);

	fclose(fp);
	scheduleSync();
	return SAVE_SAVEOK0;
}

// LE command
int load_vars_from_file(char *fname_utf8, struct VarRef *dest, int cnt) {
	FILE *fp = fc_open(fname_utf8, 'r');
	if (!fp)
		return SAVE_LOADERR;

	uint16_t *tmp = malloc(cnt * sizeof(uint16_t));
	if (!tmp)
		NOMEMERR();

	size_t size = fread(tmp, sizeof(uint16_t), cnt, fp);
	fclose(fp);

	if (size != cnt) {
		WARNING("LE: data file too small (requested = %d, loaded = %d)", cnt, size);
		cnt = size;
		// NOTE: System39.exe never returns SAVE_LOADSHORTAGE (254).
	}

	if (cnt > v_sliceSize(dest)) {
		WARNING("LE: array size too small (size = %d, data count = %d)", v_sliceSize(dest), cnt);
		cnt = v_sliceSize(dest);
	}

	int *start = v_resolveRef(dest);
	for (int i = 0; i < cnt; i++)
		start[i] = SDL_SwapLE16(tmp[i]);

	free(tmp);
	return SAVE_LOADOK;
}

int save_strs_to_file(char *fname_utf8, int start, int cnt) {
	FILE *fp = fc_open(fname_utf8, 'w');
	if (!fp)
		return SAVE_SAVEERR;
	for (int i = 0; i < cnt; i++) {
		fputs(svar_get(start + i), fp);
		fputc('\0', fp);
	}
	fclose(fp);
	scheduleSync();
	return 0;
}

int load_strs_from_file(char *fname_utf8, int start, int cnt) {
	FILE *fp = fc_open(fname_utf8, 'r');
	if (!fp)
		return SAVE_LOADERR;
	
	fseek(fp, 0L, SEEK_END);
	long filesize = ftell(fp);
	char *buf = malloc(filesize + 1);
	if (!buf)
		NOMEMERR();

	fseek(fp, 0L, SEEK_SET);
	int r = fread(buf, filesize, 1, fp);
	fclose(fp);
	if (!r) {
		free(buf);
		return SAVE_LOADERR;
	}

	buf[filesize] = '\0';  // prevents buffer overrun
	char *p = buf;
	for (int i = 0; i < cnt; i++) {
		if (p >= buf + filesize) {
			free(buf);
			return SAVE_LOADSHORTAGE;
		}
		svar_set(start + i, p);
		p += strlen(p) + 1;
	}
	free(buf);
	return 0;
}

/* セーブファイルのコピー */
int save_copyAll(int dstno, int srcno) {
	char *saveTop;
	int status, filesize;
	
	if (dstno >= SAVE_MAXNUMBER || srcno >= SAVE_MAXNUMBER) {
		return SAVE_SAVEERR;
	}
	saveTop = loadGameData(srcno, &status, &filesize);
	if (saveTop == NULL)
		return SAVE_SAVEERR;
	if (((asd_baseHdr *)saveTop)->version != SAVE_DATAVERSION) {
		WARNING("endian mismatch");
		free(saveTop);
		return SAVE_SAVEERR;
	}
	status = saveGameData(dstno, saveTop, filesize);
	
	free(saveTop);
	
	return status;
}

/* データの一部ロード */
int save_loadPartial(int no, struct VarRef *vref, int cnt) {
	if (no >= SAVE_MAXNUMBER)
		return SAVE_SAVEERR;

	cnt = min(cnt, v_sliceSize(vref));
	int *var = v_resolveRef(vref);
	
	int status, filesize;
	char *saveTop = loadGameData(no, &status, &filesize);
	if (!saveTop)
		return status;
	
	if (filesize <= sizeof(asd_baseHdr))
		goto errexit;

	asd_baseHdr *save_base = (asd_baseHdr *)saveTop;
	if (save_base->version != SAVE_DATAVERSION) {
		WARNING("endian mismatch");
		goto errexit;
	}

	if (save_base->varSys[vref->page] == 0)
		goto errexit;

	char *vtop = saveTop + save_base->varSys[vref->page] + sizeof(asd_varPageHdr);
	uint16_t *tmp = (uint16_t *)vtop + vref->index;
	for (int i = 0; i < cnt; i++) {
		*var = *tmp; tmp++; var++;
	}
	free(saveTop);
	return SAVE_LOADOK;
	
 errexit:
	if (saveTop != NULL)
		free(saveTop);
	
	return SAVE_LOADERR;
}

/* データの一部セーブ */
int save_savePartial(int no, struct VarRef *vref, int cnt) {
	if (no >= SAVE_MAXNUMBER || !varPage[vref->page].saveflag)
		return SAVE_SAVEERR;

	cnt = min(cnt, v_sliceSize(vref));
	int *var = v_resolveRef(vref);
	
	int status, filesize;
	char *saveTop = loadGameData(no, &status, &filesize);
	if (saveTop == NULL)
		return status;
	if (filesize <= sizeof(asd_baseHdr))
		goto errexit;
	asd_baseHdr *save_base = (asd_baseHdr *)saveTop;
	if (save_base->version != SAVE_DATAVERSION) {
		WARNING("endian mismatch");
		goto errexit;
	}
	char *vtop = saveTop + save_base->varSys[vref->page] + sizeof(asd_varPageHdr);
	uint16_t *tmp = (uint16_t *)vtop + vref->index;
	for (int i = 0; i < cnt; i++) {
		*tmp = (uint16_t)*var; tmp++; var++;
	}
	status = saveGameData(no, saveTop, filesize);
	free(saveTop);
	
	return status;
	
 errexit:
	if (saveTop != NULL)
		free(saveTop);
	
	return SAVE_SAVEERR;
}


/* データのロード */
int save_loadAll(int no) {
	if (no >= SAVE_MAXNUMBER)
		return SAVE_SAVEERR;

	int status, filesize;
	char *saveTop = loadGameData(no, &status, &filesize);
	if (!saveTop)
		return status;
	if (filesize <= sizeof(asd_baseHdr))
		goto errexit;
	/* 各種データの反映 */
	asd_baseHdr *save_base = (asd_baseHdr *)saveTop;
	if (save_base->version != SAVE_DATAVERSION) {
		WARNING("endian mismatch");
		goto errexit;
	}
	enum save_format format = (enum save_format)-1;
	for (int i = 0; i < sizeof(save_signature) / sizeof(save_signature[0]); i++) {
		if (!strcmp(save_signature[i], save_base->ID)) {
			format = i;
			break;
		}
	}
	if (format < 0) {
		WARNING("unrecognized save format");
		goto errexit;
	}

	nact->sel.MsgFontSize        = save_base->selMsgSize;
	nact->sel.MsgFontColor       = save_base->selMsgColor;
	nact->sel.WinBackgroundColor = save_base->selBackColor;
	nact->sel.WinFrameColor      = save_base->selFrameColor;
	nact->msg.MsgFontSize        = save_base->msgMsgSize;
	nact->msg.MsgFontColor       = save_base->msgMsgColor;
	nact->msg.WinBackgroundColor = save_base->msgBackColor;
	nact->msg.WinFrameColor      = save_base->msgFrameColor;
	sl_jmpFar2(save_base->scoPage - (format == SAVEFMT_XSYS35 ? 0 : 1), save_base->scoIndex);

	for (int i = 0; i < SELWINMAX; i++) {
		int j = format == SAVEFMT_XSYS35
			? (i + 1) % SELWINMAX  // For savedata compatibility
			: i;
		nact->sel.wininfo[j].x      = save_base->selWinInfo[i].x;
		nact->sel.wininfo[j].y      = save_base->selWinInfo[i].y;
		nact->sel.wininfo[j].width  = save_base->selWinInfo[i].width;
		nact->sel.wininfo[j].height = save_base->selWinInfo[i].height;
		// nact->sel.wininfo[i].save   = TRUE;
	}
	for (int i = 0; i < MSGWINMAX; i++) {
		int j = format == SAVEFMT_XSYS35
			? (i + 1) % MSGWINMAX  // For savedata compatibility
			: i;
		nact->msg.wininfo[j].x      = save_base->msgWinInfo[i].x;
		nact->msg.wininfo[j].y      = save_base->msgWinInfo[i].y;
		nact->msg.wininfo[j].width  = save_base->msgWinInfo[i].width;
		nact->msg.wininfo[j].height = save_base->msgWinInfo[i].height;
		// nact->msg.wininfo[i].savedImage = NULL;
		// nact->msg.wininfo[i].save   = FALSE;
	}
	/* スタックのロード */
	loadStack(format, saveTop + save_base->stackinfo);
	/* 文字列変数のロード */
	loadStrVars(format, saveTop + save_base->varStr);
	/* 数値・配列変数のロード */
	for (int i = 0; i < 256; i++) {
		if (save_base->varSys[i] != 0) {
			if (SAVE_LOADOK != loadVarPage(saveTop + save_base->varSys[i]))
				goto errexit;
			
		}
	}
	free(saveTop);
	return SAVE_LOADOK;
 errexit:
	free(saveTop);
	return SAVE_LOADERR;
}

/* データのセーブ */
int save_saveAll(int no) {
	enum save_format format = save_format;
	asd_baseHdr save_base = {};
	int totalsize = sizeof(asd_baseHdr);
	
	if (no >= SAVE_MAXNUMBER)
		return SAVE_SAVEERR;
	
	fc_backup_oldfile(nact->files.save_fname[no]);
	FILE *fp = fopen(nact->files.save_fname[no], "wb");
	if (!fp)
		return SAVE_SAVEERR;
	
	/* 各種データのセーブ */
	strncpy(save_base.ID, save_signature[format], 32);
	save_base.version       = SAVE_DATAVERSION;
	save_base.selMsgSize    = (uint8_t)nact->sel.MsgFontSize;
	save_base.selMsgColor   = (uint8_t)nact->sel.MsgFontColor;
	save_base.selBackColor  = (uint8_t)nact->sel.WinBackgroundColor;
	save_base.selFrameColor = (uint8_t)nact->sel.WinFrameColor;
	save_base.msgMsgSize    = (uint8_t)nact->msg.MsgFontSize;
	save_base.msgMsgColor   = (uint8_t)nact->msg.MsgFontColor;
	save_base.msgBackColor  = (uint8_t)nact->msg.WinBackgroundColor;
	save_base.msgFrameColor = (uint8_t)nact->msg.WinFrameColor;
	save_base.scoPage       = sl_getPage() + (format == SAVEFMT_XSYS35 ? 0 : 1);
	save_base.scoIndex      = sl_getIndex();

	for (int i = 0; i < SELWINMAX; i++) {
		int j = format == SAVEFMT_XSYS35
			? (i + 1) % SELWINMAX  // For savedata compatibility
			: i;
		save_base.selWinInfo[i].x      = (uint16_t)nact->sel.wininfo[j].x;
		save_base.selWinInfo[i].y      = (uint16_t)nact->sel.wininfo[j].y;
		save_base.selWinInfo[i].width  = (uint16_t)nact->sel.wininfo[j].width;
		save_base.selWinInfo[i].height = (uint16_t)nact->sel.wininfo[j].height;
	}
	
	for (int i = 0; i < MSGWINMAX; i++) {
		int j = format == SAVEFMT_XSYS35
			? (i + 1) % MSGWINMAX  // For savedata compatibility
			: i;
		save_base.msgWinInfo[i].x      = (uint16_t)nact->msg.wininfo[j].x;
		save_base.msgWinInfo[i].y      = (uint16_t)nact->msg.wininfo[j].y;
		save_base.msgWinInfo[i].width  = (uint16_t)nact->msg.wininfo[j].width;
		save_base.msgWinInfo[i].height = (uint16_t)nact->msg.wininfo[j].height;
	}

	fseek(fp, sizeof(asd_baseHdr), SEEK_SET);
	
	/* スタック情報 */
	save_base.stackinfo = totalsize;
	totalsize += saveStack(format, fp);
	if (ferror(fp))
		goto errexit;
	
	/* 文字列変数 */
	save_base.varStr = totalsize;
	totalsize += saveStrVars(format, fp);
	if (ferror(fp))
		goto errexit;
	
	/* 数値変数・配列変数 */
	for (int i = 0; i < 256; i++) {
		int size = saveVarPage(i, fp);
		if (size) {
			save_base.varSys[i] = totalsize;
			totalsize += size;
		} else {
			save_base.varSys[i] = 0;
		}
		if (ferror(fp))
			goto errexit;
	}
	
	fseek(fp, 0, SEEK_SET);
	
	if (1 != fwrite(&save_base, sizeof(asd_baseHdr), 1, fp))
		goto errexit;
	
	fseek(fp, 0L, SEEK_END);
	writeAsdFooter(format, fp);

	fclose(fp);
	scheduleSync();

	return SAVE_SAVEOK1;
	
 errexit:
	fclose(fp);
	return SAVE_SAVEERR;
}

