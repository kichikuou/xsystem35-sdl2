#include "config.h"

#include <limits.h>
#include <math.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include "portab.h"
#include "system.h"
#include "font.h"
#include "sdl_private.h"
#include "utfsjis.h"

typedef struct {
	int      size;
	int      type;
	TTF_Font *id;
} FontTable;

#define FONTTABLEMAX 256
static FontTable fonttbl[FONTTABLEMAX];
static int       fontcnt = 0;

static FontTable *fontset;

static FONT *this;

static void font_insert(int size, int type, TTF_Font *font) {
	fonttbl[fontcnt].size = size;
	fonttbl[fontcnt].type = type;
	fonttbl[fontcnt].id   = font;
	
	if (fontcnt >= (FONTTABLEMAX -1)) {
		WARNING("Font table is full.\n");
	} else {
		fontcnt++;
	}
}

static FontTable *font_lookup(int size, int type) {
	int i;
	
	for (i = 0; i < fontcnt; i++) {
		if (fonttbl[i].size == size && fonttbl[i].type == type) { 
			return &fonttbl[i];
		}
	}
	return NULL;
}

static void font_sdlttf_sel_font(int type, int size) {
	FontTable *tbl;


	if (NULL == (tbl = font_lookup(size, type))) {
		TTF_Font *fs;
		
		fs = TTF_OpenFontIndex(this->name[type], size, this->face[type]);
		
		if (fs == NULL) {
			WARNING("%s is not found:\n", this->name[type]);
			return;
		}
		
		font_insert(size, type, fs);
		fontset = &fonttbl[fontcnt - 1];
	} else {
		fontset = tbl;
	}
}

static agsurface_t *font_sdlttf_get_glyph(unsigned char *msg) {
	static SDL_Surface *fs;
	static agsurface_t result;

	if (fs) {
		SDL_FreeSurface(fs);
		fs = NULL;
	}

	BYTE* conv = sjis2lang(msg);

	SDL_Color color = {255, 255, 255, 0};
	if (this->antialiase_on) {
		fs = TTF_RenderUTF8_Shaded(fontset->id, conv, color, color);
	} else {
		fs = TTF_RenderUTF8_Solid(fontset->id, conv, color);
	}
	free(conv);
	if (!fs) {
		WARNING("Text rendering failed: %s\n", TTF_GetError());
		return NULL;
	}

	result.depth = fs->format->BitsPerPixel;
	result.bytes_per_pixel = fs->format->BytesPerPixel;
	result.bytes_per_line = fs->pitch;
	result.pixel = fs->pixels;
	result.width = fs->w;
	result.height = fs->h;
	return &result;
}

// SDL can't blit ARGB to an indexed bitmap properly, so we do it ourselves.
static void sdl_drawAntiAlias_8bpp(int dstx, int dsty, SDL_Surface *src, unsigned long col)
{
	SDL_LockSurface(sdl_dib);

	Uint8 cache[256*7];
	memset(cache, 0, 256);

	for (int y = 0; y < src->h && dsty + y < sdl_dib->h; y++) {
		BYTE *sp = (BYTE*)src->pixels + y * src->pitch;
		BYTE *dp = (BYTE*)sdl_dib->pixels + (dsty + y) * sdl_dib->pitch + dstx;
		for (int x = 0; x < src->w && dstx + x < sdl_dib->w; x++) {
			Uint8 r, g, b, alpha;
			SDL_GetRGBA(*((Uint32*)sp), src->format, &r, &g, &b, &alpha);
			alpha = alpha >> 5; // reduce bit depth
			if (!alpha) {
				// Transparent, do nothing
			} else if (alpha == 7) {
				*dp = col; // Fully opaque
			} else if (cache[*dp] & 1 << alpha) {
				*dp = cache[alpha << 8 | *dp]; // use cached value
			} else {
				// find nearest color in palette
				cache[*dp] |= 1 << alpha;
				int c = sdl_nearest_color(
					(sdl_col[col].r * alpha + sdl_col[*dp].r * (7 - alpha)) / 7,
					(sdl_col[col].g * alpha + sdl_col[*dp].g * (7 - alpha)) / 7,
					(sdl_col[col].b * alpha + sdl_col[*dp].b * (7 - alpha)) / 7);
				cache[alpha << 8 | *dp] = c;
				*dp = c;
			}
			sp += src->format->BytesPerPixel;
			dp++;
		}
	}

	SDL_UnlockSurface(sdl_dib);
}

static int font_sdlttf_draw_glyph(int x, int y, unsigned char *str, int cl) {
	SDL_Surface *fs;
	SDL_Rect r_src, r_dst;
	int w, h, maxy;
	BYTE *conv;
	
	if (!*str)
		return 0;
	
	conv = sjis2lang(str);
	if (this->antialiase_on) {
		fs = TTF_RenderUTF8_Blended(fontset->id, conv, sdl_col[cl]);
	} else {
		fs = TTF_RenderUTF8_Solid(fontset->id, conv, sdl_col[cl]);
	}
	if (!fs) {
		WARNING("Text rendering failed: %s\n", TTF_GetError());
		free(conv);
		return 0;
	}
	
	TTF_SizeUTF8(fontset->id, conv, &w, &h);
	y = max(0, y - (TTF_FontAscent(fontset->id) - fontset->size * 0.9));
	
	if (sdl_dib->format->BitsPerPixel == 8 && this->antialiase_on) {
		sdl_drawAntiAlias_8bpp(x, y, fs, cl);
	} else {
		setRect(r_src, 0, 0, w, h);
		setRect(r_dst, x, y, w, h);
		SDL_BlitSurface(fs, &r_src, sdl_dib, &r_dst);
	}

	SDL_FreeSurface(fs);
	free(conv);
	
	return w;
}

static boolean drawable() {
	return TRUE;
}

FONT *font_sdlttf_new() {
	FONT *f = malloc(sizeof(FONT));
	
	f->sel_font   = font_sdlttf_sel_font;
	f->get_glyph  = font_sdlttf_get_glyph;
	f->draw_glyph = font_sdlttf_draw_glyph;
	f->self_drawable = drawable;
	f->antialiase_on = FALSE;
	
	if (TTF_Init() == -1)
		SYSERROR("Failed to intialize SDL_ttf: %s\n", TTF_GetError());
	
	this = f;
	
	NOTICE("FontDevice SDL_ttf\n");
	
	return f;
}
