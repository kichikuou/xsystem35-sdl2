/*
 * volume.c  System39.ini volume controller
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

#include "config.h"

#include <stdio.h>
#include <string.h>
#include "portab.h"
#include "system.h"
#include "nact.h"
#include "volume.h"
#include "utfsjis.h"
#include "music.h"
#include "modal.h"
#include "gfx_private.h"

// Volume Valancer で扱う最大チャンネル数
#define MAXVOLCH 16

// One volume-valancer channel, as defined by VolumeValancer[] in System39.ini.
struct volume_channel {
	char *label;  // UTF-8 label, or NULL if this channel index is unused
	int vol;      // 0-100
	bool mute;
};

static int vval_max;  // 最大チャンネル番号
static struct volume_channel vval[MAXVOLCH];

static void apply_volume(void) {
	int vol[MAXVOLCH] = {0};
	for (int i = 0; i < MAXVOLCH; i++) {
		vol[i] = vval[i].mute ? 0 : vval[i].vol;
	}
	mus_vol_set_valance(vol, MAXVOLCH);
}

// Volume.sav per-channel record, matching the format used by the original
// System39.exe.
struct volume_sav_channel {
	uint8_t volume;    // 0-100
	uint8_t mute;
	uint8_t reserved;  // zero
};

void volume_init(void) {
	int i;
	char fn[256];
	int nvalancer = 0;

	FILE *fp = nact->files.init ? fopen(nact->files.init, "r") : NULL;
	if (fp) {
		char s[256], s1[256];
		while (fgets(s, 255, fp) != NULL) {
			s1[0] = '\0';
			sscanf(s, "VolumeValancer[%d] = \"%s", &i, s1);
			if (s1[0] == '\0') continue;
			if (i >= MAXVOLCH || i < 0) continue;
			s1[strlen(s1)-1] = '\0'; // remove last '"'
			vval[i].label = sjis2utf(s1);
			vval_max = max(vval_max, i);
			nvalancer++;
		}
		fclose(fp);
	}

	if (nvalancer == 0) {
		// No VolumeValancer[] is defined (or there is no System39.ini). Fall back
		// to two generic channels that control BGM and sound effects separately.
		vval[BGM_VOLVAL_CH].label = strdup(_("BGM"));
		vval[SE_VOLVAL_CH].label = strdup(_("Sound effects"));
		vval_max = SE_VOLVAL_CH;
	}

	for (i = 0; i < MAXVOLCH; i++) {
		vval[i].vol = 100;
		vval[i].mute = false;
	}

	// Load Volume.sav if it exists.
	snprintf(fn, sizeof(fn) -1, "%s/Volume.sav", nact->files.save_path);
	if (NULL != (fp = fopen(fn, "rb"))) {
		uint8_t header[2];
		if (fread(header, 1, 2, fp) == 2) {
			int nr_channels = header[1];
			for (i = 0; i < nr_channels; i++) {
				struct volume_sav_channel ch;
				if (fread(&ch, sizeof(ch), 1, fp) != 1)
					break;
				if (i >= MAXVOLCH)
					continue;
				vval[i].vol = ch.volume;
				vval[i].mute = ch.mute;
			}
		}
		fclose(fp);
	}

	apply_volume();
}

static void volume_save(void) {
	char fn[256];

	snprintf(fn, sizeof(fn) -1, "%s/Volume.sav", nact->files.save_path);
	FILE *fp = fopen(fn, "wb");
	if (!fp) {
		WARNING("Failed to save Volume.sav");
		return;
	}

	int nr_channels = vval_max + 1;
	uint8_t header[2] = { 0, nr_channels };
	fwrite(header, 1, 2, fp);
	for (int i = 0; i < nr_channels; i++) {
		struct volume_sav_channel ch = {
			.volume = vval[i].vol,
			.mute = vval[i].mute,
			.reserved = 0,
		};
		fwrite(&ch, sizeof(ch), 1, fp);
	}
	fclose(fp);
}

// The volume controller dialog.

struct volume_state {
	modal base;
};

static bool volume_build(mu_Context *ctx, modal *modal) {
	struct volume_state *st = (struct volume_state *)modal;
	bool keep_open = true;
	int count = vval_max + 1;
	struct volume_channel *ch = vval;

	int nch = 0;
	int label_w = 0;
	for (int i = 0; i < count; i++) {
		if (!ch[i].label)
			continue;
		nch++;
		int lw = ctx->text_width(ctx->style->font, ch[i].label, -1);
		if (lw > label_w)
			label_w = lw;
	}
	label_w += ctx->style->padding * 2;

	int row_h = ctx->text_height(ctx->style->font) + ctx->style->padding * 2;
	int slider_w = 144;
	int gap_w = 16;      // extra space between the slider and the mute checkbox
	const char *mute_label = _("mute");
	int mute_w = row_h + ctx->style->padding * 2
	             + ctx->text_width(ctx->style->font, mute_label, -1);
	int w = label_w + slider_w + gap_w + mute_w + ctx->style->spacing * 3
	        + ctx->style->padding * 2;
	int h = (nch + 1) * (row_h + ctx->style->spacing) + ctx->style->padding * 2
	        + ctx->style->title_height;
	mu_Rect r = mu_rect((view_w - w) / 2, (view_h - h) / 2, w, h);

	if (mu_begin_window_ex(ctx, _("Volume"), r,
	        MU_OPT_NORESIZE | MU_OPT_NOSCROLL)) {
		for (int i = 0; i < count; i++) {
			if (!ch[i].label)
				continue;
			// Each control derives its id from the value pointer, which is the
			// same across rows; push the channel index so the ids stay unique.
			mu_push_id(ctx, &i, sizeof(i));
			mu_layout_row(ctx, 4, (int[]){ label_w, slider_w, gap_w, mute_w }, 0);
			mu_label(ctx, ch[i].label);
			float v = ch[i].vol;
			if (mu_slider_ex(ctx, &v, 0, 100, 1, "%.0f", MU_OPT_ALIGNCENTER)) {
				ch[i].vol = (int)v;
				apply_volume();
			}
			mu_layout_next(ctx);  // spacer cell (gap_w)
			int m = ch[i].mute;
			if (mu_checkbox(ctx, mute_label, &m)) {
				ch[i].mute = m;
				apply_volume();
			}
			mu_pop_id(ctx);
		}

		mu_layout_row(ctx, 1, (int[]){ -1 }, 0);
		if (mu_button(ctx, _("Close")))
			keep_open = false;
		mu_end_window(ctx);
	} else {
		keep_open = false;  // The title-bar close button was clicked.
	}

	if (st->base.cancelled)  // Esc
		keep_open = false;
	return keep_open;
}

void volume_dialog_open(void) {
	struct volume_state st = {
		.base = { .build = volume_build, .handler = modal_default_handler },
	};
	modal_run(&st.base);
	volume_save();
}
