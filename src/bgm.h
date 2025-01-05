/*
 * Copyright (C) 2019 <KichikuouChrome@gmail.com>
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
#ifndef __BGM_H__
#define __BGM_H__

#include "ald_manager.h"

int musbgm_init(DRIFILETYPE type, int base_no);
int musbgm_exit(void);
int musbgm_reset(void);
int musbgm_play(int no, int time, int vol, int loop_count);
int musbgm_stop(int no, int time);
int musbgm_fade(int no, int time, int vol);
int musbgm_getpos(int no);
int musbgm_getlen(int no);
int musbgm_isplaying(int no);
int musbgm_stopall(int time);
int musbgm_wait(int no, int timeout);

#endif /* __BGM_H__ */
