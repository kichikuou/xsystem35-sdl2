/*
 * cg.c   load and display cg
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
/* $Id: cg.c,v 1.12 2001/09/16 15:59:11 chikama Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include "portab.h"
#include "system.h"
#include "nact.h"
#include "ags.h"
#include "cg.h"
#include "vsp.h"
#include "pms.h"
#include "bmp.h"
#include "qnt.h"
#include "jpeg.h"
#ifdef HAVE_WEBP
#include "webp.h"
#endif
#include "ald_manager.h"
#include "filecheck.h"
#include "cache.h"

/* VSPのパレット展開バンク */
int cg_vspPB;
/* cg,palette 展開フラグ (funciotn flag) */
int cg_fflg;
/* CGをロードした回数を書き込む変数 */
int *cg_loadCountVar;
/* Brightness (PD command) */
int cg_brightness;

#define GCMD_EXTRACTCG(c)    ((c) & 0x01)
#define GCMD_SET_PALETTE(c)  ((c) & 0x02)
#define GCMD_LOAD_PALETTE(c) ((c) & 0x04)

/* CG表示位置に関する情報 */
static CG_WHERETODISP loc_policy, loc_policy0;
static SDL_Point        loc_where, loc_where0;

/* extracted cg data cache control object */
static cacher *cacheid;

/* static methods */
static CG_TYPE check_cgformat(uint8_t *data);
static void set_vspbank(uint8_t *pic, int bank, int width, int height);
static SDL_Point set_display_loc(cgdata *cg);
static void clear_display_loc();
static void display_cg(cgdata *cg, int x, int y, int sprite_color, bool alpha_blend);
static cgdata *loader(int no);


/*
 * Identify cg format
 *   data: pointer to compressed data
 *   return: cg type 
*/
static CG_TYPE check_cgformat(uint8_t *data) {
	if (qnt_checkfmt(data)) {
		return ALCG_QNT;
	} else if (pms256_checkfmt(data)) {
		return ALCG_PMS8;
	} else if (pms64k_checkfmt(data) && nact->ags.world_depth >= 15) {
		return ALCG_PMS16;
	} else if (bmp16m_checkfmt(data) && nact->ags.world_depth >= 15) {
		return ALCG_BMP24;
	} else if (bmp256_checkfmt(data)) {
		return ALCG_BMP8;
	} else if (vsp_checkfmt(data)) {
		return ALCG_VSP;
	} else if (jpeg_checkfmt(data) && nact->ags.world_depth >= 15) {
		return ALCG_JPEG;
#ifdef HAVE_WEBP
	} else if (webp_checkfmt(data)) {
		return ALCG_WEBP;
#endif
	}
	WARNING("Unknown Cg Type");
	return ALCG_UNKNOWN;
}

/*
 * Modify pixel accoding to palette bank (vsp only)
 *   pic   : pixel to be modifyied.
 *   bank  : palette bank (use only MSB 4bit)
 *   width : image width
 *   height: image height 
*/
static void set_vspbank(uint8_t *pic, int bank, int width, int height) {
	int pixels = width * height;
	
	while (pixels--) {
		*pic = (*pic & 0x0f) | (uint8_t)bank; pic++;
	}
}

/*
 * Free data 
 *  cg: freeing data object
*/
void cgdata_free(cgdata *cg) {
	if (cg->pic) free(cg->pic);
	if (cg->pal) free(cg->pal);
	if (cg->alpha) free(cg->alpha);
	free(cg);
}

/*
 * Determine the location of display image
 *  cg: cg information
 *  return: x and y for display location
*/
static SDL_Point set_display_loc(cgdata *cg) {
	SDL_Point p;
	
	switch(loc_policy) {
	case OFFSET_ABSOLUTE_GC:
	case OFFSET_ABSOLUTE_JC:
		p.x = loc_where.x;
		p.y = loc_where.y;
		break;
	case OFFSET_RELATIVE_GC:
	case OFFSET_RELATIVE_JC:
		p.x = cg->x + loc_where.x;
		p.y = cg->y + loc_where.y;
		break;
	case OFFSET_NOMOVE:
		p.x = cg->x;
		p.y = cg->y;
		break;
	}
	return p;
}

/*
 * Reset location movement according to loc_policy
*/
static void clear_display_loc() {
	switch(loc_policy) {
	case OFFSET_ABSOLUTE_GC:
	case OFFSET_RELATIVE_GC:
		if (loc_policy0 == OFFSET_ABSOLUTE_JC ||
		    loc_policy0 == OFFSET_RELATIVE_JC ) {
			loc_policy  = loc_policy0;
			loc_where   = loc_where0;
			loc_policy0 = OFFSET_NOMOVE;
		} else {
			loc_policy = OFFSET_NOMOVE;
		}
		break;
	default:
		break;
	}
}

/*
 * Call ags to display cg
 *  cg: cg to be drawn
 *  x : display location x
 *  y : display location y
*/ 
static void display_cg(cgdata *cg, int x, int y, int sprite_color, bool alpha_blend) {
	ags_drawCg(cg, x, y, cg_brightness, sprite_color, alpha_blend);
	ags_updateArea(x, y, cg->width, cg->height);
}

/*
 * Load cg data from file or cache
 *  no: file no ( >= 0)
 *  return: cg object(extracted)
*/
static cgdata *loader(int no) {
	dridata *dfile;
	cgdata *cg = NULL;

	/* search in cache */
	if (NULL != (cg = (cgdata *)cache_foreach(cacheid, no))) return cg;
	
	/* read from file */
	if (NULL == (dfile = ald_getdata(DRIFILE_CG, no))) return NULL;
	
	/* update load cg counter */
	if (cg_loadCountVar != NULL) {
		(*(cg_loadCountVar + no + 1))++;
	}
	
	/* extract cg */
	switch (check_cgformat(dfile->data)) {
	case ALCG_VSP:
		cg = vsp_extract(dfile->data);
		break;
	case ALCG_PMS8:
		cg = pms256_extract(dfile->data);
		break;
	case ALCG_PMS16:
		cg = pms64k_extract(dfile->data);
		break;
	case ALCG_BMP8:
		cg = bmp256_extract(dfile->data);
		break;
	case ALCG_BMP24:
		cg = bmp16m_extract(dfile->data);
		break;
	case ALCG_QNT:
		cg = qnt_extract(dfile->data);
		break;
	case ALCG_JPEG:
		cg = jpeg_extract(dfile->data, dfile->size);
		break;
	default:
		break;
	}
	/* insert to cache */
	if (cg) {
		int size = cg->width * cg->height * (cg->depth / 8);
		cache_insert(cacheid, no, cg, size, NULL);
	}
	
	/* ok to free */
	ald_freedata(dfile);
	
	return cg;
}

/*
 * Initilize cache
*/
void cg_init(void) {
	cacheid = cache_new(cgdata_free);
	cg_reset();
}

void cg_reset(void) {
	cg_vspPB = -1;
	cg_fflg = 7;
	cg_loadCountVar = NULL;
	cg_brightness = 255;
	loc_policy = loc_policy0 = OFFSET_NOMOVE;
	memset(&loc_where, 0, sizeof(loc_where));
	memset(&loc_where0, 0, sizeof(loc_where0));
}

/*
 * Set cg display location
 *  x     : display location x
 *  y     : display location y
 *  policy: location offset policy
*/
void cg_set_display_location(int x, int y, CG_WHERETODISP policy) {
	if (policy == OFFSET_ABSOLUTE_GC || policy == OFFSET_RELATIVE_GC) {
		if (loc_policy == OFFSET_ABSOLUTE_JC ||
		    loc_policy == OFFSET_RELATIVE_JC) {
			loc_policy0 = loc_policy;
			loc_where0  = loc_where;
		}
	}
	
	loc_policy = policy;
	loc_where.x = x;
	loc_where.y = y;
}

/*
 * Load and display cg
 *   no : file no ( >= 0)
 *   flg: sprite color(!-1)
*/
void cg_load(int no, int flg) {
	cgdata *cg;
	SDL_Point p;
	int i, bank = cg_vspPB;
	
	/* load and extract cg */
	if (NULL == (cg = loader(no))) {
		return;
	}
	
	/* need to set vsp bank or not */
	if (GCMD_EXTRACTCG(cg_fflg) && cg->type == ALCG_VSP) {
		bank = cg_vspPB == -1 ? cg->vsp_bank : cg_vspPB;
		set_vspbank(cg->pic, bank << 4, cg->width, cg->height);
		/* copy palettes 0 -> bank */
		if (bank != 0) {
			memcpy(cg->pal + bank * 16, cg->pal, 16 * sizeof(cg->pal[0]));
		}
		if (flg != -1) {
			flg |= (bank << 4);
		}
	}
        
	/* copy palette to system */
	if (GCMD_LOAD_PALETTE(cg_fflg)) {
		switch(cg->type) {
		case ALCG_VSP:
			ags_setPalettes(cg->pal, bank << 4, 16);
			break;
		case ALCG_PMS8:
			// Avoid changing the Windows' reserved palette area (0-9, 246-255)
			if (cg->pms_bank & 1)
				ags_setPalettes(cg->pal + 10, 10,  6);
			if (cg->pms_bank & (1 << 15))
				ags_setPalettes(cg->pal + 240, 240, 6);
			for (i = 1; i < 15; i++) {
				if (cg->pms_bank & (1 << i)) {
					ags_setPalettes(cg->pal + i * 16, i * 16, 16);
				}
			}
			break;
		case ALCG_BMP8:
			ags_setPalettes(cg->pal + 10, 10, 236);
			break;
		default:
			break;
		}
	}
	
	/* refrect palette change */
	if (GCMD_SET_PALETTE(cg_fflg) && cg->depth == 8)
		ags_setPaletteToSystem(0, 256);
	
	/* draw cg */
	if (GCMD_EXTRACTCG(cg_fflg)) {
		/* set display offset */
		p = set_display_loc(cg);
		/* draw alpha pixel */
		if (cg->alpha) {
			ags_alpha_setPixel(p.x, p.y, cg->width, cg->height, cg->alpha);
		}
		/* draw cg pixel */
		display_cg(cg, p.x, p.y, flg, flg != -1);
	}
	/* clear display offset */
	clear_display_loc();
}

/*
 * Load and display cg with alpha
 *   cgno    : file no for cg ( >= 0 or -1(alpha only))
 *   shadowno: file no for alpha ( >= 0 )
*/
void cg_load_with_alpha(int cgno, int shadowno) {
	cgdata *cg = NULL, *scg;
	SDL_Point p;
	
	/* load pixel */
	if (cgno >= 0) {
		if (NULL == (cg = loader(cgno))) return;
		if (cg->type != ALCG_PMS16) {
			WARNING("commandGX cg_no != 16bitPMS");
			return;
		}
	}
	
	/* load alpha pixel */
	if (NULL == (scg = loader(shadowno))) return;
	if (scg->type != ALCG_PMS8) {
		WARNING("commandGX shadow_no != 8bitPMS");
		return;
	}
	
	/* set alpha pixel offset */
	p = set_display_loc(scg);
	/* draw alpha pixel */
	ags_alpha_setPixel(p.x, p.y, scg->width, scg->height, scg->pic);
	
	/* draw pixel */
	if (cg) {
		p = set_display_loc(cg);
		display_cg(cg, p.x, p.y, -1, true);
	}
	
	/* clear display offset */
	clear_display_loc();
}

static uint8_t* load_cg_from_file(char *fname_utf8, int *status, long *filesize) {
	int size;
	FILE *fp;
	static uint8_t *tmp;

	*status = 0;

	if (NULL == (fp = fc_open(fname_utf8, 'r'))) {
		*status = SAVE_LOADERR; return NULL;
	}

	fseek(fp, 0L, SEEK_END);
	*filesize = ftell(fp);
	if (*filesize == 0) {
		*status = SAVE_LOADERR; return NULL;
	}

	tmp = (char *)malloc(*filesize);
	if (tmp == NULL) {
		WARNING("Out of memory");
		*status = SAVE_LOADERR; return NULL;
	}
	fseek(fp, 0L, SEEK_SET);
	size = fread(tmp, 1, *filesize,fp);

	if (size != *filesize) {
		*status = SAVE_LOADSHORTAGE;
	} else {
		*status = SAVE_LOADOK;
	}

	fclose(fp);
	return tmp;
}

/*
 * Load and display cg from file 'name' (not cached right now)
 *   name: file name to be read
 *   x   : display location x
 *   y   : display location y
 *   return: file read status
*/
int cg_load_with_filename(char *fname_utf8, int x, int y) {
	int status, type;
	long filesize;
	uint8_t *data;
	cgdata *cg = NULL;
	SDL_Point p;
	
	data = load_cg_from_file(fname_utf8, &status, &filesize);
	if (data == NULL) return status;
	
	cg_set_display_location(x, y, OFFSET_ABSOLUTE_GC);
	
	type = check_cgformat(data);
	switch(type) {
	case ALCG_BMP8:
		cg = bmp256_extract(data);
		break;
	case ALCG_BMP24:
		cg = bmp16m_extract(data);
		break;
	case ALCG_JPEG:
		cg = jpeg_extract(data, filesize);
		break;
	default:
		return status;
	}
	
	/* load palette if not extracted */
	if (cg->depth == 8) {
		if (GCMD_LOAD_PALETTE(cg_fflg))
			ags_setPalettes(cg->pal + 10, 10, 236);
		if (GCMD_SET_PALETTE(cg_fflg))
			ags_setPaletteToSystem(0, 256);
	}

	/* draw cg */
	if (GCMD_EXTRACTCG(cg_fflg)) {
		/* set display offset */
		p = set_display_loc(cg);
		/* draw cg pixel */
		display_cg(cg, p.x, p.y, -1, false);
		/* clear display offset */
		clear_display_loc();
	}

	cgdata_free(cg);
	return status;
}

/*
 * Get cg information 
 *   no  : file no for cg ( >= 0 )
 *   info: information to be retored
*/
void cg_get_info(int no, SDL_Rect *info) {
	cgdata *cg = loader(no);
	SDL_Point p;
	
	if (cg == NULL) {
		info->x = info->y = info->w = info->h = 0;
	} else {
		p = set_display_loc(cg);
		info->x = p.x;
		info->y = p.y;
		info->w = cg->width;
		info->h = cg->height;
	}
}

void cg_clear_display_loc() {
	clear_display_loc();
}

SDL_Surface *cg_load_as_sdlsurface(int no) {
	dridata *dfile = ald_getdata(DRIFILE_CG, no);
	if (!dfile) return NULL;

	CG_TYPE type = check_cgformat(dfile->data);
#ifdef HAVE_WEBP
	if (type == ALCG_WEBP) {
		SDL_Surface *sf = webp_extract(dfile->data, dfile->size);
		ald_freedata(dfile);
		return sf;
	}
#endif

	cgdata *cg = NULL;
	switch (type) {
	case ALCG_PMS16:
		cg = pms64k_extract(dfile->data);
		break;
	case ALCG_QNT:
		cg = qnt_extract(dfile->data);
		break;
	default:
		WARNING("Invalid CG type");
		break;
	}
	ald_freedata(dfile);
	if (!cg) return NULL;

	SDL_Surface *pic = type == ALCG_PMS16
		? SDL_CreateRGBSurfaceWithFormatFrom(cg->pic, cg->width, cg->height, 16, cg->width * 2, SDL_PIXELFORMAT_RGB565)
		: SDL_CreateRGBSurfaceWithFormatFrom(cg->pic, cg->width, cg->height, 24, cg->width * 3, SDL_PIXELFORMAT_RGB24);

	SDL_Surface *sf = SDL_CreateRGBSurfaceWithFormat(0, cg->width, cg->height, 32,
		cg->alpha ? SDL_PIXELFORMAT_ARGB8888 : SDL_PIXELFORMAT_XRGB8888);
	SDL_BlitSurface(pic, NULL, sf, NULL);

	if (cg->alpha) {
		// Copy alpha values from cg->alpha to sf->pixels.
		uint8_t *p_ds = ALPHA_AT(sf, 0, 0);
		uint8_t *adata = cg->alpha;
		for (int y = 0; y < cg->height; y++) {
			uint8_t *p_dst = p_ds;
			for (int x = 0; x < cg->width; x++) {
				*p_dst = *adata++;
				p_dst += 4;
			}
			p_ds += sf->pitch;
		}
	}

	SDL_FreeSurface(pic);
	cgdata_free(cg);
	return sf;
}
