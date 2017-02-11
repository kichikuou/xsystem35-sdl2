#include "config.h"

#include <SDL.h>
#include <SDL_ttf.h>

#include "portab.h"
#include "system.h"
#include "font.h"

typedef struct {
	int      size;
	int      type;
	TTF_Font *id;
} FontTable;

#define FONTTABLEMAX 256
static FontTable fonttbl[FONTTABLEMAX];
static int       fontcnt = 0;

static TTF_Font *fontset;

static FONT *this;

static void font_insert(int size, int type, ttfont *fontset) {
	fonttbl[fontcnt].size = size;
	fonttbl[fontcnt].type = type;
	fonttbl[fontcnt].id   = fontset;
	
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

static void font_ttf_sel_font(int type, int size) {
	FontTable *tbl;


	if (NULL == (tbl = font_lookup(size, type))) {
		TTF_Font *fs;
		
		fs = TTF_OpenFont(this->name[type], size);
		
		if (fs == NULL) {
			WARNING("%s is not found:\n", this->name[type]);
			return;
		}
		
		font_insert(size, type, fs);
		fontset = fs;
	} else {
		fontset = tbl->id;
	}
}

static void *font_sdlttf_get_glyph(unsigned char *msg) {
}

static int font_sdlttf_draw_glyph(int x, int y, unsigned char *str, int cl) {
	SDL_Surface *fs;
	SDL_Color fg;
	SDL_Rect r_src, r_dst;
	int w, h;
	
	fg = SDL_MapRGB(sdl_dib->format,
			sdl_col[cl].r, sdl_col[cl].g, sdl_col[cl].b);
	
	if (this->antialiase_on) {
		fs = TTF_RenderText_Shaded(fontset, str, cl);
	} else {
		fs = TTF_RenderText_Solid(fonset, str, cl);
	}
	
	TTF_SizeText(fontset, str, &w, &h);
	
	setRect(r_src, 0, 0, w, h);
	setRect(r_dst, x, y, w, h);
	
	SDL_LockSurface(sdl_dib);
	SDL_BlitSurface(fs, &r_src, sdl_dib, &r_dst);
	SDL_UnlockSurface(sdl_dib);
	
	return w;
}

static boolean drawable() {
	return TRUE;
}

FONT *font_ttf_new() {
	FONT *f = g_new(FONT, 1);
	
	f->sel_font   = font_sdlttf_sel_font;
	f->get_glyph  = font_sdlttf_get_glyph;
	f->draw_glyph = font_sdlttf_draw_glyph;
	f->self_drawable = drawable;
	f->antialiase_on = FALSE;
	
	for (i = 0; i < 4; i++) {
		fontmaps[i].etype = -1;
	}
	
	TTF_Init();
	
	this = f;
	
	return f;
}
