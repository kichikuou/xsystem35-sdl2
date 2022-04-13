// #define DDEMODEV

#include "config.h"

#include <stdio.h>

#include "portab.h"
#include "system.h"
#include "xsystem35.h"
#include "modules.h"
#include "nact.h"
#include "input.h"
#include "music.h"

#ifdef DDEMODEV
#include "dDemo.h"
static struct ddemo dd;
#include "scene.c"
#endif

static void Init() {
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int *var = getCaliVariable();
	
	*var = 1;
#ifdef DDEMODEV
	dd.alk = alk_new("/home/masaki-c/game/daiakuji/dDemo.alk");
#endif
	
	DEBUG_COMMAND_YET("dDemo.Init %d,%d,%d,%p:\n", p1, p2, p3, var);
}

static void SetKeyCancelFlag() {
	int cancelflag = getCaliValue();
	
	DEBUG_COMMAND_YET("dDemo.SetKeyCancelFlag %d:\n", cancelflag);
}

static void SetLoopFlag() {
	/* Loop Flag */
	int loopflag = getCaliValue(); /* 0 なら無限繰り返し */
	
	DEBUG_COMMAND_YET("dDemo.SetLoopFlag %d:\n", loopflag);
}

static void Run() {
	DEBUG_COMMAND_YET("dDemo.Run:\n");
	
#ifdef DDEMODEV
	mus_cdrom_start(13, 1);
	// ddemo_scene();
	
	while(0 == sys_getInputInfo()) {
		usleep(1000 * 100);
	}

	mus_cdrom_stop();
#endif
}

static const ModuleFunc functions[] = {
	{"Init", Init},
	{"Run", Run},
	{"SetKeyCancelFlag", SetKeyCancelFlag},
	{"SetLoopFlag", SetLoopFlag},
};

const Module module_dDemo = {"dDemo", functions, sizeof(functions) / sizeof(ModuleFunc)};
