#ifndef XSYSTEM35_SDL_COMPAT_H
#define XSYSTEM35_SDL_COMPAT_H

#include <stdbool.h>
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
#undef SDL_LowerBlit
#undef SDL_BlitScaled
#undef SDL_SoftStretch
#undef SDL_IntersectRect
#undef SDL_UnionRect
#define SDL_FillRect sdl_fill_surface_rect
#define SDL_FillRects sdl_fill_surface_rects
#define SDL_LowerBlit sdl_blit_surface_unchecked
#define SDL_BlitScaled sdl_blit_surface_scaled
#define SDL_SoftStretch sdl_stretch_surface
#define SDL_IntersectRect sdl_get_rect_intersection
#define SDL_UnionRect sdl_get_rect_union

#endif /* XSYSTEM35_SDL_COMPAT_H */
