/*
 * xcore_input.c  キー入力処理
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
/* $Id: xcore_input.c,v 1.13 2003/04/25 17:23:55 chikama Exp $ */

#include "key.h"
#include <X11/keysym.h>
#include "ags.h"


boolean RawKeyInfo[256];
static int  convertKeyCode(unsigned int keycode);
static void keyEventProsess(XKeyEvent *e);
static void buttonEventProcess(XButtonEvent *e);


static void keyEventProsess(XKeyEvent *e) {	
	agsevent_t agse;
	boolean bool;
	int code = convertKeyCode(e->keycode);
	
	if (e->type == KeyPress) {
		bool = TRUE;
		agse.type = AGSEVENT_KEY_PRESS;
	} else {
		bool = FALSE;
		agse.type = AGSEVENT_KEY_RELEASE;
	}
	
	RawKeyInfo[code] = bool;

	if (nact->ags.eventcb) {
		agse.d1 = e->x;
		agse.d2 = e->y;
		agse.d3 = code;
		nact->ags.eventcb(&agse);
	}
}

static void buttonEventProcess(XButtonEvent *e) {
	agsevent_t agse;
	boolean bool;
	
	if (e->type == ButtonPress) {
		bool = TRUE;
		agse.type = AGSEVENT_BUTTON_PRESS;
	} else {
		bool = FALSE;
		agse.type = AGSEVENT_BUTTON_RELEASE;
	}
	
	switch(e->button) {
	case Button1:
		RawKeyInfo[KEY_MOUSE_LEFT]   = bool;
		agse.d3 = AGSEVENT_BUTTON_LEFT;
		break;
	case Button2:
		RawKeyInfo[KEY_MOUSE_MIDDLE] = bool;
		agse.d3 = AGSEVENT_BUTTON_MID;
		break;
	case Button3:
		RawKeyInfo[KEY_MOUSE_RIGHT]  = bool;
		agse.d3 = AGSEVENT_BUTTON_RIGHT;
		break;
	case Button4:
		agse.d3 = AGSEVENT_WHEEL_UP;
		break;
	case Button5:
		agse.d3 = AGSEVENT_WHEEL_DN;
		break;
	default:
		break;
	}
	
	if (nact->ags.eventcb) {
		agse.d1 = e->x;
		agse.d2 = e->y;
		nact->ags.eventcb(&agse);
	}
}

static int convertKeyCode(unsigned int keycode) {
	KeySym sym = XkbKeycodeToKeysym(x11_display, keycode, 0, 0);
	switch(sym) {
	case XK_BackSpace:
		return KEY_BS;
	case XK_Tab:
		return KEY_TAB;
	case XK_Clear:
		return KEY_CLEAR;
	case XK_Return:
	case XK_KP_Enter:
		return KEY_ENTER;
	case XK_Shift_L:
	case XK_Shift_R:
		return KEY_SHIFT;
	case XK_Control_L:
	case XK_Control_R:
		return KEY_CTRL;
	case XK_Alt_L:
	case XK_Alt_R:
		return KEY_ALT;
	case XK_Pause:
		return KEY_PAUSE;
	case XK_Caps_Lock:
		return KEY_CAPSLOCK;
	case XK_Kanji:
		return KEY_KANAKAN;
	case XK_Escape :
		return KEY_ESC;
	case XK_space:
		return KEY_SPACE;
	case XK_Page_Up:
		return KEY_PAGEUP;
	case XK_Page_Down:
		return KEY_PAGEDOWN;
	case XK_End:
		return KEY_END;
	case XK_Home:
		return KEY_HOME;
	case XK_Left:
		return KEY_LEFT;
	case XK_Up:
		return KEY_UP;
	case XK_Right:
		return KEY_RIGHT;
	case XK_Down:
		return KEY_DOWN;
	case XK_Select:
		return KEY_SELECT;
	case XK_Execute:
		return KEY_EXECUTE;
	case XK_Print:
		return KEY_PRINTSCREEN;
	case XK_Insert:
		return KEY_INS;
	case XK_Delete:
		return KEY_DEL;
	case XK_Help:
		return KEY_HELP;
	case XK_0:
		return KEY_0;
	case XK_1:
		return KEY_1;
	case XK_2:
		return KEY_2;
	case XK_3:
		return KEY_3;
	case XK_4:
		return KEY_4;
	case XK_5:
		return KEY_5;
	case XK_6:
		return KEY_6;
	case XK_7:
		return KEY_7;
	case XK_8:
		return KEY_8;
	case XK_9:
		return KEY_9;
	case XK_a:
	case XK_A:
		return KEY_A;
	case XK_b:
	case XK_B:
		return KEY_B;
	case XK_c:
	case XK_C:
		return KEY_C;
	case XK_d:
	case XK_D:
		return KEY_D;
	case XK_e:
	case XK_E:
		return KEY_E;
	case XK_f:
	case XK_F:
		return KEY_F;
	case XK_g:
	case XK_G:
		return KEY_G;
	case XK_h:
	case XK_H:
		return KEY_H;
	case XK_i:
	case XK_I:
		return KEY_I;
	case XK_j:
	case XK_J:
		return KEY_J;
	case XK_k:
	case XK_K:
		return KEY_K;
	case XK_l:
	case XK_L:
		return KEY_L;
	case XK_m:
	case XK_M:
		return KEY_M;
	case XK_n:
	case XK_N:
		return KEY_N;
	case XK_o:
	case XK_O:
		return KEY_O;
	case XK_p:
	case XK_P:
		return KEY_P;
	case XK_q:
	case XK_Q:
		return KEY_Q;
	case XK_r:
	case XK_R:
		return KEY_R;
	case XK_s:
	case XK_S:
		return KEY_S;
	case XK_t:
	case XK_T:
		return KEY_T;
	case XK_u:
	case XK_U:
		return KEY_U;
	case XK_v:
	case XK_V:
		return KEY_V;
	case XK_w:
	case XK_W:
		return KEY_W;
	case XK_x:
	case XK_X:
		return KEY_X;
	case XK_y:
	case XK_Y:
		return KEY_Y;
	case XK_z:
	case XK_Z:
		return KEY_Z;
	case XK_KP_Insert:
	case XK_KP_0:
		return KEY_PAD_0;
	case XK_KP_End:
	case XK_KP_1:
		return KEY_PAD_1;
	case XK_KP_Down:
	case XK_KP_2:
		return KEY_PAD_2;
	case XK_KP_Page_Down:
	case XK_KP_3:
		return KEY_PAD_3;
	case XK_KP_Left:
	case XK_KP_4:
		return KEY_PAD_4;
	case XK_KP_5:
		return KEY_PAD_5;
	case XK_KP_Right:
	case XK_KP_6:
		return KEY_PAD_6;
	case XK_KP_Home:
	case XK_KP_7:
		return KEY_PAD_7;
	case XK_KP_Up:
	case XK_KP_8:
		return KEY_PAD_8;
	case XK_KP_Page_Up:
	case XK_KP_9:
		return KEY_PAD_9;
	case XK_KP_Multiply:
		return KEY_PAD_STAR;
	case XK_KP_Add:
		return KEY_PAD_PLUS;
	case XK_KP_Separator:
		return KEY_PAD_SEP;
	case XK_KP_Subtract:
		return KEY_PAD_MINUS;
	case XK_KP_Delete:
		return KEY_PAD_DOT;
	case XK_KP_Divide:
		return KEY_PAD_SLASH;
	case XK_F1:
		return KEY_F1;
	case XK_F2:
		return KEY_F2;
	case XK_F3:
		return KEY_F3;
	case XK_F4:
		return KEY_F4;
	case XK_F5:
		return KEY_F5;
	case XK_F6:
		return KEY_F6;
	case XK_F7:
		return KEY_F7;
	case XK_F8:
		return KEY_F8;
	case XK_F9:
		return KEY_F9;
	case XK_F10:
		return KEY_F10;
	case XK_F11:
		return KEY_F11;
	case XK_F12:
		return KEY_F12;
	case XK_F13:
		return KEY_F13;
	case XK_F14:
		return KEY_F14;
	case XK_F15:
		return KEY_F15;
	case XK_F16:
		return KEY_F16;
	case XK_F17:
		return KEY_F17;
	case XK_F18:
		return KEY_F18;
	case XK_F19:
		return KEY_F19;
	case XK_F20:
		return KEY_F20;
	case XK_F21:
		return KEY_F21;
	case XK_F22:
		return KEY_F22;
	case XK_F23:
		return KEY_F23;
	case XK_F24:
		return KEY_F24;
	case XK_Num_Lock:
		return KEY_NUMLOCK;
	case XK_Scroll_Lock:
		return KEY_SCROLL_LOCK;
	default:
		return KEY_UNDEFINED;
	}
	return KEY_UNDEFINED;
}
