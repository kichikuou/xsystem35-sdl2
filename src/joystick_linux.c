/*
 * joystick_linux.c  joystick interface for linux (kernel 2.2.x)
 *
 * Copyright (C) 1999- Fumihiko Murata <fmurata@p1.tcnet.ne.jp>
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
/* $Id: joystick_linux.c,v 1.8 2000/11/25 13:09:08 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/joystick.h>
#include "joystick.h"
#include "portab.h"

#ifndef JOY_DEVICE
#define JOY_DEVICE "/dev/js0"
#endif

static char *dev_joy = JOY_DEVICE;
static int joy_fd=-1,joy_dat=0,joy_sens=19660;
static unsigned char joy_axes,joy_buttons;
static boolean joy_enable=TRUE;
static struct js_event js;

void joy_set_devicename(char *name) {
	if (0 == strcmp("none", name)) dev_joy = NULL;
	else                           dev_joy = strdup(name);
}

int joy_open(void) {
	if( dev_joy == NULL ) return -1;
	joy_fd=open(dev_joy,O_RDONLY);
	if( joy_fd < 0 ) {
		joy_enable=FALSE;
		return joy_fd;
	}
	ioctl(joy_fd,JSIOCGAXES,&joy_axes);
	ioctl(joy_fd,JSIOCGBUTTONS,&joy_buttons);
	printf("open %s axes %d buttons %d\n",dev_joy,joy_axes,joy_buttons);

	return joy_fd;
}

void joy_close(void) {
	if( joy_fd < 0 ) return;
	close(joy_fd);
	return;
}

int joy_getinfo(void) {
	int err;
	struct timeval tv;
	fd_set set;
	int max_fd;

	return 0;
	if( joy_fd < 0 || !joy_enable ) return 0;
	max_fd = joy_fd+1;
	tv.tv_sec = 0;
	tv.tv_usec = 1000;
	FD_ZERO(&set);
	FD_SET(joy_fd,&set);
	while( select(max_fd,&set,NULL,NULL,&tv) ) {
		if( FD_ISSET(joy_fd,&set) ) {
			err=read(joy_fd,&js,sizeof(js));
			if( err != sizeof(js) ) return joy_dat;
			switch(js.type & ~JS_EVENT_INIT) {
			case JS_EVENT_BUTTON:
				if( js.number < 4 ) {
					joy_dat &= ~(1<<(js.number+4));
					joy_dat |= js.value<<(js.number+4);
				}
				break;
			case JS_EVENT_AXIS:
				if( js.number == 0 ) {
					joy_dat &= ~(0x0c);
					if( js.value >  joy_sens ) joy_dat |= 8;
					if( js.value < -joy_sens ) joy_dat |= 4;
				} else if( js.number == 1 ) {
					joy_dat &= ~(0x03);
					if( js.value >  joy_sens ) joy_dat |= 2;
					if( js.value < -joy_sens ) joy_dat |= 1;
				}
				break;
			}
		}
	}
	return joy_dat;
}

#if 0
boolean joy_getinfo(int *data) {
	int err;

	if( joy_fd < 0 || !joy_enable ) return FALSE;

	err = read(joy_fd, &js, sizeof(js));
	if( err != sizeof(js) ) {
		*data = joy_dat;
		return TRUE;
	}
	switch(js.type & ~JS_EVENT_INIT) {
	case JS_EVENT_BUTTON:
		if( js.number < 4 ) {
			joy_dat &= ~(1<<(js.number+4));
			joy_dat |= js.value<<(js.number+4);
		}
		break;
	case JS_EVENT_AXIS:
		if( js.number == 0 ) {
			joy_dat &= ~(0x0c);
			if( js.value >  joy_sens ) joy_dat |= 8;
			if( js.value < -joy_sens ) joy_dat |= 4;
		} else if( js.number == 1 ) {
			joy_dat &= ~(0x03);
			if( js.value >  joy_sens ) joy_dat |= 2;
			if( js.value < -joy_sens ) joy_dat |= 1;
		}
		break;
	}
	*data = joy_dat;
	return TRUE;
}
#endif
