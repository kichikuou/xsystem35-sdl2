#include "config.h"

#include <stdio.h>
#include "portab.h"

#include "music.h"
#include "night.h"

struct _ntsnd {
	int no;
	int vol;
	int loop;
	bool waitend;
};
static struct _ntsnd ntsnd[10];


void nt_voice_set(int no) {
	if (night.waitskiplv < 1) {
		mus_wav_load(100, no);
		mus_wav_play(100, 1);
	}
}

void nt_cd_play(int no) {
	muscd_start(no + 1, 0);
}

void nt_cd_stop(int msec) {
	mus_mixer_fadeout_start(MIX_CD, msec, 0, true);
}

void nt_cd_mute(bool mute) {
	if (mute) {
		mus_mixer_fadeout_start(MIX_CD, 0, 0, false);
	} else {
		mus_mixer_fadeout_start(MIX_CD, 0, 100, false);
	}
}

void nt_snd_setwave(int ch, int no) {
	if (ch < 1) return;
	ntsnd[ch -1].no = no;
	mus_wav_load(ch, no);
}

void nt_snd_setloop(int ch, int num) {
	if (ch < 1) return;

	ntsnd[ch -1].loop = num;
}

void nt_snd_setvol(int ch, int vol) {
	if (ch < 1) return;

	ntsnd[ch -1].vol = vol;
}

void nt_snd_waitend(int ch, bool waitend) {
	if (ch < 1) return;

	ntsnd[ch -1].waitend = waitend;
}

void nt_snd_play(int ch) {
	if (ch < 1) return;

	mus_wav_play(ch, ntsnd[ch -1].loop);
	if (ntsnd[ch -1].waitend) {
		mus_wav_waitend(ch);
	}
}

void nt_snd_stop(int ch, int msec) {
	if (ch < 1) return;
	
	mus_wav_fadeout_start(ch, msec, 0, true);
}

void nt_snd_stopall(int msec) {
	int i;
	
	for (i = 1; i <= 10; i++) {
		mus_wav_fadeout_start(i, msec, 0, true);
	}
}
