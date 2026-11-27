#include <stdarg.h>

#include "effect.h"
#include "gfx.h"
#include "gfx_private.h"
#include "unittest.h"

SDL_Renderer *gfx_renderer;
SDL_Texture *gfx_texture;
surface_t *gfx_dibinfo;
SDL_Surface *main_surface;
int view_w, view_h;

static void (*redraw)(void *);
static void *redraw_arg;

void gfx_setRedrawHandler(void (*handler)(void *), void *arg) {
	redraw = handler;
	redraw_arg = arg;
}

void gfx_updateScreen(void) {
	if (redraw)
		redraw(redraw_arg);
}

void gfx_requestRedraw(void) {
	gfx_updateScreen();
}

SDL_Surface *gfx_createSurfaceView(SDL_Surface *sf, int x, int y, int w, int h) {
	return sdl_CreateSurfaceFrom(w, h, sdl_SurfaceFormat(sf), PIXEL_AT(sf, x, y), sf->pitch);
}

SDL_Surface *gfx_dib_to_surface_with_alpha(int x, int y, int w, int h) {
	ASSERT_TRUE(false);
	return NULL;
}

SDL_Surface *gfx_dib_to_surface_colorkey(int x, int y, int w, int h, int col) {
	ASSERT_TRUE(false);
	return NULL;
}

void sys_error(char *format, ...) {
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	exit(1);
}

void sys_message(int level, char *format, ...) {
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
}

static void mosaic_test(int w, int h, enum effect_type type) {
	view_w = w;
	view_h = h;
	main_surface = sdl_CreateSurface(w, h, SDL_PIXELFORMAT_XRGB8888);
	ASSERT_TRUE(main_surface);
	gfx_renderer = SDL_CreateSoftwareRenderer(main_surface);
	ASSERT_TRUE(gfx_renderer);
	ASSERT_TRUE(sdl_RenderTargetSupported(gfx_renderer));

	SDL_Surface *old = sdl_CreateSurface(w, h, SDL_PIXELFORMAT_XRGB8888);
	SDL_Surface *new = sdl_CreateSurface(w, h, SDL_PIXELFORMAT_XRGB8888);
	ASSERT_TRUE(old && new);
	ASSERT_TRUE(SDL_FillSurfaceRect(old, NULL, SDL_MapSurfaceRGB(old, 255, 0, 0)));
	ASSERT_TRUE(SDL_FillSurfaceRect(new, NULL, SDL_MapSurfaceRGB(new, 0, 0, 255)));
	surface_t old_as = { .sdl_surface = old };
	surface_t new_as = { .sdl_surface = new };
	SDL_Rect rect = { .w = w, .h = h };
	struct effect *eff = effect_init(&rect, &old_as, 0, 0, &new_as, 0, 0, type);
	ASSERT_TRUE(eff);

	const float progress[] = { 0.f, 0.01f, 0.39f, 0.5f, 0.61f, 0.99f, 1.f };
	for (int i = 0; i < sizeof(progress) / sizeof(*progress); i++) {
		SDL_ClearError();
		effect_step(eff, progress[i]);
		SDL_RenderPresent(gfx_renderer);
		ASSERT_STRCMP(SDL_GetError(), "");
		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {
				Uint8 r, g, b;
				sdl_GetSurfaceRGB(*(Uint32 *)PIXEL_AT(main_surface, x, y), main_surface, &r, &g, &b);
				ASSERT_EQUAL(g, 0);
				if (type == EFFECT_MOSAIC || progress[i] == 1.f) {
					ASSERT_EQUAL(r, 0);
					ASSERT_EQUAL(b, 255);
				} else {
					ASSERT_TRUE(abs(r - (int)((1.f - progress[i]) * 255)) <= 2);
					ASSERT_TRUE(abs(b - (int)(progress[i] * 255)) <= 2);
				}
			}
		}
	}
	effect_finish(eff);
	SDL_DestroySurface(old);
	SDL_DestroySurface(new);
	SDL_DestroyRenderer(gfx_renderer);
	SDL_DestroySurface(main_surface);
}

int main(void) {
#if XSYSTEM35_SDL_VERSION == 2
	ASSERT_EQUAL(SDL_Init(SDL_INIT_VIDEO), 0);
#else
	ASSERT_TRUE(SDL_Init(SDL_INIT_VIDEO));
#endif
	const int sizes[][2] = { {640, 480}, {37, 29}, {1, 1}, {1, 9}, {9, 1} };
	for (int i = 0; i < sizeof(sizes) / sizeof(*sizes); i++) {
		mosaic_test(sizes[i][0], sizes[i][1], EFFECT_MOSAIC);
		mosaic_test(sizes[i][0], sizes[i][1], EFFECT_CROSSFADE_MOSAIC);
	}
	SDL_Quit();
	return 0;
}
