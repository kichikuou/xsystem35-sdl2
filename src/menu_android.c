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

#include "config.h"

#include <SDL.h>
#include <jni.h>

#include "system.h"
#include "portab.h"
#include "menu.h"
#include "nact.h"

#define STRING "Ljava/lang/String;"

void menu_open(void) {
	return;
}

void menu_quitmenu_open(void) {
	const SDL_MessageBoxButtonData buttons[] = {
		{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Quit" },
		{ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "Cancel" },
	};
	const SDL_MessageBoxData messagebox_data = {
		.flags = SDL_MESSAGEBOX_INFORMATION,
		.window = NULL,
		.title = "Quit game?",
		.message = "Any unsaved progress will be lost.",
		.numbuttons = SDL_arraysize(buttons),
		.buttons = buttons,
	};
	int buttonid = 0;
	if (SDL_ShowMessageBox(&messagebox_data, &buttonid) < 0) {
		WARNING("error displaying message box");
		return;
	}
	if (buttonid == 1) {
		nact_quit(false);
	}
}

bool menu_inputstring(INPUTSTRING_PARAM *p) {
	static char buf[256];
	JNIEnv *env = SDL_AndroidGetJNIEnv();
	if ((*env)->PushLocalFrame(env, 16) < 0) {
		WARNING("Failed to allocate JVM local references");
		return false;
	}

	jstring msg = (*env)->NewStringUTF(env, p->title);
	jstring oldstring = (*env)->NewStringUTF(env, p->oldstring);
	if (!msg || !oldstring) {
		WARNING("Failed to allocate a string");
		(*env)->PopLocalFrame(env, NULL);
		return false;
	}

	jobject context = SDL_AndroidGetActivity();
	jmethodID mid = (*env)->GetMethodID(env, (*env)->GetObjectClass(env, context),
										"inputString", "(" STRING STRING "I)" STRING);
	jstring newstring = (jstring)(*env)->CallObjectMethod(env, context, mid, msg, oldstring, p->max);
	if (newstring) {
		const char *newstr_utf8 = (*env)->GetStringUTFChars(env, newstring, NULL);
		strcpy(buf, newstr_utf8);
		(*env)->ReleaseStringUTFChars(env, newstring, newstr_utf8);
		(*env)->PopLocalFrame(env, NULL);
		p->newstring = buf;
		return true;
	}

	(*env)->PopLocalFrame(env, NULL);
	p->newstring = p->oldstring;
	return false;
}

bool menu_inputstring2(INPUTSTRING_PARAM *p) {
	p->newstring = p->oldstring;
	return true;
}

bool menu_inputnumber(INPUTNUM_PARAM *p) {
	JNIEnv *env = SDL_AndroidGetJNIEnv();
	if ((*env)->PushLocalFrame(env, 16) < 0) {
		WARNING("Failed to allocate JVM local references");
		return false;
	}

	jstring msg = (*env)->NewStringUTF(env, p->title);
	if (!msg) {
		WARNING("Failed to allocate a string");
		(*env)->PopLocalFrame(env, NULL);
		return false;
	}

	jobject context = SDL_AndroidGetActivity();
	jmethodID mid = (*env)->GetMethodID(env, (*env)->GetObjectClass(env, context),
										"inputNumber", "(" STRING "III)I");
	p->value = (*env)->CallIntMethod(env, context, mid, msg, p->min, p->max, p->def);

	(*env)->PopLocalFrame(env, NULL);
	return true;
}

void menu_init(void) {
	return;
}

void menu_gtkmainiteration() {
	return;
}

void menu_setSkipState(bool enabled, bool activated) {
}
