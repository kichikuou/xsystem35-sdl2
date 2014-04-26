#include "config.h"

#include <stdio.h>

#include "portab.h"
#include "system.h"
#include "xsystem35.h"
#include "nact.h"

void MakeMapSetParam() {
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();

	DEBUG_COMMAND_YET("oujimisc.MakeMapSetParam %d,%d,%d,%d:\n", p1,p2,p3,p4);
}

void MakeMapSetChipParam() {
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();

	DEBUG_COMMAND_YET("oujimisc.MakeMapSeChipParam %d,%d,%d,%d,%d:\n", p1,p2,p3,p4,p5);
	
}

void MakeMapDraw() {
	int p1 = getCaliValue(); /* ISys3xDIB */
	int p2 = getCaliValue(); 
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int *p6 = getCaliVariable();
	int *p7 = getCaliVariable();
	int *p8 = getCaliVariable();
	
	DEBUG_COMMAND_YET("oujimisc.MakeMapDraw %d,%d,%d,%d,%d,%p,%p,%p:\n", p1,p2,p3,p4,p5,p6,p7,p8);
}

void MakeMapInit() {
	int p1 = getCaliValue(); /* ISys3x */
	
	DEBUG_COMMAND_YET("oujimisc.MakeMapInit %d:\n", p1);
}	

void DrawNumber() {
	int p1 = getCaliValue(); /* ISys3xDIB */
	int p2 = getCaliValue(); 
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	int p8 = getCaliValue();

	DEBUG_COMMAND_YET("oujimisc.DrawNumber %d,%d,%d,%d,%d,%d,%d,%d:\n", p1,p2,p3,p4,p5,p6,p7,p8);
}

void TempMapCreateShadow() {
	DEBUG_COMMAND_YET("oujimisc.TempMapCreateShadow:\n");
}

void TempMapInit() {
	int p1 = getCaliValue(); /* ISys3x */
	int p2 = getCaliValue();
	
	DEBUG_COMMAND_YET("oujimisc.TempMapInit %d,%d:\n", p1,p2);
}

void TempMapLoadToShadow() {
	int p1 = getCaliValue();
	int *p2 = getCaliVariable();
	int *p3 = getCaliVariable();
	int *p4 = getCaliVariable();
	int p5 = getCaliValue();

	DEBUG_COMMAND_YET("oujimisc.TempMapLoadToShadow %d,%p,%p,%p,%d:\n", p1,p2,p3,p4,p5);
}

void TempMapSaveToShadow() {
	int p1 = getCaliValue();
	int *p2 = getCaliVariable();
	int *p3 = getCaliVariable();
	int *p4 = getCaliVariable();
	int p5 = getCaliValue();

	DEBUG_COMMAND_YET("oujimisc.TempMapSaveToShadow %d,%p,%p,%p,%d:\n", p1,p2,p3,p4,p5);
}

void TempMapFileSave() {
	int p1 = getCaliValue();
	
	DEBUG_COMMAND_YET("oujimisc.TempMapFileSave %d:\n", p1);
}

void TempMapFileLoad() {
	int p1 = getCaliValue();
	
	DEBUG_COMMAND_YET("oujimisc.TempMapFileLoad %d:\n", p1);
}

void CalcMoveDiffer() {
	int *p1 = getCaliVariable();
	int *p2 = getCaliVariable();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int *p7 = getCaliVariable();
	int p8 = getCaliValue();

	DEBUG_COMMAND_YET("oujimisc.CalcMoveDiffer %p,%p,%d,%d,%d,%d,%p,%d:\n", p1,p2,p3,p4,p5,p6,p7,p8);
}
