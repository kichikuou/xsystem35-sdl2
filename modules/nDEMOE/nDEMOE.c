#include "config.h"

#include <stdio.h>

#include "portab.h"
#include "system.h"
#include "xsystem35.h"
#include "nact.h"

void Init() {
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int *var = getCaliVariable();
	
	DEBUG_COMMAND_YET("nDEMOE.Init %p:\n", var);
}

void SetKeyCancelFlag() {
	int p1 = getCaliValue();
	
	DEBUG_COMMAND_YET("nDEMOE.SetKeyCancelFlag %d:\n", p1);
}

void Run() {
	DEBUG_COMMAND_YET("nDEMOE.Run:\n");
}

