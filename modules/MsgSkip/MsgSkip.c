#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "portab.h"
#include "system.h"
#include "filecheck.h"
#include "xsystem35.h"
#include "modules.h"
#include "nact.h"
#include "msgskip.h"
#include "utfsjis.h"

static bool valid;
static int action;

static void Init() {
	int *p1 = getCaliVariable();
	int p2 = getCaliValue(); /* ISys3x */
	int p3 = getCaliValue();
	int p4 = getCaliValue();

	action = p3;
	valid = true;
	*p1 = 0;

	TRACE("MsgSkip.Init %p,%d,%d,%d:", p1, p2, p3, p4);
}

static void Start() {
	int *p1 = getCaliVariable();
	int p2 = getCaliValue();

	char *fname_utf8 = sjis2utf(svar_get(p2));
	char *path = fc_get_path(fname_utf8);
	msgskip_init(path);
	free(path);
	free(fname_utf8);

	TRACE("MsgSkip.Start %p,%d:", p1, p2);
}

static void SetValid() {
	int p1 = getCaliValue();

	valid = p1;
	msgskip_pause(!valid);

	TRACE("MsgSkip.SetValid %d:", p1);
}

static void GetValid() {
	int *p1 = getCaliVariable();

	*p1 = valid;

	TRACE("MsgSkip.GetValid %p:", p1);
}

static void SetAction() {
	int p1 = getCaliValue();

	action = p1;

	TRACE("MsgSkip.SetAcion %d:", p1);
}

static void GetAction() {
	int *p1 = getCaliVariable();

	*p1 = action;

	TRACE("MsgSkip.GetAcion %p:", p1);
}

static void PushStr() {
	int p1 = getCaliValue();
	
	TRACE_UNIMPLEMENTED("MsgSkip.PushStr %d:", p1);
}

static void PopStr() {
	int p1 = getCaliValue();
	
	TRACE_UNIMPLEMENTED("MsgSkip.PopStr %d:", p1);
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
