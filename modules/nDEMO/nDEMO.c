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
	
	TRACE_UNIMPLEMENTED("nDEMO.Init %p:", var);
}

static void SetKeyCancelFlag() {
	int p1 = getCaliValue();
	
	TRACE_UNIMPLEMENTED("nDEMO.SetKeyCancelFlag %d:", p1);
}

static void SetLoopFlag() {
	int p1 = getCaliValue();
	
	TRACE_UNIMPLEMENTED("nDEMO.SetLoopFlag %d:", p1);
}

static void Run() {
	TRACE_UNIMPLEMENTED("nDEMO.Run:");
}

static const ModuleFunc functions[] = {
	{"Init", Init},
	{"Run", Run},
	{"SetKeyCancelFlag", SetKeyCancelFlag},
	{"SetLoopFlag", SetLoopFlag},
};

const Module module_nDEMO = {"nDEMO", functions, sizeof(functions) / sizeof(ModuleFunc)};
