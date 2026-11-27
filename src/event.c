/*
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

#include "config.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "sdl_compat.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "event.h"
#include "portab.h"
#include "system.h"
#include "debugger.h"
#include "nact.h"
#include "gfx.h"
#include "scheduler.h"
#include "menu.h"
#include "modal.h"
#include "input.h"
#include "msgskip.h"
#include "hacks.h"
#include "volume.h"
#include "mixer.h"
#include "alicehub_bridge.h"
#ifdef __SWITCH__
#include "hub/controller_runtime.h"
#include "hub/runtime_menu.h"
#include "hub/runtime_menu_engines.h"
#include "escalayer_modifier.h"
#include "gfx_private.h"
#endif

static void get_event(void);
static void keyEventProsess(SDL_KeyboardEvent *e, bool pressed);

static uint32_t custom_event_type = (uint32_t)-1;

enum CustomEventCode {
	SIMULATE_RIGHT_BUTTON,
	DEBUGGER_COMMAND,
	WIN_MENU_COMMAND,
	VIRTUAL_MENU_COMMAND,
};

static SDL_Point mouse_pos;
static int mouseb;
static int mouse_wheel_up, mouse_wheel_down;
static bool RawKeyInfo[NUM_KEYCODES];

/* SDL Joystick */
static int joyinfo=0;

static int keytable[SDL_SCANCODE_COUNT] = {
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

static int joy_device_index = -1;
static SDL_Joystick *js;
#ifdef __SWITCH__
static uint32_t switch_joy_buttons;
#endif

// id is a device index in SDL2, and an instance ID in SDL3.
static bool joy_open(int id) {
	if (js)
		return false;

#if XSYSTEM35_SDL_VERSION == 2
	js = SDL_JoystickOpen(id);
#else
	js = SDL_OpenJoystick(id);
#endif
	if (!js)
		return false;

	const char *name = SDL_GetJoystickName(js);
	int axes = SDL_GetNumJoystickAxes(js);
	int buttons = SDL_GetNumJoystickButtons(js);
	SDL_SetJoystickEventsEnabled(true);
	NOTICE("SDL joystick '%s' %d axes %d buttons", name, axes, buttons);
	return true;
}

static bool joy_init(void) {
	SDL_Init(SDL_INIT_JOYSTICK);

#if XSYSTEM35_SDL_VERSION == 2
	if (joy_device_index >= 0)
		return joy_open(joy_device_index);

	for (int i = 0; i < SDL_NumJoysticks(); i++) {
		if (joy_open(i))
			return true;
	}
	return false;
#else
	int count = 0;
	SDL_JoystickID *ids = SDL_GetJoysticks(&count);
	bool opened = false;
	if (joy_device_index >= 0) {
		if (joy_device_index < count)
			opened = joy_open(ids[joy_device_index]);
	} else {
		for (int i = 0; i < count && !opened; i++)
			opened = joy_open(ids[i]);
	}
	SDL_free(ids);
	return opened;
#endif
}

void event_init(void) {
	if (custom_event_type == (uint32_t)-1)
		custom_event_type = SDL_RegisterEvents(1);
	joy_init();
}

void event_remove(void) {
	if (js) {
		SDL_CloseJoystick(js);
		js = NULL;
	}
#ifdef ALICEHUB_IOS
	custom_event_type = (uint32_t)-1;
	memset(RawKeyInfo, 0, sizeof(RawKeyInfo));
#endif
}

void event_set_joy_device_index(int index) {
	joy_device_index = index;
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
		.mousex = mouse_pos.x,
		.mousey = mouse_pos.y
	};
	nact->ags.eventcb(&agse);  // Async in emscripten

#ifdef __EMSCRIPTEN__
	// HACK: this ensures that callers of this function are instrumented.
	// This is necessary because ASYNCIFY_PROPAGATE_ADD does not work:
	// https://github.com/emscripten-core/emscripten/issues/23015
	emscripten_sleep(0);
#endif
}

static void set_raw_key(int code, bool pressed) {
	RawKeyInfo[code] = pressed;
	send_agsevent(pressed ? AGSEVENT_KEY_PRESS : AGSEVENT_KEY_RELEASE, code);
}

void event_set_mouse_internal_location(int x, int y) {
	mouse_pos.x = x;
	mouse_pos.y = y;
	send_agsevent(AGSEVENT_MOUSE_MOTION, 0);
}

SDL_Point event_get_touch_position(const SDL_TouchFingerEvent *e) {
	int view_w, view_h;
	gfx_getViewSize(&view_w, &view_h);
	return (SDL_Point){e->x * view_w, e->y * view_h};
}

// Stores a deferred touch event (valid if .timestamp != 0) to add a delay
// between mouse pointer movement and mouse button state change caused by a
// touch event. This prevents the game from processing a button down event
// before reading the pointer position.
static SDL_TouchFingerEvent deferred_touch_event;
static int touch_button = 0;
#define TOUCH_EVENT_DELAY 20

static void defer_touch_event(SDL_TouchFingerEvent *e) {
	switch (deferred_touch_event.type) {
	case SDL_EVENT_FINGER_DOWN:
		mouseb |= 1 << SDL_BUTTON_LEFT;
		RawKeyInfo[mouse_to_rawkey(SDL_BUTTON_LEFT)] = true;
		break;
	case SDL_EVENT_FINGER_UP:
		mouseb &= ~(1 << SDL_BUTTON_LEFT | 1 << SDL_BUTTON_RIGHT);
		RawKeyInfo[mouse_to_rawkey(SDL_BUTTON_LEFT)] = false;
		RawKeyInfo[mouse_to_rawkey(SDL_BUTTON_RIGHT)] = false;
		break;
	default:
		break;
	}
	if (e)
		deferred_touch_event = *e;
	else
		deferred_touch_event.timestamp = 0;
}

static void fire_deferred_touch_event(void) {
	if (deferred_touch_event.timestamp && sdl_TimestampToTicks(deferred_touch_event.timestamp) + TOUCH_EVENT_DELAY < SDL_GetTicks()) {
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

static void handle_render_event(SDL_Event *e, bool virtual_input) {
#ifdef __SWITCH__
	if (e->type == SDL_EVENT_QUIT) {
		nact_quit(false);
		return;
	}
	if (switch_controls_touch(e)) {
		gfx_requestRedraw();
		return;
	}
#endif
	if (modal_handle_event(e))
		return;

	switch (e->type) {
	case SDL_EVENT_QUIT:
		menu_quitmenu_open();
		break;

#if XSYSTEM35_SDL_VERSION == 2
	case SDL_WINDOWEVENT:
		switch (e->window.event) {
		case SDL_WINDOWEVENT_EXPOSED:
			gfx_requestRedraw();
			break;
		case SDL_WINDOWEVENT_FOCUS_LOST:
			volume_on_window_focus(false);
			break;
		case SDL_WINDOWEVENT_FOCUS_GAINED:
			volume_on_window_focus(true);
#ifdef __ANDROID__
			gfx_android_on_foreground();
#endif
			break;
		}
		break;
#ifdef _WIN32
	case SDL_SYSWMEVENT:
		win_menu_onSysWMEvent(e->syswm.msg);
		break;
#endif
#else
	case SDL_EVENT_WINDOW_EXPOSED:
		gfx_requestRedraw();
		break;
	case SDL_EVENT_WINDOW_FOCUS_LOST:
		volume_on_window_focus(false);
		break;
	case SDL_EVENT_WINDOW_FOCUS_GAINED:
		volume_on_window_focus(true);
#ifdef __ANDROID__
		gfx_android_on_foreground();
#endif
		break;
#endif
	case SDL_EVENT_WILL_ENTER_BACKGROUND:
#ifdef __SWITCH__
		switch_menu35_release_skip();
		event_reset_input_state();
#endif
#ifdef __ANDROID__
		gfx_android_on_background();
		msgskip_syncFile();
#endif
		mixer_pause_all(true);
		break;
	case SDL_EVENT_DID_ENTER_FOREGROUND:
#ifdef __ANDROID__
		gfx_android_on_foreground();
#endif
		mixer_pause_all(false);
		gfx_requestRedraw();
		break;
	case SDL_EVENT_KEY_DOWN:
		keyEventProsess(&e->key, true);
		break;
	case SDL_EVENT_KEY_UP:
		keyEventProsess(&e->key, false);
		switch (sdl_KeyboardEventKey(&e->key)) {
		case SDLK_F1:
			msgskip_activate(!msgskip_isActivated());
			break;
		case SDLK_F4:
			gfx_setFullscreen(!gfx_isFullscreen());
			break;
		}
#ifdef __ANDROID__
		if (sdl_KeyboardEventScancode(&e->key) == SDL_SCANCODE_AC_BACK) {
			menu_quitmenu_open();
		}
#endif
		break;
	case SDL_EVENT_MOUSE_MOTION:
		event_set_mouse_internal_location(e->motion.x, e->motion.y);
#ifdef _WIN32
		win_menu_onMouseMotion(e->motion.x, e->motion.y);
#endif
		break;

	case SDL_EVENT_MOUSE_WHEEL:
		{
			int y = sdl_MouseWheelEventY(&e->wheel) * (e->wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -1 : 1);
			if (y > 0)
				mouse_wheel_up += y;
			else if (y < 0)
				mouse_wheel_down -= y;
			send_agsevent(AGSEVENT_MOUSE_WHEEL, y);
			break;
		}

	case SDL_EVENT_MOUSE_BUTTON_DOWN:
		mouseb |= (1 << e->button.button);
		RawKeyInfo[mouse_to_rawkey(e->button.button)] = true;
		send_agsevent(AGSEVENT_BUTTON_PRESS, mouse_to_agsevent(e->button.button));
		break;

	case SDL_EVENT_MOUSE_BUTTON_UP:
		mouseb &= (0xffffffff ^ (1 << e->button.button));
		RawKeyInfo[mouse_to_rawkey(e->button.button)] = false;
		send_agsevent(AGSEVENT_BUTTON_RELEASE, mouse_to_agsevent(e->button.button));
		if (e->button.button == SDL_BUTTON_MIDDLE) {
			if (virtual_input) {
				SDL_Event menu = { .user = { .type = custom_event_type, .code = VIRTUAL_MENU_COMMAND } };
				SDL_PushEvent(&menu);
			} else {
				menu_open();
			}
		}
		break;

	case SDL_EVENT_FINGER_DOWN: {
#ifdef __SWITCH__
		int fingers = switch_controls_game_fingers(&e->tfinger);
#else
		int fingers = sdl_CountTouchFingers(&e->tfinger);
#endif
		alicehub_on_touch_input();
		if (fingers >= 3) {
			touch_button = 0;
			deferred_touch_event.timestamp = 0;
			menu_open();
		} else if (fingers >= 2) {
			touch_button = SDL_BUTTON_RIGHT;
			deferred_touch_event.timestamp = 0;
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
				deferred_touch_event.timestamp = 0;
				mouseb |= 1 << SDL_BUTTON_RIGHT;
				RawKeyInfo[mouse_to_rawkey(SDL_BUTTON_RIGHT)] = true;
			} else {
				button = SDL_BUTTON_LEFT;
				mouse_pos = event_get_touch_position(&e->tfinger);
				send_agsevent(AGSEVENT_MOUSE_MOTION, 0);
				defer_touch_event(&e->tfinger);
			}
			touch_button = button;
			send_agsevent(AGSEVENT_BUTTON_PRESS, mouse_to_agsevent(button));
		}
		break;
	}

	case SDL_EVENT_FINGER_UP:
#ifdef __SWITCH__
		if (switch_controls_game_fingers(&e->tfinger) == 0) {
#else
		if (sdl_CountTouchFingers(&e->tfinger) == 0) {
#endif
			int btn = touch_button ? touch_button : ((mouseb & (1 << SDL_BUTTON_LEFT)) ? SDL_BUTTON_LEFT : SDL_BUTTON_RIGHT);
			touch_button = 0;
			mouse_pos = event_get_touch_position(&e->tfinger);
			send_agsevent(AGSEVENT_BUTTON_RELEASE, mouse_to_agsevent(btn));
			defer_touch_event(&e->tfinger);
		}
		break;

	case SDL_EVENT_FINGER_MOTION:
		alicehub_on_touch_input();
		mouse_pos = event_get_touch_position(&e->tfinger);
		send_agsevent(AGSEVENT_MOUSE_MOTION, 0);
		break;

	case SDL_EVENT_JOYSTICK_ADDED:
		joy_open(e->jdevice.which);
		break;

#ifdef __SWITCH__
	case SDL_JOYDEVICEREMOVED:
		if (js && SDL_JoystickInstanceID(js) == e->jdevice.which) {
			event_remove();
			joyinfo = 0;
			switch_joy_buttons = 0;
			joy_init();
		}
		break;
#endif

	case SDL_EVENT_JOYSTICK_AXIS_MOTION:
#ifdef __SWITCH__
		if (e->jaxis.axis > 1)
			break;
		joyinfo &= e->jaxis.axis == 0 ? ~0xc : ~3;
#endif
		if (abs(e->jaxis.value) < 0x4000) {
			joyinfo &= e->jaxis.axis == 0 ? ~0xc : ~3;
		} else {
			int i = (e->jaxis.axis == 0 ? 2 : 0) +
				(e->jaxis.value > 0 ? 1 : 0);
			joyinfo |= 1 << i;
		}
		break;

	case SDL_EVENT_JOYSTICK_BALL_MOTION:
		break;

	case SDL_EVENT_JOYSTICK_HAT_MOTION:
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

	case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
	case SDL_EVENT_JOYSTICK_BUTTON_UP:
		{
#ifdef __SWITCH__
			if (switch_controls_remapping()) {
				if (js && SDL_JoystickInstanceID(js) == e->jbutton.which) {
					uint32_t button = switch_controls_joystick_button(e->jbutton.button);
					if (e->type == SDL_EVENT_JOYSTICK_BUTTON_DOWN)
						switch_joy_buttons |= button;
					else
						switch_joy_buttons &= ~button;
				}
				break;
			}
#endif
			int mask = 0;
			switch (e->jbutton.button) {
			case 0: case 1: case 2: case 3:
				mask = 1 << (e->jbutton.button + 4);
				break;
#if defined(__SWITCH__)
			case 12: mask = SYS35KEY_LEFT;  break;
			case 13: mask = SYS35KEY_UP;    break;
			case 14: mask = SYS35KEY_RIGHT; break;
			case 15: mask = SYS35KEY_DOWN;  break;
#elif defined(__EMSCRIPTEN__)
			// Gamepads known to browsers get a "standard" mapping where D-pad
			// keys are mapped to buttons 12-15.
			case 12: mask = SYS35KEY_UP;    break;
			case 13: mask = SYS35KEY_DOWN;  break;
			case 14: mask = SYS35KEY_LEFT;  break;
			case 15: mask = SYS35KEY_RIGHT; break;
#endif
			}
			if (mask) {
				if (e->type == SDL_EVENT_JOYSTICK_BUTTON_DOWN)
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
			case VIRTUAL_MENU_COMMAND:
				menu_open();
				break;
#ifdef _WIN32
			case WIN_MENU_COMMAND:
				win_menu_onCommand((uintptr_t)e->user.data1);
				break;
#endif
			}
		}
		break;
	}
}

void event_handle_event(SDL_Event *e) {
	gfx_convertEventCoordinates(e);
	handle_render_event(e, false);
}

/* Event処理 */
static void get_event(void) {
	enum scheduler_event scheduler_event = SCHEDULER_EVENT_INPUT_CHECK_MISS;

#ifdef __SWITCH__
	if (switch_menu35_poll(gfx_renderer)) {
		event_reset_input_state();
		deferred_touch_event.timestamp = 0;
		touch_button = 0;
		mouse_wheel_up = mouse_wheel_down = 0;
		if (!switch_menu_lifecycle_pending()) {
			gfx_requestRedraw();
			gfx_updateScreen();
		}
	}
	escalayer_modifier_tick();
#endif
	fire_deferred_touch_event();
	alicehub_pump_events();

	SDL_Event e;
	while (SDL_PollEvent(&e)) {
		scheduler_event = SCHEDULER_EVENT_INPUT_CHECK_HIT;
		event_handle_event(&e);
	}
	if (sys_is_quit_requested())
		nact_quit(false);
	scheduler_on_event(scheduler_event);
	if (game_id == GAME_RANCE4_V2)
		rance4v2_hack();
}

/* キー情報の取得 */
static void keyEventProsess(SDL_KeyboardEvent *e, bool pressed) {
	int code = keytable[sdl_KeyboardEventScancode(e)];
	set_raw_key(code, pressed);
}

int event_get_key(void) {
	int rt;
	
	get_event();
	
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

bool event_rawkey_down(unsigned code) {
#ifdef __SWITCH__
	if (code == KEY_CTRL && switch_menu35_ctrl_skip())
		return true;
#endif
	return code < NUM_KEYCODES && RawKeyInfo[code];
}

void event_reset_input_state(void) {
	memset(RawKeyInfo, 0, sizeof(RawKeyInfo));
	mouseb = 0;
	joyinfo = 0;
#ifdef __SWITCH__
	switch_joy_buttons = 0;
#endif
}

int event_get_mouse(SDL_Point *p) {
	get_event();
	
	if (p) {
		*p = mouse_pos;
	}

	int m1 = mouseb & (1 << 1) ? SYS35KEY_RET : 0;
	int m2 = mouseb & (1 << 3) ? SYS35KEY_SPC : 0;
	return m1 | m2;
}

void event_get_wheel(vmvar_t *forward, vmvar_t *back) {
	*forward = mouse_wheel_up;
	*back = mouse_wheel_down;

#ifdef __EMSCRIPTEN__
	EM_ASM( xsystem35.texthook.disableWheelEvent(100) );
#endif
}

void event_clear_wheel(void) {
	mouse_wheel_up = mouse_wheel_down = 0;
}

int event_get_joy(void) {
	get_event();
#ifdef __SWITCH__
	if (switch_controls_remapping())
		return joyinfo | (switch_controls_buttons(switch_joy_buttons) & 0xff);
#endif
	return joyinfo;
}

void event_inject_mouse_motion(int x, int y) {
#ifdef ALICEHUB_IOS
	SDL_Event e = { .motion = { .type = SDL_EVENT_MOUSE_MOTION, .x = x, .y = y } };
	handle_render_event(&e, true);
#else
	event_set_mouse_internal_location(x, y);
#endif
}

void event_inject_mouse_button(int sdl_button, bool pressed, int x, int y) {
#ifdef ALICEHUB_IOS
	SDL_Event e = { .button = {
		.type = pressed ? SDL_EVENT_MOUSE_BUTTON_DOWN : SDL_EVENT_MOUSE_BUTTON_UP,
		.button = sdl_button, .x = x >= 0 ? x : mouse_pos.x, .y = y >= 0 ? y : mouse_pos.y,
	} };
	handle_render_event(&e, true);
#else
	(void)x;
	(void)y;
	int rawkey = mouse_to_rawkey(sdl_button);
	int agsbtn = mouse_to_agsevent(sdl_button);
	if (pressed)
		mouseb |= (1 << sdl_button);
	else
		mouseb &= ~(1 << sdl_button);
	if (rawkey)
		RawKeyInfo[rawkey] = pressed;
	if (agsbtn)
		send_agsevent(pressed ? AGSEVENT_BUTTON_PRESS : AGSEVENT_BUTTON_RELEASE, agsbtn);
#endif
}

void event_inject_mouse_wheel(int dir) {
#ifdef ALICEHUB_IOS
	SDL_Event e = { .wheel = { .type = SDL_EVENT_MOUSE_WHEEL, .y = dir,
#if XSYSTEM35_SDL_VERSION >= 3
		.integer_y = dir,
#endif
	} };
	handle_render_event(&e, true);
#else
	if (dir > 0)
		mouse_wheel_up++;
	else if (dir < 0)
		mouse_wheel_down++;
	if (dir != 0)
		send_agsevent(AGSEVENT_MOUSE_WHEEL, dir);
#endif
}

void event_inject_rawkey(int keycode, bool pressed) {
	if (keycode <= 0 || keycode >= 256)
		return;
#ifdef ALICEHUB_IOS
	for (int i = 1; i < SDL_SCANCODE_COUNT; i++) {
		if (keytable[i] != keycode)
			continue;
		SDL_Event e = { .key = { .type = pressed ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP,
#if XSYSTEM35_SDL_VERSION == 2
			.keysym = { .scancode = i, .sym = SDL_GetKeyFromScancode(i) },
#else
			.scancode = i, .key = SDL_GetKeyFromScancode(i, SDL_KMOD_NONE, true),
#endif
		} };
		handle_render_event(&e, true);
		return;
	}
#endif
	set_raw_key(keycode, pressed);
}

void event_inject_joy_mask(int mask, bool pressed) {
	if (pressed)
		joyinfo |= mask;
	else
		joyinfo &= ~mask;
}

void event_post_debugger_command(void *data) {
	SDL_Event event = {
		.user = {
			.type = custom_event_type,
			.code = DEBUGGER_COMMAND,
			.data1 = data
		}
	};
	SDL_PushEvent(&event);
}

#ifdef _WIN32
void event_post_win_menu_command(uintptr_t wparam) {
	SDL_Event event = {
		.user = {
			.type = custom_event_type,
			.code = WIN_MENU_COMMAND,
			.data1 = (void *)wparam
		}
	};
	SDL_PushEvent(&event);
}
#endif

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
