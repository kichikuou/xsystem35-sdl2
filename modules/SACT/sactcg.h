/*
 * sactcg.h: CG作成
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
/* $Id: sactcg.h,v 1.2 2003/11/16 15:29:52 chikama Exp $ */

#ifndef __SACTCG_H__
#define __SACTCG_H__

#include "portab.h"
#include "sact.h"

cginfo_t *scg_addref(int no);
void scg_deref(cginfo_t *cg);
void scg_create(int wNumCG, int wWidth, int wHeight, int wR, int wG, int wB, int wBlendRate);
void scg_create_reverse(int NumCG, int wNumSrcCG, int wReverseX, int wReverseY);
void scg_create_stretch(int wNumCG, int wWidth, int wHeight, int wNumSrcCG);
void scg_create_blend(int wNumDstCG, int wNumBaseCG, int wX, int wY, int wNumBlendCG, int wAlphaMapMode);
void scg_create_text(int wNumCG, int wSize, int wR, int wG, int wB, int wText);
void scg_create_textnum(int wNumCG, int wSize, int wR, int wG, int wB, int wFigs, int wZeroPadding, int wValue);
void scg_copy(int wNumDstCG, int wNumSrcCG);
void scg_cut(int wNumDstCG, int wNumSrcCG, int wX, int wY, int wWidth, int wHeight);
void scg_partcopy(int wNumDstCG, int wNumSrcCG, int wX, int wY, int wWidth, int wHeight);
void scg_freeall(void);
void scg_free(int cg);
int scg_querytype(int wNumCG);
bool scg_querysize(int wNumCG, int *w, int *h);
int scg_querybpp(int wNumCG);
bool scg_existalphamap(int wNumCG);

#endif
