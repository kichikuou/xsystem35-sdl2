/*
 * music_fader.h  music server fader part
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
/* $Id: music_fader.h,v 1.3 2003/04/25 17:23:55 chikama Exp $ */

#ifndef __MUSIC_FADER_H__
#define __MUSIC_FADER_H__

#include "portab.h"

extern int musfade_init();
extern int musfade_exit();
extern int musfade_new(int dev, int subdev, int time, int ed_vol, int stop);
extern int musfade_stop(int dev, int subdev);
extern boolean musfade_getstate(int dev, int subdev);
extern int musfade_getvol(int dev);
extern int musfade_cb();
extern int musfade_setvolval(int *valance, int num);


#endif /* __MUSIC_FADER_H__ */
