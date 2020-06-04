#include "config.h"

#include <stdio.h>

#include "portab.h"
#include "system.h"
#include "xsystem35.h"
#include "modules.h"
#include "nact.h"
static void Init() {
	int p1 = getCaliValue();
	
	DEBUG_COMMAND_YET("Confirm.Init %d:\n", p1);
}

static void ExistKeyFile() {
	char *p1 = sys_getString(0);
	int *p2 = getCaliVariable();
	int *var = getCaliVariable();
	
	*var = 1;
	
	DEBUG_COMMAND_YET("Confirm.ExistKeyFile %s,%p,%p:\n",p1, p2, var);
}

static void CheckProtectFile() {
	char *p1 = sys_getString(0);
	int *p2 = getCaliVariable();
	int *var = getCaliVariable();
	
	*var = 1;

	DEBUG_COMMAND_YET("Confirm.CheckProtectFile %s,%p,%p:\n", p1,p2,var);
}

static void CreateKeyFile() {
	char *p1 = sys_getString(0);
	int *p2 = getCaliVariable();
	int *var = getCaliVariable();
	
	*var = 1;

	DEBUG_COMMAND_YET("Confirm.CreateKeyFile %s,%p,%p:\n", p1,p2,var);
}

static const ModuleFunc functions[] = {
	{"CheckProtectFile", CheckProtectFile},
	{"CreateKeyFile", CreateKeyFile},
	{"ExistKeyFile", ExistKeyFile},
	{"Init", Init},
};

const Module module_Confirm = {"Confirm", functions, sizeof(functions) / sizeof(ModuleFunc)};
