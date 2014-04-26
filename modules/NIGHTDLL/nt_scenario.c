#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "portab.h"
#include "system.h"
#include "counter.h"
#include "imput.h"
#include "xsystem35.h"
#include "nact.h"
#include "scenario.h"
#include "s39ain.h"
#include "night.h"
#include "nt_msg.h"

/* defined by cmd_check.c */
extern void check_command(int c0);

#define MAINLOOP_EVENTCHECK_INTERVAL 16 /* 16 msec */


struct _scoadr {
	int page;
	int index;
};

static struct _scoadr scene2adr(int no) {
	int i;
	char label[7] = {0};
	struct _scoadr adr = {0, 0};
	
	snprintf(label, 7, "E%05d", no);
	WARNING("seaching %6s\n", label);
	for (i = 0; i < nact->ain.fncnum; i++) {
		if (0 == strncmp(nact->ain.fnc[i].name, label, 6)) {
			adr.page  = nact->ain.fnc[i].page;
			adr.index = nact->ain.fnc[i].index;
			return adr;
		}
	}
	
	WARNING("no scene %d\n", no);
	return adr;
}


void nt_sco_init() {
}

static void ntmain(struct _scoadr inadr) {
	int scono = 0, cnt = 0;
	struct _scoadr curadr;

	while (!nact->is_quit) {
		DEBUG_MESSAGE("%d:%x\n", sl_getPage(), sl_getIndex());
		//WARNING("%d:%x\n", sl_getPage(), sl_getIndex());
		if (!nact->popupmenu_opened) {
			check_command(sl_getc());
			if (sl_getPage()  == inadr.page &&
			    sl_getIndex() == inadr.index) {
				// ~E%05dからの戻り
				if (nact->fnc_return_value == 0) {
					break;
				} else {
					scono = nact->fnc_return_value;
				}
				curadr = scene2adr(scono);
				sl_callFar2(curadr.page -1, curadr.index);
			}
		}
		
		if (!nact->is_message_locked) {
			if (get_high_counter(SYSTEMCOUNTER_MAINLOOP)) {
				sys_getInputInfo();
				reset_counter_high(SYSTEMCOUNTER_MAINLOOP, MAINLOOP_EVENTCHECK_INTERVAL, 0);
			}
		}
		if (cnt == 10000) {
			usleep(10); /* XXXX */
			cnt = 0;
		}
		cnt++;
		nact->callback();
	}
}

/*
  mode = 0: はじめから
         1: 途中から
*/
int nt_sco_main(int mode) {
	int scono = 1, cnt = 0;
	struct _scoadr stadr, curadr;
	
	sys_exit(2);
	
	return 1;
}

void nt_sco_callevent(int ev) {
	struct _scoadr stadr, curadr;
	
	// InitGameMainが呼ばれたときのアドレス=関数終了時の戻りアドレス
	stadr.page = sl_getPage();
	stadr.index = sl_getIndex();
	
	// シーン１
	curadr = scene2adr(ev);
	sl_callFar2(curadr.page -1, curadr.index);
	
	ntmain(stadr);
	
	sl_jmpFar2(stadr.page, stadr.index);
}


boolean nt_sco_is_natsu() {
	if (night.Month < 6 || night.Month > 9) return FALSE;
	if (night.Month == 6 && night.Day < 11) return FALSE;
	return TRUE;
}

