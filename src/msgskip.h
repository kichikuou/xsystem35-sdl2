/*
 * Copyright (C) 2020 <KichikuouChrome@gmail.com>
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

#ifndef __MSGSKIP_H__
#define __MSGSKIP_H__

#include "portab.h"

#define MSGSKIP_SKIP_UNSEEN		1
#define MSGSKIP_STOP_ON_UNSEEN	2
#define MSGSKIP_STOP_ON_MENU	4
#define MSGSKIP_STOP_ON_CLICK	8

extern void msgskip_init(const char *msgskip_file);
extern boolean msgskip_isSkipping(void);
extern boolean msgskip_isActivated(void);
extern void msgskip_enableMenu(boolean enable);
extern void msgskip_activate(boolean bool);
extern void msgskip_onMessage(void);
extern void msgskip_onAinMessage(int msgid);
extern unsigned msgskip_getFlags();
extern void msgskip_setFlags(unsigned flags, unsigned mask);
extern void msgskip_pause(boolean pause);

#endif // __MSGSKIP_H__
