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
#include <gtk/gtk.h>
#endif

#include <SDL.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#ifdef _WIN32
#include "win/dialog.h"
#endif

#include "nact.h"
#include "debugger.h"
#include "portab.h"
#include "xsystem35.h"
#include "nact.h"
#include "profile.h"
#include "randMT.h"
#include "ags.h"
#include "font.h"
#include "gfx.h"
#include "event.h"
#include "menu.h"
#include "music.h"
#include "music_cdrom.h"
#include "savedata.h"
#include "scenario.h"
#include "variable.h"
#include "ald_manager.h"
#include "gameresource.h"
#include "filecheck.h"
#include "s39init.h"
#include "msgskip.h"
#include "texthook.h"

static char *gameResourceFile = "xsystem35.gr";
static void    sys35_usage(bool verbose);
static void    sys35_init();
static void    sys35_ParseOption(int *argc, char **argv);
static void    check_profile();

static const char *render_driver = NULL;

enum {
	DEBUGGER_DISABLED,
	DEBUGGER_CUI,
	DEBUGGER_DAP,
} debugger_mode = DEBUGGER_DISABLED;

static int audio_buffer_size = 0;

/* font name from rcfile */
static const char *fontname_tt[FONTTYPEMAX] = {DEFAULT_GOTHIC_TTF, DEFAULT_MINCHO_TTF};
static char fontface[FONTTYPEMAX];

static const char *savedir;
static bool font_noantialias;
static bool enable_zb = false;
static bool integer_scaling = false;
/* fullscreen on from command line */
static bool fs_on;

static void sys35_usage(bool verbose) {
	if (verbose) {
		puts("System35 for X Window System [proj. Rainy Moon]");
		puts("             (C) Masaki Chikama(Wren) 1997-2002");
		puts("                                  Version "VERSION"\n");
	}
	puts("Usage: xsystem35 [OPTIONS]\n");
	puts("OPTIONS");
	puts(" -gamefile file          : set game resource file to 'file'");
	puts(" -game game              : enable game-specific hacks");
	puts(" -savedir dir            : directory to save game state files");
	puts(" -saveformat fmt         : save file format. 'xsystem35', 'system36' or 'system39' (default)");
	puts(" -renderer name          : set rendering driver name to 'name'");
	puts(" -playlist file          : load CD playlist from 'file'");
	puts(" -texthook mode          : text hook mode. 'none' (default), 'print' or 'copy'");
	puts(" -texthook_suppress list : suppress text hook on specified pages");
	
	puts(" -M?                     : select output midi methos");
#ifdef ENABLE_MIDI_SDLMIXER
	puts(" -Me                     : SDL_mixer midi player");
#endif
#ifdef ENABLE_MIDI_PORTMIDI
	puts(" -Mp?                    : ALSA (via PortMidi) (?:devicenumber)");
#endif
	puts(" -M0                     : Disable MIDI output");
	
	puts(" -devjoy device          : joystick device index (0-)");

	puts(" -ttfont_mincho          : set TrueType font for mincho");
	puts(" -ttfont_gothic          : set TrueType font for mincho");
	
#ifdef DEBUG
	puts(" -debuglv #              : logging level");
	puts("                         :  1: warings");
	puts("                         :  2: unimplemented commands");
	puts("                         :  5: command trace");
	puts("                         :  6: message trace");
#endif
#ifdef ENABLE_DEBUGGER
	puts(" -debug                  : start with debugger");
#endif
	puts(" -noantialias            : never use antialiased string");
	puts(" -fullscreen             : start with fullscreen");
	puts(" -integerscale           : use integer scaling when resizing");
	puts(" -noimagecursor          : disable image cursor");
	puts(" -version                : show version");
	puts(" -h                      : show this message");
	puts(" --help                  : show this message");
	exit(1);
}

static void sys35_init() {
	int i;
	
	nact_init();
	
	sl_init();

	v_init();
	
	ags_init(render_driver, enable_zb);

	for (i = 0; i < FONTTYPEMAX; i++)
		font_set_name_and_index(i, fontname_tt[i], fontface[i]);
	
	gfx_setFullscreen(fs_on);
	nact->ags.noantialias = font_noantialias;
	ags_setAntialiasedStringMode(!font_noantialias);

	sgenrand(getpid());

	if (nact->files.ain)
		s39ain_init(nact->files.ain, &nact->ain);
	msgskip_init(nact->files.msgskip);
}

void sys35_remove(void) {
	dbg_quit();
	mus_exit(); 
	ags_remove();
#ifdef ENABLE_GTK
	s39ini_remove();
#endif
}

static void sys_reset(void) {
	nact_reset();
	ags_reset();
	mus_reset();
	s39ain_reset(&nact->ain);
}

static bool set_texthook_mode(const char *mode) {
	if (!strcasecmp(mode, "none")) {
		texthook_set_mode(TEXTHOOK_NONE);
	} else if (!strcasecmp(mode, "print")) {
		texthook_set_mode(TEXTHOOK_PRINT);
	} else if (!strcasecmp(mode, "copy")) {
		texthook_set_mode(TEXTHOOK_COPY);
	} else {
		return false;
	}
	return true;
}

static void sys35_ParseOption(int *argc, char **argv) {
	int i;
	FILE *fp;
	for (i = 1; i < *argc; i++) {
		if (!strcmp(argv[i], "-h")) {
			sys35_usage(true);
		}
		if (!strcmp(argv[i], "--help")) {
			sys35_usage(true);
		}
	}
	for (i = 1; i < *argc; i++) {
		if (0 == strcmp(argv[i], "-savedir")) {
			if (i == *argc - 1) {
				fprintf(stderr, "xsystem35: The -savedir option requires directory name\n\n");
				sys35_usage(false);
			}
			savedir = argv[++i];
		}
		if (0 == strcmp(argv[i], "-gamefile")) {
			if (i == *argc - 1) {
				fprintf(stderr, "xsystem35: The -gamefile option requires file value\n\n");
				sys35_usage(false);
			}
			if (NULL == (fp = fopen(argv[i + 1],"r"))) {
				fprintf(stderr, "xsystem35: gamefile '%s' not found\n\n", argv[i + 1]);
				sys35_usage(false);
			}
			fclose(fp);
			gameResourceFile = argv[i + 1];
		} else if (0 == strcmp(argv[i], "-debug")) {
			debugger_mode = DEBUGGER_CUI;
		} else if (0 == strcmp(argv[i], "-debug_dap")) {
			debugger_mode = DEBUGGER_DAP;
		} else if (0 == strcmp(argv[i], "-renderer")) {
			render_driver = argv[i + 1];
		} else if (0 == strcmp(argv[i], "-playlist") || 0 == strcmp(argv[i], "-devcd")) {
			if (argv[i + 1] != NULL) {
				muscd_set_playlist(argv[i + 1]);
			}
		} else if (0 == strcmp(argv[i], "-texthook")) {
			if (argv[i + 1] != NULL) {
				if (!set_texthook_mode(argv[i + 1])) {
					fprintf(stderr, "xsystem35: Invalid texthook mode '%s'\n\n", argv[i + 1]);
					sys35_usage(false);
				}
			}
		} else if (0 == strcmp(argv[i], "-texthook_suppress")) {
			if (argv[i + 1] != NULL) {
				texthook_set_suppression_list(argv[i + 1]);
			}
		} else if (0 == strncmp(argv[i], "-M", 2)) {
			int subdev = 0;
			if (argv[i][3] != '\0') {
				subdev = (argv[i][3] - '0') << 8;
			}
			midi_set_output_device(argv[i][2] | subdev);
		} else if (0 == strcmp(argv[i], "-devjoy")) {
			if (argv[i + 1] != NULL) {
				event_set_joy_device_index(atoi(argv[i + 1]));
			}
		} else if (0 == strcmp(argv[i], "-fullscreen")) {
			fs_on = true;
		} else if (0 == strcmp(argv[i], "-noantialias")) {
			font_noantialias = true;
		} else if (0 == strcmp(argv[i], "-enable_zb")) {
			enable_zb = true;
		} else if (0 == strcmp(argv[i], "-ttfont_gothic")) {
			if (argv[i + 1] != NULL) {
				fontname_tt[FONT_GOTHIC] = argv[i + 1];
			}
		} else if (0 == strcmp(argv[i], "-ttfont_mincho")) {
			if (argv[i + 1] != NULL) {
				fontname_tt[FONT_MINCHO] = argv[i + 1];
			}
		} else if (0 == strcmp(argv[i], "-noimagecursor")) {
			nact->ags.noimagecursor = true;
		} else if (0 == strcmp(argv[i], "-debuglv")) {
			if (argv[i + 1] != NULL) {
				sys_set_debug_level(argv[i + 1][0] - '0');
			}
		} else if (0 == strcmp(argv[i], "-version")) {
			puts(VERSION);
			exit(0);
		} else if (0 == strcmp(argv[i], "-integerscale")) {
			integer_scaling = true;
		} else if (0 == strcmp(argv[i], "-game")) {
			if (argv[i + 1] != NULL) {
				enable_hack_by_gameid(argv[i + 1]);
			}
		} else if (0 == strcmp(argv[i], "-saveformat")) {
			if (argv[i + 1] != NULL) {
				save_setFormat(argv[i + 1]);
			}
		}
	}
}

static void check_profile() {
	const char *param;
	
	param = get_profile("savedir");
	if (param) {
		savedir = param;
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
	bool antialias;
	if (get_boolean_profile("antialias", &antialias)) {
		font_noantialias = !antialias;
	}

	/* Enable ZB command (font weight) */
	get_boolean_profile("enable_zb", &enable_zb);

	/* Audio buffer size */
	param = get_profile("audio_buffer_size");
	if (param) {
		audio_buffer_size = atoi(param);
	}

	/* Rendering driver */
	param = get_profile("render_driver");
	if (param) {
		render_driver = param;
	}

	/* CD-ROM playlist */
	param = get_profile("playlist");
	if (!param)
		param = get_profile("cdrom_device");  // For backwards compatibility
	if (param) {
		muscd_set_playlist(param);
	}

	/* joystick device name の設定 */
	param = get_profile("joy_device");
	if (param) {
		event_set_joy_device_index(atoi(param));
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
	/* disable image cursor */
	get_boolean_profile("no_imagecursor", &nact->ags.noimagecursor);

	/* enable integer scaling */
	get_boolean_profile("integerscale", &integer_scaling);

	/* enable game-specific hacks */
	param = get_profile("game");
	if (param) {
		enable_hack_by_gameid(param);
	}
	/* Save file format */
	param = get_profile("saveformat");
	if (param) {
		save_setFormat(param);
	}

	/* text hook */
	param = get_profile("texthook");
	if (param) {
		if (!set_texthook_mode(param)) {
			sys_error("Invalid texthook mode '%s'", param);
		}
	}
	param = get_profile("texthook_suppress");
	if (param) {
		texthook_set_suppression_list(param);
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

static void registerGameFiles(void) {
	if (nact->files.cnt[DRIFILE_SCO] == 0)
		SYSERROR("No Scenario data available");
	for (int type = 0; type < DRIFILETYPEMAX; type++) {
		bool use_mmap = true;
		if (debugger_mode != DEBUGGER_DISABLED && type == DRIFILE_SCO) {
			// Do not mmap scenario files so that BREAKPOINT instructions can be inserted.
			use_mmap = false;
		}
		ald_init(type, nact->files.game_fname[type], nact->files.cnt[type], use_mmap);
	}
	if (nact->files.save_path)
		fc_init(nact->files.save_path);
}

int main(int argc, char **argv) {
#ifdef HAVE_SIGACTION
	sys_set_signalhandler(SIGINT, SIG_IGN);
#endif
	
#ifdef __ANDROID__
	// Handle -gamedir option here so that .xsys35rc is loaded from that directory.
	if (strcmp(argv[1], "-gamedir") == 0)
		chdir(argv[2]);
#endif
	
	load_profile();
	check_profile();
	sys35_ParseOption(&argc, argv);
	
	if (!initGameResource(&nact->files, gameResourceFile, savedir)) {
#ifdef _WIN32
		sys_show_message_box(MESSAGEBOX_ERROR, "xsystem35", "Cannot find scenario file (*SA.ALD)");
		exit(1);
#else
		sys35_usage(true);
#endif
	}
	registerGameFiles();
	
#ifdef HAVE_SIGACTION
	init_signalhandler();
#endif

	mus_init(audio_buffer_size);

#ifdef ENABLE_NLS
        bindtextdomain (PACKAGE, LOCALEDIR);
        textdomain(PACKAGE);
#endif

	sys35_init();
	if (integer_scaling)
		gfx_setIntegerScaling(true);

#ifdef ENABLE_GTK
	gtk_init(&argc, &argv);
	s39ini_init();
#endif
	menu_init();
	
	if (debugger_mode != DEBUGGER_DISABLED) {
#ifdef ENABLE_DEBUGGER
		char symbols_path[500];
		snprintf(symbols_path, sizeof(symbols_path), "%s.symbols", nact->files.game_fname[DRIFILE_SCO][0]);
		dbg_init(symbols_path, debugger_mode == DEBUGGER_DAP);
#else
		sys_error("Debugging is not supported in this build.");
#endif
	}

	for (;;) {
		nact_main();
#ifndef __EMSCRIPTEN__
		if (!nact->restart)
			break;
#endif
		sys_reset();
	}

	sys35_remove();
	
	return 0;
}

#ifdef __EMSCRIPTEN__

EMSCRIPTEN_KEEPALIVE
void sys_restart(void) {
	nact_quit(true);
}

#endif
