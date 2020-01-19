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
#include "portab.h"
#include "cdrom.h"

static int cdrom_init(char *);
static int cdrom_exit();
static int cdrom_start(int, int);
static int cdrom_stop();
static int cdrom_getPlayingInfo(cd_time *);

#define cdrom cdrom_android
cdromdevice_t cdrom = {
	cdrom_init,
	cdrom_exit,
	cdrom_start,
	cdrom_stop,
	cdrom_getPlayingInfo,
	NULL,
	NULL
};

int cdrom_init(char *name) {
	return OK;
}

int cdrom_exit() {
	cdrom_stop();
	return OK;
}

#define COMMAND_CDROM_START 0x8000

int cdrom_start(int trk, int loop) {
	JNIEnv *env = SDL_AndroidGetJNIEnv();
	jobject context = SDL_AndroidGetActivity();
	jmethodID mid = (*env)->GetMethodID(env, (*env)->GetObjectClass(env, context),
										"cddaStart", "(II)V");
	(*env)->CallVoidMethod(env, context, mid, trk, loop);
	(*env)->DeleteLocalRef(env, context);
	return OK;
}

int cdrom_stop() {
	JNIEnv *env = SDL_AndroidGetJNIEnv();
	jobject context = SDL_AndroidGetActivity();
	jmethodID mid = (*env)->GetMethodID(env, (*env)->GetObjectClass(env, context),
										"cddaStop", "()V");
	(*env)->CallVoidMethod(env, context, mid);
	(*env)->DeleteLocalRef(env, context);
	return OK;
}

int cdrom_getPlayingInfo (cd_time *info) {
	JNIEnv *env = SDL_AndroidGetJNIEnv();
	jobject context = SDL_AndroidGetActivity();
	jmethodID mid = (*env)->GetMethodID(env, (*env)->GetObjectClass(env, context),
										"cddaCurrentPosition", "()I");
	int t = (*env)->CallIntMethod(env, context, mid);
	(*env)->DeleteLocalRef(env, context);

	info->t = t & 0xff;
	FRAMES_TO_MSF(t >> 8, &info->m, &info->s, &info->f);
	return OK;
}
