#include "config.h"

#include <stdio.h>

#include "portab.h"
#include "system.h"
#include "xsystem35.h"
#include "nact.h"

void Init() {
	int p1 = getCaliValue(); /* ISys3x  */
	int p2 = getCaliValue(); /* IWinMsg */
	int p3 = getCaliValue(); /* ITimer  */
	int *var = getCaliVariable();
	
	*var = 0;
	
	DEBUG_COMMAND_YET("nDEMO.Init %p:\n", var);
}

void SetKeyCancelFlag() {
	int p1 = getCaliVariable();
	
	DEBUG_COMMAND_YET("nDEMO.SetKeyCancelFlag %d:\n", p1);
}

void SetLoopFlag() {
	int p1 = getCaliVariable();
	
	DEBUG_COMMAND_YET("nDEMO.SetLoopFlag %d:\n", p1);
}

void Run() {
	DEBUG_COMMAND_YET("nDEMO.Run:\n");
}

