/*
 * Copyright (C) 2020 <KichikuouChrome@gmail.com>
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

#include <SDL.h>
#include <jni.h>
#include "system.h"
#include "portab.h"
#include "midi.h"

static void midi_stop(void);

static bool midi_initialize(int subdev) {
	return true;
}

static void midi_exit(void) {
	midi_stop();
}

static void midi_reset(void) {
	midi_stop();
}

static bool midi_start(int no, int loop, const uint8_t *data, int datalen) {
	JNIEnv *env = SDL_AndroidGetJNIEnv();
	if ((*env)->PushLocalFrame(env, 16) < 0) {
		WARNING("Failed to allocate JVM local references");
		return false;
	}

	char path[PATH_MAX];
	snprintf(path, PATH_MAX, "%s/tmp.mid", SDL_AndroidGetInternalStoragePath());
	FILE* fp = fopen(path, "w");
	if (!fp) {
		WARNING("Failed to create temporary file");
		(*env)->PopLocalFrame(env, NULL);
		return false;
	}
	fwrite(data, datalen, 1, fp);
	fclose(fp);

	jstring path_str = (*env)->NewStringUTF(env, path);
	if (!path_str) {
		WARNING("Failed to allocate a string");
		(*env)->PopLocalFrame(env, NULL);
		return false;
	}

	jobject context = SDL_AndroidGetActivity();
	jmethodID mid = (*env)->GetMethodID(env, (*env)->GetObjectClass(env, context),
										"midiStart", "(Ljava/lang/String;Z)V");
	(*env)->CallVoidMethod(env, context, mid, path_str, loop == 0);
	(*env)->PopLocalFrame(env, NULL);

	return true;
}

static void midi_stop(void) {
	JNIEnv *env = SDL_AndroidGetJNIEnv();
	if ((*env)->PushLocalFrame(env, 16) < 0) {
		WARNING("Failed to allocate JVM local references");
		return;
	}
	jobject context = SDL_AndroidGetActivity();
	jmethodID mid = (*env)->GetMethodID(env, (*env)->GetObjectClass(env, context),
										"midiStop", "()V");
	(*env)->CallVoidMethod(env, context, mid);
	(*env)->PopLocalFrame(env, NULL);
}

static void midi_pause(void) {
	// FIXME
}

static void midi_unpause(void) {
	// FIXME
}

static bool midi_get_playing_info(midiplaystate *st) {
	st->in_play = false;
	st->loc_ms  = 0;

	JNIEnv *env = SDL_AndroidGetJNIEnv();
	if ((*env)->PushLocalFrame(env, 16) < 0) {
		WARNING("Failed to allocate JVM local references");
		return false;
	}
	jobject context = SDL_AndroidGetActivity();
	jmethodID mid = (*env)->GetMethodID(env, (*env)->GetObjectClass(env, context),
										"midiCurrentPosition", "()I");
	int pos = (*env)->CallIntMethod(env, context, mid);
	(*env)->PopLocalFrame(env, NULL);

	if (pos >= 0) {
		st->in_play = true;
		st->loc_ms = pos;
	}
	return true;
}

static int midi_getflag(int mode, int index) {
	return 0;
}

static bool midi_setflag(int mode, int index, int val) {
	return false;
}

static bool midi_fadestart(int time, int volume, int stop) {
	return false; // FIXME
}

static bool midi_fading(void) {
	return false; // FIXME
}

mididevice_t midi_android = {
	.init = midi_initialize,
	.exit = midi_exit,
	.reset = midi_reset,
	.start = midi_start,
	.stop = midi_stop,
	.pause = midi_pause,
	.unpause = midi_unpause,
	.getpos = midi_get_playing_info,
	.getflag = midi_getflag,
	.setflag = midi_setflag,
	.fadestart = midi_fadestart,
	.fading = midi_fading,
};
