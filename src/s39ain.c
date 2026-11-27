/*
 * s39ain.c  System39.ain read
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
/* $Id: s39ain.c,v 1.9 2003/07/21 23:06:47 chikama Exp $ */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "portab.h"
#include "system.h"
#include "LittleEndian.h"
#include "s39ain.h"
#include "xsystem35.h"
#include "modules.h"
#include "nact.h"
#include "utfsjis.h"

static bool s39ain_msg_is_valid_utf8(const uint8_t *s, bool *has_non_ascii) {
	while (*s) {
		if (*s < 0x80) {
			s++;
		} else if ((s[0] & 0xe0) == 0xc0 && s[0] >= 0xc2 && (s[1] & 0xc0) == 0x80) {
			*has_non_ascii = true;
			s += 2;
		} else if ((s[0] & 0xf0) == 0xe0 && (s[1] & 0xc0) == 0x80 && (s[2] & 0xc0) == 0x80) {
			if ((s[0] == 0xe0 && s[1] < 0xa0) || (s[0] == 0xed && s[1] >= 0xa0))
				return false;
			*has_non_ascii = true;
			s += 3;
		} else if ((s[0] & 0xf8) == 0xf0 && s[0] <= 0xf4 && (s[1] & 0xc0) == 0x80 && (s[2] & 0xc0) == 0x80 && (s[3] & 0xc0) == 0x80) {
			if ((s[0] == 0xf0 && s[1] < 0x90) || (s[0] == 0xf4 && s[1] >= 0x90))
				return false;
			*has_non_ascii = true;
			s += 4;
		} else {
			return false;
		}
	}
	return true;
}

/*
  system39.ain の読み込み
*/
bool s39ain_init(const char *path_to_ain, S39AIN *ain) {
	FILE *fp;
	long len;
	char *buf;
	unsigned char *p;
	int i;
	
	if (NULL == (fp =  fopen(path_to_ain, "rb"))) {
		WARNING("fail to open %s", path_to_ain);
		return false;
	}
	
	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	buf = malloc(len + 4); /* +4 : VARI/MSGI... 拡張のため */
	if (fread(buf, len, 1, fp) != 1) {
		WARNING("%s: %s", path_to_ain, strerror(errno));
		free(buf);
		fclose(fp);
		return false;
	}
	fclose(fp);
	
	p = buf;
	/* first check */
	if (0 != strncmp(p, "AIN", 3)) {
		WARNING("%s is not ain file", path_to_ain);
		free(buf);
		return false;
	}
	
	/* decode .ain file (thanx to Tajiri) */
	i = len -4;
	p = buf +4;
	for (; i > 0; i--) {
		unsigned char b = (*p) >> 6;
		unsigned char c = (*p) << 2;
		*p = b | c;
		p++;
	}
	
	p = buf +8;
	if (0 != strncmp(p, "HEL0", 4)) {
		WARNING("%s is illigal ain file", path_to_ain);
		free(buf);
		return false;
	}
	p += 8;
	ain->dllnum = LittleEndian_getDW(p, 0);
	ain->dll = calloc(ain->dllnum, sizeof(S39AIN_DLLINF));
	
	p += 4;
	for (i = 0; i < ain->dllnum; i++) {
		int fn, j;
		
		ain->dll[i].name = p;
		p += strlen(p) + 1;
		
		fn = LittleEndian_getDW(p, 0); /* number of function in DLL */
		p += 4;
		
		ain->dll[i].function_num = fn;
		ain->dll[i].function = malloc(sizeof(S39AIN_DLLFN) * fn);
		for (j = 0; j < fn; j++) {
			int argc, k;
			
			ain->dll[i].function[j].name = p;
			p += strlen(p) + 1;
			
			argc = LittleEndian_getDW(p, 0); /* number of argument */
			p += 4;
			
			ain->dll[i].function[j].argc = argc;
			ain->dll[i].function[j].argv = malloc(sizeof(int) * argc);
			for (k = 0; k < argc; k++) {
				ain->dll[i].function[j].argv[k] = LittleEndian_getDW(p, 0);
				p += 4;
			}
			ain->dll[i].function[j].entrypoint = NULL;
		}
	}
	
	/* check FUNC */
	if (0 == strncmp(p, "FUNC", 4)) {
		ain->fncnum = LittleEndian_getDW(p, 8);
		
		ain->fnc = malloc(sizeof(S39AIN_FUNCNAME) * ain->fncnum);
		p += 12;
		
		for (i = 0; i < ain->fncnum; i++) {
			ain->fnc[i].name = p;
			p += strlen(p) + 1;
			ain->fnc[i].page  = LittleEndian_getW(p, 0);
			ain->fnc[i].index = LittleEndian_getDW(p, 2);
			p += 6;
		}
	}
	
	/* check VARI */
	if (0 == strncmp(p, "VARI", 4)) {
		ain->varnum = LittleEndian_getDW(p, 8);

		ain->var = malloc(sizeof(char *) * ain->varnum);
		p += 12;

		for (i = 0; i < ain->varnum; i++) {
			ain->var[i] = p;
			p += strlen(p) + 1;
		}
		v_set_names(ain->varnum, ain->var);
	}
	
	/* check MSGI */
	if (0 == strncmp(p, "MSGI", 4)) {
		ain->msgnum = LittleEndian_getDW(p, 8);
		
		ain->msg = malloc(sizeof(char *) * ain->msgnum);
		p += 12;
		
		for (i = 0; i < ain->msgnum; i++) {
			ain->msg[i] = p;
			p += strlen(p) + 1;
		}
	}

	if (!sys_isCharacterEncodingLocked() && ain->msgnum > 0) {
		int sjis_err = 0, gbk_err = 0, gbk_valid = 0;
		int utf8_non_ascii = 0, utf8_err = 0;
		int step = ain->msgnum > 2000 ? ain->msgnum / 2000 : 1;
		for (int m = 0; m < ain->msgnum; m += step) {
			const uint8_t *msg = (const uint8_t *)ain->msg[m];
			bool has_non_ascii = false;
			if (s39ain_msg_is_valid_utf8(msg, &has_non_ascii)) {
				if (has_non_ascii) utf8_non_ascii++;
			} else {
				utf8_err++;
			}
			cjk_score_sample(msg, &sjis_err, &gbk_err, &gbk_valid);
		}
		if (utf8_non_ascii > 0 && utf8_err == 0)
			sys_setDefaultCharacterEncoding(UTF8, false);
		else if (gbk_valid > 0 && sjis_err > gbk_err)
			sys_setDefaultCharacterEncoding(GBK, false);
	}

	for (i = 0; i < ain->dllnum; i++)
		resolve_module(&ain->dll[i]);

#ifdef ALICEHUB_IOS
	ain->file_data = buf;
#endif
	return true;
}

#ifdef ALICEHUB_IOS
void s39ain_fini(S39AIN *ain) {
	s39ain_reset(ain);
	for (int i = 0; i < ain->dllnum; i++) {
		for (int j = 0; j < ain->dll[i].function_num; j++)
			free(ain->dll[i].function[j].argv);
		free(ain->dll[i].function);
	}
	free(ain->dll);
	free(ain->fnc);
	free(ain->var);
	free(ain->msg);
	free(ain->file_data);
}
#endif

void s39ain_reset(S39AIN *ain) {
	for (int i = 0; i < ain->dllnum; i++) {
		if (ain->dll[i].reset)
			ain->dll[i].reset();
	}
}
