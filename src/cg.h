/*
 * cg.h   load and display cg
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
/* $Id: cg.h,v 1.14 2001/09/16 15:59:11 chikama Exp $ */

#ifndef __CG__
#define __CG__

#include <SDL_rect.h>
#include "portab.h"

struct SDL_Surface;

/*
 * available cg format id
*/
typedef enum {
	ALCG_UNKNOWN = 1,
	ALCG_VSP,
	ALCG_PMS8,
	ALCG_PMS16,
	ALCG_BMP8,
	ALCG_BMP24,
	ALCG_QNT,
	ALCG_JPEG,
	ALCG_WEBP,
} CG_TYPE;

/*
 * information for display cg data
*/
typedef struct {
	CG_TYPE type;    // cg format type
	int x;           // default display location x
	int y;           // default display location y
	int width;       // image width
	int height;      // image height
	int depth;       // pixel depth (8, 16 or 24)
	
	uint8_t *pic;    // extracted pixel data
	uint8_t *alpha;  // extracted alpha data if exists
	SDL_Color *pal;  // 256-color palette if exists
	
	int vsp_bank;    // palette bank for vsp
	int pms_bank;    // palette bank for pms
} cgdata;

/*
 * location for draw image policy
*/ 
typedef enum {
	OFFSET_NOMOVE,       /* use location in data                 */
	OFFSET_ABSOLUTE_GC,  /* absolute location and use only once  */
	OFFSET_ABSOLUTE_JC,  /* absolute location and use til J4 cmd */
	OFFSET_RELATIVE_GC,  /* relative location and use only once  */
	OFFSET_RELATIVE_JC   /* relative location and use til J4 cmd */
} CG_WHERETODISP;

extern void cg_init(void);
extern void cg_reset(void);
extern void cg_set_display_location(int x, int y, CG_WHERETODISP policy);
extern void cg_load(int no, int flg);
extern void cg_load_with_alpha(int cgno, int shadowno);
extern int  cg_load_with_filename(char *fname_utf8, int x, int y);
extern void cg_get_info(int no, SDL_Rect *info);
extern void cg_clear_display_loc();
extern void cgdata_free(cgdata *cg);
extern struct SDL_Surface *cg_load_as_sdlsurface(int no);

extern int cg_vspPB;
extern int cg_fflg;
extern int *cg_loadCountVar;
extern int cg_brightness;

static inline uint32_t rgb565_to_rgb888(uint16_t rgb) {
	uint32_t r = rgb >> 11;
	uint32_t g = (rgb >> 5) & 0x3f;
	uint32_t b = rgb & 0x1f;
	return (r << 19) | (r >> 2 << 16) | (g << 10) | (g >> 4 << 8) | (b << 3) | (b >> 2);
}

#endif /* !__CG__ */
