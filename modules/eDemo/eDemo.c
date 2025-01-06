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
        
        TRACE_UNIMPLEMENTED("eDEMO.Init %d,%d,%d,%p:", p1, p2, p3, var);
}

static void SetKeyCancelFlag() {
        int cancelflag = getCaliValue();
        
        TRACE_UNIMPLEMENTED("eDEMO.SetKeyCancelFlag %d:", cancelflag);
}

static void SetLoopFlag() {
        /* Loop Flag */
        int loopflag = getCaliValue(); /* 0 なら無限繰り返し */
        
        TRACE_UNIMPLEMENTED("eDEMO.SetLoopFlag %d:", loopflag);
}

static void SetDrawStaffFlag() {
        /* Loop Flag */
        int staffflag = getCaliValue();
        
        TRACE_UNIMPLEMENTED("eDEMO.SetDrawStaffFlag %d:", staffflag);
}

static void Run() {
        TRACE_UNIMPLEMENTED("eDEMO.Run:");
}

static const ModuleFunc functions[] = {
	{"Init", Init},
	{"Run", Run},
	{"SetDrawStaffFlag", SetDrawStaffFlag},
	{"SetKeyCancelFlag", SetKeyCancelFlag},
	{"SetLoopFlag", SetLoopFlag},
};

const Module module_eDemo = {"eDemo", functions, sizeof(functions) / sizeof(ModuleFunc)};
