/*
 * sdl_darw.c  SDL event handler
 *
 * Copyright (C) 2000-     Fumihiko Murata       <fmurata@p1.tcnet.ne.jp>
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
/* $Id: sdl_event.c,v 1.5 2001/12/16 17:12:56 chikama Exp $ */

#include "config.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <SDL.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "portab.h"
#include "system.h"
#include "debugger.h"
#include "nact.h"
#include "sdl_core.h"
#include "gfx_private.h"
#include "scheduler.h"
#include "menu.h"
#include "input.h"
#include "msgskip.h"
#include "hacks.h"

static void sdl_getEvent(void);
static void keyEventProsess(SDL_KeyboardEvent *e, bool pressed);

static uint32_t custom_event_type = (uint32_t)-1;

enum CustomEventCode {
	SIMULATE_RIGHT_BUTTON,
	DEBUGGER_COMMAND,
};

/* pointer の状態 */
static int mousex, mousey, mouseb;
static int mouse_wheel_up, mouse_wheel_down;
bool RawKeyInfo[256];

/* SDL Joystick */
static int joyinfo=0;

static int sdl_keytable[SDL_NUM_SCANCODES] = {
	[SDL_SCANCODE_A] = KEY_A,
	[SDL_SCANCODE_B] = KEY_B,
	[SDL_SCANCODE_C] = KEY_C,
	[SDL_SCANCODE_D] = KEY_D,
	[SDL_SCANCODE_E] = KEY_E,
	[SDL_SCANCODE_F] = KEY_F,
	[SDL_SCANCODE_G] = KEY_G,
	[SDL_SCANCODE_H] = KEY_H,
	[SDL_SCANCODE_I] = KEY_I,
	[SDL_SCANCODE_J] = KEY_J,
	[SDL_SCANCODE_K] = KEY_K,
	[SDL_SCANCODE_L] = KEY_L,
	[SDL_SCANCODE_M] = KEY_M,
	[SDL_SCANCODE_N] = KEY_N,
	[SDL_SCANCODE_O] = KEY_O,
	[SDL_SCANCODE_P] = KEY_P,
	[SDL_SCANCODE_Q] = KEY_Q,
	[SDL_SCANCODE_R] = KEY_R,
	[SDL_SCANCODE_S] = KEY_S,
	[SDL_SCANCODE_T] = KEY_T,
	[SDL_SCANCODE_U] = KEY_U,
	[SDL_SCANCODE_V] = KEY_V,
	[SDL_SCANCODE_W] = KEY_W,
	[SDL_SCANCODE_X] = KEY_X,
	[SDL_SCANCODE_Y] = KEY_Y,
	[SDL_SCANCODE_Z] = KEY_Z,
	[SDL_SCANCODE_1] = KEY_1,
	[SDL_SCANCODE_2] = KEY_2,
	[SDL_SCANCODE_3] = KEY_3,
	[SDL_SCANCODE_4] = KEY_4,
	[SDL_SCANCODE_5] = KEY_5,
	[SDL_SCANCODE_6] = KEY_6,
	[SDL_SCANCODE_7] = KEY_7,
	[SDL_SCANCODE_8] = KEY_8,
	[SDL_SCANCODE_9] = KEY_9,
	[SDL_SCANCODE_0] = KEY_0,
	[SDL_SCANCODE_RETURN] = KEY_RETURN,
	[SDL_SCANCODE_ESCAPE] = KEY_ESCAPE,
	[SDL_SCANCODE_BACKSPACE] = KEY_BACKSPACE,
	[SDL_SCANCODE_TAB] = KEY_TAB,
	[SDL_SCANCODE_SPACE] = KEY_SPACE,
	// SDL_SCANCODE_MINUS
	// SDL_SCANCODE_EQUALS
	// SDL_SCANCODE_LEFTBRACKET
	// SDL_SCANCODE_RIGHTBRACKET
	// SDL_SCANCODE_BACKSLASH
	// SDL_SCANCODE_NONUSHASH
	// SDL_SCANCODE_SEMICOLON
	// SDL_SCANCODE_APOSTROPHE
	// SDL_SCANCODE_GRAVE
	// SDL_SCANCODE_COMMA
	// SDL_SCANCODE_PERIOD
	// SDL_SCANCODE_SLASH
	[SDL_SCANCODE_CAPSLOCK] = KEY_CAPSLOCK,
	[SDL_SCANCODE_F1] = KEY_F1,
	[SDL_SCANCODE_F2] = KEY_F2,
	[SDL_SCANCODE_F3] = KEY_F3,
	[SDL_SCANCODE_F4] = KEY_F4,
	[SDL_SCANCODE_F5] = KEY_F5,
	[SDL_SCANCODE_F6] = KEY_F6,
	[SDL_SCANCODE_F7] = KEY_F7,
	[SDL_SCANCODE_F8] = KEY_F8,
	[SDL_SCANCODE_F9] = KEY_F9,
	[SDL_SCANCODE_F10] = KEY_F10,
	[SDL_SCANCODE_F11] = KEY_F11,
	[SDL_SCANCODE_F12] = KEY_F12,
	[SDL_SCANCODE_PRINTSCREEN] = KEY_PRINTSCREEN,
	[SDL_SCANCODE_SCROLLLOCK] = KEY_SCROLLLOCK,
	[SDL_SCANCODE_PAUSE] = KEY_PAUSE,
	[SDL_SCANCODE_INSERT] = KEY_INSERT,
	[SDL_SCANCODE_HOME] = KEY_HOME,
	[SDL_SCANCODE_PAGEUP] = KEY_PAGEUP,
	[SDL_SCANCODE_DELETE] = KEY_DELETE,
	[SDL_SCANCODE_END] = KEY_END,
	[SDL_SCANCODE_PAGEDOWN] = KEY_PAGEDOWN,
	[SDL_SCANCODE_RIGHT] = KEY_RIGHT,
	[SDL_SCANCODE_LEFT] = KEY_LEFT,
	[SDL_SCANCODE_DOWN] = KEY_DOWN,
	[SDL_SCANCODE_UP] = KEY_UP,
	[SDL_SCANCODE_KP_DIVIDE] = KEY_PAD_DIVIDE,
	[SDL_SCANCODE_KP_MULTIPLY] = KEY_PAD_MULTIPLY,
	[SDL_SCANCODE_KP_MINUS] = KEY_PAD_MINUS,
	[SDL_SCANCODE_KP_PLUS] = KEY_PAD_PLUS,
	[SDL_SCANCODE_KP_ENTER] = KEY_RETURN,
	[SDL_SCANCODE_KP_1] = KEY_PAD_1,
	[SDL_SCANCODE_KP_2] = KEY_PAD_2,
	[SDL_SCANCODE_KP_3] = KEY_PAD_3,
	[SDL_SCANCODE_KP_4] = KEY_PAD_4,
	[SDL_SCANCODE_KP_5] = KEY_PAD_5,
	[SDL_SCANCODE_KP_6] = KEY_PAD_6,
	[SDL_SCANCODE_KP_7] = KEY_PAD_7,
	[SDL_SCANCODE_KP_8] = KEY_PAD_8,
	[SDL_SCANCODE_KP_9] = KEY_PAD_9,
	[SDL_SCANCODE_KP_0] = KEY_PAD_0,
	[SDL_SCANCODE_KP_PERIOD] = KEY_PAD_PERIOD,
	[SDL_SCANCODE_KP_EQUALS] = KEY_PAD_EQUALS,
	[SDL_SCANCODE_F13] = KEY_F13,
	[SDL_SCANCODE_F14] = KEY_F14,
	[SDL_SCANCODE_F15] = KEY_F15,
	[SDL_SCANCODE_F16] = KEY_F16,
	[SDL_SCANCODE_F17] = KEY_F17,
	[SDL_SCANCODE_F18] = KEY_F18,
	[SDL_SCANCODE_F19] = KEY_F19,
	[SDL_SCANCODE_F20] = KEY_F20,
	[SDL_SCANCODE_F21] = KEY_F21,
	[SDL_SCANCODE_F22] = KEY_F22,
	[SDL_SCANCODE_F23] = KEY_F23,
	[SDL_SCANCODE_F24] = KEY_F24,
	[SDL_SCANCODE_EXECUTE] = KEY_EXECUTE,
	[SDL_SCANCODE_HELP] = KEY_HELP,
	[SDL_SCANCODE_SELECT] = KEY_SELECT,
	[SDL_SCANCODE_CLEAR] = KEY_CLEAR,
	[SDL_SCANCODE_SEPARATOR] = KEY_SEPARATOR,
	[SDL_SCANCODE_LCTRL] = KEY_CTRL,
	[SDL_SCANCODE_LSHIFT] = KEY_SHIFT,
	[SDL_SCANCODE_LALT] = KEY_ALT,
	[SDL_SCANCODE_RCTRL] = KEY_CTRL,
	[SDL_SCANCODE_RSHIFT] = KEY_SHIFT,
	[SDL_SCANCODE_RALT] = KEY_ALT,
};

void sdl_event_init(void) {
	if (custom_event_type == (uint32_t)-1)
		custom_event_type = SDL_RegisterEvents(1);
}

static int mouse_to_rawkey(int button) {
	switch(button) {
	case SDL_BUTTON_LEFT:
		return KEY_MOUSE_LEFT;
	case SDL_BUTTON_MIDDLE:
		return KEY_MOUSE_MIDDLE;
	case SDL_BUTTON_RIGHT:
		return KEY_MOUSE_RIGHT;
	}
	return 0;
}

static int mouse_to_agsevent(int button) {
	switch(button) {
	case SDL_BUTTON_LEFT:
		return AGSEVENT_BUTTON_LEFT;
	case SDL_BUTTON_MIDDLE:
		return AGSEVENT_BUTTON_MID;
	case SDL_BUTTON_RIGHT:
		return AGSEVENT_BUTTON_RIGHT;
	}
	return 0;
}

EMSCRIPTEN_KEEPALIVE
void send_agsevent(enum agsevent_type type, int code) {
	if (!nact->ags.eventcb)
		return;
	agsevent_t agse = {
		.type = type,
		.code = code,
		.mousex = mousex,
		.mousey = mousey
	};
	nact->ags.eventcb(&agse);  // Async in emscripten

#ifdef __EMSCRIPTEN__
	// HACK: this ensures that callers of this function are instrumented.
	// This is necessary because ASYNCIFY_PROPAGATE_ADD does not work:
	// https://github.com/emscripten-core/emscripten/issues/23015
	emscripten_sleep(0);
#endif
}

void sdl_setCursorInternalLocation(int x, int y) {
	mousex = x;
	mousey = y;
	send_agsevent(AGSEVENT_MOUSE_MOTION, 0);
}

// Stores a deferred touch event (valid if .timestamp != 0) to add a delay
// between mouse pointer movement and mouse button state change caused by a
// touch event. This prevents the game from processing a button down event
// before reading the pointer position.
static SDL_TouchFingerEvent deferred_touch_event;
#define TOUCH_EVENT_DELAY 20

static void defer_touch_event(SDL_TouchFingerEvent *e) {
	switch (deferred_touch_event.type) {
	case SDL_FINGERDOWN:
		mouseb |= 1 << SDL_BUTTON_LEFT;
		RawKeyInfo[mouse_to_rawkey(SDL_BUTTON_LEFT)] = true;
		break;
	case SDL_FINGERUP:
		mouseb &= ~(1 << SDL_BUTTON_LEFT | 1 << SDL_BUTTON_RIGHT);
		RawKeyInfo[mouse_to_rawkey(SDL_BUTTON_LEFT)] = false;
		RawKeyInfo[mouse_to_rawkey(SDL_BUTTON_RIGHT)] = false;
		break;
	}
	if (e)
		deferred_touch_event = *e;
	else
		deferred_touch_event.timestamp = 0;
}

static void fire_deferred_touch_event(void) {
	if (deferred_touch_event.timestamp && deferred_touch_event.timestamp + TOUCH_EVENT_DELAY < SDL_GetTicks()) {
		defer_touch_event(NULL);
	}
}

// Improves map navigation of Rance4 v2. See also the function comment of
// rance4_Y3_IM_hack() in cmdy.c.
static void rance4v2_hack(void) {
	// Do not wait for vsync while a mouse button or an arrow key is pressed.
	if (mouseb ||
		(RawKeyInfo[KEY_UP]    || RawKeyInfo[KEY_PAD_8]) ||
		(RawKeyInfo[KEY_DOWN]  || RawKeyInfo[KEY_PAD_2]) ||
		(RawKeyInfo[KEY_LEFT]  || RawKeyInfo[KEY_PAD_4]) ||
		(RawKeyInfo[KEY_RIGHT] || RawKeyInfo[KEY_PAD_6]))
		cancel_yield();
}

void sdl_handle_event(SDL_Event *e) {
	if (sdl_custom_event_handler && sdl_custom_event_handler(e))
		return;

	switch (e->type) {
	case SDL_QUIT:
		menu_quitmenu_open();
		break;

	case SDL_WINDOWEVENT:
		switch (e->window.event) {
		case SDL_WINDOWEVENT_EXPOSED:
			gfx_dirty = true;
			break;
		}
		break;
#ifdef _WIN32
	case SDL_SYSWMEVENT:
		win_menu_onSysWMEvent(e->syswm.msg);
		break;
#endif
	case SDL_APP_DIDENTERFOREGROUND:
		gfx_dirty = true;
		break;
	case SDL_KEYDOWN:
		keyEventProsess(&e->key, true);
		break;
	case SDL_KEYUP:
		keyEventProsess(&e->key, false);
		switch (e->key.keysym.sym) {
		case SDLK_F1:
			msgskip_activate(!msgskip_isActivated());
			break;
		case SDLK_F4:
			gfx_setFullscreen(!gfx_fullscreen);
			break;
		}
#ifdef __ANDROID__
		if (e->key.keysym.scancode == SDL_SCANCODE_AC_BACK) {
			menu_quitmenu_open();
		}
#endif
		break;
	case SDL_MOUSEMOTION:
		sdl_setCursorInternalLocation(e->motion.x, e->motion.y);
#ifdef _WIN32
		win_menu_onMouseMotion(e->motion.x, e->motion.y);
#endif
		break;

	case SDL_MOUSEWHEEL:
		{
			int y = e->wheel.y * (e->wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -1 : 1);
			if (y > 0)
				mouse_wheel_up += y;
			else if (y < 0)
				mouse_wheel_down -= y;
			send_agsevent(AGSEVENT_MOUSE_WHEEL, y);
			break;
		}

	case SDL_MOUSEBUTTONDOWN:
		mouseb |= (1 << e->button.button);
		RawKeyInfo[mouse_to_rawkey(e->button.button)] = true;
		send_agsevent(AGSEVENT_BUTTON_PRESS, mouse_to_agsevent(e->button.button));
		break;

	case SDL_MOUSEBUTTONUP:
		mouseb &= (0xffffffff ^ (1 << e->button.button));
		RawKeyInfo[mouse_to_rawkey(e->button.button)] = false;
		send_agsevent(AGSEVENT_BUTTON_RELEASE, mouse_to_agsevent(e->button.button));
		if (e->button.button == 2) {
			menu_open();
		}
		break;

	case SDL_FINGERDOWN:
		if (SDL_GetNumTouchFingers(e->tfinger.touchId) >= 2) {
			mouseb &= ~(1 << SDL_BUTTON_LEFT);
			mouseb |= 1 << SDL_BUTTON_RIGHT;
			RawKeyInfo[mouse_to_rawkey(SDL_BUTTON_LEFT)] = false;
			RawKeyInfo[mouse_to_rawkey(SDL_BUTTON_RIGHT)] = true;
			send_agsevent(AGSEVENT_BUTTON_PRESS, AGSEVENT_BUTTON_RIGHT);
		} else {
			// SDL_RendererEventWatch clamps touch locations outside of the
			// viewport to 0.0-1.0. Treat such events as right-clicks.
			int button;
			if  (e->tfinger.x == 0.0f || e->tfinger.x == 1.0f || e->tfinger.y == 0.0f || e->tfinger.y == 1.0f) {
				button = SDL_BUTTON_RIGHT;
				mouseb |= 1 << SDL_BUTTON_RIGHT;
				RawKeyInfo[mouse_to_rawkey(SDL_BUTTON_RIGHT)] = true;
			} else {
				button = SDL_BUTTON_LEFT;
				mousex = e->tfinger.x * view_w;
				mousey = e->tfinger.y * view_h;
				send_agsevent(AGSEVENT_MOUSE_MOTION, 0);
				defer_touch_event(&e->tfinger);
			}
			send_agsevent(AGSEVENT_BUTTON_PRESS, mouse_to_agsevent(button));
		}
		break;

	case SDL_FINGERUP:
		if (SDL_GetNumTouchFingers(e->tfinger.touchId) == 0) {
			int ags_button = (mouseb & 1 << SDL_BUTTON_LEFT) ? AGSEVENT_BUTTON_LEFT : AGSEVENT_BUTTON_RIGHT;
			mousex = e->tfinger.x * view_w;
			mousey = e->tfinger.y * view_h;
			send_agsevent(AGSEVENT_BUTTON_RELEASE, ags_button);
			defer_touch_event(&e->tfinger);
		}
		break;

	case SDL_FINGERMOTION:
		mousex = e->tfinger.x * view_w;
		mousey = e->tfinger.y * view_h;
		send_agsevent(AGSEVENT_MOUSE_MOTION, 0);
		break;

	case SDL_JOYDEVICEADDED:
		sdl_joy_open(e->jdevice.which);
		break;

	case SDL_JOYAXISMOTION:
		if (abs(e->jaxis.value) < 0x4000) {
			joyinfo &= e->jaxis.axis == 0 ? ~0xc : ~3;
		} else {
			int i = (e->jaxis.axis == 0 ? 2 : 0) +
				(e->jaxis.value > 0 ? 1 : 0);
			joyinfo |= 1 << i;
		}
		break;

	case SDL_JOYBALLMOTION:
		break;

	case SDL_JOYHATMOTION:
		joyinfo &= ~(SYS35KEY_UP | SYS35KEY_DOWN | SYS35KEY_LEFT | SYS35KEY_RIGHT);
		switch (e->jhat.value) {
		case SDL_HAT_UP:        joyinfo |= SYS35KEY_UP;    break;
		case SDL_HAT_DOWN:      joyinfo |= SYS35KEY_DOWN;  break;
		case SDL_HAT_LEFT:      joyinfo |= SYS35KEY_LEFT;  break;
		case SDL_HAT_RIGHT:     joyinfo |= SYS35KEY_RIGHT; break;
		case SDL_HAT_LEFTUP:    joyinfo |= SYS35KEY_LEFT  | SYS35KEY_UP;   break;
		case SDL_HAT_RIGHTUP:   joyinfo |= SYS35KEY_RIGHT | SYS35KEY_UP;   break;
		case SDL_HAT_LEFTDOWN:  joyinfo |= SYS35KEY_LEFT  | SYS35KEY_DOWN; break;
		case SDL_HAT_RIGHTDOWN: joyinfo |= SYS35KEY_RIGHT | SYS35KEY_DOWN; break;
		}
		break;

	case SDL_JOYBUTTONDOWN:
	case SDL_JOYBUTTONUP:
		{
			int mask = 0;
			switch (e->jbutton.button) {
			case 0: case 1: case 2: case 3:
				mask = 1 << (e->jbutton.button + 4);
				break;
#ifdef __EMSCRIPTEN__
			// Gamepads known to browsers get a "standard" mapping where D-pad
			// keys are mapped to buttons 12-15.
			case 12: mask = SYS35KEY_UP;    break;
			case 13: mask = SYS35KEY_DOWN;  break;
			case 14: mask = SYS35KEY_LEFT;  break;
			case 15: mask = SYS35KEY_RIGHT; break;
#endif
			}
			if (mask) {
				if (e->jbutton.state == SDL_PRESSED)
					joyinfo |= mask;
				else
					joyinfo &= ~mask;
			}
		}
		break;
	default:
		if (e->type == custom_event_type) {
			switch (e->user.code) {
			case SIMULATE_RIGHT_BUTTON:
				if ((intptr_t)e->user.data1) {
					mouseb |= 1 << SDL_BUTTON_RIGHT;
					RawKeyInfo[KEY_MOUSE_RIGHT] = true;
					send_agsevent(AGSEVENT_BUTTON_PRESS, AGSEVENT_BUTTON_RIGHT);
				} else {
					mouseb &= ~(1 << SDL_BUTTON_RIGHT);
					RawKeyInfo[KEY_MOUSE_RIGHT] = false;
					send_agsevent(AGSEVENT_BUTTON_RELEASE, AGSEVENT_BUTTON_RIGHT);
				}
				break;
			case DEBUGGER_COMMAND:
				dbg_post_command(e->user.data1);
				break;
			}
		}
		break;
	}
}

/* Event処理 */
static void sdl_getEvent(void) {
	enum scheduler_event scheduler_event = SCHEDULER_EVENT_INPUT_CHECK_MISS;

	fire_deferred_touch_event();

	SDL_Event e;
	while (SDL_PollEvent(&e)) {
		scheduler_event = SCHEDULER_EVENT_INPUT_CHECK_HIT;
		sdl_handle_event(&e);
	}
	scheduler_on_event(scheduler_event);
	if (game_id == GAME_RANCE4_V2)
		rance4v2_hack();
}

/* キー情報の取得 */
static void keyEventProsess(SDL_KeyboardEvent *e, bool pressed) {
	int code = sdl_keytable[e->keysym.scancode];
	RawKeyInfo[code] = pressed;
	send_agsevent(pressed ? AGSEVENT_KEY_PRESS : AGSEVENT_KEY_RELEASE, code);
}

int sdl_getKeyInfo() {
	int rt;
	
	sdl_getEvent();
	
	rt = ((RawKeyInfo[KEY_UP]     || RawKeyInfo[KEY_PAD_8])       |
	      ((RawKeyInfo[KEY_DOWN]  || RawKeyInfo[KEY_PAD_2]) << 1) |
	      ((RawKeyInfo[KEY_LEFT]  || RawKeyInfo[KEY_PAD_4]) << 2) |
	      ((RawKeyInfo[KEY_RIGHT] || RawKeyInfo[KEY_PAD_6]) << 3) |
	      (RawKeyInfo[KEY_RETURN] << 4) |
	      (RawKeyInfo[KEY_SPACE ] << 5) |
	      (RawKeyInfo[KEY_ESCAPE] << 6) |
	      (RawKeyInfo[KEY_TAB]    << 7));
	
	return rt;
}

int sdl_getMouseInfo(MyPoint *p) {
	sdl_getEvent();
	
	if (p) {
		p->x = mousex;
		p->y = mousey;
	}

	int m1 = mouseb & (1 << 1) ? SYS35KEY_RET : 0;
	int m2 = mouseb & (1 << 3) ? SYS35KEY_SPC : 0;
	return m1 | m2;
}

void sdl_getWheelInfo(int *forward, int *back) {
	*forward = mouse_wheel_up;
	*back = mouse_wheel_down;

#ifdef __EMSCRIPTEN__
	EM_ASM( xsystem35.texthook.disableWheelEvent(100) );
#endif
}

void sdl_clearWheelInfo(void) {
	mouse_wheel_up = mouse_wheel_down = 0;
}

int sdl_getJoyInfo(void) {
	sdl_getEvent();
	return joyinfo;
}

void sdl_post_debugger_command(void *data) {
	SDL_Event event = {
		.user = {
			.type = custom_event_type,
			.code = DEBUGGER_COMMAND,
			.data1 = data
		}
	};
	SDL_PushEvent(&event);
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
void simulate_right_button(int pressed) {
	SDL_Event event = {
		.user = {
			.type = custom_event_type,
			.code = SIMULATE_RIGHT_BUTTON,
			.data1 = (void*)pressed
		}
	};
	SDL_PushEvent(&event);
}
#endif
