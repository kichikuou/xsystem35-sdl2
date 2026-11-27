#ifndef __ALICEHUB_BRIDGE_H__
#define __ALICEHUB_BRIDGE_H__

#include <stdbool.h>

// Implemented in event.c to update input state without exposing static globals.
void event_inject_mouse_motion(int x, int y);
void event_inject_mouse_button(int sdl_button, bool pressed, int x, int y);
void event_inject_mouse_wheel(int dir);
void event_inject_rawkey(int keycode, bool pressed);
void event_inject_joy_mask(int mask, bool pressed);

#if defined(__ANDROID__) || defined(ALICEHUB_IOS)
void alicehub_pump_events(void);
void alicehub_on_touch_input(void);
#else
static inline void alicehub_pump_events(void) {}
static inline void alicehub_on_touch_input(void) {}
#endif

#endif /* __ALICEHUB_BRIDGE_H__ */
