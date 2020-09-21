#include "config.h"

#include <stdio.h>
#include <string.h>

#include "portab.h"
#include "system.h"
#include "xsystem35.h"
#include "modules.h"
#include "nact.h"
#include "message.h"

static char *msgskipfile;

static void Init() {
        int *p1 = getCaliVariable();
        int p2 = getCaliValue(); /* ISys3x */
        int p3 = getCaliValue();
        int p4 = getCaliValue();

	
	DEBUG_COMMAND_YET("MsgSkip.Init %p,%d,%d,%d:\n", p1, p2, p3, p4);
}

static void Start() {
        int *p1 = getCaliVariable();
        int p2 = getCaliValue();

	msgskipfile = strdup(svar_get(p2));
	
	DEBUG_COMMAND_YET("MsgSkip.Start %p,%d:\n", p1, p2);
}

static void SetValid() {
        int p1 = getCaliValue();

	
	DEBUG_COMMAND("MsgSkip.SetValid %d:\n", p1);
}

static void GetValid() {
        int *p1 = getCaliVariable();
	
	DEBUG_COMMAND_YET("MsgSkip.GetValid %p:\n", p1);
}

static void SetAction() {
        int p1 = getCaliValue();
	
	DEBUG_COMMAND("MsgSkip.SetAcion %d:\n", p1);
}

static void GetAction() {
        int *p1 = getCaliVariable();
	
	DEBUG_COMMAND_YET("MsgSkip.GetAcion %p:\n", p1);
}

static void PushStr() {
        int p1 = getCaliValue();
	
	DEBUG_COMMAND_YET("MsgSkip.PushStr %d:\n", p1);
}

static void PopStr() {
        int p1 = getCaliValue();
	
	DEBUG_COMMAND_YET("MsgSkip.PopStr %d:\n", p1);
}

static const ModuleFunc functions[] = {
	{"GetAction", GetAction},
	{"GetValid", GetValid},
	{"Init", Init},
	{"PopStr", PopStr},
	{"PushStr", PushStr},
	{"SetAction", SetAction},
	{"SetValid", SetValid},
	{"Start", Start},
};

const Module module_MsgSkip = {"MsgSkip", functions, sizeof(functions) / sizeof(ModuleFunc)};
