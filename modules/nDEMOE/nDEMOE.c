#include "config.h"

#include <stdio.h>

#include "portab.h"
#include "system.h"
#include "xsystem35.h"
#include "modules.h"
#include "nact.h"

static void Init() {
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int *var = getCaliVariable();
	
	DEBUG_COMMAND_YET("nDEMOE.Init %p:", var);
}

static void SetKeyCancelFlag() {
	int p1 = getCaliValue();
	
	DEBUG_COMMAND_YET("nDEMOE.SetKeyCancelFlag %d:", p1);
}

static void Run() {
	DEBUG_COMMAND_YET("nDEMOE.Run:");
}

static const ModuleFunc functions[] = {
	{"Init", Init},
	{"Run", Run},
	{"SetKeyCancelFlag", SetKeyCancelFlag},
};

const Module module_nDEMOE = {"nDEMOE", functions, sizeof(functions) / sizeof(ModuleFunc)};
