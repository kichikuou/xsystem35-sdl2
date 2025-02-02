/*
 * cmd2F.c  SYSTEM35 0x2f,xx command
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
/* $Id: cmd2F.c,v 1.35 2006/04/21 16:39:02 chikama Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "portab.h"
#include "xsystem35.h"
#include "scenario.h"
#include "savedata.h"
#include "menu.h"
#include "input.h"
#include "msgskip.h"
#include "selection.h"
#include "message.h"
#include "music.h"
#include "utfsjis.h"
#include "hankaku.h"
#include "ags.h"
#include "sdl_core.h"
#include "ald_manager.h"
#include "LittleEndian.h"
#include "hacks.h"
#include "filecheck.h"

/* 選択 Window OPEN 時 callback */
static int cb_sel_init_page = 0;
static int cb_sel_init_address = 0;

static const char REGREADSTRING_RESULT[] = "*regReadString*";

static SDL_Rect clip_window;

extern INPUTSTRING_PARAM mi_param;

extern void commandH();
extern void commandHH();

typedef struct {
	int page;
	int index;
} fncall_table;

#define FCTBL_MAX 1024
static fncall_table fnctbl[FCTBL_MAX];

void commands2F00() {
	/* テキストカラーをスタックからポップして設定する */
	sl_popState(STACK_TEXTCOLOR);
	TRACE("TOC:");
}

void commands2F01() {
	/* テキストフォントサイズをスタックからポップして設定する */
	sl_popState(STACK_TEXTSIZE);
	TRACE("TOS:");
}

void commands2F02() {
	/* 現在のテキストカラーをスタックにプッシュする */
	int type = getCaliValue();
	switch (type) {
	case 0: sl_pushTextColor(type, nact->msg.MsgFontColor); break;
	case 1: sl_pushTextColor(type, nact->sel.MsgFontColor); break;
	default: WARNING("TPC: unknown type %d", type); break;
	}
	TRACE("TPC %d", exp);
}

void commands2F03() {
	/* 現在のテキストフォントサイズをスタックにプッシュする */
	int type = getCaliValue();
	switch (type) {
	case 0: sl_pushTextSize(type, nact->msg.MsgFontSize); break;
	case 1: sl_pushTextSize(type, nact->sel.MsgFontSize); break;
	default: WARNING("TPS: unknown type %d", type); break;
	}
	TRACE("TPS %d", exp);
}

void commands2F04() {
	/* テキスト表示位置をスタックからポップして設定する */
	sl_popState(STACK_TEXTLOC);
	TRACE("TOP:");
}

void commands2F05() {
	/* 現在のテキスト表示位置をスタックにプッシュする */
	MyPoint loc;
	msg_getMessageLocation(&loc);
	sl_pushTextLoc(loc.x, loc.y);
	TRACE("TPP:");
}

void commands2F08() {
	/* アンチエイリアシング付きテキスト描画のフラグ設定 */
	int exp = getCaliValue();
	
	ags_setAntialiasedStringMode(exp == 1);
	
	TRACE("TAA %d:", exp);
}

void commands2F09() {
	/* アンチエイリアシング付きテキスト描画のフラグ取得 */
	int *var = getCaliVariable();

	*var = ags_getAntialiasedStringMode() ? 1 : 0;
	
	TRACE("TAB %d:", *var);
}

void commands2F0A() {
	/* Wavデータを読み込む */
	int eCh = getCaliValue();
	int eLinkNum = getCaliValue();

	mus_wav_load(eCh, eLinkNum);
	
	TRACE("wavLoad %d, %d:", eCh, eLinkNum);
}

void commands2F0B() {
	/* Wavを再生する */
	int eCh = getCaliValue();
	int eLoopFlag = getCaliValue();

	mus_wav_play(eCh, eLoopFlag);
	
	TRACE("wavPlay %d, %d", eCh, eLoopFlag);
}

void commands2F0C() {
	int eCh = getCaliValue();
	
	mus_wav_stop(eCh);
	
	TRACE("wavStop %d", eCh);
}

void commands2F0D() {
	int eCh = getCaliValue();
	
	mus_wav_unload(eCh);
	
	TRACE("wavUnload %d:", eCh);
}

void commands2F0E() {
	int eCh = getCaliValue();
	int *vResult = getCaliVariable();
	int pos = mus_wav_get_playposition(eCh);
	
	*vResult = ((pos == 0 || pos == 65535) ? 0 : 1);
	
	TRACE("wavIsPlay %d, %d:", eCh, *vResult);
}

void commands2F0F() {
	int eCh       = getCaliValue();
	int eTime     = getCaliValue();
	int eVolume   = getCaliValue();
	int eStopFlag = getCaliValue();

	mus_wav_fadeout_start(eCh, eTime, eVolume, eStopFlag);
	
	TRACE("wavFade %d, %d, %d, %d:", eCh, eTime, eVolume, eStopFlag);
}

void commands2F10() {
	int eCh = getCaliValue();
	int *vResult = getCaliVariable();
	
	*vResult = (mus_wav_fadeout_get_state(eCh) ? 1 : 0);
	
	TRACE("wavIsFade %d, %d:", eCh, *vResult);
}

void commands2F11() {
	int eCh = getCaliValue();

	mus_wav_fadeout_stop(eCh);
	
	TRACE("wavStopFade %d:", eCh);
}

void commands2F12() {
	const char *sText = sl_getString(0);
	
	TRACE("trace %s:", sText);

}

void commands2F13() {
	int eCh = getCaliValue();
	int eX = getCaliValue();
	int eY = getCaliValue();
	int eZ = getCaliValue();

	TRACE_UNIMPLEMENTED("wav3DSetPos %d, %d, %d, %d:", eCh, eX, eY, eZ);
}

void commands2F14() {
	TRACE_UNIMPLEMENTED("wav3DCommit:");
}

void commands2F15() {
	int eCh = getCaliValue();
	int *eX = getCaliVariable();
	int *eY = getCaliVariable();
	int *eZ = getCaliVariable();

	*eX = 0;
	*eY = 0;
	*eZ = 0;
	TRACE_UNIMPLEMENTED("wav3DGetPos %d, %d, %d, %d:", eCh, *eX, *eY, *eZ);
}

void commands2F16() {
	int eX = getCaliValue();
	int eY = getCaliValue();
	int eZ = getCaliValue();
	
	TRACE_UNIMPLEMENTED("wav3DSetPosL %d, %d, %d:", eX, eY, eZ);
}

void commands2F17() {
	int *vX = getCaliVariable();
	int *vY = getCaliVariable();
	int *vZ = getCaliVariable();
	
	*vX = 0;
	*vY = 0;
	*vZ = 0;
	TRACE_UNIMPLEMENTED("wav3DGetPosL %d, %d, %d:%s", *vX, *vY, *vZ);
}

void commands2F18() {
	int eCh = getCaliValue();
	int eTime = getCaliValue();
	int eX = getCaliValue();
	int eY = getCaliValue();
	int eZ = getCaliValue();

	TRACE_UNIMPLEMENTED("wav3DFadePos %d, %d, %d, %d, %d:", eCh, eTime, eX, eY, eZ);
}

void commands2F19() {
	int eCh = getCaliValue();
	int *vResult = getCaliVariable();

	*vResult = 0;
	TRACE_UNIMPLEMENTED("wav3DIsFadePos %d, %d:", eCh, *vResult);
}

void commands2F1A() {
	int eCh = getCaliValue();

	TRACE_UNIMPLEMENTED("wav3DStopFadePos %d:", eCh);
}

void commands2F1B() {
	int eTime = getCaliValue();
	int eX = getCaliValue();
	int eY = getCaliValue();
	int eZ = getCaliValue();

	TRACE_UNIMPLEMENTED("wav3DFadePosL %d, %d, %d, %d:", eTime, eX, eY, eZ);
}

void commands2F1C() {
	int *vResult = getCaliVariable();
	
	*vResult = 0;
	TRACE_UNIMPLEMENTED("wav3DIsFadePosL %d:", *vResult);
}

void commands2F1D() {
	TRACE_UNIMPLEMENTED("wav3DStopFadePosL");
}

void commands2F1E() {
	int eLinkNum = getCaliValue();
	int eLoop = getCaliValue();

	if (eLinkNum > 0) {
		mus_pcm_start(eLinkNum , eLoop);
	}
	
	TRACE("sndPlay %d, %d:", eLinkNum, eLoop);
}

void commands2F1F() {
	mus_pcm_stop(0);
	
	TRACE("sndStop:");
}

void commands2F20() {
	int *vResult = getCaliVariable();
	
	mus_pcm_get_playposition(vResult);
	
	TRACE("sndIsPlay %d:", *vResult);
}

void commands2F21() {
	const char *str = sl_getString(0);
	
	sys_addMsg(str);
}

void commands2F23() {
	int x = getCaliValue();
	int y = getCaliValue();
	const char *vFileName = sl_getString(0);
	
	char *fname_utf8 = toUTF8(vFileName);
	sysVar[0] = cg_load_with_filename(fname_utf8, x, y);
	free(fname_utf8);
	
	TRACE("LC(new) %d, %d, %s:", x, y, vFileName);
}

void commands2F24() {
	int type = sl_getc();
	const char *file_name = sl_getString(0);
	int var, cnt;
	struct VarRef vref;

	char *fname_utf8 = toUTF8(file_name);
	switch (type) {
	case 0:
		getCaliArray(&vref);
		var = vref.var;
		cnt = getCaliValue();
		sysVar[0] = load_vars_from_file(fname_utf8, &vref, cnt);
		break;
	case 1:
		var = getCaliValue();
		cnt = getCaliValue();
		sysVar[0] = load_strs_from_file(fname_utf8, var, cnt);
		break;
	default:
		var = getCaliValue();
		cnt = getCaliValue();
		WARNING("Unknown LE command type %d", type);
		break;
	}
	free(fname_utf8);
	
	TRACE("LE(new) %d, %s, %d, %d:", type, file_name, var, cnt);
}

void commands2F25() {
	int file_name = getCaliValue();
	const char *title = sl_getString(0);
	const char *filter = sl_getString(0);

	TRACE_UNIMPLEMENTED("LXG %d, %s, %s:", file_name, title, filter);
}

void commands2F26() {
	int dst_no  = getCaliValue();
	int max_len = getCaliValue();
	const char *title  = sl_getString(0);
	char *t1, *t2, *t3;
	
	t1 = toUTF8(title);
	t2 = toUTF8(svar_get(dst_no));
	
	mi_param.title = t1;
	mi_param.oldstring = t2;
	mi_param.max = max_len;
	
	menu_inputstring(&mi_param);
	if (mi_param.newstring == NULL) {
		svar_set(dst_no, NULL);
		free(t1); free(t2);
		return;
	}
	
	t3 = fromUTF8(mi_param.newstring);
	
	svar_set(dst_no, t3);

	free(t1);
	free(t2);
	free(t3);
	
	TRACE("MI(new) %d, %d, %s:", dst_no, max_len, title);
}

void commands2F27() {
	int num = getCaliValue();
	const char *string = sl_getString(0);

	svar_set(num, string);
	TRACE("MS(new) %d, %s:", num, string);
}

void commands2F28() {
	const char *title = sl_getString(0);
	
	if (nact->game_title_utf8)
		free(nact->game_title_utf8);
	nact->game_title_utf8 = toUTF8(title);

	ags_setWindowTitle(nact->game_title_utf8);

	enable_hack_by_title(nact->game_title_utf8);

	TRACE("MT(new) %s:",title);
}

/* defined in cmdn.c */
extern INPUTNUM_PARAM ni_param;
void commands2F29() {
	const char *title = sl_getString(0);
	char *t;
	
	if (ni_param.title != NULL) {
		free(ni_param.title);
	}
	t = toUTF8(title);
	ni_param.title = t;
	
	TRACE("NT(new) %s:", title);
}

void commands2F2A() {
	int type = sl_getc();
	const char *file_name = sl_getString(0);
	int var, cnt;
	struct VarRef vref;

	char *fname_utf8 = toUTF8(file_name);
	switch(type) {
	case 0:
		getCaliArray(&vref);
		var = vref.var;
		cnt = getCaliValue();
		sysVar[0] = save_vars_to_file(fname_utf8, &vref, cnt);
		break;
	case 1:
		var = getCaliValue();
		cnt = getCaliValue();
		sysVar[0] = save_strs_to_file(fname_utf8, var, cnt);
		break;
	default:
		var = getCaliValue();
		cnt = getCaliValue();
		WARNING("Unknown QE command");
		break;
	}
	free(fname_utf8);

	TRACE("QE(new) %d, %s, %d, %d:", type, file_name, var, cnt);
}

void commands2F2B() {
	int type = sl_getc();
	const char *work_dir = sl_getString(0);
	const char *file_name = sl_getString(0);

	TRACE_UNIMPLEMENTED("UP(new) %d, %s, %s:", type, work_dir, file_name);
}

void commands2F2D() {
	int eCh = getCaliValue();
	int eTime = getCaliValue();
	
	/* 該当する eCh が演奏中の場合 eTime だけ wait をいれる */
	mus_wav_waittime(eCh, eTime);
	
	TRACE("wavWaitTime %d, %d:", eCh, eTime);
}

void commands2F2E() {
	int eCh = getCaliValue();
	int *vTime = getCaliVariable();
	
	*vTime = mus_wav_get_playposition(eCh);
	
	TRACE("wavGetPlayPos %d, %d:", eCh, *vTime);
}

void commands2F2F() {
	int eCh = getCaliValue();
	
	/* 該当する eCh が演奏中の場合、終了まで待つ  */
	/* ただし、無限ループの場合は１回目終了時まで */
	mus_wav_waitend(eCh);
	
	TRACE("wavWaitEnd %d:", eCh);
}

void commands2F30() {
	int eCh = getCaliValue();
	int *vTime = getCaliVariable();
	
	*vTime = mus_wav_wavtime(eCh);
	
	TRACE("wavGetWavTime %d, %d:", eCh, *vTime);
}

void commands2F31() {
	int fPage = sl_getw();
	int fIndex = sl_getaddr();
	
	sel_setCallback(1, fPage, fIndex);
	TRACE("menuSetCbkSelect page=%d, index=%x:", fPage, fIndex);
}

void commands2F32() {
	int fPage = sl_getw();
	int fIndex = sl_getaddr();
	
	sel_setCallback(2, fPage, fIndex);
	TRACE("menuSetCbkCancel page=%d, index=%x:", fPage, fIndex);
}

void commands2F33() {
	sel_setCallback(1, 0, 0);
	TRACE("menuClearCbkSelect:");
}

void commands2F34() {
	sel_setCallback(2, 0, 0);
	TRACE("menuClearCbkCancel:");
}

void commands2F35() {
	int eCh = getCaliValue();
	int eMode = getCaliValue();
	
	TRACE_UNIMPLEMENTED("wav3DSetMode %d, %d:", eCh, eMode);
}

void commands2F36() {
	int eDestX     = getCaliValue();
	int eDestY     = getCaliValue();
	int eDestWidth  = getCaliValue();
	int eDestHeight = getCaliValue();
	int eSrcX      = getCaliValue();
	int eSrcY      = getCaliValue();
	int eSrcWidth  = getCaliValue();
	int eSrcHeight = getCaliValue();
	int eMode      = getCaliValue();
	
	ags_scaledCopyArea(eSrcX, eSrcY, eSrcWidth, eSrcHeight,
			   eDestX, eDestY, eDestWidth, eDestHeight, 0);
	ags_updateArea(eDestX, eDestY, eDestWidth, eDestHeight);
	
	TRACE("grCopyStrerch %d, %d, %d, %d, %d, %d, %d, %d, %d:",
		      eDestX, eDestY, eDestWidth, eDestHeight,
		      eSrcX, eSrcY, eSrcWidth, eSrcHeight, eMode);
}

void commands2F37() {
	int eX      = getCaliValue();
	int eY      = getCaliValue();
	int eWidth  = getCaliValue();
	int eHeight = getCaliValue();
	int eType   = getCaliValue();
	
	TRACE_UNIMPLEMENTED("grFilterRect %d, %d, %d, %d, %d:",
			  eX, eY, eWidth, eHeight, eType);
}

void commands2F38() {
	sys_clearWheelInfo();
	TRACE("iptClearWheelCount:");
}

void commands2F39() {
	int *vForward = getCaliVariable();
	int *vBack = getCaliVariable();
	
	sys_getWheelInfo(vForward, vBack);
	TRACE("iptGetWheelCount %d, %d:", *vForward, *vBack);
}

void commands2F3A() {
	int *vSize = getCaliVariable();

	*vSize = nact->sel.MsgFontSize;
	TRACE("menuGetFontSize %d:", *vSize);
}

void commands2F3B() {
	int *vSize = getCaliVariable();
	
	*vSize = nact->msg.MsgFontSize;
	TRACE("msgGetFontSize %d:", *vSize);
}	

void commands2F3C() {
	int eNum = getCaliValue();
	int ePos = getCaliValue();
	int *vResult = getCaliVariable();
	
	*vResult = svar_getCharType(eNum, ePos);
	
	TRACE("strGetCharType %d, %d, %d:", eNum, ePos, *vResult);
}

void commands2F3D() {
	int eNum = getCaliValue();
	int *vResult = getCaliVariable();
	
	*vResult = svar_width(eNum);

	TRACE("strGetLengthASCII %d, %d:", eNum, *vResult);
}

void commands2F3E() {
	// no-op
	TRACE("sysWinMsgLock:");
}

void commands2F3F() {
	// no-op
	TRACE("sysWinMsgUnlock:");
}

void commands2F40() {
	int *vAry = getCaliVariable();
	int eCount = getCaliValue();
	int eNum = getCaliValue();
	int *vResult = getCaliVariable();
	int cnt = 0;
	
	TRACE("aryCmpCount %d, %d, %d, %d:", *vAry, eCount, eNum, *vResult);
	
	while(eCount--) {
		if (*vAry == eNum) cnt++;
		vAry++;
	}
	*vResult = cnt;
}

void commands2F41() {
	int *vAry = getCaliVariable();
	int eNumof = getCaliValue();
	int eNum = getCaliValue();
	int eTransNum = getCaliValue();
	int eMaxTrans = getCaliValue();
	int *vResult = getCaliVariable();
	int cnt = 0;
	
	TRACE("aryCmpTrans %d, %d, %d, %d, %d, %d:", 
		      *vAry, eNumof, eNum, eTransNum, eMaxTrans, *vResult);
	
	while(eNumof--) {
		if (*vAry == eNum) {
			*vAry = eTransNum;
			cnt++;
			if (cnt >= eMaxTrans) break;
		}
		vAry++;
	}
	*vResult = cnt;
}

void commands2F42() {
	int eDestX    = getCaliValue();
	int eDestY    = getCaliValue();
	int eWidth    = getCaliValue();
	int eHeight   = getCaliValue();
	int eDestRate = getCaliValue();
	int eSrcX     = getCaliValue();
	int eSrcY     = getCaliValue();
	int eSrcRate  = getCaliValue();
	int eType     = getCaliValue();

	TRACE_UNIMPLEMENTED("grBlendColorRect %d, %d, %d, %d, %d, %d, %d, %d, %d:",
			  eDestX, eDestY, eWidth, eHeight, eDestRate,
			  eSrcX, eSrcY, eSrcRate, eType);
}

void commands2F43() {
	int eX = getCaliValue();
	int eY = getCaliValue();
	int eLength = getCaliValue();
	int eColor  = getCaliValue();

	sdl_fillCircle(eX, eY, eLength, eColor);
	ags_updateArea(eX, eY, eLength, eLength);

	TRACE("grDrawFillCircle %d, %d, %d, %d:", eX, eY, eLength, eColor);
}

void commands2F44() {
	int num1 = getCaliValue();
	int fig  = getCaliValue();
	int num2 = getCaliValue();
	char buf[256];

	svar_set(num1, format_number(num2, fig, buf));
	
	TRACE("MHH %d, %d, %d:", num1, fig, num2);
}

void commands2F45() {
	int fPage = sl_getw();
	int fIndex = sl_getaddr();
	
	cb_sel_init_page    = fPage;
	cb_sel_init_address = fIndex;
	
	TRACE("menuSetCbkInit page=%d, index=%x:", fPage, fIndex);
}

void commands2F46() {
	cb_sel_init_page = 0;
	TRACE("menuClearCbkInit:");
}

void commands2F47() {
	if (cb_sel_init_page != 0) {
		sl_jmpNear(sl_getIndex());
		sl_callFar2(cb_sel_init_page -1, cb_sel_init_address);
	}
	TRACE("new ]:");
}

void commands2F48() {
	const char *sText = sl_getString(0);
	
	TRACE_UNIMPLEMENTED("sysOpenShell %s:", sText);
}

void commands2F49() {
	const char *sTitle = sl_getString(0);
	const char *sUrl = sl_getString(0);
	
	TRACE_UNIMPLEMENTED("sysAddWebMenu %s, %s:", sTitle, sUrl);
}

void commands2F4A() {
	int eTime = getCaliValue();
	
	ags_setCursorMoveTime(eTime);
	TRACE("iptSetMoveCursorTime %d:", eTime);
}

void commands2F4B() {
	int *vTime = getCaliVariable();
	
	*vTime = ags_getCursorMoveTime();
	TRACE("iptGetMoveCursorTime %d:", *vTime);
}

void commands2F4C() {
	int eX      = getCaliValue();
	int eY      = getCaliValue();
	int eSrcX   = getCaliValue();
	int eSrcY   = getCaliValue();
	int eWidth  = getCaliValue();
	int eHeight = getCaliValue();

	MyRectangle r = {eSrcX, eSrcY, eWidth, eHeight};
	MyPoint p = {eX, eY};
	
	sdl_updateArea(&r, &p);
	
	TRACE("grBlt %d, %d, %d, %d, %d, %d:",
		      eX, eY, eSrcX, eSrcY, eWidth, eHeight);
}

void commands2F4D() {
	int eNum = getCaliValue();
	const char *sText = sl_getString(0);

	TRACE_UNIMPLEMENTED("LXWT %d, %s:", eNum, sText);
}

void commands2F4E() {
	int eNum = getCaliValue();
	int eTextNum = getCaliValue();

	TRACE_UNIMPLEMENTED("LXWS %d, %d:", eNum, eTextNum);
}

void commands2F4F() {
	int eNum = getCaliValue();
	int eType = getCaliValue();

	TRACE_UNIMPLEMENTED("LXWE %d, %d:", eNum, eType);
}

void commands2F50() {
	int eFile = getCaliValue();
	int nFlg = sl_getc();
	int eNum = getCaliValue();

	TRACE_UNIMPLEMENTED("LXWH %d, %d, %d:", eFile, nFlg, eNum);
}

void commands2F51() {
	int eFile = getCaliValue();
	int nFlg  = sl_getc();
	int eNum  = getCaliValue();

	TRACE_UNIMPLEMENTED("LXWHH %d, %d, %d:", eFile, nFlg, eNum);
}

void commands2F52() {
	int eNum = getCaliValue();
	if (eNum <= 0) return;

	const char *s = SDL_GetPlatform();
	svar_set(eNum, s);
	TRACE("sysGetOSName %d: %s", eNum, s);
}

void commands2F53() {
	int eFlag = getCaliValue();
	
	nact->patch_ec = eFlag;
	
	TRACE("patchEC %d:", eFlag);
}

void commands2F54() {
	int eX = getCaliValue();
	int eY = getCaliValue();
	int eWidth = getCaliValue();
	int eHeight = getCaliValue();

	clip_window.x = eX;
	clip_window.y = eY;
	clip_window.w = eWidth;
	clip_window.h = eHeight;
	
	TRACE("mathSetClipWindow %d, %d, %d, %d:", eX, eY, eWidth, eHeight);
}

void commands2F55() {
	int *vDx = getCaliVariable();
	int *vDy = getCaliVariable();
	int *vSx = getCaliVariable();
	int *vSy = getCaliVariable();
	int *vWidth  = getCaliVariable();
	int *vHeight = getCaliVariable();

	TRACE("mathClip %d, %d, %d, %d, %d, %d:", *vDx, *vDy, *vSx, *vSy, *vWidth, *vHeight);

	SDL_Rect r = { *vDx, *vDy, *vWidth, *vHeight };
	if (SDL_IntersectRect(&r, &clip_window, &r)) {
		*vDx = r.x;
		*vDy = r.y;
		*vWidth = r.w;
		*vHeight = r.h;
	} else {
		*vWidth = 0;
		*vHeight = 0;
	}
}

void commands2F56() {
	int file_name = getCaliValue();
	const char *title = sl_getString(0);
	const char *folder = sl_getString(0);

	TRACE_UNIMPLEMENTED("LXF %d, %s, %s:", file_name, title, folder);
}

void commands2F57() {
	const char *sTitle = sl_getString(0);
	int eStrVar = getCaliValue();
	int eLength = getCaliValue();
	int *vResult = getCaliVariable();

	char *t1, *t2, *t3;
	INPUTSTRING_PARAM p;
	
	t1 = toUTF8(sTitle);
	t2 = toUTF8(svar_get(eStrVar));
	p.title = t1;
	p.oldstring = t2;
	p.max = eLength;
	
	menu_inputstring(&p);
	if (p.newstring == NULL) {
		*vResult = 65535;
	} else {
		t3 = fromUTF8(p.newstring);
		svar_set(eStrVar, t3);
		*vResult = svar_length(eStrVar);
		free(t3);
	}
	free(t1);
	free(t2);
	TRACE("strInputDlg %s, %d, %d, %d:",
		      sTitle, eStrVar, eLength, *vResult);
}

void commands2F58() {
	int eNum = getCaliValue();
	int *vResult = getCaliVariable();
	
	*vResult = sjis_has_hankaku(svar_get(eNum)) ? 1 : 0;
	
	TRACE("strCheckASCII %d, %d:", eNum, *vResult);
}

void commands2F59() {
	int eNum = getCaliValue();
	int *vResult = getCaliVariable();
	
	*vResult = sjis_has_zenkaku(svar_get(eNum)) ? 1 : 0;
	
	TRACE("strCheckSJIS %d, %d:", eNum, *vResult);
}

void commands2F5A() {
	const char *sText = sl_getString(0);
	char *t1;
	
	t1 = toUTF8(sText);
	sdl_showMessageBox(MESSAGEBOX_INFO, nact->game_title_utf8, t1);
	free(t1);
	
	TRACE("strMessageBox %s:", sText);
}

void commands2F5B() {
	int eNum = getCaliValue();
	char *t1;
	
	t1 = toUTF8(svar_get(eNum));
	sdl_showMessageBox(MESSAGEBOX_INFO, nact->game_title_utf8, t1);
	free(t1);
	
	TRACE("strMessageBoxStr %d:", eNum);
}

void commands2F5C() {
	int eDestX = getCaliValue();
	int eDestY = getCaliValue();
	int eSrcX = getCaliValue();
	int eSrcY = getCaliValue();
	int eWidth = getCaliValue();
	int eHeight = getCaliValue();
	int eAlpha = getCaliValue();
	
	ags_copyArea_shadow_withrate(eSrcX, eSrcY, eWidth, eHeight, eDestX, eDestY, eAlpha);
	TRACE("grCopyUseAMapUseA %d, %d, %d, %d, %d, %d, %d:",
		      eDestX, eDestY, eSrcX, eSrcY, eWidth, eHeight, eAlpha);
}

void commands2F5D() {
	int eNum = getCaliValue();
	int eData = getCaliValue();
	
	TRACE_UNIMPLEMENTED("grSetCEParam %d, %d:", eNum, eData);
}

struct grEffectMoveView_data {
	int origin_x;
	int origin_y;
	int target_x;
	int target_y;
};

static void grEffectMoveView_step(void *data, float progress) {
	struct grEffectMoveView_data *d = data;
	int x = d->origin_x + (d->target_x - d->origin_x) * progress;
	int y = d->origin_y + (d->target_y - d->origin_y) * progress;
	ags_setViewArea(x, y, nact->ags.view_area.w, nact->ags.view_area.h);
	ags_updateFull();
}

void commands2F5E() {
	int eX = getCaliValue();
	int eY = getCaliValue();
	int eTime = getCaliValue();
	int eFlag = getCaliValue();

	struct grEffectMoveView_data data = {
		.origin_x = nact->ags.view_area.x,
		.origin_y = nact->ags.view_area.y,
		.target_x = eX,
		.target_y = eY
	};
	ags_runEffect(eTime, eFlag, grEffectMoveView_step, &data);
	TRACE("grEffectMoveView %d, %d, %d, %d:", eX, eY, eTime, eFlag);
}

void commands2F5F() {
	int eSize = getCaliValue();
	
	TRACE_UNIMPLEMENTED("cgSetCacheSize %d:", eSize);
}

void commands2F61() {
	int eChar = getCaliValue();
	int eCGNum = getCaliValue();
	
	TRACE_UNIMPLEMENTED("gaijiSet %d, %d:", eChar, eCGNum);
}

void commands2F62() {
	TRACE_UNIMPLEMENTED("gaijiClearAll:");
}

void commands2F63() {
	int *vSelect = getCaliVariable();
	
	*vSelect = sel_getLastElement();

	TRACE("menuGetLatestSelect %d:", *vSelect);
}

void commands2F64() {
	int eType = getCaliValue();
	int eNum  = getCaliValue();
	int *vResult = getCaliVariable();
	DRIFILETYPE t = DRIFILE_DATA;

	switch(eType) {
	case 0:
		t = DRIFILE_DATA;
		break;
	case 1:
		t = DRIFILE_CG;
		break;
	case 2:
		t = DRIFILE_MIDI;
		break;
	case 3:
		t = DRIFILE_SCO;
		break;
	case 4:
		t = DRIFILE_WAVE;
		break;
	case 5:
		t = DRIFILE_RSC;
		break;
	}

	*vResult = ald_is_linked(t, eNum - 1) ? 1 : 0;

	TRACE("lnkIsLink %d, %d, %d:", eType, eNum, *vResult);
}

void commands2F65() {
	int eType = getCaliValue();
	int eNum  = getCaliValue();
	int *vResult = getCaliVariable();
	
	DRIFILETYPE t = DRIFILE_DATA;

	switch(eType) {
	case 0:
		t = DRIFILE_DATA;
		break;
	case 1:
		t = DRIFILE_CG;
		break;
	case 2:
		t = DRIFILE_MIDI;
		break;
	case 3:
		t = DRIFILE_SCO;
		break;
	case 4:
		t = DRIFILE_WAVE;
		break;
	case 5:
		t = DRIFILE_RSC;
		break;
	}

	*vResult = ald_exists(t, eNum - 1) ? 1 : 0;

	TRACE("lnkIsData %d, %d, %d:", eType, eNum, *vResult);
}

void commands2F66() {
	int eNum = getCaliValue();
	int page  = sl_getw();
	int index = sl_getdw();
	
	TRACE("fncSetTable %d, %d,%d:", eNum, page, index);

	if (eNum >= FCTBL_MAX) {
		WARNING("fncSetTable: too many table number %d", eNum);
		return;
	}
	
	fnctbl[eNum].page  = page;
	fnctbl[eNum].index = index;
}

void commands2F67() {
	int eNum = getCaliValue();
	int eStrNum = getCaliValue();
	int *vResult = getCaliVariable();
	int i;

	for (i = 0; i < nact->ain.fncnum; i++) {
		if (0 == strcmp(nact->ain.fnc[i].name, svar_get(eStrNum))) {
			fnctbl[eNum].page  = nact->ain.fnc[i].page;
			fnctbl[eNum].index = nact->ain.fnc[i].index;
			break;
		}
	}
	
	*vResult = (i == nact->ain.fncnum) ? 0 : !0;
	
	TRACE("fncSetTableFromStr %d, %d, %d:", eNum, eStrNum, *vResult);
}

void commands2F68() {
	int eNum = getCaliValue();
	
	TRACE("fncClearTable %d:", eNum);
	
	if (eNum >= FCTBL_MAX) {
		WARNING("fncClearTable: too many table number %d", eNum);
		return;
	}
	
	fnctbl[eNum].page  = 0;
	fnctbl[eNum].index = 0;
}

void commands2F69() {
	int eNum = getCaliValue();
	
	TRACE("fncCall %d:", eNum);
	
	if (eNum >= FCTBL_MAX) {
		WARNING("fncClearTable: too many table number %d", eNum);
		return;
	}
	
	if (fnctbl[eNum].page == 0 && fnctbl[eNum].index == 0) {
		return;
	}
	
	sl_callFar2(fnctbl[eNum].page -1, fnctbl[eNum].index);
}

void commands2F6A() {
	int eNum = getCaliValue();

	nact->fnc_return_value = eNum;
	
	TRACE("fncSetReturnCode %d:", eNum);
}

void commands2F6B() {
	int *vNum = getCaliVariable();
	
	*vNum = nact->fnc_return_value;
	
	TRACE("fncGetReturnCode %d:", *vNum);
}

void commands2F6C() {
	int eFlag = getCaliValue();
	
	nact->is_msg_out = eFlag != 0;
	
	TRACE("msgSetOutputFlag %d:", eFlag);
}

void commands2F6D() {
	int eNum = getCaliValue();
	int *vResult = getCaliVariable();

	*vResult = save_delete_file(eNum -1);
	
	TRACE("saveDeleteFile %d, %d:", eNum, *vResult);
}

void commands2F6E() {
	int eFlag = getCaliValue();
	
	TRACE_UNIMPLEMENTED("wav3DSetUseFlag %d:", eFlag);
}

void commands2F6F() {
	int eCh       = getCaliValue();
	int eTime     = getCaliValue();
	int eVolume   = getCaliValue();
	int eStopFlag = getCaliValue();
	
	mus_wav_fadeout_start(eCh, eTime, eVolume, eStopFlag);
	
	TRACE("wavFadeVolume %d, %d, %d, %d:", eCh, eTime, eVolume, eStopFlag);
}

void commands2F70() {
	int eFlag = getCaliValue();
	
	nact->patch_emen = eFlag;
	
	TRACE("patchEMEN %d:", eFlag);
}

void commands2F71() {
	int eFlag = getCaliValue();

	msgskip_enableMenu(eFlag);

	TRACE("wmenuEnableMsgSkip %d:", eFlag);
}

void commands2F72() {
	int *vFlag = getCaliVariable();
	
	TRACE_UNIMPLEMENTED("winGetFlipFlag %p:", vFlag);
}

void commands2F73() {
	int *vTrack = getCaliVariable();
	
	*vTrack = muscd_get_maxtrack();
	
	TRACE("cdGetMaxTrack %d:", *vTrack);
}

void commands2F74() {
	const char *str1 = sl_getString(0);
	int *var   = getCaliVariable();
	
	TRACE_UNIMPLEMENTED("dlgErrorOkCancel %s,%d:", str1, *var);
}

void commands2F75() {
	int eNumof = getCaliValue();
	
	sel_reduce(eNumof);
	
	TRACE("menuReduce %d:", eNumof);
}

void commands2F76() {
	int *var   = getCaliVariable();

	*var = sel_getnumberof();
	
	//TRACE("menuGetNumof %p:", var);
	TRACE("menuGetNumof %d:", *var);
}

void commands2F77() {
	int eStrNum    = getCaliValue();
	int eSelectNum = getCaliValue();
	
	svar_set(eStrNum, sel_gettext(eSelectNum));
	
	TRACE("menuGetText %d,%d:", eStrNum, eSelectNum);
}

void commands2F78() {
	int eSelectNum = getCaliValue();
	int eClearFlag = getCaliValue();

	sel_goto(eSelectNum, eClearFlag);
	
	TRACE("menuGoto %d,%d:", eSelectNum, eClearFlag);
}

void commands2F79() {
	int eSelectNum = getCaliValue();
	int eClearFlag = getCaliValue();
	
	sel_returengoto(eSelectNum, eClearFlag);
	
	TRACE("menuReturnGoto %d,%d:", eSelectNum, eClearFlag);
}

void commands2F7A() {
	// no-op
	TRACE("menuFreeShelterDIB:");
}

void commands2F7B() {
	// no-op
	TRACE("msgFreeShelterDIB:");
}

void commands2F7C() {
	if (!nact->files.ain) {
		if (nact->files.gr_fname) {
			sys_error("System39.ain is needed to run this game. Make sure you have an \"Ain\" line in %s.", nact->files.gr_fname);
		} else {
			sys_error("Ain file is needed to run this game. Make sure you have System39.ain file in the game directory.");
		}
	}

	int index = sl_getdw();
	if (index >= nact->ain.msgnum) {
		WARNING("message id out of bounds (%d)", index);
		return;
	}
	sys_addMsg(nact->ain.msg[index]);
	msgskip_onAinMessage(index);

	TRACE("2F7C %d:", index);
}

void commands2F7D() {
	int index = sl_getdw();
	
	commandH();
	msgskip_onAinMessage(index);

	TRACE("2F7D %d:", index);
}

void commands2F7E() {
	int index = sl_getdw();
	
	commandHH();
	msgskip_onAinMessage(index);

	TRACE("2F7E %d:", index);
}

void commands2F7F() {
	int index = sl_getdw();
	int p1    = getCaliValue();
	
	sys_addMsg(svar_get(p1));
	msgskip_onAinMessage(index);

	TRACE("2F7F %d, %d(%s):", index, p1, svar_get(p1));
}

void commands2F80() {
	int page  = sl_getw();
	int index = sl_getdw();
	
	if (NULL == (nact->datatbl_addr = sl_setDataTable(page -1, index))) {
		WARNING("dataSetPointer set failed");
	}
	
	TRACE("dataSetPointer %d,%x", page, index);
}

void commands2F81() {
	int *vData = getCaliVariable();
	int eNumof = getCaliValue();
	int i;
	uint16_t *p = (uint16_t *)nact->datatbl_addr;

	for (i = 0; i < eNumof; i++) {
		*vData = LittleEndian_getW((uint8_t *)p, 0);
		p++;
		vData++;
	}
	
	nact->datatbl_addr = p;
	
	TRACE("dataGetWORD %p,%d(%p):", vData, eNumof, nact->datatbl_addr);
}

void commands2F82() {
	int eStrNum = getCaliValue();
	int eNumof  = getCaliValue();
	int i;
	char *p = (char *)nact->datatbl_addr;
	
	TRACE("dataGetString %d,%d:", eStrNum, eNumof);
	
	for (i = 0; i < eNumof; i++) {
		svar_set(eStrNum + i, p);
		p += (strlen(p) + 1);
	}

	nact->datatbl_addr = p;
}

void commands2F83() {
	int eNumof  = getCaliValue();
	uint16_t *p = (uint16_t *)nact->datatbl_addr;

	p += eNumof;

	nact->datatbl_addr = p;
	
	TRACE("dataSkipWORD %d:", eNumof);
}

void commands2F84() {
	int eNumof  = getCaliValue();
	int i;
	char *p = (char *)nact->datatbl_addr;
	
	for (i = 0; i < eNumof; i++) {
		p += (strlen(p) + 1);
	}
	
	nact->datatbl_addr = p;
	
	TRACE("dataSkipString %d:", eNumof);
}

void commands2F85() {
	int *vResult = getCaliVariable();
	
	*vResult = nact->ain.varnum;
	
	TRACE("varGetNumof %p:", vResult);
}

void commands2F86() {
	int eFlag = getCaliValue();
	
	nact->patch_g0 = eFlag;
	
	TRACE("patchG0 %d:", eFlag);
}

void commands2F87() {
	int eSubKeyStrNum = getCaliValue();
	int eBaneStrNum   = getCaliValue();
	int eResultStrNum = getCaliValue();
	int *vResult      = getCaliVariable();

	// This command is used to check if the game is installed, so we always
	// return 1 with the dummy string REGREADSTRING_RESULT.
	svar_set(eResultStrNum, REGREADSTRING_RESULT);
	*vResult = 1;
	
	TRACE("regReadString %d,%d,%d,%d:",
		      eSubKeyStrNum, eBaneStrNum, eResultStrNum, *vResult);
}

void commands2F88() {
	int eFileNameStrNum = getCaliValue();
	int *vResult        = getCaliVariable();

	const char *fname = svar_get(eFileNameStrNum);
	if (strcmp(fname, REGREADSTRING_RESULT) == 0) {
		// If this command is called with the result of regReadString, the game
		// is checking if the game CD is inserted. We return 1 in this case.
		*vResult = 1;
	} else {
		char *fname_utf8 = sjis2utf(fname);
		*vResult = fc_exists(fname_utf8) ? 1 : 0;
		free(fname_utf8);
	}
	
	TRACE("fileCheckExist %d,%d:", eFileNameStrNum, *vResult);
}

void commands2F89() {
	int eYear = getCaliValue();
	int eMonth = getCaliValue();
	int eData = getCaliValue();
	int *vResult = getCaliVariable();
	
	TRACE_UNIMPLEMENTED("timeCheckCurDate %d,%d,%d,%p:", eYear, eMonth, eData, vResult);
}

void commands2F8A() {
	const char *s2 = sl_getConstString();
	const char *s3 = sl_getConstString();

	TRACE_UNIMPLEMENTED("dlgManualProtect %s,%s:", s2, s3);
}

void commands2F8B() {
	const char *s2 = sl_getConstString();
	int c3 = getCaliValue();
	int c4 = getCaliValue();
	const char *s6 = sl_getConstString();
	int *c7 = getCaliVariable();

	*c7 = 1;
	
	TRACE_UNIMPLEMENTED("fileCheckDVD %s,%d,%d,%s,%p:", s2, c3, c4, s6, c7);
}

void commands2F8C() {
	TRACE("sysReset:");
	nact_quit(TRUE);
}

void cmd2F_reset(void) {
	cb_sel_init_page = 0;
	cb_sel_init_address = 0;
	memset(fnctbl, 0, sizeof(fnctbl));
}
