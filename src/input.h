/*
 * input.c キーボードマウス関連
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
/* $Id: input.h,v 1.15 2001/03/30 19:16:38 chikama Exp $ */

#ifndef __INPUT__
#define __INPUT__

#include "portab.h"
#include "graphics.h"

#define SYS35KEY_NULL  0
#define SYS35KEY_UP    1
#define SYS35KEY_DOWN  2
#define SYS35KEY_LEFT  4
#define SYS35KEY_RIGHT 8
#define SYS35KEY_RET  16
#define SYS35KEY_SPC  32
#define SYS35KEY_ESC  64
#define SYS35KEY_TAB 128

#define KEYWAIT_NONCANCELABLE 0
#define KEYWAIT_CANCELABLE    1
#define KEYWAIT_SKIPPABLE     2

// System3.x key codes (IG command, etc.)
// These are originated from Windows virtual key codes (VK_*).
#define NUM_KEYCODES 256
enum keycode {
	KEY_MOUSE_LEFT   = 0x01,  // マウスの左ボタン
	KEY_MOUSE_RIGHT  = 0x02,  // マウスの右ボタン
	KEY_CONTROL      = 0x03,  // コントロール ブレーク処理に使用
	KEY_MOUSE_MIDDLE = 0x04,  // マウスの中央ボタン (3つボタンのマウス)
	KEY_BACKSPACE    = 0x08,  // BackSpaceキー
	KEY_TAB          = 0x09,  // Tabキー
	KEY_CLEAR        = 0x0c,  // Clearキー
	KEY_RETURN       = 0x0d,  // Enterキー
	KEY_SHIFT        = 0x10,  // Shiftキー
	KEY_CTRL         = 0x11,  // Ctrlキー
	KEY_ALT          = 0x12,  // Altキー
	KEY_PAUSE        = 0x13,  // Pauseキー
	KEY_CAPSLOCK     = 0x14,  // Caps Lockキー
	KEY_KANAKAN      = 0x15,  // 英数カナキー
	KEY_ESCAPE       = 0x1b,  // Escキー
	KEY_SPACE        = 0x20,  // Spaceキー
	KEY_PAGEUP       = 0x21,  // Page Upキー
	KEY_PAGEDOWN     = 0x22,  // Page Downキー
	KEY_END          = 0x23,  // Endキー
	KEY_HOME         = 0x24,  // Homeキー
	KEY_LEFT         = 0x25,  // ←キー
	KEY_UP           = 0x26,  // ↑キー
	KEY_RIGHT        = 0x27,  // →キー
	KEY_DOWN         = 0x28,  // ↓キー
	KEY_SELECT       = 0x29,  // Selectキー
	KEY_EXECUTE      = 0x2b,  // Executeキー
	KEY_PRINTSCREEN  = 0x2c,  // Print Screenキー (Windows 3.0以降用)
	KEY_INSERT       = 0x2d,  // Insキー
	KEY_DELETE       = 0x2e,  // Delキー
	KEY_HELP         = 0x2f,  // Helpキー
	KEY_0            = 0x30,  // 0キー
	KEY_1            = 0x31,  // 1キー
	KEY_2            = 0x32,  // 2キー
	KEY_3            = 0x33,  // 3キー
	KEY_4            = 0x34,  // 4キー
	KEY_5            = 0x35,  // 5キー
	KEY_6            = 0x36,  // 6キー
	KEY_7            = 0x37,  // 7キー
	KEY_8            = 0x38,  // 8キー
	KEY_9            = 0x39,  // 9キー
	KEY_PAD_EQUALS   = 0x3d,
	KEY_A            = 0x41,  // Aキー
	KEY_B            = 0x42,  // Bキー
	KEY_C            = 0x43,  // Cキー
	KEY_D            = 0x44,  // Dキー
	KEY_E            = 0x45,  // Eキー
	KEY_F            = 0x46,  // Fキー
	KEY_G            = 0x47,  // Gキー
	KEY_H            = 0x48,  // Hキー
	KEY_I            = 0x49,  // Iキー
	KEY_J            = 0x4a,  // Jキー
	KEY_K            = 0x4b,  // Kキー
	KEY_L            = 0x4c,  // Lキー
	KEY_M            = 0x4d,  // Mキー
	KEY_N            = 0x4e,  // Nキー
	KEY_O            = 0x4f,  // Oキー
	KEY_P            = 0x50,  // Pキー
	KEY_Q            = 0x51,  // Qキー
	KEY_R            = 0x52,  // Rキー
	KEY_S            = 0x53,  // Sキー
	KEY_T            = 0x54,  // Tキー
	KEY_U            = 0x55,  // Uキー
	KEY_V            = 0x56,  // Vキー
	KEY_W            = 0x57,  // Wキー
	KEY_X            = 0x58,  // Xキー
	KEY_Y            = 0x59,  // Yキー
	KEY_Z            = 0x5a,  // Zキー
	KEY_PAD_0        = 0x60,  // テンキーの0キー
	KEY_PAD_1        = 0x61,  // テンキーの1キー
	KEY_PAD_2        = 0x62,  // テンキーの2キー
	KEY_PAD_3        = 0x63,  // テンキーの3キー
	KEY_PAD_4        = 0x64,  // テンキーの4キー
	KEY_PAD_5        = 0x65,  // テンキーの5キー
	KEY_PAD_6        = 0x66,  // テンキーの6キー
	KEY_PAD_7        = 0x67,  // テンキーの7キー
	KEY_PAD_8        = 0x68,  // テンキーの8キー
	KEY_PAD_9        = 0x69,  // テンキーの9キー
	KEY_PAD_MULTIPLY = 0x6a,  // テンキーの*キー
	KEY_PAD_PLUS     = 0x6b,  // テンキーの+キー
	KEY_SEPARATOR    = 0x6c,  // Separatorキー
	KEY_PAD_MINUS    = 0x6d,  // テンキーの－キー
	KEY_PAD_PERIOD   = 0x6e,  // テンキーの.キー
	KEY_PAD_DIVIDE   = 0x6f,  // テンキーの/キー
	KEY_F1           = 0x70,  // F1キー
	KEY_F2           = 0x71,  // F2キー
	KEY_F3           = 0x72,  // F3キー
	KEY_F4           = 0x73,  // F4キー
	KEY_F5           = 0x74,  // F5キー
	KEY_F6           = 0x75,  // F6キー
	KEY_F7           = 0x76,  // F7キー
	KEY_F8           = 0x77,  // F8キー
	KEY_F9           = 0x78,  // F9キー
	KEY_F10          = 0x79,  // F10キー
	KEY_F11          = 0x7a,  // F11キー
	KEY_F12          = 0x7b,  // F12キー
	KEY_F13          = 0x7c,  // F13キー
	KEY_F14          = 0x7d,  // F14キー
	KEY_F15          = 0x7e,  // F15キー
	KEY_F16          = 0x7f,  // F16キー
	KEY_F17          = 0x80,  // F17キー
	KEY_F18          = 0x81,  // F18キー
	KEY_F19          = 0x82,  // F19キー
	KEY_F20          = 0x83,  // F20キー
	KEY_F21          = 0x84,  // F21キー
	KEY_F22          = 0x85,  // F22キー
	KEY_F23          = 0x86,  // F23キー
	KEY_F24          = 0x87,  // F24キー
	KEY_NUMLOCK      = 0x90,  // Num Lockキー
	KEY_SCROLLLOCK   = 0x91,  // Scroll Lockキー
};

extern boolean RawKeyInfo[NUM_KEYCODES];

extern int sys_keywait(int msec, unsigned flags);
extern int sys_getMouseInfo(MyPoint *p, boolean is_dibgeo);
extern int sys_getInputInfo(void);
extern int sys_getKeyInfo(void);
extern int sys_getJoyInfo(void);
extern void sys_getWheelInfo(int *forward, int *back);
extern void sys_clearWheelInfo(void);
extern void sys_key_releasewait(int key, boolean zi_mask_enabled);
extern void sys_hit_any_key();
extern void set_hak_keymode(int key, int mode);

#endif /* __INPUT__ */
