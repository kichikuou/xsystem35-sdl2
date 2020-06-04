#include "config.h"

#include <stdio.h>

#include "portab.h"
#include "system.h"
#include "xsystem35.h"
#include "modules.h"
#include "nact.h"

static void Init() {
	int p1 = getCaliValue(); /* ISys3x */
	int p2 = getCaliValue(); /* IWinMsg */
	int p3 = getCaliValue(); /* ITimer */
	int *var = getCaliVariable();
	
	*var = 1;
	
	DEBUG_COMMAND_YET("AliceLogo.Init %d,%d,%d,%p:\n", p1, p2, p3, var);
}

static void SetWaveNum() {
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	
	DEBUG_COMMAND_YET("AliceLogo.SetWaveNum %d,%d:\n", p1, p2);
}

static void Run() {
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	
	DEBUG_COMMAND_YET("AliceLogo.Run %d,%d:\n", p1, p2);
}

static const ModuleFunc functions[] = {
	{"Init", Init},
	{"Run", Run},
	{"SetWaveNum", SetWaveNum},
};

const Module module_AliceLogo = {"AliceLogo", functions, sizeof(functions) / sizeof(ModuleFunc)};
