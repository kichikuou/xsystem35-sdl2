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

#ifndef __MODULES_H__
#define __MODULES_H__

typedef struct {
	const char *name;
	void (*entrypoint)(void);
} ModuleFunc;

typedef struct {
	const char *name;
	const ModuleFunc *funcs;  // must be sorted by name
	int nfunc;
} Module;

extern const Module module_AliceLogo;
extern const Module module_Confirm;
extern const Module module_Gpx;
extern const Module module_Math;
extern const Module module_MsgSkip;
extern const Module module_NIGHTDLL;
extern const Module module_NightDemonDemo;
extern const Module module_RandMT;
extern const Module module_SACT;
extern const Module module_ShArray;
extern const Module module_ShCalc;
extern const Module module_ShGraph;
extern const Module module_ShPort;
extern const Module module_ShSound;
extern const Module module_ShString;
extern const Module module_dDemo;
extern const Module module_eDemo;
extern const Module module_eeDemo;
extern const Module module_nDEMO;
extern const Module module_nDEMOE;
extern const Module module_oDEMO;
extern const Module module_oujimisc;
extern const Module module_tDemo;

struct S39AIN_DLLINF;
int resolve_module(struct S39AIN_DLLINF *dll);

#endif /* __MODULES_H__ */
