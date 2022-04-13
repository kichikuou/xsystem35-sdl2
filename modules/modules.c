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

#include "config.h"

#include <stdlib.h>
#include <string.h>

#include "system.h"
#include "s39ain.h"
#include "modules.h"

static const Module *modules[] = {
	&module_AliceLogo,
	&module_Confirm,
	&module_Gpx,
	&module_Math,
	&module_MsgSkip,
	&module_NIGHTDLL,
	&module_NightDemonDemo,
	&module_RandMT,
	&module_SACT,
	&module_ShArray,
	&module_ShCalc,
	&module_ShGraph,
	&module_ShPort,
	&module_ShSound,
	&module_ShString,
	&module_dDemo,
	&module_eDemo,
	&module_eeDemo,
	&module_nDEMO,
	&module_nDEMOE,
	&module_oDEMO,
	&module_oujimisc,
	&module_tDemo,
	NULL
};

static int compare_name(const void *f1, const void *f2) {
	return strcmp(((const ModuleFunc *)f1)->name,
				  ((const ModuleFunc *)f2)->name);
}

static void resolve_func(S39AIN_DLLFN *func, const Module *mod) {
	ModuleFunc key = {.name = func->name};
	ModuleFunc *mf = bsearch(&key, mod->funcs, mod->nfunc, sizeof(ModuleFunc), compare_name);

	if (mf)
		func->entrypoint = mf->entrypoint;
	else
		WARNING("Cannot resolve DLL function %s.%s\n", mod->name, func->name);
}

int resolve_module(S39AIN_DLLINF *dll) {
	if (dll->function_num == 0)
		return OK;

	for (const Module **mod = modules; *mod; mod++) {
		if (strcasecmp((*mod)->name, dll->name) == 0) {
			for (int i = 0; i < dll->function_num; i++)
				resolve_func(&dll->function[i], *mod);
			return OK;
		}
	}
	WARNING("Cannot resolve DLL: %s\n", dll->name);
	return NG;
}
