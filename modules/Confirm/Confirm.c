#include "config.h"

#include <stdio.h>

#include "portab.h"
#include "system.h"
#include "xsystem35.h"
#include "modules.h"
#include "nact.h"
static void Init() {
	int p1 = getCaliValue();
	
	TRACE_UNIMPLEMENTED("Confirm.Init %d:", p1);
}

static void ExistKeyFile() {
	const char *p1 = sl_getString(0);
	int *p2 = getCaliVariable();
	int *var = getCaliVariable();
	
	*var = 1;
	
	TRACE_UNIMPLEMENTED("Confirm.ExistKeyFile %s,%p,%p:",p1, p2, var);
}

static void CheckProtectFile() {
	const char *p1 = sl_getString(0);
	int *p2 = getCaliVariable();
	int *var = getCaliVariable();
	
	*var = 1;

	TRACE_UNIMPLEMENTED("Confirm.CheckProtectFile %s,%p,%p:", p1,p2,var);
}

static void CreateKeyFile() {
	const char *p1 = sl_getString(0);
	int *p2 = getCaliVariable();
	int *var = getCaliVariable();
	
	*var = 1;

	TRACE_UNIMPLEMENTED("Confirm.CreateKeyFile %s,%p,%p:", p1,p2,var);
}

static const ModuleFunc functions[] = {
	{"CheckProtectFile", CheckProtectFile},
	{"CreateKeyFile", CreateKeyFile},
	{"ExistKeyFile", ExistKeyFile},
	{"Init", Init},
};

const Module module_Confirm = {"Confirm", functions, sizeof(functions) / sizeof(ModuleFunc)};
