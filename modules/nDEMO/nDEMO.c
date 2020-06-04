#include "config.h"

#include <stdio.h>

#include "portab.h"
#include "system.h"
#include "xsystem35.h"
#include "modules.h"
#include "nact.h"

static void Init() {
	int p1 = getCaliValue(); /* ISys3x  */
	int p2 = getCaliValue(); /* IWinMsg */
	int p3 = getCaliValue(); /* ITimer  */
	int *var = getCaliVariable();
	
	*var = 0;
	
	DEBUG_COMMAND_YET("nDEMO.Init %p:\n", var);
}

static void SetKeyCancelFlag() {
	int p1 = getCaliValue();
	
	DEBUG_COMMAND_YET("nDEMO.SetKeyCancelFlag %d:\n", p1);
}

static void SetLoopFlag() {
	int p1 = getCaliValue();
	
	DEBUG_COMMAND_YET("nDEMO.SetLoopFlag %d:\n", p1);
}

static void Run() {
	DEBUG_COMMAND_YET("nDEMO.Run:\n");
}

static const ModuleFunc functions[] = {
	{"Init", Init},
	{"Run", Run},
	{"SetKeyCancelFlag", SetKeyCancelFlag},
	{"SetLoopFlag", SetLoopFlag},
};

const Module module_nDEMO = {"nDEMO", functions, sizeof(functions) / sizeof(ModuleFunc)};
