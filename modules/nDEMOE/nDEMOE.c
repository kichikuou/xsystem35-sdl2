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
	
	TRACE_UNIMPLEMENTED("nDEMOE.Init %p:", var);
}

static void SetKeyCancelFlag() {
	int p1 = getCaliValue();
	
	TRACE_UNIMPLEMENTED("nDEMOE.SetKeyCancelFlag %d:", p1);
}

static void Run() {
	TRACE_UNIMPLEMENTED("nDEMOE.Run:");
}

static const ModuleFunc functions[] = {
	{"Init", Init},
	{"Run", Run},
	{"SetKeyCancelFlag", SetKeyCancelFlag},
};

const Module module_nDEMOE = {"nDEMOE", functions, sizeof(functions) / sizeof(ModuleFunc)};
