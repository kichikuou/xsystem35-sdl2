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
#ifdef ALICEHUB_IOS
#include "escalayer_modifier.h"
#include "xsystem35_main.h"
#include <setjmp.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <dirent.h>
#include <stdarg.h>
#include <locale.h>
#include <unistd.h>

#ifdef HAVE_SIGACTION
#include <signal.h>
#endif

#include "sdl_compat.h"
#if XSYSTEM35_SDL_VERSION == 3
#include <SDL3/SDL_main.h>
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
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
#include "cg.h"
#include "gfx.h"
#include "event.h"
#include "menu.h"
#include "music.h"
#include "music_cdrom.h"
#include "audio_meta.h"
#include "hacks.h"
#include "savedata.h"
#include "scenario.h"
#include "variable.h"
#include "ald_manager.h"
#include "gameresource.h"
#include "filecheck.h"
#include "volume.h"
#include "msgskip.h"
#include "texthook.h"
#include "utfsjis.h"
#ifdef __SWITCH__
#include "launcher.h"
void freeGameResource(GameResource *gr);
#endif

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
static bool integer_scaling = false;
/* fullscreen on from command line */
static bool fs_on;

#ifdef ALICEHUB_IOS
static _Thread_local jmp_buf *exit_target;
static int exit_code;
static bool music_started;
static bool graphics_started;

void xsystem35_ios_exit(int code) {
	if (exit_target) {
		exit_code = code;
		longjmp(*exit_target, 1);
	}
}

void sl_fini(void);
void ald_fini(void);
void msgskip_fini(void);
void cg_fini(void);
void freeGameResource(GameResource *gr);
void s39ain_fini(S39AIN *ain);
void profile_fini(void);
#endif

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
	puts(" -encoding enc           : set script text encoding ('sjis', 'gbk', or 'utf8')");
	puts(" -savedir dir            : directory to save game state files");
	puts(" -saveformat fmt         : save file format. 'xsystem35', 'system36' or 'system39' (default)");
	puts(" -renderer name          : set rendering driver name to 'name'");
	puts(" -playlist file          : load CD playlist from 'file'");
	puts(" -texthook mode          : text hook mode. 'none' (default), 'print' or 'copy'");
	puts(" -texthook_suppress list : suppress text hook on specified pages");
	puts(" -censor file            : mosaic images listed in 'file'");
	
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
#ifdef ALICEHUB_IOS
	sys_exit(1);
#else
	exit(1);
#endif
}

static void sys35_init() {
	int i;
	
	sl_init();

	nact_init();

	v_init();
	
#ifdef ALICEHUB_IOS
	graphics_started = true;
#endif
	ags_init(render_driver);

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
	msgskip_syncFile();
	dbg_quit();
	mus_exit();
	ags_remove();
	cmdf_reset();
}

static void sys_reset(void) {
	nact_reset();
	ags_reset();
	mus_reset();
	s39ain_reset(&nact->ain);
}

static bool parse_encoding_option(const char *mode, CharacterEncoding *out) {
	if (!strcasecmp(mode, "sjis") || !strcasecmp(mode, "shift_jis") || !strcasecmp(mode, "shift-jis") || !strcasecmp(mode, "cp932")) {
		*out = SHIFT_JIS;
		return true;
	}
	if (!strcasecmp(mode, "gbk") || !strcasecmp(mode, "gb2312") || !strcasecmp(mode, "gb18030") || !strcasecmp(mode, "cp936")) {
		*out = GBK;
		return true;
	}
	if (!strcasecmp(mode, "utf8") || !strcasecmp(mode, "utf-8")) {
		*out = UTF8;
		return true;
	}
	return false;
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
		} else if (0 == strcmp(argv[i], "-censor")) {
			if (argv[i + 1] != NULL) {
				load_censor_list(argv[i + 1]);
			}
		} else if (0 == strncmp(argv[i], "-M", 2)) {
			int device = -1;
			if (argv[i][2] != '\0' && argv[i][3] != '\0') {
				device = atoi(&argv[i][3]);
			}
			midi_set_output_device(argv[i][2], device);
		} else if (0 == strcmp(argv[i], "-devjoy")) {
			if (argv[i + 1] != NULL) {
				event_set_joy_device_index(atoi(argv[i + 1]));
			}
		} else if (0 == strcmp(argv[i], "-fullscreen")) {
			fs_on = true;
		} else if (0 == strcmp(argv[i], "-noantialias")) {
			font_noantialias = true;
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
#ifdef ALICEHUB_IOS
			sys_exit(0);
#else
			exit(0);
#endif
		} else if (0 == strcmp(argv[i], "-integerscale")) {
			integer_scaling = true;
		} else if (0 == strcmp(argv[i], "-mute_on_unfocus")) {
			volume_set_mute_on_unfocus(true);
		} else if (0 == strcmp(argv[i], "-game")) {
			if (argv[i + 1] != NULL) {
				enable_hack_by_gameid(argv[i + 1]);
			}
		} else if (0 == strcmp(argv[i], "-saveformat")) {
			if (argv[i + 1] != NULL) {
				save_setFormat(argv[i + 1]);
			}
		} else if (0 == strcmp(argv[i], "-gamedir")) {
			if (i + 1 < *argc)
				i++;
		} else if (0 == strcmp(argv[i], "-encoding")) {
			if (i + 1 < *argc && argv[i + 1] != NULL) {
				CharacterEncoding enc;
				if (!parse_encoding_option(argv[++i], &enc)) {
					fprintf(stderr, "xsystem35: Invalid encoding '%s'\n\n", argv[i]);
					sys35_usage(false);
				}
				sys_setDefaultCharacterEncoding(enc, true);
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
		int device = -1;
		if (param[0] != '\0' && param[1] != '\0') {
			device = atoi(param + 1);
		}
		midi_set_output_device(*param, device);
	}
	/* disable image cursor */
	get_boolean_profile("no_imagecursor", &nact->ags.noimagecursor);

	/* enable integer scaling */
	get_boolean_profile("integerscale", &integer_scaling);

	/* mute audio while the window is unfocused */
	bool mute_on_unfocus;
	if (get_boolean_profile("mute_on_unfocus", &mute_on_unfocus)) {
		volume_set_mute_on_unfocus(mute_on_unfocus);
	}

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

	/* Censor list for streamer mode */
	param = get_profile("censor");
	if (param) {
		load_censor_list(param);
	}
	param = get_profile("encoding");
	if (param) {
		CharacterEncoding enc;
		if (parse_encoding_option(param, &enc))
			sys_setDefaultCharacterEncoding(enc, true);
	}
}

#ifdef HAVE_SIGACTION
static volatile sig_atomic_t quit_requested = 0;

bool sys_is_quit_requested(void) {
	return quit_requested != 0;
}

static void signal_handler(int sig_num) {
	(void)sig_num;
	quit_requested = 1;
}

static void signal_handler_segv(int sig_num) {
	char msg[128];
	snprintf(msg, sizeof(msg), "Signal %d @ page %03d, 0x%05x", sig_num, sl_getPage(), sl_getIndex());
	fprintf(stderr, "PID(%d), %s\n", (int)getpid(), msg);
	sys_write_crash_log(msg);
	_exit(1);
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

static void init_signalhandler(void) {
	quit_requested = 0;
	sys_set_signalhandler(SIGINT, signal_handler);
	sys_set_signalhandler(SIGTERM, signal_handler);
	sys_set_signalhandler(SIGPIPE, SIG_IGN);
	sys_set_signalhandler(SIGSEGV, signal_handler_segv);
}
#else
bool sys_is_quit_requested(void) {
	return false;
}
#endif

static bool is_uncensored_cg_ald_name(const char *name) {
	if (!name || !name[0] || name[0] == '.')
		return false;
	size_t len = strlen(name);
	if (len < 14 || strcasecmp(name + len - 4, ".ald") != 0)
		return false;
	static const char needle[] = "uncensored";
	for (const char *p = name; *p; p++) {
		size_t k = 0;
		while (needle[k] && p[k] && tolower((unsigned char)p[k]) == needle[k])
			k++;
		if (needle[k] == '\0')
			return true;
	}
	return false;
}

static void register_uncensored_cg_overlay(bool use_mmap) {
	DIR *dir = opendir(".");
	if (!dir)
		return;
	char *chosen = NULL;
	struct dirent *ent;
	while ((ent = readdir(dir)) != NULL) {
		if (!is_uncensored_cg_ald_name(ent->d_name))
			continue;
		if (!chosen ||
		    !strcasecmp(ent->d_name, "AliceHubUncensoredCG.ald") ||
		    (strcasecmp(chosen, "AliceHubUncensoredCG.ald") != 0 &&
		     strcasecmp(ent->d_name, chosen) < 0)) {
			free(chosen);
			chosen = strdup(ent->d_name);
		}
	}
	closedir(dir);
	if (chosen) {
		ald_init_overlay(DRIFILE_CG, chosen, use_mmap);
		free(chosen);
	}
}

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
	register_uncensored_cg_overlay(true);
	if (nact->files.save_path)
		fc_init(nact->files.save_path);
}

#ifdef __SWITCH__
static bool switch_open_game_resources(const char *save_dir)
{
	bool resource_ok = initGameResource(&nact->files, gameResourceFile, save_dir);
	for (int type = 0; resource_ok && type < DRIFILETYPEMAX; type++)
		resource_ok = dri_validate(nact->files.game_fname[type], nact->files.cnt[type],
					   type == DRIFILE_SCO);
	if (resource_ok)
		return true;
	alicehub_switch_set_launch_error("Cannot read game data; check the scenario ALDs and xsystem35.gr");
	freeGameResource(&nact->files);
	memset(&nact->files, 0, sizeof(nact->files));
	return false;
}
#endif

#ifdef ALICEHUB_IOS
static int run_xsystem35(int argc, char **argv) {
#elif defined(__SWITCH__)
int alicehub_switch_engine_main(int argc, char **argv) {
#else
int main(int argc, char **argv) {
#endif
#ifdef __ANDROID__
	static bool sdl_main_entered = false;
	if (sdl_main_entered) {
		WARNING("Prevented duplicate SDL_main re-entry in :game process");
		return 0;
	}
	sdl_main_entered = true;
#endif
#ifdef HAVE_SIGACTION
	sys_set_signalhandler(SIGINT, SIG_IGN);
#endif
	
	// Handle -gamedir option before loading .xsys35rc from that directory.
	for (int i = 1; i + 1 < argc; i++) {
		if (strcmp(argv[i], "-gamedir") == 0 && argv[i + 1] && argv[i + 1][0]) {
			if (chdir(argv[i + 1]) != 0) {
#ifdef __SWITCH__
				alicehub_switch_set_launch_error("Cannot open the game folder");
				return 1;
#else
				WARNING("Failed to chdir to -gamedir '%s'", argv[i + 1]);
#endif
			}
			break;
		}
	}

#ifdef __SWITCH__
	const char *switch_savedir = NULL;
	for (int i = 1; i + 1 < argc; ++i) {
		if (!strcmp(argv[i], "-savedir")) {
			switch_savedir = argv[i + 1];
			break;
		}
	}
	if (!switch_open_game_resources(switch_savedir))
		return 1;
#endif
	
	// Load optional Longinus CHS.TBL character remap table before profile/options.
	if (chs_tbl_load_from_dir(".")) {
		sys_setDefaultCharacterEncoding(SHIFT_JIS, true);
	}

#ifdef ALICEHUB_IOS
	const char *gothic = getenv("XSYSTEM35_FONT_GOTHIC");
	const char *mincho = getenv("XSYSTEM35_FONT_MINCHO");
	if (gothic && *gothic) fontname_tt[FONT_GOTHIC] = gothic;
	if (mincho && *mincho) fontname_tt[FONT_MINCHO] = mincho;
#endif
	load_profile();
	check_profile();
	sys35_ParseOption(&argc, argv);
	
#ifndef __SWITCH__
	if (!initGameResource(&nact->files, gameResourceFile, savedir)) {
#ifdef _WIN32
		sys_show_message_box(MESSAGEBOX_ERROR, "xsystem35", "Cannot find scenario file (*SA.ALD)");
		exit(1);
#else
		sys35_usage(true);
#endif
	}
#endif
	registerGameFiles();
	
#ifdef HAVE_SIGACTION
	init_signalhandler();
#endif

#ifdef ALICEHUB_IOS
	music_started = true;
#endif
	mus_init(audio_buffer_size);

#ifdef HAVE_LIBINTL
	setlocale(LC_ALL, "");
	// Keep numeric formatting locale-independent (use '.' as the decimal
	// point) so only message translation follows the user's locale.
	setlocale(LC_NUMERIC, "C");
	bindtextdomain (PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
#else
	builtin_nls_init();
#endif

	sys35_init();
	if (integer_scaling)
		gfx_setIntegerScaling(true);

	volume_init();
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

#ifndef ALICEHUB_IOS
	sys35_remove();
#endif
	
	return 0;
}

#ifdef ALICEHUB_IOS
int xsystem35_main(int argc, char **argv) {
#ifdef HAVE_SIGACTION
	quit_requested = 0;
#endif
	escalayer_modifier_reset(true);
	jmp_buf target;
	exit_target = &target;
	if (setjmp(target) == 0)
		exit_code = run_xsystem35(argc, argv);
	exit_target = NULL;

	msgskip_fini();
	s39ain_fini(&nact->ain);
	if (music_started) {
		mus_exit();
		music_started = false;
	}
	muscd_reset_playlist();
	audio_meta_fini();
	if (graphics_started) {
		ags_remove();
		font_fini();
		cg_fini();
		graphics_started = false;
	} else {
		SDL_Quit();
	}
	sl_fini();
	v_set_names(0, NULL);
	v_reset();
	cmdf_reset();
	freeGameResource(&nact->files);
	ald_fini();
	fc_fini();
	volume_fini();
	hacks_reset();
	save_resetFormat();
	chs_tbl_reset();
	memset(nact, 0, sizeof(*nact));
	profile_fini();
	gameResourceFile = "xsystem35.gr";
	render_driver = NULL;
	savedir = NULL;
	font_noantialias = integer_scaling = fs_on = false;
	audio_buffer_size = 0;
	debugger_mode = DEBUGGER_DISABLED;
	memset(fontface, 0, sizeof(fontface));
	fontname_tt[FONT_GOTHIC] = DEFAULT_GOTHIC_TTF;
	fontname_tt[FONT_MINCHO] = DEFAULT_MINCHO_TTF;
	sys_resetCharacterEncoding();
	escalayer_modifier_reset(false);
	return exit_code;
}
#endif

#ifdef __EMSCRIPTEN__

EMSCRIPTEN_KEEPALIVE
void sys_restart(void) {
	nact_quit(true);
}

#endif
