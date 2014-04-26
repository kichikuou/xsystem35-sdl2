/*
 * cmdh.c  SYSTEM35 H command
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
/* $Id: cmdh.c,v 1.5 2000/09/10 10:39:48 chikama Exp $ */

#include <stdio.h>
#include <string.h>
#include "portab.h"
#include "xsystem35.h"

/* defined by hankana2sjis.c */
extern char *num2sjis(int num);

void commandH() {
	int fig = sys_getc();
	int num = getCaliValue();
	char _work1[10], _work2[512];
	char *work1 = _work1, *work2 = _work2;
	int len;
	
	*work2 = 0;
	sprintf(work1, "%d", num);
	if (fig != 0) {
		len = strlen(work1);
		if (fig > len) {
			/* 空白でうめる */
			len = fig - len;
			while(len--) {
				strcat(work2, num2sjis(10));
			}
		} else {
			work1 += (len - fig);
		}
	}
	while(*work1) {
		strcat(work2, num2sjis((*work1) - '0')); work1++;
	}
	
	sys_addMsg(work2);
	DEBUG_COMMAND("H %d,%d:\n",fig,num);
}

void commandHH(void) {
	int fig = sys_getc();
	int num = getCaliValue();
	char s[256];
	if( fig ) {
		char *ss="%%%dd";
		char sss[256];
		sprintf(sss,ss,fig);
		sprintf(s,sss,num);
	} else {
		sprintf(s,"%d",num);
	}
	sys_addMsg(s);
	DEBUG_COMMAND("HH %d,%d:\n",fig,num);
}
