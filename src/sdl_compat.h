#ifndef XSYSTEM35_SDL_COMPAT_H
#define XSYSTEM35_SDL_COMPAT_H

#include <stdbool.h>
#include <limits.h>
#include <stdint.h>

#ifndef XSYSTEM35_SDL_VERSION
#define XSYSTEM35_SDL_VERSION 2
#endif

#if XSYSTEM35_SDL_VERSION == 2
#include <SDL.h>
#ifdef _WIN32
#include <SDL_syswm.h>
#endif
#elif XSYSTEM35_SDL_VERSION == 3
#include <SDL3/SDL.h>
#else
#error "Unsupported XSYSTEM35_SDL_VERSION"
#endif

#if SDL_MAJOR_VERSION != XSYSTEM35_SDL_VERSION
#error "The selected SDL headers do not match XSYSTEM35_SDL_VERSION"
#endif

#if XSYSTEM35_SDL_VERSION == 3
#undef SDL_SwapLE16
#undef SDL_SwapLE32
#undef SDL_PIXELFORMAT_RGB888
#define SDL_SwapLE16 SDL_Swap16LE
#define SDL_SwapLE32 SDL_Swap32LE
#define SDL_PIXELFORMAT_RGB888 SDL_PIXELFORMAT_XRGB8888
#endif

#if XSYSTEM35_SDL_VERSION == 2
typedef SDL_RWops sdl_iostream_t;
#else
typedef SDL_IOStream sdl_iostream_t;
#endif

static inline sdl_iostream_t *sdl_io_from_file(
	const char *path, const char *mode)
{
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_RWFromFile(path, mode);
#else
	return SDL_IOFromFile(path, mode);
#endif
}

static inline sdl_iostream_t *sdl_io_from_const_memory(
	const void *data, size_t size)
{
#if XSYSTEM35_SDL_VERSION == 2
	if (size > INT_MAX) {
		SDL_SetError("Memory stream is too large");
		return NULL;
	}
	return SDL_RWFromConstMem(data, (int)size);
#else
	return SDL_IOFromConstMem(data, size);
#endif
}

static inline int64_t sdl_get_io_size(sdl_iostream_t *stream)
{
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_RWsize(stream);
#else
	return SDL_GetIOSize(stream);
#endif
}

static inline size_t sdl_read_io(
	sdl_iostream_t *stream, void *destination, size_t bytes)
{
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_RWread(stream, destination, 1, bytes);
#else
	return SDL_ReadIO(stream, destination, bytes);
#endif
}

static inline bool sdl_close_io(sdl_iostream_t *stream)
{
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_RWclose(stream) == 0;
#else
	return SDL_CloseIO(stream);
#endif
}

static inline bool sdl_load_wav_io(sdl_iostream_t *stream, bool close_stream,
	SDL_AudioSpec *spec, uint8_t **buffer, uint32_t *length)
{
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_LoadWAV_RW(stream, close_stream ? 1 : 0,
		spec, buffer, length) != NULL;
#else
	return SDL_LoadWAV_IO(
		stream, close_stream, spec, buffer, length);
#endif
}

static inline void sdl_free_wav(uint8_t *buffer)
{
#if XSYSTEM35_SDL_VERSION == 2
	SDL_FreeWAV(buffer);
#else
	SDL_free(buffer);
#endif
}

static inline SDL_AudioFormat sdl_audio_s16le_format(void)
{
#if XSYSTEM35_SDL_VERSION == 2
	return AUDIO_S16LSB;
#else
	return SDL_AUDIO_S16LE;
#endif
}

static inline bool sdl_get_current_display_mode(SDL_DisplayMode *mode)
{
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_GetCurrentDisplayMode(0, mode) == 0;
#else
	const SDL_DisplayMode *current =
		SDL_GetCurrentDisplayMode(SDL_GetPrimaryDisplay());
	if (!current)
		return false;
	*mode = *current;
	return true;
#endif
}

static inline bool sdl_get_desktop_display_mode(SDL_DisplayMode *mode)
{
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_GetDesktopDisplayMode(0, mode) == 0;
#else
	const SDL_DisplayMode *desktop =
		SDL_GetDesktopDisplayMode(SDL_GetPrimaryDisplay());
	if (!desktop)
		return false;
	*mode = *desktop;
	return true;
#endif
}

static inline SDL_Window *sdl_create_window(const char *title, int width,
	int height, SDL_WindowFlags flags)
{
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED, width, height, flags);
#else
	float scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
	if (scale <= 0.0f)
		scale = 1.0f;
	width = (int)(width * scale + 0.5f);
	height = (int)(height * scale + 0.5f);
#ifndef __EMSCRIPTEN__
	flags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;
#endif
	return SDL_CreateWindow(title, width, height, flags);
#endif
}

static inline void sdl_set_window_content_size(
	SDL_Window *window, int width, int height)
{
#if XSYSTEM35_SDL_VERSION == 2
	SDL_SetWindowSize(window, width, height);
#else
	float pixel_density = SDL_GetWindowPixelDensity(window);
	float scale = SDL_GetWindowDisplayScale(window);
	if (pixel_density > 0.0f && scale > 0.0f)
		scale /= pixel_density;
	else
		scale = 1.0f;
	SDL_SetWindowSize(window, (int)(width * scale + 0.5f),
		(int)(height * scale + 0.5f));
#endif
}

static inline bool sdl_set_window_fullscreen(
	SDL_Window *window, bool fullscreen)
{
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_SetWindowFullscreen(window,
		fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0) == 0;
#else
	return SDL_SetWindowFullscreen(window, fullscreen);
#endif
}

static inline SDL_Renderer *sdl_create_renderer(SDL_Window *window)
{
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_CreateRenderer(window, -1, 0);
#else
	return SDL_CreateRenderer(window, NULL);
#endif
}

static inline bool sdl_set_render_logical_presentation(SDL_Renderer *renderer,
	int width, int height, bool integer_scaling)
{
#if XSYSTEM35_SDL_VERSION == 2
	if (SDL_RenderSetLogicalSize(renderer, width, height) < 0)
		return false;
	return SDL_RenderSetIntegerScale(renderer,
		integer_scaling ? SDL_TRUE : SDL_FALSE) == 0;
#else
	return SDL_SetRenderLogicalPresentation(renderer, width, height,
		integer_scaling ? SDL_LOGICAL_PRESENTATION_INTEGER_SCALE :
		SDL_LOGICAL_PRESENTATION_LETTERBOX);
#endif
}

static inline SDL_Point sdl_render_coordinates_to_window(
	SDL_Renderer *renderer, int x, int y)
{
	SDL_Point point;
#if XSYSTEM35_SDL_VERSION == 2
	SDL_RenderLogicalToWindow(renderer, x, y, &point.x, &point.y);
#else
	float window_x = x;
	float window_y = y;
	SDL_RenderCoordinatesToWindow(renderer, x, y, &window_x, &window_y);
	point.x = (int)SDL_lroundf(window_x);
	point.y = (int)SDL_lroundf(window_y);
#endif
	return point;
}

static inline bool sdl_render_texture(SDL_Renderer *renderer,
	SDL_Texture *texture, const SDL_Rect *source, const SDL_Rect *destination)
{
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_RenderCopy(renderer, texture, source, destination) == 0;
#else
	SDL_FRect source_float;
	SDL_FRect destination_float;
	const SDL_FRect *source_ptr = NULL;
	const SDL_FRect *destination_ptr = NULL;
	if (source) {
		source_float = (SDL_FRect){source->x, source->y,
			source->w, source->h};
		source_ptr = &source_float;
	}
	if (destination) {
		destination_float = (SDL_FRect){destination->x, destination->y,
			destination->w, destination->h};
		destination_ptr = &destination_float;
	}
	return SDL_RenderTexture(
		renderer, texture, source_ptr, destination_ptr);
#endif
}

static inline bool sdl_render_texture_float(SDL_Renderer *renderer,
	SDL_Texture *texture, const SDL_Rect *source, const SDL_FRect *destination)
{
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_RenderCopyF(renderer, texture, source, destination) == 0;
#else
	SDL_FRect source_float;
	const SDL_FRect *source_ptr = NULL;
	if (source) {
		source_float = (SDL_FRect){source->x, source->y,
			source->w, source->h};
		source_ptr = &source_float;
	}
	return SDL_RenderTexture(renderer, texture, source_ptr, destination);
#endif
}

static inline bool sdl_render_texture_rotated(SDL_Renderer *renderer,
	SDL_Texture *texture, const SDL_Rect *source, const SDL_Rect *destination,
	double angle, const SDL_Point *center, int flip)
{
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_RenderCopyEx(renderer, texture, source, destination, angle,
		center, (SDL_RendererFlip)flip) == 0;
#else
	SDL_FRect source_float;
	SDL_FRect destination_float;
	SDL_FPoint center_float;
	const SDL_FRect *source_ptr = NULL;
	const SDL_FRect *destination_ptr = NULL;
	const SDL_FPoint *center_ptr = NULL;
	if (source) {
		source_float = (SDL_FRect){source->x, source->y,
			source->w, source->h};
		source_ptr = &source_float;
	}
	if (destination) {
		destination_float = (SDL_FRect){destination->x, destination->y,
			destination->w, destination->h};
		destination_ptr = &destination_float;
	}
	if (center) {
		center_float = (SDL_FPoint){center->x, center->y};
		center_ptr = &center_float;
	}
	return SDL_RenderTextureRotated(renderer, texture, source_ptr,
		destination_ptr, angle, center_ptr, (SDL_FlipMode)flip);
#endif
}

static inline bool sdl_render_fill_rect(
	SDL_Renderer *renderer, const SDL_Rect *rect)
{
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_RenderFillRect(renderer, rect) == 0;
#else
	SDL_FRect float_rect;
	const SDL_FRect *rect_ptr = NULL;
	if (rect) {
		float_rect = (SDL_FRect){rect->x, rect->y, rect->w, rect->h};
		rect_ptr = &float_rect;
	}
	return SDL_RenderFillRect(renderer, rect_ptr);
#endif
}

static inline bool sdl_render_line(SDL_Renderer *renderer,
	float x1, float y1, float x2, float y2)
{
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_RenderDrawLineF(renderer, x1, y1, x2, y2) == 0;
#else
	return SDL_RenderLine(renderer, x1, y1, x2, y2);
#endif
}

static inline bool sdl_render_line_int(SDL_Renderer *renderer,
	int x1, int y1, int x2, int y2)
{
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_RenderDrawLine(renderer, x1, y1, x2, y2) == 0;
#else
	return SDL_RenderLine(renderer, x1, y1, x2, y2);
#endif
}

static inline bool sdl_get_texture_size(
	SDL_Texture *texture, int *width, int *height)
{
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_QueryTexture(texture, NULL, NULL, width, height) == 0;
#else
	float float_width;
	float float_height;
	if (!SDL_GetTextureSize(texture, &float_width, &float_height))
		return false;
	if (width)
		*width = (int)float_width;
	if (height)
		*height = (int)float_height;
	return true;
#endif
}

static inline bool sdl_render_target_supported(SDL_Renderer *renderer)
{
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_RenderTargetSupported(renderer) == SDL_TRUE;
#else
	(void)renderer;
	return true;
#endif
}

static inline bool sdl_set_texture_scale_mode_nearest(SDL_Texture *texture)
{
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_SetTextureScaleMode(texture, SDL_ScaleModeNearest) == 0;
#else
	return SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
#endif
}

static inline bool sdl_set_render_clip_rect(
	SDL_Renderer *renderer, const SDL_Rect *rect)
{
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_RenderSetClipRect(renderer, rect) == 0;
#else
	return SDL_SetRenderClipRect(renderer, rect);
#endif
}

#if XSYSTEM35_SDL_VERSION == 2
static inline SDL_Color sdl_vertex_color(
	uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
	return (SDL_Color){red, green, blue, alpha};
}
#else
static inline SDL_FColor sdl_vertex_color(
	uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
	return (SDL_FColor){red / 255.0f, green / 255.0f,
		blue / 255.0f, alpha / 255.0f};
}
#endif

static inline uint32_t sdl_surface_format(const SDL_Surface *surface)
{
#if XSYSTEM35_SDL_VERSION == 2
	return surface->format->format;
#else
	return surface->format;
#endif
}

static inline int sdl_surface_bits_per_pixel(const SDL_Surface *surface)
{
	return SDL_BITSPERPIXEL(sdl_surface_format(surface));
}

static inline int sdl_surface_bytes_per_pixel(const SDL_Surface *surface)
{
	return SDL_BYTESPERPIXEL(sdl_surface_format(surface));
}

static inline uint32_t sdl_surface_alpha_mask(const SDL_Surface *surface)
{
#if XSYSTEM35_SDL_VERSION == 2
	return surface->format->Amask;
#else
	return SDL_GetPixelFormatDetails(surface->format)->Amask;
#endif
}

static inline SDL_Palette *sdl_get_surface_palette(SDL_Surface *surface)
{
#if XSYSTEM35_SDL_VERSION == 2
	return surface->format->palette;
#else
	return SDL_GetSurfacePalette(surface);
#endif
}

static inline SDL_Surface *sdl_create_surface(
	int width, int height, int depth, uint32_t format)
{
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_CreateRGBSurfaceWithFormat(0, width, height, depth, format);
#else
	(void)depth;
	SDL_Surface *surface = SDL_CreateSurface(width, height, format);
	if (surface && format == SDL_PIXELFORMAT_INDEX8 &&
	    !SDL_CreateSurfacePalette(surface)) {
		SDL_DestroySurface(surface);
		return NULL;
	}
	return surface;
#endif
}

static inline SDL_Surface *sdl_create_surface_from(void *pixels,
	int width, int height, int depth, int pitch, uint32_t format)
{
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_CreateRGBSurfaceWithFormatFrom(
		pixels, width, height, depth, pitch, format);
#else
	(void)depth;
	SDL_Surface *surface = SDL_CreateSurfaceFrom(
		width, height, format, pixels, pitch);
	if (surface && format == SDL_PIXELFORMAT_INDEX8 &&
	    !SDL_CreateSurfacePalette(surface)) {
		SDL_DestroySurface(surface);
		return NULL;
	}
	return surface;
#endif
}

static inline SDL_Surface *sdl_create_surface_with_masks(int width, int height,
	int depth, uint32_t red, uint32_t green, uint32_t blue, uint32_t alpha)
{
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_CreateRGBSurface(
		0, width, height, depth, red, green, blue, alpha);
#else
	return SDL_CreateSurface(width, height,
		SDL_GetPixelFormatForMasks(depth, red, green, blue, alpha));
#endif
}

static inline void sdl_destroy_surface(SDL_Surface *surface)
{
#if XSYSTEM35_SDL_VERSION == 2
	SDL_FreeSurface(surface);
#else
	SDL_DestroySurface(surface);
#endif
}

static inline SDL_Palette *sdl_create_palette(int colors)
{
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_AllocPalette(colors);
#else
	return SDL_CreatePalette(colors);
#endif
}

static inline void sdl_destroy_palette(SDL_Palette *palette)
{
#if XSYSTEM35_SDL_VERSION == 2
	SDL_FreePalette(palette);
#else
	SDL_DestroyPalette(palette);
#endif
}

static inline bool sdl_set_surface_palette(
	SDL_Surface *surface, SDL_Palette *palette)
{
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_SetSurfacePalette(surface, palette) == 0;
#else
	return SDL_SetSurfacePalette(surface, palette);
#endif
}

static inline bool sdl_set_surface_color_key(
	SDL_Surface *surface, bool enabled, uint32_t key)
{
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_SetColorKey(surface, enabled ? SDL_TRUE : SDL_FALSE, key) == 0;
#else
	return SDL_SetSurfaceColorKey(surface, enabled, key);
#endif
}

static inline uint32_t sdl_map_rgb(
	SDL_Surface *surface, uint8_t red, uint8_t green, uint8_t blue)
{
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_MapRGB(surface->format, red, green, blue);
#else
	return SDL_MapRGB(SDL_GetPixelFormatDetails(surface->format),
		SDL_GetSurfacePalette(surface), red, green, blue);
#endif
}

static inline uint32_t sdl_map_rgba(SDL_Surface *surface, uint8_t red,
	uint8_t green, uint8_t blue, uint8_t alpha)
{
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_MapRGBA(surface->format, red, green, blue, alpha);
#else
	return SDL_MapRGBA(SDL_GetPixelFormatDetails(surface->format),
		SDL_GetSurfacePalette(surface), red, green, blue, alpha);
#endif
}

static inline uint32_t sdl_map_rgba_format(uint32_t format, uint8_t red,
	uint8_t green, uint8_t blue, uint8_t alpha)
{
#if XSYSTEM35_SDL_VERSION == 2
	SDL_PixelFormat *details = SDL_AllocFormat(format);
	uint32_t pixel = SDL_MapRGBA(details, red, green, blue, alpha);
	SDL_FreeFormat(details);
	return pixel;
#else
	return SDL_MapRGBA(
		SDL_GetPixelFormatDetails(format), NULL, red, green, blue, alpha);
#endif
}

static inline void sdl_get_rgb(uint32_t pixel, SDL_Surface *surface,
	uint8_t *red, uint8_t *green, uint8_t *blue)
{
#if XSYSTEM35_SDL_VERSION == 2
	SDL_GetRGB(pixel, surface->format, red, green, blue);
#else
	SDL_GetRGB(pixel, SDL_GetPixelFormatDetails(surface->format),
		SDL_GetSurfacePalette(surface), red, green, blue);
#endif
}

static inline void sdl_get_rgba(uint32_t pixel, SDL_Surface *surface,
	uint8_t *red, uint8_t *green, uint8_t *blue, uint8_t *alpha)
{
#if XSYSTEM35_SDL_VERSION == 2
	SDL_GetRGBA(pixel, surface->format, red, green, blue, alpha);
#else
	SDL_GetRGBA(pixel, SDL_GetPixelFormatDetails(surface->format),
		SDL_GetSurfacePalette(surface), red, green, blue, alpha);
#endif
}

static inline SDL_Surface *sdl_convert_surface(
	SDL_Surface *surface, uint32_t format)
{
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_ConvertSurfaceFormat(surface, format, 0);
#else
	return SDL_ConvertSurface(surface, format);
#endif
}

static inline bool sdl_fill_surface_rect(
	SDL_Surface *surface, const SDL_Rect *rect, uint32_t color)
{
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_FillRect(surface, rect, color) == 0;
#else
	return SDL_FillSurfaceRect(surface, rect, color);
#endif
}

static inline bool sdl_fill_surface_rects(SDL_Surface *surface,
	const SDL_Rect *rects, int count, uint32_t color)
{
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_FillRects(surface, rects, count, color) == 0;
#else
	return SDL_FillSurfaceRects(surface, rects, count, color);
#endif
}

static inline bool sdl_blit_surface(SDL_Surface *source,
	const SDL_Rect *source_rect, SDL_Surface *destination,
	SDL_Rect *destination_rect)
{
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_BlitSurface(
		source, source_rect, destination, destination_rect) == 0;
#else
	bool result = SDL_BlitSurface(
		source, source_rect, destination, destination_rect);
	if (!result || !destination_rect)
		return result;

	SDL_Rect clipped_source = {0, 0, source->w, source->h};
	SDL_Rect clipped_destination = {
		destination_rect->x, destination_rect->y, 0, 0};
	if (source_rect) {
		SDL_Rect intersection;
		if (!SDL_GetRectIntersection(
				source_rect, &clipped_source, &intersection))
			goto empty;
		clipped_destination.x += intersection.x - source_rect->x;
		clipped_destination.y += intersection.y - source_rect->y;
		clipped_source = intersection;
	}

	clipped_destination.w = clipped_source.w;
	clipped_destination.h = clipped_source.h;
	SDL_Rect destination_clip;
	if (!SDL_GetSurfaceClipRect(destination, &destination_clip))
		return false;
	if (!SDL_GetRectIntersection(&clipped_destination,
			&destination_clip, &clipped_destination))
		goto empty;

	*destination_rect = clipped_destination;
	return true;

empty:
	destination_rect->w = 0;
	destination_rect->h = 0;
	return true;
#endif
}

static inline bool sdl_blit_surface_unchecked(SDL_Surface *source,
	SDL_Rect *source_rect, SDL_Surface *destination, SDL_Rect *destination_rect)
{
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_LowerBlit(
		source, source_rect, destination, destination_rect) == 0;
#else
	return SDL_BlitSurfaceUnchecked(
		source, source_rect, destination, destination_rect);
#endif
}

static inline bool sdl_blit_surface_scaled(SDL_Surface *source,
	const SDL_Rect *source_rect, SDL_Surface *destination,
	SDL_Rect *destination_rect)
{
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_BlitScaled(
		source, source_rect, destination, destination_rect) == 0;
#else
	return SDL_BlitSurfaceScaled(source, source_rect, destination,
		destination_rect, SDL_SCALEMODE_NEAREST);
#endif
}

static inline bool sdl_stretch_surface(SDL_Surface *source,
	const SDL_Rect *source_rect, SDL_Surface *destination,
	const SDL_Rect *destination_rect)
{
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_SoftStretch(
		source, source_rect, destination, destination_rect) == 0;
#else
	return SDL_StretchSurface(source, source_rect, destination,
		destination_rect, SDL_SCALEMODE_NEAREST);
#endif
}

static inline bool sdl_get_rect_intersection(const SDL_Rect *a,
	const SDL_Rect *b, SDL_Rect *result)
{
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_IntersectRect(a, b, result) == SDL_TRUE;
#else
	return SDL_GetRectIntersection(a, b, result);
#endif
}

static inline void sdl_get_rect_union(
	const SDL_Rect *a, const SDL_Rect *b, SDL_Rect *result)
{
#if XSYSTEM35_SDL_VERSION == 2
	SDL_UnionRect(a, b, result);
#else
	SDL_GetRectUnion(a, b, result);
#endif
}

/* Keep call sites readable while routing renamed SDL2 surface APIs here. */
#undef SDL_FillRect
#undef SDL_FillRects
#undef SDL_BlitSurface
#undef SDL_LowerBlit
#undef SDL_BlitScaled
#undef SDL_SoftStretch
#undef SDL_IntersectRect
#undef SDL_UnionRect
#define SDL_FillRect sdl_fill_surface_rect
#define SDL_FillRects sdl_fill_surface_rects
#define SDL_BlitSurface sdl_blit_surface
#define SDL_LowerBlit sdl_blit_surface_unchecked
#define SDL_BlitScaled sdl_blit_surface_scaled
#define SDL_SoftStretch sdl_stretch_surface
#define SDL_IntersectRect sdl_get_rect_intersection
#define SDL_UnionRect sdl_get_rect_union

#if XSYSTEM35_SDL_VERSION == 2
typedef SDL_mutex sdl_mutex_t;
typedef SDL_cond sdl_condition_t;
typedef SDL_atomic_t sdl_atomic_int_t;
#else
typedef SDL_Mutex sdl_mutex_t;
typedef SDL_Condition sdl_condition_t;
typedef SDL_AtomicInt sdl_atomic_int_t;
#endif

static inline sdl_mutex_t *sdl_create_mutex(void)
{
	return SDL_CreateMutex();
}

static inline void sdl_destroy_mutex(sdl_mutex_t *mutex)
{
	SDL_DestroyMutex(mutex);
}

static inline void sdl_lock_mutex(sdl_mutex_t *mutex)
{
	SDL_LockMutex(mutex);
}

static inline void sdl_unlock_mutex(sdl_mutex_t *mutex)
{
	SDL_UnlockMutex(mutex);
}

static inline sdl_condition_t *sdl_create_condition(void)
{
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_CreateCond();
#else
	return SDL_CreateCondition();
#endif
}

static inline void sdl_destroy_condition(sdl_condition_t *condition)
{
#if XSYSTEM35_SDL_VERSION == 2
	SDL_DestroyCond(condition);
#else
	SDL_DestroyCondition(condition);
#endif
}

static inline void sdl_signal_condition(sdl_condition_t *condition)
{
#if XSYSTEM35_SDL_VERSION == 2
	SDL_CondSignal(condition);
#else
	SDL_SignalCondition(condition);
#endif
}

static inline void sdl_wait_condition(
	sdl_condition_t *condition, sdl_mutex_t *mutex)
{
#if XSYSTEM35_SDL_VERSION == 2
	SDL_CondWait(condition, mutex);
#else
	SDL_WaitCondition(condition, mutex);
#endif
}

static inline bool sdl_wait_condition_timeout(sdl_condition_t *condition,
	sdl_mutex_t *mutex, uint32_t timeout_ms)
{
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_CondWaitTimeout(condition, mutex, timeout_ms) == 0;
#else
	return SDL_WaitConditionTimeout(condition, mutex, timeout_ms);
#endif
}

static inline bool sdl_compare_and_swap_atomic_int(
	sdl_atomic_int_t *value, int expected, int desired)
{
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_AtomicCAS(value, expected, desired) == SDL_TRUE;
#else
	return SDL_CompareAndSwapAtomicInt(value, expected, desired);
#endif
}

static inline int sdl_set_atomic_int(sdl_atomic_int_t *value, int desired)
{
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_AtomicSet(value, desired);
#else
	return SDL_SetAtomicInt(value, desired);
#endif
}

static inline int sdl_get_atomic_int(sdl_atomic_int_t *value)
{
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_AtomicGet(value);
#else
	return SDL_GetAtomicInt(value);
#endif
}

#endif /* XSYSTEM35_SDL_COMPAT_H */
