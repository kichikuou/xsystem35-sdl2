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
#include <glib.h>
#include "portab.h"
#include "system.h"
#include "graphics.h"
#include "nact.h"
#include "ags.h"
#include "cg.h"
#include "vsp.h"
#include "pms.h"
#include "bmp.h"
#include "qnt.h"
#include "ald_manager.h"
#include "savedata.h"
#include "cache.h"

/* VSPのパレット展開バンク */
int cg_vspPB = -1;
/* cg,pallet 展開フラグ (funciotn flag) */
int cg_fflg = 7;
/* CGをロードした回数を書き込む変数 */
int *cg_loadCountVar = NULL;
/* CGの明度指定 */
int cg_alphaLevel = 255;

#define GCMD_EXTRACTCG(c)    ((c) & 0x01)
#define GCMD_SET_PALLET(c)  ((c) & 0x02)
#define GCMD_LOAD_PALLET(c) ((c) & 0x04)

/* CG表示位置に関する情報 */
static CG_WHERETODISP loc_policy = OFFSET_NOMOVE, loc_policy0;
static MyPoint        loc_where, loc_where0;

/* extracted cg data cache control object */
static cacher *cacheid;

/* static methods */
static CG_TYPE check_cgformat(BYTE *data);
static void set_vspbank(BYTE *pic, int bank, int width, int height);
static void cgdata_free(cgdata *cg);
static MyPoint set_display_loc(cgdata *cg);
static void clear_display_loc();
static void display_cg(cgdata *cg, int x, int y);
static cgdata *loader(int no);


/*
 * Identify cg format
 *   data: pointer to compressed data
 *   return: cg type 
*/
static CG_TYPE check_cgformat(BYTE *data) {
	if (qnt_checkfmt(data)) {
		return ALCG_QNT;
	} else if (pms256_checkfmt(data)) {
		return ALCG_PMS8;
	} else if (pms64k_checkfmt(data) && nact->sys_world_depth >= 15) {
		return ALCG_PMS16;
	} else if (bmp16m_checkfmt(data) && nact->sys_world_depth >= 15) {
		return ALCG_BMP24;
	} else if (bmp256_checkfmt(data)) {
		return ALCG_BMP8;
	} else if (vsp_checkfmt(data)) {
		return ALCG_VSP;
	}
	WARNING("Unknown Cg Type\n");
	return ALCG_UNKNOWN;
}

/*
 * Modify pixel accoding to pallet bank (vsp only)
 *   pic   : pixel to be modifyied.
 *   bank  : pallet bank (use only MSB 4bit)
 *   width : image width
 *   height: image height 
*/
static void set_vspbank(BYTE *pic, int bank, int width, int height) {
	int pixels = width * height;
	
	while (pixels--) {
		*pic = (*pic & 0x0f) | (BYTE)bank; pic++;
	}
}

/*
 * Free data 
 *  cg: freeing data object
*/
static void cgdata_free(cgdata *cg) {
	if (cg->pic) g_free(cg->pic);
	if (cg->pal) g_free(cg->pal);
	if (cg->alpha) g_free(cg->alpha);
	g_free(cg);
}

/*
 * Determine the location of display image
 *  cg: cg information
 *  return: x and y for display location
*/
static MyPoint set_display_loc(cgdata *cg) {
	MyPoint p;
	
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
static void display_cg(cgdata *cg, int x, int y) {
	/* always set cg's alpha level */
	cg->alphalevel = cg_alphaLevel;
	
	/* draw cg */
	switch(cg->type) {
	case ALCG_VSP:
	case ALCG_PMS8:
	case ALCG_BMP8:
		ags_drawCg8bit(cg, x, y); break;
	case ALCG_PMS16:
	case ALCG_BMP24:
		ags_drawCg16bit(cg, x, y); break;
	default:
		break;
	}
	
	/* update drawn area */
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
	int type, size = 0;

	/* search in cache */
	if (NULL != (cg = (cgdata *)cache_foreach(cacheid, no))) return cg;
	
	/* read from file */
	if (NULL == (dfile = ald_getdata(DRIFILE_CG, no))) return NULL;
	
	/* update load cg counter */
	if (cg_loadCountVar != NULL) {
		(*(cg_loadCountVar + no + 1))++;
	}
	
	/* check loaded cg format */
	type = check_cgformat(dfile->data);
	
	/* extract cg */
	/*  size is only pixel data size */
	if (GCMD_EXTRACTCG(cg_fflg)) {
		switch(type) {
		case ALCG_VSP:
			cg = vsp_extract(dfile->data);
			size = cg->width * cg->height;
			break;
		case ALCG_PMS8:
			cg = pms256_extract(dfile->data);
			size = cg->width * cg->height;
			break;
		case ALCG_PMS16:
			cg = pms64k_extract(dfile->data);
			size = (cg->width * cg->height) * sizeof(WORD);
			break;
		case ALCG_BMP8:
			cg = bmp256_extract(dfile->data);
			size = cg->width * cg->height;
			break;
		case ALCG_BMP24:
			cg = bmp16m_extract(dfile->data);
			size = (cg->width * cg->height) * sizeof(WORD);
			break;
		case ALCG_QNT:
			cg = qnt_extract(dfile->data);
			size = (cg->width * cg->height) * 3;
			break;
		default:
			break;
		}
		/* insert to cache */
		if (cg)
			cache_insert(cacheid, no, cg, size, NULL);
	}
	
	/* load pallet if not extracted */
	if (GCMD_LOAD_PALLET(cg_fflg) && cg == NULL) {
		/* XXXX うむ、こいつらどこで解放するんだ */
		switch(type) {
		case ALCG_VSP:
			cg = vsp_getpal(dfile->data);
			break;
		case ALCG_PMS8:
			cg = pms_getpal(dfile->data);
			break;
		case ALCG_BMP8:
			cg = bmp_getpal(dfile->data);
			break;
		default:
			break;
		}
	}
	
	/* ok to free */
	ald_freedata(dfile);
	
	return cg;
}

/*
 * Initilize cache
*/
void cg_init() {
	cacheid = cache_new(cgdata_free);
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
	MyPoint p;
	int i, bank = cg_vspPB;
	
	/* load and extract cg */
	if (NULL == (cg = loader(no))) {
		return;
	}
	
	/* need to set vsp bank or not */
	if (GCMD_EXTRACTCG(cg_fflg) && cg->type == ALCG_VSP) {
		bank = cg_vspPB == -1 ? cg->vsp_bank : cg_vspPB;
		set_vspbank(cg->pic, bank << 4, cg->width, cg->height);
		/* copy pallets 0 -> bank */
		{
			int i, i_dst = bank << 4;
			for (i = 0; i < 16; i++) {
				cg->pal->red[i + i_dst]   = cg->pal->red[i];
				cg->pal->green[i + i_dst] = cg->pal->green[i];
				cg->pal->blue[i + i_dst]  = cg->pal->blue[i];
			}
		}
		if (flg != -1) {
			flg |= (bank << 4);
		}
	}
        
	/* copy pallet to system */
	if (GCMD_LOAD_PALLET(cg_fflg)) {
		switch(cg->type) {
		case ALCG_VSP:
			ags_setPallets(cg->pal, 0, bank << 4, 16);
			break;
		case ALCG_PMS8:
			if (cg->pms_bank & 1)
				ags_setPallets(cg->pal, 10, 10,  6);
			if (cg->pms_bank & (1 << 15))
				//ags_setPallets(cg->pal, 240, 240, 15);
				ags_setPallets(cg->pal, 240, 240, 10);
			for (i = 1; i < 15; i++) {
				if (cg->pms_bank & (1 << i)) {
					ags_setPallets(cg->pal, i * 16, i * 16, 16);
				}
			}
			break;
		case ALCG_BMP8:
			ags_setPallets(cg->pal, 10, 10, 236);
			break;
		default:
			break;
		}
		/* pallet load only */
		if (cg->pic == NULL) {
			cgdata_free(cg);
		}
	}
	
	/* refrect pallet change */
	if (GCMD_SET_PALLET(cg_fflg)) {
		switch(cg->type) {
		case ALCG_VSP:
		case ALCG_PMS8:
		case ALCG_BMP8:
			ags_setPalletToSystem(0, 256);
			break;
		default:
			break;
		}
	}
	
	/* draw cg */
	switch(cg->type) {
	case ALCG_VSP:
	case ALCG_PMS8:
	case ALCG_BMP8:
		if (GCMD_EXTRACTCG(cg_fflg)) {
			/* set display offset */
			p = set_display_loc(cg);
			/* draw cg pixel */
			cg->spritecolor = flg;
			display_cg(cg, p.x, p.y);
			/* clear display offset */
			clear_display_loc();
		}
		break;
	case ALCG_PMS16:
	case ALCG_BMP24:
		if (GCMD_EXTRACTCG(cg_fflg)) {
			/* set display offset */
			p = set_display_loc(cg);
			/* draw alpha pixel */
			if (cg->alpha) {
				ags_alpha_setPixel(p.x, p.y, cg->width, cg->height, cg->alpha);
			}
			/* draw cg pixel */
			cg->spritecolor = flg;
			display_cg(cg, p.x, p.y);
			/* clear display offset */
			clear_display_loc();
		}
		break;
	default:
		break;
	} 
}

/*
 * Load and display cg with alpha
 *   cgno    : file no for cg ( >= 0 or -1(alpha only))
 *   shadowno: file no for alpha ( >= 0 )
*/
void cg_load_with_alpha(int cgno, int shadowno) {
	cgdata *cg = NULL, *scg;
	MyPoint p;
	
	/* load pixel */
	if (cgno >= 0) {
		if (NULL == (cg = loader(cgno))) return;
		if (cg->type != ALCG_PMS16) {
			WARNING("commandGX cg_no != 16bitPMS\n");
			return;
		}
	}
	
	/* load alpha pixel */
	if (NULL == (scg = loader(shadowno))) return;
	if (scg->type != ALCG_PMS8) {
		WARNING("commandGX shadow_no != 8bitPMS\n");
		return;
	}
	
	/* set alpha pixel offset */
	p = set_display_loc(scg);
	/* draw alpha pixel */
	ags_alpha_setPixel(p.x, p.y, scg->width, scg->height, scg->pic);
	
	/* draw pixel */
	if (cg) {
		p = set_display_loc(cg);
		display_cg(cg, p.x, p.y);
	}
	
	/* clear display offset */
	clear_display_loc();
}

/*
 * Load and display cg from file 'name' (not cached right now)
 *   name: file name to be read
 *   x   : display location x
 *   y   : display location y
 *   return: file read status
*/
int cg_load_with_filename(char *name, int x, int y) {
	int status, type;
	BYTE *data;
	cgdata *cg = NULL;
	MyPoint p;
	
	data = load_cg_with_file(name, &status);
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
	default:
		return status;
	}
	
	/* load pallet if not extracted */
	if (GCMD_LOAD_PALLET(cg_fflg)) {
		if (cg->type == ALCG_BMP8)
			ags_setPallets(cg->pal, 10, 10, 236);
	}
	
	if (GCMD_SET_PALLET(cg_fflg)) {
		if (cg->type == ALCG_BMP8)
			ags_setPalletToSystem(0, 256);
	}

	/* draw cg */
	if (GCMD_EXTRACTCG(cg_fflg)) {
		/* set display offset */
		p = set_display_loc(cg);
		/* draw cg pixel */
		cg->spritecolor = -1;
		display_cg(cg, p.x, p.y);
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
void cg_get_info(int no, MyRectangle *info) {
	cgdata *cg = loader(no);
	MyPoint p;
	
	if (cg == NULL) {
		info->x = info->y = info->width = info->height = 0;
	} else {
		p = set_display_loc(cg);
		info->x = p.x;
		info->y = p.y;
		info->width  = cg->width;
		info->height = cg->height;
	}
}

cgdata *cg_loadonly(int no) {
	return loader(no);
}

void cg_clear_display_loc() {
	clear_display_loc();
}
