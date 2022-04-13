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
        
        DEBUG_COMMAND_YET("eeDEMO.Init %d,%d,%d,%p:\n", p1, p2, p3, var);
}

static void SetKeyCancelFlag() {
        int cancelflag = getCaliValue();
        
        DEBUG_COMMAND_YET("eeDEMO.SetKeyCancelFlag %d:\n", cancelflag);
}

static void SetLoopFlag() {
        /* Loop Flag */
        int loopflag = getCaliValue(); /* 0 なら無限繰り返し */
        
        DEBUG_COMMAND_YET("eeDEMO.SetLoopFlag %d:\n", loopflag);
}

static void Run() {
        int p1 = getCaliValue();
	
        DEBUG_COMMAND_YET("eeDEMO.Run %d:\n", p1);
}

static const ModuleFunc functions[] = {
	{"Init", Init},
	{"Run", Run},
	{"SetKeyCancelFlag", SetKeyCancelFlag},
	{"SetLoopFlag", SetLoopFlag},
};

const Module module_eeDemo = {"eeDemo", functions, sizeof(functions) / sizeof(ModuleFunc)};
