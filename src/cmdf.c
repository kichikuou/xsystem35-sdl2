/*
 * cmdf.c  SYSTEM35 F command
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
 *
*/
/* $Id: cmdf.c,v 1.15 2006/04/21 16:39:02 chikama Exp $ */

#include <stdio.h>
#include <string.h>
#include "portab.h"
#include "xsystem35.h"
#include "scenario.h"
#include "nact.h"
#include "LittleEndian.h"

static void commandF1();
static void commandF2();
static void commandF3();
static void commandF4();
static void commandF5();
static void commandF6();
static void commandF7();
static void commandF8();
static void commandF9();
static void commandF10();
static void commandF11();

/* F6/7 ÍÑÊÑ¿ô */
static int *F6Index[256];

void commandF() {
	switch (sys_getc()) {
	case 1:
		commandF1(); break;
	case 2:
		commandF2(); break;
	case 3:
		commandF3(); break;
	case 4:
		commandF4(); break;
	case 5:
		commandF5(); break;
	case 6:
		commandF6(); break;
	case 7:
		commandF7(); break;
	case 8:
		commandF8(); break;
	case 9:
		commandF9(); break;
	case 10:
		commandF10(); break;
	case 11:
		commandF11(); break;
	default:
		break;
	}
}

static void commandF1() {
	int str_number = sys_getCaliValue();
	int skip       = sys_getCaliValue();
	int i;
	char *p;
	
	DEBUG_COMMAND("F1 %d,%d:\n", str_number, skip);
	
	p = (char *)nact->datatbl_addr;
	for (i = 0; i < skip; i++) {
		p += (strlen(p) + 1);
	}
	
	v_strcpy(str_number - 1 , p);
	p += (strlen(p) + 1);
	nact->datatbl_addr = (void *)p;
}

static void commandF2() {
	int *read_var = sys_getCaliVariable();
	int skip      = sys_getCaliValue();
	WORD *p = (WORD *)nact->datatbl_addr;
	
	p += skip;
	
	*read_var = LittleEndian_getW((BYTE *)p, 0);
	
	p++;
	nact->datatbl_addr = (void *)p;
	
	DEBUG_COMMAND("F2 %d,%d:\n", *read_var, skip);
}

static void commandF3() {
	int *read_var = sys_getCaliVariable();
	int skip      = sys_getCaliValue();
	
	*read_var = LittleEndian_getW(nact->datatbl_addr, skip *2);
	
	DEBUG_COMMAND("F3 %d,%d:\n", *read_var, skip);
}

static void commandF4() {
	int *read_var = sys_getCaliVariable();
	int count     = sys_getCaliValue();
	int i;
	WORD *p = (WORD *)nact->datatbl_addr;
	
	for (i = 0; i < count; i++) {
		*read_var = LittleEndian_getW((BYTE *)p, 0);
		p++;
		read_var++;
	}
	
	nact->datatbl_addr = p;
	
	DEBUG_COMMAND("F4 %d,%d:\n", *read_var, count);
}

static void commandF5() {
	int *read_var = sys_getCaliVariable();
	int count     = sys_getCaliValue();
	int i;
	
	for (i = 0; i < count; i++) {
		*read_var = LittleEndian_getW(nact->datatbl_addr, i * 2);
		read_var++;
	}
	
	DEBUG_COMMAND("F5 %d,%d:\n", *read_var, count);
}

static void commandF6() {
	int *var  = sys_getCaliVariable();
	int index = sys_getCaliValue();
	
	F6Index[index] = var;
	
	DEBUG_COMMAND("F6 %d,%d:\n", *var, index);
}

static void commandF7() {
	int data_width = sys_getCaliValue();
	int count      = sys_getCaliValue();
	int i, j;
	WORD *p = (WORD *)nact->datatbl_addr;
	
	for (i = 0; i < count; i++) {
		for (j = 0; j < data_width; j++) {
			*(F6Index[j] + i) = LittleEndian_getW((BYTE *)p, 0);
			p++;
		}
	}
	
	nact->datatbl_addr = p;
	
	DEBUG_COMMAND("F7 %d,%d\n", data_width, count);
}

static void commandF8() {
	int snum = sys_getCaliValue();
	int scnt = sys_getCaliValue();
	DEBUG_COMMAND_YET("F8 %d,%d:\n", snum, scnt);
}

static void commandF9() {
	int snum = sys_getCaliValue();
	int scnt = sys_getCaliValue();
	DEBUG_COMMAND_YET("F9 %d,%d:\n", snum, scnt);
}

static void commandF10() {
	int snum = sys_getCaliValue();
	int scnt = sys_getCaliValue();
	DEBUG_COMMAND_YET("F10 %d,%d:\n", snum, scnt);
}

static void commandF11() {
	int snum = sys_getCaliValue();
	int ends = sys_getCaliValue();
	DEBUG_COMMAND_YET("F11 %d,%d:\n", snum, ends);
}
