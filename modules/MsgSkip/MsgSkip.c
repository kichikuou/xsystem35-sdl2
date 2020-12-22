#include "config.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "portab.h"
#include "system.h"
#include "filecheck.h"
#include "mmap.h"
#include "xsystem35.h"
#include "modules.h"
#include "nact.h"
#include "message.h"
#include "input.h"
#include "utfsjis.h"

typedef struct {
	uint32_t msgnum;
	uint8_t seen[];
} MsgSkipData;

static struct {
	mmap_t *map;
	MsgSkipData *data;
	int action;
	boolean valid;
} msgskip;

static void msg_callback(int msgid) {
	if (!msgskip.valid || !msgskip.data || (unsigned)msgid >= nact->ain.msgnum)
		return;
	uint8_t bit = 1 << (msgid & 7);
	if (msgskip.data->seen[msgid >> 3] & bit) {
		enable_msgSkip(TRUE);
	} else {
		msgskip.data->seen[msgid >> 3] |= bit;
		set_skipMode(FALSE);
		enable_msgSkip(FALSE);
	}
}

static int open_msgskip_file(const char *fname_utf8) {
	char *path = fc_get_path(fname_utf8);
	size_t length = ((nact->ain.msgnum + 7) >> 3) + 4;
	mmap_t *m = map_file_readwrite(path, length);
	free(path);
	if (!m)
		return 1;

	msgskip.map = m;
	msgskip.data = m->addr;
	msgskip.data->msgnum = nact->ain.msgnum;
	return 0;
}

static void Init() {
	int *p1 = getCaliVariable();
	int p2 = getCaliValue(); /* ISys3x */
	int p3 = getCaliValue();
	int p4 = getCaliValue();

	msgskip.action = p3;
	msgskip.valid = true;
	nact->msgskip_callback = &msg_callback;
	*p1 = 0;

	DEBUG_COMMAND("MsgSkip.Init %p,%d,%d,%d:\n", p1, p2, p3, p4);
}

static void Start() {
	int *p1 = getCaliVariable();
	int p2 = getCaliValue();

	char *file = sjis2utf(svar_get(p2));
	*p1 = open_msgskip_file(file);
	free(file);

	DEBUG_COMMAND("MsgSkip.Start %p,%d:\n", p1, p2);
}

static void SetValid() {
	int p1 = getCaliValue();

	msgskip.valid = p1;

	DEBUG_COMMAND("MsgSkip.SetValid %d:\n", p1);
}

static void GetValid() {
	int *p1 = getCaliVariable();

	*p1 = msgskip.valid;

	DEBUG_COMMAND("MsgSkip.GetValid %p:\n", p1);
}

static void SetAction() {
	int p1 = getCaliValue();

	msgskip.action = p1;

	DEBUG_COMMAND("MsgSkip.SetAcion %d:\n", p1);
}

static void GetAction() {
	int *p1 = getCaliVariable();

	*p1 = msgskip.action;

	DEBUG_COMMAND("MsgSkip.GetAcion %p:\n", p1);
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
