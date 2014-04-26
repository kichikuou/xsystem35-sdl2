/*
 * mixer_alsa.c  ALSA mixer lowlevel access (for 0.5.x)
 *
 * Copyright (C) 1997-1998 Masaki Chikama (Wren) <chikama@kasumi.ipl.mech.nagoya-u.ac.jp>
 *               1998-                           <masaki-c@is.aist-nara.ac.jp>
 * rewrited      2000-     Fumihiko Murata <fmurata@p1.tcnet.ne.jp>
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
/* $Id: mixer_alsa.c,v 1.11 2006/04/21 16:40:48 chikama Exp $ */

#include "config.h"
#include "audio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/asoundlib.h>

#include "audio_alsa.h"

#define MIXER_ELEMENT_MAX 100
static snd_mixer_eid_t eid[MIXER_ELEMENT_MAX];
static snd_mixer_element_t einit[MIXER_ELEMENT_MAX];
static int emax;

/*
  level: 0-100
*/
static void mixer_set_level(audiodevice_t *dev, int ch, int level) {
	snd_mixer_t *mixer_handle;
	mixer_alsa05_t *amix = (mixer_alsa05_t *)dev->data_mix;
	int i, err;
	int lv;
	
#if 0
	printf("mixer set '%s',%d,%d = %3d (%3d %3d %3d)\n" ,
		   amix->elem.eid.name,amix->elem.eid.index,amix->elem.eid.type,
		   level,mix->vol,mix->vol_full,mix->range.scale);
#endif
	
	if (0 > snd_mixer_open(&mixer_handle, amix->card, amix->mix_dev)) {
		WARNING("mixer_set_level(): Opening mixer device %d failed\n",
			amix->mix_dev);
		return;
	}
	
	for (i = 0; i < amix->e[ch].data.volume1.voices; i++) {
		//lv = (amix->ei[ch].data.volume1.prange[i].max * level / 100);
		lv = (einit[amix->connect[ch]].data.volume1.pvoices[i] * level / 100);
		amix->e[ch].data.volume1.pvoices[i] = lv;
	}
	
	if (0 > (err = snd_mixer_element_write(mixer_handle, &amix->e[ch]))) {
		WARNING("mixer_set_level(lv%d): mixer write failed %s\n",
			level, snd_strerror(err));
	}
	snd_mixer_close(mixer_handle);
}

static int mixer_get_level(audiodevice_t *dev, int ch) {
	// mixer_alsa05_t *amix = (mixer_alsa05_t *)dev->data_mix;

#if 0	
	if (snd_mixer_open(&mixer_handle, amix->card, amix->mix_dev) < 0) {
		WARNING("mixer_get_level(): Opening mixer device (%d,%d) failed\n",
			amix->card, amix->mix_dev);
		return 0;
	}
	
	if (snd_mixer_element_read(mixer_handle, &amix->elem) < 0) {
		WARNING("mixer_get_level(): mixser read failed\n");
		return 0;
	}
	
	snd_mixer_close(mixer_handle);
	return amix->Epvol1(elem.)[0];
#endif
	return 0;
}

void mixer_exit(audiodevice_t *dev) {
	snd_mixer_t *mixer_handle;
	mixer_alsa05_t *amix = (mixer_alsa05_t *)dev->data_mix;
	int i;
	
	if (0 > snd_mixer_open(&mixer_handle, amix->card, amix->mix_dev)) {
		WARNING("mixer_exit(): Opening mixer device %d failed\n",
			amix->mix_dev);
		g_free(dev->data_mix);
		return;
	}
	
	for (i = 0; i < emax; i++) {
		if (0 > snd_mixer_element_write(mixer_handle, &einit[i])) {
			WARNING("mixer_exit(): write device failed\n");
			continue;
		}
	}
	
	g_free(dev->data_mix);
}

int mixer_init(audiodevice_t *dev, int card, int mixdev, snd_ctl_hw_info_t *hwinfo[]) {
	snd_mixer_t *handle;
	mixer_alsa05_t *mix;
	snd_mixer_elements_t es;
	int i, j, k;
	
	if (mixdev == -1) {
		// 自動の場合、指定の card から使用可能な mixer device を
		// 選択し SND_MIXER_ETYPE_VOLUME1 があれば、そのデバイスを使う
		for (i = 0; i < hwinfo[card]->mixerdevs; i++) {
			if (0 > snd_mixer_open(&handle, card, i)) {
				return NG;
			}
			memset(&es, 0, sizeof(snd_mixer_elements_t));
			
			// 全 element 数を取得
			if (0 > snd_mixer_elements(handle, &es)) {
				snd_mixer_close(handle);
				return NG;
			}
			
			// elemnt が 1つもない場合はerror
			if (es.elements_over < 1) {
				snd_mixer_close(handle);
				return NG;
			}
			
			// element の情報を格納する場所の確保
			es.pelements = g_new(snd_mixer_eid_t, es.elements_over);
			// 実際に elements を取得
			es.elements_size = es.elements_over;
			es.elements = 0;
			if (0 > snd_mixer_elements(handle, &es)) {
				g_free(es.pelements);
				snd_mixer_close(handle);
				return NG;
			}
			snd_mixer_close(handle);
			
			for (k = 0; k < es.elements; k++) {
				if (es.pelements[k].type == SND_MIXER_ETYPE_VOLUME1) {
					// found
					g_free(es.pelements);
					mixdev = i;
					goto out;
				}
			}
			g_free(es.pelements);
		}
	}
 out:
	if (0 > snd_mixer_open(&handle, card, mixdev)) {
		return NG;
	}
	memset(&es, 0, sizeof(snd_mixer_elements_t));
	
	// 全 element 数を取得
	if (0 > snd_mixer_elements(handle, &es)) {
		snd_mixer_close(handle);
		return NG;
	}
			
	// elemnt が 1つもない場合はerror
	if (es.elements_over < 1) {
		snd_mixer_close(handle);
		return NG;
	}
	
	// element の情報を格納する場所の確保
	es.pelements = g_new(snd_mixer_eid_t, es.elements_over);
	// 実際に elements を取得
	es.elements_size = es.elements_over;
	es.elements = 0;
	if (0 > snd_mixer_elements(handle, &es)) {
		g_free(es.pelements);
		snd_mixer_close(handle);
		return NG;
	}

	mix = g_new0(mixer_alsa05_t, 1);
	mix->card = card;
	mix->mix_dev = mixdev;
	
	for (i = 0, j = 0; i < es.elements && j < MIXER_ELEMENT_MAX; i++) {
		if (es.pelements[i].type != SND_MIXER_ETYPE_VOLUME1) continue;
		
		eid[j] = es.pelements[i];
		
		// 初期接続設定
		if (0 == strncmp(SND_MIXER_OUT_MASTER, eid[j].name, strlen(SND_MIXER_OUT_MASTER)) && eid[j].index == 0) {
			mix->connect[MIX_MASTER] = j;
		}
		if (0 == strncmp(SND_MIXER_IN_CD, eid[j].name, strlen(SND_MIXER_IN_CD)) && eid[j].index == 0) {
			mix->connect[MIX_CD] = j;
		}
		if (0 == strncmp(SND_MIXER_IN_SYNTHESIZER, eid[j].name, strlen(SND_MIXER_IN_SYNTHESIZER)) && eid[j].index == 0) {
			mix->connect[MIX_MIDI] = j;
		}
		if (0 == strncmp(SND_MIXER_IN_PCM, eid[j].name, strlen(SND_MIXER_IN_PCM)) && eid[j].index == 0) {
			mix->connect[MIX_PCM] = j;
		}
		
		// 初期 volume の保存
		einit[j].eid = eid[j];
		if (0 > snd_mixer_element_build(handle, &einit[j])) {
			WARNING("snd_mixer_element fail\n");
		}
		NOTICE("%s (%d)\n", eid[j].name, einit[j].data.volume1.pvoices[0]);
		j++;
	}
	emax = j;
	
	// element と element info の取得
	for (i = 0; i < MIX_NRDEVICES; i++) {
		mix->e[i].eid  = eid[mix->connect[i]];
		mix->ei[i].eid = eid[mix->connect[i]];
		snd_mixer_element_build(handle, &mix->e[i]);
		snd_mixer_element_info_build(handle, &mix->ei[i]);
	}
	dev->data_mix = mix;
	
	snd_mixer_close(handle);
	
	g_free(es.pelements);
	
	return 0;
}

