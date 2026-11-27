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

#include <limits.h>
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

static enum save_format save_format = SAVEFMT_SYS38;

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

static void loadStack(enum save_format format, const char *buf) {
	asd_stackHdr head;
	memcpy(&head, buf, sizeof(head));
	const uint8_t *data = (const uint8_t *)(buf + sizeof(asd_stackHdr));
	sl_loadStack(format, (uint8_t *)data, head.size);
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

static void loadStrVars(enum save_format format, const char *buf) {
	asd_strVarHdr head;
	memcpy(&head, buf, sizeof(head));
	const int base = format == SAVEFMT_XSYS35 ? 1 : 0;

	int cnt = head.count;
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
	vmvar_t *var = varPage[page].value;
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

static int validateVarPage(const char *buf, size_t max_size) {
	if (max_size < sizeof(asd_varPageHdr)) {
		WARNING("Truncated variable page header");
		return SAVE_LOADERR;
	}
	asd_varPageHdr head;
	memcpy(&head, buf, sizeof(head));
	int page = head.pageNo;
	if (page < 0 || page >= 256) {
		WARNING("Invalid variable page number in savedata: %d", page);
		return SAVE_LOADERR;
	}
	if (head.size < 0 || (size_t)head.size > max_size - sizeof(asd_varPageHdr)) {
		WARNING("Invalid variable page size in savedata: page=%d size=%d", page, head.size);
		return SAVE_LOADERR;
	}

	int cnt = head.size / (int)sizeof(uint16_t);
	if (varPage[page].size < cnt || varPage[page].value == NULL) {
		if (cnt <= 0 || cnt > 65536) {
			WARNING("Invalid variable page allocation size in savedata: page=%d size=%d", page, cnt);
			return SAVE_LOADERR;
		}
	}
	return SAVE_LOADOK;
}

static int loadVarPage(const char *buf, size_t max_size) {
	if (validateVarPage(buf, max_size) != SAVE_LOADOK)
		return SAVE_LOADERR;
	asd_varPageHdr head;
	memcpy(&head, buf, sizeof(head));
	int page = head.pageNo;
	int cnt = head.size / (int)sizeof(uint16_t);
	if (varPage[page].size < cnt || varPage[page].value == NULL) {
		if (!v_allocatePage(page, cnt, true)) {
			WARNING("Array allocation failed: page=%d size=%d", page, cnt);
			return SAVE_LOADERR;
		}
	}
	vmvar_t *var = varPage[page].value;
	const uint8_t *data = (const uint8_t *)(buf + sizeof(asd_varPageHdr));
	for (int i = 0; i < cnt; i++) {
		uint16_t v;
		memcpy(&v, data + (size_t)i * sizeof(uint16_t), sizeof(v));
		*var++ = v;
	}

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

static char *save_resolve_slot_path(int no) {
	if (no < 0 || no >= SAVE_MAXNUMBER || !nact->files.save_fname[no])
		return NULL;
	return fc_resolve_existing_path(nact->files.save_fname[no]);
}

static bool save_prepare_write(int no) {
	if (no < 0 || no >= SAVE_MAXNUMBER || !nact->files.save_fname[no])
		return false;
	const char *canonical = nact->files.save_fname[no];
	if (access(canonical, F_OK) == 0) {
		fc_backup_oldfile(canonical);
		return true;
	}
	return false;
}

static void save_finalize_write(int no, bool backed_up) {
	if (no < 0 || no >= SAVE_MAXNUMBER || !nact->files.save_fname[no])
		return;
	const char *canonical = nact->files.save_fname[no];
#ifndef __EMSCRIPTEN__
	char *backup_path = NULL;
	if (!backed_up) {
		backup_path = malloc(strlen(canonical) + 2);
		if (backup_path) {
			strcpy(backup_path, canonical);
			strcat(backup_path, ".");
		}
	}
	fc_remove_case_variants(canonical, true, backup_path);
	free(backup_path);
#else
	(void)backed_up;
	fc_remove_case_variants(canonical, true, NULL);
#endif
}

/* ゲームデータのロード

 no:    セーブファイル番号 0~
 *status:  ステータス
 *size: データの大きさを返すポインタ

あとで free(*buf)するのを忘れないように
*/
static void* loadGameData(int no, int *status, int *size) {
	void *buf = NULL;
	FILE *fp = NULL;
	char *resolved = save_resolve_slot_path(no);
	if (!resolved)
		goto errexit;
	fp = fopen(resolved, "rb");
	free(resolved);
	if (!fp)
		goto errexit;
	fseek(fp, 0L, SEEK_END);
	long filesize = ftell(fp);
	if (filesize <= 0)
		goto errexit;

	buf = malloc((size_t)filesize);
	if (buf == NULL)
		goto errexit;

	fseek(fp, 0L, SEEK_SET);
	if (fread(buf, (size_t)filesize, 1, fp) != 1)
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

	if (no < 0 || no >= SAVE_MAXNUMBER || !nact->files.save_fname[no])
		return SAVE_SAVEERR;

	bool backed_up = save_prepare_write(no);
	fp = fopen(nact->files.save_fname[no], "wb");
	if (fp == NULL) {
		return SAVE_SAVEERR;
	}
	if (1 != fwrite(buf, size, 1, fp)) {
		status = SAVE_SAVEERR;
	}
	if (fclose(fp) != 0) {
		status = SAVE_SAVEERR;
	}
	if (status == SAVE_SAVEOK1) {
		save_finalize_write(no, backed_up);
		scheduleSync();
	}
	return status;
}

bool save_setFormat(const char *format_name) {
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
		return false;
	}
	return true;
}

void save_resetFormat(void) {
	save_format = SAVEFMT_SYS38;
}

/* savefile を参照 */
const char *save_get_file(int index) {
	if (index < 0 || index >= SAVE_MAXNUMBER)
		return NULL;
	const char *fname = nact->files.save_fname[index];
	if (!fname)
		return NULL;
	char *resolved = save_resolve_slot_path(index);
	if (resolved) {
		if (strcmp(resolved, fname) == 0) {
			free(resolved);
			return fname;
		}
		static char resolved_buf[PATH_MAX];
		snprintf(resolved_buf, sizeof(resolved_buf), "%s", resolved);
		free(resolved);
		return resolved_buf;
	}
	return fname;
}

/* savefile を削除 */
int save_delete_file(int index) {
	if (index < 0 || index >= SAVE_MAXNUMBER || !nact->files.save_fname[index])
		return 0;
	fc_remove_case_variants(nact->files.save_fname[index], false, NULL);
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

	vmvar_t *p = v_resolveRef(src);
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

	vmvar_t *start = v_resolveRef(dest);
	for (int i = 0; i < cnt; i++)
		start[i] = SDL_Swap16LE(tmp[i]);

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
	
	if (dstno < 0 || dstno >= SAVE_MAXNUMBER || srcno < 0 || srcno >= SAVE_MAXNUMBER) {
		return SAVE_SAVEERR;
	}
	saveTop = loadGameData(srcno, &status, &filesize);
	if (saveTop == NULL)
		return SAVE_SAVEERR;
	if ((size_t)filesize < sizeof(asd_baseHdr)) {
		free(saveTop);
		return SAVE_SAVEERR;
	}
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
	if (no < 0 || no >= SAVE_MAXNUMBER || !vref || vref->page < 0 || vref->page >= 256 || vref->index < 0 || cnt < 0)
		return SAVE_LOADERR;

	cnt = min(cnt, v_sliceSize(vref));
	vmvar_t *var = v_resolveRef(vref);
	
	int status, filesize;
	char *saveTop = loadGameData(no, &status, &filesize);
	if (!saveTop)
		return status;
	
	if ((size_t)filesize <= sizeof(asd_baseHdr))
		goto errexit;

	asd_baseHdr *save_base = (asd_baseHdr *)saveTop;
	if (save_base->version != SAVE_DATAVERSION) {
		WARNING("endian mismatch");
		goto errexit;
	}

	if (save_base->varSys[vref->page] == 0)
		goto errexit;

	if (save_base->varSys[vref->page] < (int)sizeof(asd_baseHdr) ||
	    (size_t)save_base->varSys[vref->page] + sizeof(asd_varPageHdr) > (size_t)filesize)
		goto errexit;

	size_t data_off = (size_t)save_base->varSys[vref->page] + sizeof(asd_varPageHdr);
	size_t elem_off = (size_t)vref->index * sizeof(uint16_t);
	size_t copy_bytes = (size_t)cnt * sizeof(uint16_t);
	if (elem_off > (size_t)filesize - data_off || copy_bytes > (size_t)filesize - data_off - elem_off)
		goto errexit;

	const uint8_t *vtop = (const uint8_t *)(saveTop + data_off + elem_off);
	for (int i = 0; i < cnt; i++) {
		uint16_t v;
		memcpy(&v, vtop + (size_t)i * sizeof(uint16_t), sizeof(v));
		*var++ = v;
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
	if (no < 0 || no >= SAVE_MAXNUMBER || !vref || vref->page < 0 || vref->page >= 256 || vref->index < 0 || cnt < 0 || !varPage[vref->page].saveflag)
		return SAVE_SAVEERR;

	cnt = min(cnt, v_sliceSize(vref));
	vmvar_t *var = v_resolveRef(vref);
	
	int status, filesize;
	char *saveTop = loadGameData(no, &status, &filesize);
	if (saveTop == NULL)
		return status;
	if ((size_t)filesize <= sizeof(asd_baseHdr))
		goto errexit;
	asd_baseHdr *save_base = (asd_baseHdr *)saveTop;
	if (save_base->version != SAVE_DATAVERSION) {
		WARNING("endian mismatch");
		goto errexit;
	}
	if (save_base->varSys[vref->page] == 0)
		goto errexit;
	if (save_base->varSys[vref->page] < (int)sizeof(asd_baseHdr) ||
	    (size_t)save_base->varSys[vref->page] + sizeof(asd_varPageHdr) > (size_t)filesize)
		goto errexit;

	size_t data_off = (size_t)save_base->varSys[vref->page] + sizeof(asd_varPageHdr);
	size_t elem_off = (size_t)vref->index * sizeof(uint16_t);
	size_t copy_bytes = (size_t)cnt * sizeof(uint16_t);
	if (elem_off > (size_t)filesize - data_off || copy_bytes > (size_t)filesize - data_off - elem_off)
		goto errexit;

	uint8_t *vtop = (uint8_t *)(saveTop + data_off + elem_off);
	for (int i = 0; i < cnt; i++) {
		uint16_t v = (uint16_t)*var++;
		memcpy(vtop + (size_t)i * sizeof(uint16_t), &v, sizeof(v));
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
	if (no < 0 || no >= SAVE_MAXNUMBER)
		return SAVE_LOADERR;

	int status, filesize;
	char *saveTop = loadGameData(no, &status, &filesize);
	if (!saveTop)
		return status;
	if ((size_t)filesize <= sizeof(asd_baseHdr))
		goto errexit;
	/* 各種データの事前検証 (ライブ状態を変更する前にすべて検証) */
	asd_baseHdr *save_base = (asd_baseHdr *)saveTop;
	if (save_base->version != SAVE_DATAVERSION) {
		WARNING("endian mismatch");
		goto errexit;
	}
	if (!memchr(save_base->ID, '\0', sizeof(save_base->ID))) {
		WARNING("unterminated save signature");
		goto errexit;
	}
	enum save_format format = (enum save_format)-1;
	for (size_t i = 0; i < sizeof(save_signature) / sizeof(save_signature[0]); i++) {
		if (!strcmp(save_signature[i], save_base->ID)) {
			format = (enum save_format)i;
			break;
		}
	}
	if (format < 0) {
		WARNING("unrecognized save format");
		goto errexit;
	}

	if (save_base->stackinfo < (int)sizeof(asd_baseHdr) ||
	    (size_t)save_base->stackinfo + sizeof(asd_stackHdr) > (size_t)filesize ||
	    save_base->varStr < (int)sizeof(asd_baseHdr) ||
	    (size_t)save_base->varStr + sizeof(asd_strVarHdr) > (size_t)filesize)
		goto errexit;

	asd_stackHdr stack_hdr;
	memcpy(&stack_hdr, saveTop + save_base->stackinfo, sizeof(stack_hdr));
	if (stack_hdr.size < 0 ||
	    (size_t)stack_hdr.size > (size_t)filesize - (size_t)save_base->stackinfo - sizeof(asd_stackHdr)) {
		WARNING("Invalid stack section size in savedata: %d", stack_hdr.size);
		goto errexit;
	}

	asd_strVarHdr str_hdr;
	memcpy(&str_hdr, saveTop + save_base->varStr, sizeof(str_hdr));
	const int str_base = format == SAVEFMT_XSYS35 ? 1 : 0;
	if (str_hdr.size < 0 ||
	    (size_t)str_hdr.size > (size_t)filesize - (size_t)save_base->varStr - sizeof(asd_strVarHdr) ||
	    str_hdr.count < (1 - str_base)) {
		WARNING("Invalid string variable header in savedata: size=%d count=%d", str_hdr.size, str_hdr.count);
		goto errexit;
	}
	{
		const char *sp = saveTop + save_base->varStr + sizeof(asd_strVarHdr);
		const char *send = sp + (size_t)str_hdr.size;
		for (int i = 0; i < str_hdr.count; i++) {
			const char *nul = memchr(sp, '\0', (size_t)(send - sp));
			if (!nul) {
				WARNING("Truncated string variable data in savedata at index %d", i);
				goto errexit;
			}
			sp = nul + 1;
		}
	}

	for (int i = 0; i < 256; i++) {
		if (save_base->varSys[i] != 0) {
			if (save_base->varSys[i] < (int)sizeof(asd_baseHdr) ||
			    (size_t)save_base->varSys[i] >= (size_t)filesize)
				goto errexit;
			if (SAVE_LOADOK != validateVarPage(saveTop + save_base->varSys[i],
			                                   (size_t)filesize - (size_t)save_base->varSys[i]))
				goto errexit;
		}
	}

	/* 各種データの反映 */
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
		// nact->sel.wininfo[i].save   = true;
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
		// nact->msg.wininfo[i].save   = false;
	}
	/* スタックのロード */
	loadStack(format, saveTop + save_base->stackinfo);
	/* 文字列変数のロード */
	loadStrVars(format, saveTop + save_base->varStr);
	/* 数値・配列変数のロード */
	for (int i = 0; i < 256; i++) {
		if (save_base->varSys[i] != 0) {
			if (SAVE_LOADOK != loadVarPage(saveTop + save_base->varSys[i],
			                               (size_t)filesize - (size_t)save_base->varSys[i]))
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
	
	if (no < 0 || no >= SAVE_MAXNUMBER || !nact->files.save_fname[no])
		return SAVE_SAVEERR;
	
	bool backed_up = save_prepare_write(no);
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
	if (ferror(fp))
		goto errexit;

	if (fclose(fp) != 0)
		return SAVE_SAVEERR;
	save_finalize_write(no, backed_up);
	scheduleSync();

	return SAVE_SAVEOK1;
	
 errexit:
	fclose(fp);
	return SAVE_SAVEERR;
}
