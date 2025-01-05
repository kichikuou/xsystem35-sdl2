/*
 * Copyright (C) 2025 <KichikuouChrome@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#include "config.h"
#include "modules.h"
#include "nact.h"
#include "bgm.h"
#include "music.h"
#include "music_pcm.h"

#define SLOT 128

static int current_no = 0;

static void Init(void) {
	int isys3x = getCaliValue();
	TRACE("S3xMusic.Init %d:", isys3x);
}

static void CreateChannel(void) {
	int nr_channels = getCaliValue();
	if (nr_channels != 2)
		SYSERROR("Unsupported number of channels: %d", nr_channels);
	TRACE("S3xMusic.CreateChannel %d:", nr_channels);
}

static void Prepare(void) {
	int ch = getCaliValue();
	int no = getCaliValue();
	int *result = getCaliVariable();

	if (ch == 0) {
		current_no = no;
		*result = 1;
	} else {
		*result = muspcm_load_bgm(SLOT, no) == OK;
	}
	TRACE("S3xMusic.Prepare %d, %d => %d:", ch, no, *result);
}

static void Unprepare(void) {
	int ch = getCaliValue();

	if (ch == 0) {
		if (current_no) {
			musbgm_stop(current_no, 0);
			current_no = 0;
		}
	} else {
		muspcm_unload(SLOT);
	}
	TRACE("S3xMusic.Unprepare %d:", ch);
}

static void Play(void) {
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	TRACE_UNIMPLEMENTED("S3xMusic.Play %d, %d:", p1, p2);
}

static void FadePlay(void) {
	int ch = getCaliValue();
	int fadein_time = getCaliValue();
	int volume = getCaliValue();
	int loop_count = getCaliValue();  // 0 = infinite

	if (ch == 0) {
		musbgm_play(current_no, fadein_time, volume, loop_count);
	} else {
		muspcm_start(SLOT, loop_count == 0 ? -1 : loop_count);
	}
	TRACE("S3xMusic.FadePlay %d, %d, %d, %d:", ch, fadein_time, volume, loop_count);
}

static void PlayPosSample(void) {
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	TRACE_UNIMPLEMENTED("S3xMusic.PlayPosSample %d, %d, %d, %d:", p1, p2, p3, p4);
}

static void IsPlay(void) {
	int ch = getCaliValue();
	int *result = getCaliVariable();

	if (ch == 0) {
		*result = current_no && musbgm_isplaying(current_no);
	} else {
		*result = muspcm_isplaying(SLOT);
	}
	TRACE("S3xMusic.IsPlay %d => %d:", ch, *result);
}

static void Stop(void) {
	int ch = getCaliValue();

	if (ch == 0) {
		if (current_no)
			musbgm_stop(current_no, 0);
		current_no = 0;
	} else {
		muspcm_stop(SLOT);
	}
	TRACE("S3xMusic.Stop %d:", ch);
}

static void Restart(void) {
	int p1 = getCaliValue();
	TRACE_UNIMPLEMENTED("S3xMusic.Restart %d:", p1);
}

static void GetPlayPos(void) {
	int ch = getCaliValue();
	int *hour = getCaliVariable();
	int *min = getCaliVariable();
	int *sec = getCaliVariable();
	int *msec = getCaliVariable();
	if (ch == 0) {
		int time = musbgm_getpos(current_no);
		*hour = time / 360000;
		*min = (time / 6000) % 60;
		*sec = (time / 100) % 60;
		*msec = (time % 100) * 10;
	} else {
		// not implemented
		*hour = 0;
		*min = 0;
		*sec = 0;
		*msec = 0;
	}
	TRACE("S3xMusic.GetPlayPos %d => %d, %d, %d, %d:", ch, *hour, *min, *sec, *msec);
}

static void GetPlayPosSample(void) {
	int p1 = getCaliValue();
	int *var1 = getCaliVariable();
	int *var2 = getCaliVariable();
	TRACE_UNIMPLEMENTED("S3xMusic.GetPlayPosSample %d, %p, %p:", p1, var1, var2);
}

static void SetRepeatStart(void) {
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	TRACE_UNIMPLEMENTED("S3xMusic.SetRepeatStart %d, %d, %d:", p1, p2, p3);
}

static void SetRepeatEnd(void) {
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	TRACE_UNIMPLEMENTED("S3xMusic.SetRepeatEnd %d, %d, %d:", p1, p2, p3);
}

static void FadeVolume(void) {
	int ch = getCaliValue();
	int time = getCaliValue();
	int volume = getCaliValue();
	int stop_after_fade = getCaliValue();

	if (ch == 0) {
		if (stop_after_fade) {
			musbgm_stop(current_no, time / 10);
		} else {
			musbgm_fade(current_no, time / 10, volume);
		}
	} else {
		if (stop_after_fade) {
			muspcm_fadeout(SLOT, time);
		} else {
			muspcm_setvol(SLOT, volume);
		}
	}
	TRACE("S3xMusic.FadeVolume %d, %d, %d, %d:", ch, time, volume, stop_after_fade);
}

static void IsFade(void) {
	int ch = getCaliValue();
	int *result = getCaliVariable();
	*result = 0;
	TRACE_UNIMPLEMENTED("S3xMusic.IsFade %d => %d:", ch, *result);
}

static void StopFade(void) {
	int ch = getCaliValue();
	TRACE_UNIMPLEMENTED("S3xMusic.StopFade %d:", ch);
}

static void WaitEnd(void) {
	int p1 = getCaliValue();
	int p2 = getCaliValue();  // IWinMsg
	TRACE_UNIMPLEMENTED("S3xMusic.WaitEnd %d, %d:", p1, p2);
}

static void sleep(void) {
	int p1 = getCaliValue();  // IWinMsg
	TRACE_UNIMPLEMENTED("S3xMusic.sleep %d:", p1);
}

static const ModuleFunc functions[] = {
	{"CreateChannel", CreateChannel},
	{"FadePlay", FadePlay},
	{"FadeVolume", FadeVolume},
	{"GetPlayPos", GetPlayPos},
	{"GetPlayPosSample", GetPlayPosSample},
	{"Init", Init},
	{"IsFade", IsFade},
	{"IsPlay", IsPlay},
	{"Play", Play},
	{"PlayPosSample", PlayPosSample},
	{"Prepare", Prepare},
	{"Restart", Restart},
	{"SetRepeatEnd", SetRepeatEnd},
	{"SetRepeatStart", SetRepeatStart},
	{"Stop", Stop},
	{"StopFade", StopFade},
	{"Unprepare", Unprepare},
	{"WaitEnd", WaitEnd},
	{"sleep", sleep},
};

const Module module_S3xMusic = {"S3xMusic", functions, sizeof(functions) / sizeof(ModuleFunc)};
