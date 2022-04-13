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
        
        *var = 1;
        
        DEBUG_COMMAND_YET("eDEMO.Init %d,%d,%d,%p:\n", p1, p2, p3, var);
}

static void SetKeyCancelFlag() {
        int cancelflag = getCaliValue();
        
        DEBUG_COMMAND_YET("eDEMO.SetKeyCancelFlag %d:\n", cancelflag);
}

static void SetLoopFlag() {
        /* Loop Flag */
        int loopflag = getCaliValue(); /* 0 なら無限繰り返し */
        
        DEBUG_COMMAND_YET("eDEMO.SetLoopFlag %d:\n", loopflag);
}

static void SetDrawStaffFlag() {
        /* Loop Flag */
        int staffflag = getCaliValue();
        
        DEBUG_COMMAND_YET("eDEMO.SetDrawStaffFlag %d:\n", staffflag);
}

static void Run() {
        DEBUG_COMMAND_YET("eDEMO.Run:\n");
}

static const ModuleFunc functions[] = {
	{"Init", Init},
	{"Run", Run},
	{"SetDrawStaffFlag", SetDrawStaffFlag},
	{"SetKeyCancelFlag", SetKeyCancelFlag},
	{"SetLoopFlag", SetLoopFlag},
};

const Module module_eDemo = {"eDemo", functions, sizeof(functions) / sizeof(ModuleFunc)};
