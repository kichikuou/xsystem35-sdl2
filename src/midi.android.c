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

#include "sdl_compat.h"
#include <jni.h>
#include <limits.h>
#include <stdio.h>
#include "system.h"
#include "portab.h"
#include "midi.h"
#include "music_private.h"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

static void midi_stop(void);
static int current_vol = 100;

static bool jni_check_clear_exception(JNIEnv *env) {
	if ((*env)->ExceptionCheck(env)) {
		(*env)->ExceptionClear(env);
		return true;
	}
	return false;
}

static float calc_midi_gain(void) {
	float g = (float)(current_vol * prv.volval[BGM_VOLVAL_CH]) / 10000.0f;
	if (g < 0.0f) return 0.0f;
	if (g > 1.0f) return 1.0f;
	return g;
}

static void call_activity_void(const char *method) {
	if (!method)
		return;
	JNIEnv *env = SDL_GetAndroidJNIEnv();
	if (!env)
		return;
	if ((*env)->PushLocalFrame(env, 16) < 0) {
		jni_check_clear_exception(env);
		WARNING("Failed to allocate JVM local references");
		return;
	}
	jobject context = SDL_GetAndroidActivity();
	jclass cls = NULL;
	if (jni_check_clear_exception(env) || !context)
		goto cleanup;
	cls = (*env)->GetObjectClass(env, context);
	if (jni_check_clear_exception(env) || !cls)
		goto cleanup;
	jmethodID mid = (*env)->GetMethodID(env, cls, method, "()V");
	if (jni_check_clear_exception(env) || !mid)
		goto cleanup;
	(*env)->CallVoidMethod(env, context, mid);
	jni_check_clear_exception(env);

cleanup:
	if (cls) (*env)->DeleteLocalRef(env, cls);
	if (context) (*env)->DeleteLocalRef(env, context);
	(*env)->PopLocalFrame(env, NULL);
}

static void midi_apply_volume(void) {
	JNIEnv *env = SDL_GetAndroidJNIEnv();
	if (!env)
		return;
	if ((*env)->PushLocalFrame(env, 16) < 0) {
		jni_check_clear_exception(env);
		WARNING("Failed to allocate JVM local references");
		return;
	}
	jobject context = SDL_GetAndroidActivity();
	jclass cls = NULL;
	if (jni_check_clear_exception(env) || !context)
		goto cleanup;
	cls = (*env)->GetObjectClass(env, context);
	if (jni_check_clear_exception(env) || !cls)
		goto cleanup;
	jmethodID mid = (*env)->GetMethodID(env, cls, "midiSetVolume", "(F)V");
	if (jni_check_clear_exception(env) || !mid)
		goto cleanup;
	(*env)->CallVoidMethod(env, context, mid, (jfloat)calc_midi_gain());
	jni_check_clear_exception(env);

cleanup:
	if (cls) (*env)->DeleteLocalRef(env, cls);
	if (context) (*env)->DeleteLocalRef(env, context);
	(*env)->PopLocalFrame(env, NULL);
}

static bool midi_initialize(int subdev) {
	current_vol = 100;
	return true;
}

static void midi_exit(void) {
	midi_stop();
}

static void midi_reset(void) {
	current_vol = 100;
	midi_stop();
}

static bool midi_start(int no, int loop, const uint8_t *data, int datalen) {
	(void)no;
	current_vol = 100;
	if (!data || datalen <= 0)
		return false;

	JNIEnv *env = SDL_GetAndroidJNIEnv();
	if (!env) {
		WARNING("Failed to obtain JNIEnv");
		return false;
	}

	const char *storage_path = SDL_GetAndroidInternalStoragePath();
	if (!storage_path || !storage_path[0]) {
		WARNING("Failed to obtain Android internal storage path");
		return false;
	}

	char path[PATH_MAX];
	int path_len = snprintf(path, sizeof(path), "%s/tmp.mid", storage_path);
	if (path_len < 0 || (size_t)path_len >= sizeof(path)) {
		WARNING("Temporary MIDI file path too long");
		return false;
	}

	FILE *fp = fopen(path, "wb");
	if (!fp) {
		WARNING("Failed to create temporary file");
		return false;
	}
	size_t size = (size_t)datalen;
	size_t written = fwrite(data, 1, size, fp);
	int close_rc = fclose(fp);
	if (written != size || close_rc != 0) {
		WARNING("Failed to write temporary MIDI file");
		return false;
	}

	if ((*env)->PushLocalFrame(env, 16) < 0) {
		jni_check_clear_exception(env);
		WARNING("Failed to allocate JVM local references");
		return false;
	}

	bool ok = false;
	jstring path_str = NULL;
	jobject context = NULL;
	jclass cls = NULL;

	path_str = (*env)->NewStringUTF(env, path);
	if (jni_check_clear_exception(env) || !path_str) {
		WARNING("Failed to allocate a string");
		goto cleanup;
	}

	context = SDL_GetAndroidActivity();
	if (jni_check_clear_exception(env) || !context)
		goto cleanup;

	cls = (*env)->GetObjectClass(env, context);
	if (jni_check_clear_exception(env) || !cls)
		goto cleanup;

	jmethodID setvol = (*env)->GetMethodID(env, cls, "midiSetVolume", "(F)V");
	if (!jni_check_clear_exception(env) && setvol) {
		(*env)->CallVoidMethod(env, context, setvol, (jfloat)calc_midi_gain());
		jni_check_clear_exception(env);
	}

	jmethodID mid = (*env)->GetMethodID(env, cls, "midiStart", "(Ljava/lang/String;I)V");
	if (jni_check_clear_exception(env) || !mid)
		goto cleanup;

	(*env)->CallVoidMethod(env, context, mid, path_str, (jint)loop);
	if (!jni_check_clear_exception(env))
		ok = true;

cleanup:
	if (cls) (*env)->DeleteLocalRef(env, cls);
	if (context) (*env)->DeleteLocalRef(env, context);
	if (path_str) (*env)->DeleteLocalRef(env, path_str);
	(*env)->PopLocalFrame(env, NULL);
	return ok;
}

static void midi_stop(void) {
	call_activity_void("midiStop");
}

static void midi_pause(void) {
	call_activity_void("midiPause");
}

static void midi_unpause(void) {
	call_activity_void("midiUnpause");
}

static bool midi_get_playing_info(midiplaystate *st) {
	if (!st)
		return false;
	st->in_play = false;
	st->loc_ms  = 0;

	JNIEnv *env = SDL_GetAndroidJNIEnv();
	if (!env)
		return false;
	if ((*env)->PushLocalFrame(env, 16) < 0) {
		jni_check_clear_exception(env);
		WARNING("Failed to allocate JVM local references");
		return false;
	}

	bool ok = false;
	jint pos = -1;
	jobject context = SDL_GetAndroidActivity();
	jclass cls = NULL;
	if (jni_check_clear_exception(env) || !context)
		goto cleanup;
	cls = (*env)->GetObjectClass(env, context);
	if (jni_check_clear_exception(env) || !cls)
		goto cleanup;
	jmethodID mid = (*env)->GetMethodID(env, cls, "midiCurrentPosition", "()I");
	if (jni_check_clear_exception(env) || !mid)
		goto cleanup;
	pos = (*env)->CallIntMethod(env, context, mid);
	if (!jni_check_clear_exception(env))
		ok = true;

cleanup:
	if (cls) (*env)->DeleteLocalRef(env, cls);
	if (context) (*env)->DeleteLocalRef(env, context);
	(*env)->PopLocalFrame(env, NULL);

	if (!ok)
		return false;
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
	current_vol = volume < 0 ? 0 : (volume > 100 ? 100 : volume);
	if (time <= 0) {
		midi_apply_volume();
		if (stop)
			midi_stop();
		return true;
	}
	JNIEnv *env = SDL_GetAndroidJNIEnv();
	if (!env)
		return false;
	if ((*env)->PushLocalFrame(env, 16) < 0) {
		jni_check_clear_exception(env);
		WARNING("Failed to allocate JVM local references");
		return false;
	}

	bool ok = false;
	jobject context = SDL_GetAndroidActivity();
	jclass cls = NULL;
	if (jni_check_clear_exception(env) || !context)
		goto cleanup;
	cls = (*env)->GetObjectClass(env, context);
	if (jni_check_clear_exception(env) || !cls)
		goto cleanup;
	jmethodID mid = (*env)->GetMethodID(env, cls, "midiFadeStart", "(IFZ)V");
	if (jni_check_clear_exception(env) || !mid)
		goto cleanup;
	(*env)->CallVoidMethod(env, context, mid, (jint)time, (jfloat)calc_midi_gain(), (jboolean)(stop != 0));
	if (!jni_check_clear_exception(env))
		ok = true;

cleanup:
	if (cls) (*env)->DeleteLocalRef(env, cls);
	if (context) (*env)->DeleteLocalRef(env, context);
	(*env)->PopLocalFrame(env, NULL);
	return ok;
}

static bool midi_fading(void) {
	JNIEnv *env = SDL_GetAndroidJNIEnv();
	if (!env)
		return false;
	if ((*env)->PushLocalFrame(env, 16) < 0) {
		jni_check_clear_exception(env);
		return false;
	}

	jboolean fading = JNI_FALSE;
	jobject context = SDL_GetAndroidActivity();
	jclass cls = NULL;
	if (jni_check_clear_exception(env) || !context)
		goto cleanup;
	cls = (*env)->GetObjectClass(env, context);
	if (jni_check_clear_exception(env) || !cls)
		goto cleanup;
	jmethodID mid = (*env)->GetMethodID(env, cls, "midiIsFading", "()Z");
	if (jni_check_clear_exception(env) || !mid)
		goto cleanup;
	fading = (*env)->CallBooleanMethod(env, context, mid);
	if (jni_check_clear_exception(env))
		fading = JNI_FALSE;

cleanup:
	if (cls) (*env)->DeleteLocalRef(env, cls);
	if (context) (*env)->DeleteLocalRef(env, context);
	(*env)->PopLocalFrame(env, NULL);
	return fading != JNI_FALSE;
}

static void midi_reapply_volume(void) {
	midi_apply_volume();
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
	.reapply_volume = midi_reapply_volume,
};
