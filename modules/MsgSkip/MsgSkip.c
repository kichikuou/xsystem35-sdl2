#include "config.h"

#include <stdio.h>
#include <string.h>

#include "portab.h"
#include "system.h"
#include "xsystem35.h"
#include "nact.h"
#include "message.h"

static char *msgskipfile;

void Init() {
        int *p1 = getCaliVariable();
        int p2 = getCaliValue(); /* ISys3x */
        int p3 = getCaliValue();
        int p4 = getCaliValue();

	
	DEBUG_COMMAND_YET("MsgSkip.Init %p,%d,%d,%d:\n", p1, p2, p3, p4);
}

void Start() {
        int *p1 = getCaliVariable();
        int p2 = getCaliValue();

	msgskipfile = strdup(v_str(p2 -1));
	
	DEBUG_COMMAND_YET("MsgSkip.Start %p,%d:\n", p1, p2);
}

void SetValid() {
        int p1 = getCaliValue();

	
	DEBUG_COMMAND("MsgSkip.SetValid %d:\n", p1);
}

void GetValid() {
        int *p1 = getCaliVariable();
	
	DEBUG_COMMAND_YET("MsgSkip.GetValid %p:\n", p1);
}

void SetAction() {
        int p1 = getCaliValue();
	
	DEBUG_COMMAND("MsgSkip.SetAcion %d:\n", p1);
}

void GetAction() {
        int *p1 = getCaliVariable();
	
	DEBUG_COMMAND_YET("MsgSkip.GetAcion %p:\n", p1);
}

void PushStr() {
        int p1 = getCaliValue();
	
	DEBUG_COMMAND_YET("MsgSkip.PushStr %d:\n", p1);
}

void PopStr() {
        int p1 = getCaliValue();
	
	DEBUG_COMMAND_YET("MsgSkip.PopStr %d:\n", p1);
}

