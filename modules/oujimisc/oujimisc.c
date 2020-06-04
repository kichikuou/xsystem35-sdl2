#include "config.h"

#include <stdio.h>

#include "portab.h"
#include "system.h"
#include "xsystem35.h"
#include "modules.h"
#include "nact.h"

static void MakeMapSetParam() {
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();

	DEBUG_COMMAND_YET("oujimisc.MakeMapSetParam %d,%d,%d,%d:\n", p1,p2,p3,p4);
}

static void MakeMapSetChipParam() {
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();

	DEBUG_COMMAND_YET("oujimisc.MakeMapSeChipParam %d,%d,%d,%d,%d:\n", p1,p2,p3,p4,p5);
	
}

static void MakeMapDraw() {
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

static void MakeMapInit() {
	int p1 = getCaliValue(); /* ISys3x */
	
	DEBUG_COMMAND_YET("oujimisc.MakeMapInit %d:\n", p1);
}	

static void DrawNumber() {
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

static void TempMapCreateShadow() {
	DEBUG_COMMAND_YET("oujimisc.TempMapCreateShadow:\n");
}

static void TempMapInit() {
	int p1 = getCaliValue(); /* ISys3x */
	int p2 = getCaliValue();
	
	DEBUG_COMMAND_YET("oujimisc.TempMapInit %d,%d:\n", p1,p2);
}

static void TempMapLoadToShadow() {
	int p1 = getCaliValue();
	int *p2 = getCaliVariable();
	int *p3 = getCaliVariable();
	int *p4 = getCaliVariable();
	int p5 = getCaliValue();

	DEBUG_COMMAND_YET("oujimisc.TempMapLoadToShadow %d,%p,%p,%p,%d:\n", p1,p2,p3,p4,p5);
}

static void TempMapSaveToShadow() {
	int p1 = getCaliValue();
	int *p2 = getCaliVariable();
	int *p3 = getCaliVariable();
	int *p4 = getCaliVariable();
	int p5 = getCaliValue();

	DEBUG_COMMAND_YET("oujimisc.TempMapSaveToShadow %d,%p,%p,%p,%d:\n", p1,p2,p3,p4,p5);
}

static void TempMapFileSave() {
	int p1 = getCaliValue();
	
	DEBUG_COMMAND_YET("oujimisc.TempMapFileSave %d:\n", p1);
}

static void TempMapFileLoad() {
	int p1 = getCaliValue();
	
	DEBUG_COMMAND_YET("oujimisc.TempMapFileLoad %d:\n", p1);
}

static void CalcMoveDiffer() {
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

static const ModuleFunc functions[] = {
	{"CalcMoveDiffer", CalcMoveDiffer},
	{"DrawNumber", DrawNumber},
	{"MakeMapDraw", MakeMapDraw},
	{"MakeMapInit", MakeMapInit},
	{"MakeMapSetChipParam", MakeMapSetChipParam},
	{"MakeMapSetParam", MakeMapSetParam},
	{"TempMapCreateShadow", TempMapCreateShadow},
	{"TempMapFileLoad", TempMapFileLoad},
	{"TempMapFileSave", TempMapFileSave},
	{"TempMapInit", TempMapInit},
	{"TempMapLoadToShadow", TempMapLoadToShadow},
	{"TempMapSaveToShadow", TempMapSaveToShadow},
};

const Module module_oujimisc = {"oujimisc", functions, sizeof(functions) / sizeof(ModuleFunc)};
