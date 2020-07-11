/*
 * xsystem35.c  SYSTEM35 デコーダ
 *
 * Copyright (C) 1997-1998 Masaki Chikama (Wren) <chikama@kasumi.ipl.mech.nagoya-u.ac.jp>
 *               1998-                           <masaki-c@is.aist-nara.ac.jp>
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
/* $Id: xsystem35.c,v 1.77 2003/11/16 15:29:52 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

#ifdef HAVE_SIGACTION
#include <signal.h>
#endif

#ifdef ENABLE_GTK
#  define GTK_RC_NAME ".gtk/gtkrc"
#  include <gtk/gtk.h>
#endif

#ifdef ENABLE_SDL
#  include <SDL.h>
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#ifdef __ANDROID__
#include <android/log.h>
#endif

#include "nact.h"
#include "portab.h"
#include "xsystem35.h"
#include "nact.h"
#include "profile.h"
#include "randMT.h"
#include "counter.h"
#include "ags.h"
#include "graphicsdevice.h"
#include "menu.h"
#include "music.h"
#include "scenario.h"
#include "variable.h"
#include "gameresource.h"
#include "filecheck.h"
#include "joystick.h"
#include "s39init.h"

#ifdef ENABLE_MMX
#include "haveunit.h"
#endif

static char *gameResourceFile = "xsystem35.gr";
static void    sys35_usage(boolean verbose);
static void    sys35_init();
static void    sys35_remove();
static void    sys35_ParseOption(int *argc, char **argv);
static void    check_profile();

/* for debugging */
static FILE *fpdebuglog;
static int debuglv = DEBUGLEVEL;
int sys_nextdebuglv;

static int audio_buffer_size = 0;

/* font name from rcfile */
static char *fontname[FONTTYPEMAX];
static char *fontname_tt[FONTTYPEMAX] = {DEFAULT_GOTHIC_TTF, DEFAULT_MINCHO_TTF};
static boolean isjix0213_tt[FONTTYPEMAX];
static char fontface[FONTTYPEMAX];

#ifdef ENABLE_SDLTTF
#define DEFAULT_FONT_DEVICE FONT_SDLTTF
#elif defined(ENABLE_SDL)
#define DEFAULT_FONT_DEVICE FONT_FT2
#else
#define DEFAULT_FONT_DEVICE FONT_X11
#endif

static fontdev_t fontdev = DEFAULT_FONT_DEVICE;
static boolean font_noantialias;

/* fullscreen on from command line */
static boolean fs_on;

// for reboot
static int saved_argc;
static char **saved_argv;

static void sys35_usage(boolean verbose) {
	if (verbose) {
		puts("System35 for X Window System [proj. Rainy Moon]");
		puts("             (C) Masaki Chikama(Wren) 1997-2002");
		puts("                                  Version "VERSION"\n");
	}
	puts("Usage: xsystem35 [OPTIONS]\n");
	puts("OPTIONS");
	puts(" -gamefile file : set game resouce file to 'file'");
	puts(" -no-shm        : don't use MIT-SHM (use in another display)");
	puts(" -devcd device  : set cdrom device name to 'device'");
	puts(" -devmidi device: set midi device name to 'device'");
	
	puts(" -M?            : select output midi methos");
#ifdef ENABLE_MIDI_SDLMIXER
	puts(" -Me            : SDL_mixer midi player");
#endif
#ifdef ENABLE_MIDI_RAWMIDI
	puts(" -Mr            : Raw Midi device");
#endif
#ifdef ENABLE_MIDI_SEQMIDI
	puts(" -Ms?           : Sequenceer device (?:devicenumber)");
#endif
	puts(" -M0            : Disable MIDI output");
	
	puts(" -devjoy device : set joystic device name to 'device'");
	puts("                    if 'device' is set to 'none', don't use the device");
	puts(" -savekanji #   : kanji code of filename (0 or 1 ... 0:utf-8, 1:sjis)");

	puts(" -devfont device: select font device");
#ifdef ENABLE_SDLTTF
	puts(" -devfont sdl   : SDL_ttf");
#endif
#ifdef ENABLE_FT2
	puts(" -devfont ttf   : FreeType");
#endif
#ifdef ENABLE_X11FONT
	puts(" -devfont x11   : x11");
#endif

#ifdef ENABLE_SDLTTF
	puts("                : default is sdl");
#elif defined(ENABLE_SDL)
	puts("                : default is ft2");
#else
	puts("                : default is x11");
#endif

#if defined(ENABLE_SDLTTF) || defined(ENABLE_FT2)
	puts(" -ttfont_mincho: set TrueType font for mincho");
	puts(" -ttfont_gothic: set TrueType font for mincho");
#endif
	puts(" -font_mincho  : set X11(gtk) font for mincho");
	puts(" -font_gothic  : set X11(gtk) font for mincho");
	
#ifdef DEBUG
	puts(" -debuglv #     : debug level");
	puts("                :  0: critical error message only ");
	puts("                :  1: + waring message");
	puts("                :  2: + not implemented command message");
	puts("                :  5: + implemented command (write to logfile)");
	puts("                :  6: + message (write to logfile)");
#endif  
	puts(" -noantialias   : never use antialiased string");
	puts(" -fullscreen    : start with fullscreen");
	puts(" -noimagecursor : disable image cursor");
	puts(" -version       : show version");
	puts(" -h             : show this message");
	puts(" --help         : show this message");
	exit(1);
}

void sys_message(char *format, ...) {
	if (debuglv < sys_nextdebuglv)
		return;

	va_list args;
	va_start(args, format);

#ifdef __ANDROID__
	const int prio_table[] = {
		ANDROID_LOG_FATAL,
		ANDROID_LOG_ERROR,
		ANDROID_LOG_WARN,
		ANDROID_LOG_INFO,
		ANDROID_LOG_INFO,
		ANDROID_LOG_VERBOSE,
	};
	int prio = prio_table[min(sys_nextdebuglv, 5)];
	__android_log_vprint(prio, "xsystem35", format, args);
#elif defined (DEBUG)
	if (sys_nextdebuglv >= 5) {
		vfprintf(fpdebuglog, format, args);
		fflush(fpdebuglog);
	} else {
		vfprintf(stderr, format, args);
	}
#else
	vfprintf(stderr, format, args);
#endif
	va_end(args);
}

void sys_error(char *format, ...) {
	va_list args;
	
	va_start(args, format);
#ifdef __ANDROID__
	__android_log_vprint(ANDROID_LOG_FATAL, "xsystem35", format, args);
#else
	vfprintf(stderr, format, args);
#endif
	va_end(args);
	sys35_remove();
	exit(1);
}

void sys_exit(int code) {
	sys35_remove();
#ifdef __EMSCRIPTEN__
	EM_ASM( xsystem35.shell.quit(); );
	Sleep(1000000000);
#else
	exit(code);
#endif
}

static int check_fontdev(char *devname) {
#ifdef ENABLE_SDLTTF
	if (0 == strcmp(devname, "sdl")) {
		return FONT_SDLTTF;
	}
#endif
#ifdef ENABLE_FT2
	if (0 == strcmp(devname, "ft2")) {
		return FONT_FT2;
	}
#endif
#ifdef ENABLE_X11FONT
	if (0 == strcmp(devname, "x11")) {
		return FONT_X11;
	}
#endif
	return DEFAULT_FONT_DEVICE;
}

static void sys35_init() {
	int i;
	
	nact_init();
	
	sl_init();

	v_initVars();
	
	nact->fontdev = fontdev;
	
	ags_init();

	for (i = 0; i < FONTTYPEMAX; i++) {
		switch(fontdev) {
		case FONT_FT2:
		case FONT_SDLTTF:
			nact->ags.font->name[i] = fontname_tt[i];
			nact->ags.font->isJISX0213[i] = isjix0213_tt[i];
			nact->ags.font->face[i] = fontface[i];
			break;
			
		case FONT_X11:
			if (fontname[i] == NULL) {
				nact->ags.font->name[i] = strdup(FONT_DEFAULTNAME_X);
			} else {
				nact->ags.font->name[i] = fontname[i];
			}
			break;
		default:
			break;
		}
	}
	
	ags_fullscreen(fs_on);
	nact->noantialias = font_noantialias;
	ags_setAntialiasedStringMode(!font_noantialias);
	
	
	reset_counter(0);

	sgenrand(getpid());

#ifdef ENABLE_MMX
	nact->mmx_is_ok = ((haveUNIT() & tMMX) ? TRUE : FALSE);
#endif

	msg_init();
	sel_init();

	s39ain_init();
#ifdef ENABLE_GTK
	s39ini_init();
#endif
}

static void sys35_remove() {
	mus_exit(); 
	ags_remove();
#ifdef ENABLE_GTK
	s39ini_remove();
#endif
	/* joy_close(); */
#if DEBUG
	if (debuglv >= 3) {
		fclose(fpdebuglog);
	}
#endif
}

void sys_reset() {
	mus_exit();
	ags_remove();
#ifdef ENABLE_GTK
	s39ini_remove();
#endif
	
	execvp(saved_argv[0], saved_argv);
	sys_error("exec fail");
}

static void sys35_ParseOption(int *argc, char **argv) {
	int i;
	FILE *fp;
	for (i = 1; i < *argc; i++) {
		if (!strcmp(argv[i], "-h")) {
			sys35_usage(TRUE);
		}
		if (!strcmp(argv[i], "--help")) {
			sys35_usage(TRUE);
		}
	}
	for (i = 1; i < *argc; i++) {
		if (0 == strcmp(argv[i], "-gamefile")) {
			if (i == *argc - 1) {
				fprintf(stderr, "xsystem35: The -gamefile option requires file value\n\n");
				sys35_usage(FALSE);
			}
			if (NULL == (fp = fopen(argv[i + 1],"r"))) {
				fprintf(stderr, "xsystem35: gamefile '%s' not found\n\n", argv[i + 1]);
				sys35_usage(FALSE);
			}
			fclose(fp);
			gameResourceFile = argv[i + 1];
		} else if (0 == strcmp(argv[i], "-no-shm")) {
			SetNoShmMode();
		} else if (0 == strcmp(argv[i], "-devcd")) {
			if (argv[i + 1] != NULL) {
				cd_set_devicename(argv[i + 1]);
			}
		} else if (0 == strcmp(argv[i], "-devmidi")) {
			if (argv[i + 1] != NULL) {
				midi_set_devicename(argv[i + 1]);
			}
		} else if (0 == strncmp(argv[i], "-M", 2)) {
			int subdev = 0;
			if (argv[i][3] != '\0') {
				subdev = (argv[i][3] - '0') << 8;
			}
			midi_set_output_device(argv[i][2] | subdev);
		} else if (0 == strcmp(argv[i], "-devjoy")) {
			if (argv[i + 1] != NULL) {
				joy_set_devicename(argv[i + 1]);
			}
		} else if (0 == strcmp(argv[i], "-savekanji")) {
			if (argv[i + 1] != NULL) {
				fc_set_default_kanjicode(argv[i + 1][0] - '0');
			}
		} else if (0 == strcmp(argv[i], "-fullscreen")) {
			fs_on = TRUE;
		} else if (0 == strcmp(argv[i], "-noantialias")) {
			font_noantialias = TRUE;
		} else if (0 == strcmp(argv[i], "-devfont")) {
			if (argv[i + 1] != NULL) {
				fontdev = check_fontdev(argv[i + 1]);
			}
		} else if (0 == strcmp(argv[i], "-font_gothic")) {
			if (argv[i + 1] != NULL) {
				fontname[FONT_GOTHIC] = argv[i + 1];
			}
		} else if (0 == strcmp(argv[i], "-font_mincho")) {
			if (argv[i + 1] != NULL) {
				fontname[FONT_MINCHO] = argv[i + 1];
			}
		} else if (0 == strcmp(argv[i], "-ttfont_gothic")) {
			if (argv[i + 1] != NULL) {
				fontname_tt[FONT_GOTHIC] = argv[i + 1];
			}
		} else if (0 == strcmp(argv[i], "-ttfont_mincho")) {
			if (argv[i + 1] != NULL) {
				fontname_tt[FONT_MINCHO] = argv[i + 1];
			}
		} else if (0 == strcmp(argv[i], "-noimagecursor")) {
			nact->noimagecursor = TRUE;
		} else if (0 == strcmp(argv[i], "-debuglv")) {
			if (argv[i + 1] != NULL) {
				debuglv = argv[i + 1][0] - '0';
			}
		} else if (0 == strcmp(argv[i], "-version")) {
			puts(VERSION);
			exit(0);
		}
	}
}

static void check_profile() {
	char *param;
	
	/* フォントデバイスの選択 */
	param = get_profile("font_device");
	if (param) {
		fontdev = check_fontdev(param);
	}
	/* ゴシックフォントの設定 */
	param = get_profile("font_gothic");
	if (param) {
		fontname[FONT_GOTHIC] = param;
	}
	/* 明朝フォントの設定 */
	param = get_profile("font_mincho");
	if (param) {
		fontname[FONT_MINCHO] = param;
	}
	/* ゴシックフォント(TT)の設定 */
	param = get_profile("ttfont_gothic");
	if (param) {
		fontname_tt[FONT_GOTHIC] = param;
	}
	/* 明朝フォント(TT)の設定 */
	param = get_profile("ttfont_mincho");
	if (param) {
		fontname_tt[FONT_MINCHO] = param;
	}
	/* ゴシックフォント(TT)のコード設定 */
	param = get_profile("ttfont_gothic_code");
	if (param) {
		isjix0213_tt[FONT_GOTHIC] = (strcmp("jisx0213",param) == 0 ? TRUE : FALSE);
	}
	/* 明朝フォント(TT)のコード設定 */
	param = get_profile("ttfont_mincho_code");
	if (param) {
		isjix0213_tt[FONT_MINCHO] = (strcmp("jisx0213",param) == 0 ? TRUE : FALSE);
	}
	/* ゴシックフォント(TT)のフェイス指定 */
	param = get_profile("ttfont_gothic_face");
	if (param) {
		fontface[FONT_GOTHIC] = *param - '0';
	}
	/* 明朝フォント(TT)のフェイス指定 */
	param = get_profile("ttfont_mincho_face");
	if (param) {
		fontface[FONT_MINCHO] = *param - '0';
	}
	/* Font antialiasing */
	param = get_profile("antialias");
	if (param) {
		if (0 == strcmp(param, "No")) {
			font_noantialias = TRUE;
		}
	}
	/* Audio buffer size */
	param = get_profile("audio_buffer_size");
	if (param) {
		audio_buffer_size = atoi(param);
	}

	/* CD-ROM device name の設定 */
	param = get_profile("cdrom_device");
	if (param) {
		cd_set_devicename(param);
	}
	/* joystick device name の設定 */
	param = get_profile("joy_device");
	if (param) {
		joy_set_devicename(param);
	}
	/* Raw MIDI device name の設定 */
	param = get_profile("midi_device");
	if (param) {
		midi_set_devicename(param);
	}
	/* MIDI output device の設定 */
	param = get_profile("midi_output_device");
	if (param) {
		int subdev = 0;
		if (*(param+1) != '\0') {
			subdev = (*(param + 1) - '0') << 8;
		}
		midi_set_output_device(*param | subdev);
	}
	/* no-shm flag */
	param = get_profile("no_shm");
	if (param) {
		if (0 == strcmp(param, "Yes")) {
			SetNoShmMode();
		}
	}
	/* qe-kanjicode flag */
	param = get_profile("qe-kanjicode");
	if (param) {
		fc_set_default_kanjicode(*param - '0');
	}
	/* disable image cursor */
	param = get_profile("no_imagecursor");
	if (param) {
		if (0 == strcmp(param, "Yes")) {
			nact->noimagecursor = TRUE;
		}
	}
}

#ifdef HAVE_SIGACTION
static void signal_handler(int sig_num) {
	sys35_remove();
	exit(1);
}

static void signal_handler_segv(int sig_num) {
	fprintf(stderr, "PID(%d), sigsegv caught @ %03d, %05x\n", (int)getpid(), sl_getPage(), sl_getIndex());
	sys35_remove();
	exit(1);
}

void sys_set_signalhandler(int SIG, void (*handler)(int)) {
	struct sigaction act;
	sigset_t smask;
	
	sigemptyset(&smask);
	sigaddset(&smask, SIG);
	
	act.sa_handler = handler;
	act.sa_mask = smask;
	act.sa_flags = 0;
	
	sigaction(SIG, &act, NULL);
}

static void init_signalhandler() {
	sys_set_signalhandler(SIGINT, signal_handler);
	sys_set_signalhandler(SIGTERM, signal_handler);
	sys_set_signalhandler(SIGPIPE, SIG_IGN);
	sys_set_signalhandler(SIGABRT, signal_handler);
	sys_set_signalhandler(SIGSEGV, signal_handler_segv);
}
#endif

int main(int argc, char **argv) {
	char *homedir = getenv("HOME"), *rc_name, *rc_path;
#ifdef HAVE_SIGACTION
	sys_set_signalhandler(SIGINT, SIG_IGN);
#endif
	
	saved_argc = argc;
	saved_argv = argv;

#ifdef __ANDROID__
	// Handle -gamedir option here so that .xsys35rc is loaded from that directory.
	if (strcmp(argv[1], "-gamedir") == 0)
		chdir(argv[2]);
#endif
	
	load_profile();
	check_profile();
	sys35_ParseOption(&argc, argv);
	
#if DEBUG
	if (debuglv >= 5) {
		if (NULL == (fpdebuglog = fopen(DEBUGLOGFILE, "w"))) {
			fpdebuglog = stderr;
		}
	}
#endif
	if (!initGameDataResorce(gameResourceFile))
		sys35_usage(TRUE);
	
#ifdef HAVE_SIGACTION
	init_signalhandler();
#endif

	mus_init(audio_buffer_size);

#ifdef ENABLE_NLS
        bindtextdomain (PACKAGE, LOCALEDIR);
        textdomain(PACKAGE);
#endif

#ifdef ENABLE_GTK
	gtk_set_locale();
	gtk_init(&argc, &argv);
#endif

	sys35_init();	
	
#ifdef ENABLE_GTK
	rc_name = get_profile("gtkrc_path");
	if (!rc_name) {
		rc_name = GTK_RC_NAME;
	}
	rc_path = (char *)malloc(sizeof(char) * (strlen(homedir) + strlen(rc_name)) + 2);
	strcpy(rc_path, homedir);
	strcat(rc_path, "/");
	strcat(rc_path, rc_name);
	
	gtk_rc_parse(rc_path);
	free(rc_path);
	menu_init();
#endif
	
	nact_main();
#ifdef __EMSCRIPTEN__
	Sleep(1000000000);
#endif
	sys35_remove();
	
	return 0;
}
