/*
 * Copyright (C) 2026 <KichikuouChrome@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "sdl_compat.h"
#include "unittest.h"

void sdl_compat_test(void)
{
	SDL_Event event = {0};
	ASSERT_EQUAL(sdl_get_window_event(&event),
		SDL_COMPAT_WINDOW_EVENT_NONE);
#if XSYSTEM35_SDL_VERSION == 2
	event.type = SDL_WINDOWEVENT;
	event.window.event = SDL_WINDOWEVENT_EXPOSED;
#else
	event.type = SDL_EVENT_WINDOW_EXPOSED;
#endif
	ASSERT_EQUAL(sdl_get_window_event(&event),
		SDL_COMPAT_WINDOW_EVENT_EXPOSED);

	SDL_KeyboardEvent keyboard_event = {0};
#if XSYSTEM35_SDL_VERSION == 2
	keyboard_event.keysym.scancode = SDL_SCANCODE_F1;
	keyboard_event.keysym.sym = SDLK_F1;
#else
	keyboard_event.scancode = SDL_SCANCODE_F1;
	keyboard_event.key = SDLK_F1;
#endif
	ASSERT_EQUAL(sdl_keyboard_event_scancode(&keyboard_event),
		SDL_SCANCODE_F1);
	ASSERT_EQUAL(sdl_keyboard_event_key(&keyboard_event), SDLK_F1);

	SDL_TouchFingerEvent touch_event = {0};
#if XSYSTEM35_SDL_VERSION == 2
	touch_event.touchId = 42;
#else
	touch_event.touchID = 42;
#endif
	ASSERT_EQUAL(sdl_touch_event_id(&touch_event), 42);

	SDL_JoyButtonEvent joystick_button_event = {0};
#if XSYSTEM35_SDL_VERSION == 2
	joystick_button_event.state = SDL_PRESSED;
#else
	joystick_button_event.down = true;
#endif
	ASSERT_TRUE(sdl_joystick_button_event_pressed(&joystick_button_event));

	static const uint8_t data[] = {1, 2, 3, 4};
	sdl_iostream_t *stream = sdl_io_from_const_memory(data, sizeof(data));
	ASSERT_TRUE(stream);
	ASSERT_EQUAL(sdl_get_io_size(stream), sizeof(data));
	uint8_t buffer[3];
	ASSERT_EQUAL(sdl_read_io(stream, buffer, sizeof(buffer)), sizeof(buffer));
	ASSERT_EQUAL(buffer[0], 1);
	ASSERT_EQUAL(buffer[1], 2);
	ASSERT_EQUAL(buffer[2], 3);
	ASSERT_TRUE(sdl_close_io(stream));

	SDL_Surface *source = sdl_create_surface(
		4, 4, 32, SDL_PIXELFORMAT_XRGB8888);
	SDL_Surface *destination = sdl_create_surface(
		4, 4, 32, SDL_PIXELFORMAT_XRGB8888);
	ASSERT_TRUE(source);
	ASSERT_TRUE(destination);

	SDL_Rect destination_rect = {-1, 1, 99, 99};
	ASSERT_TRUE(SDL_BlitSurface(
		source, NULL, destination, &destination_rect));
	ASSERT_EQUAL(destination_rect.x, 0);
	ASSERT_EQUAL(destination_rect.y, 1);
	ASSERT_EQUAL(destination_rect.w, 3);
	ASSERT_EQUAL(destination_rect.h, 3);

	SDL_Rect source_rect = {-1, 1, 4, 4};
	destination_rect = (SDL_Rect){1, -1, 99, 99};
	ASSERT_TRUE(SDL_BlitSurface(
		source, &source_rect, destination, &destination_rect));
	ASSERT_EQUAL(destination_rect.x, 2);
	ASSERT_EQUAL(destination_rect.y, 0);
	ASSERT_EQUAL(destination_rect.w, 2);
	ASSERT_EQUAL(destination_rect.h, 2);

	source_rect = (SDL_Rect){10, 10, 1, 1};
	destination_rect = (SDL_Rect){1, 2, 99, 99};
	ASSERT_TRUE(SDL_BlitSurface(
		source, &source_rect, destination, &destination_rect));
	ASSERT_EQUAL(destination_rect.x, 1);
	ASSERT_EQUAL(destination_rect.y, 2);
	ASSERT_EQUAL(destination_rect.w, 0);
	ASSERT_EQUAL(destination_rect.h, 0);

	sdl_destroy_surface(destination);
	sdl_destroy_surface(source);
}
