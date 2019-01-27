/*
 * music_pcm.h  music server PCM part
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
/* $Id: music_pcm.h,v 1.4 2003/08/22 17:09:23 chikama Exp $ */

#ifndef __MUSIC_PCM_H__
#define __MUSIC_PCM_H__

extern int muspcm_init(void);
extern int muspcm_exit(void);
extern int muspcm_load_no(int slot, int no);
extern int muspcm_load_mixlr(int slot, int noL, int noR);
extern int muspcm_unload(int slot);
extern int muspcm_start(int slot, int loop);
extern int muspcm_stop(int slot);
extern int muspcm_fadeout(int slot, int msec);
extern int muspcm_pause(int slot);
extern int muspcm_unpause(int slot);
extern int muspcm_getpos(int slot);
extern int muspcm_setvol(int dev, int slot, int lv);
extern int muspcm_getwavelen(int slot);
extern boolean muspcm_isplaying(int slot);
extern int muspcm_waitend(int slot);

#endif /* __MUSIC_PCM_H__ */
