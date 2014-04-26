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
	
	*var = 1;
	
	DEBUG_COMMAND_YET("tDemo.Init %d,%d,%d,%p:\n", p1, p2, p3, var);
}

void SetKeyCancelFlag() {
	int cancelflag = getCaliValue();
	
	DEBUG_COMMAND_YET("tDemo.SetKeyCancelFlag %d:\n", cancelflag);
}

void SetLoopFlag() {
	/* Loop Flag */
	int loopflag = getCaliValue(); /* 0 なら無限繰り返し */
	
	DEBUG_COMMAND_YET("tDemo.SetLoopFlag %d:\n", loopflag);
}

void Run() {
	DEBUG_COMMAND_YET("tDemo.Run:\n");
}

