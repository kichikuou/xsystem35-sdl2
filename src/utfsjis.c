/*
 * utfsjis.c -- utf-8/sjis related function
 *
 * Copyright (C) 1997 Yutaka OIWA <oiwa@is.s.u-tokyo.ac.jp>
 *
 * written for Satoshi KURAMOCHI's "eplaymidi"
 *                                   <satoshi@ueda.info.waseda.ac.jp>
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
/* $Id: eucsjis.c,v 1.2 2000/09/20 10:33:16 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <dirent.h>

#include "portab.h"
#include "utfsjis.h"
#include "s2utbl.h"
#include "gbk_table.h"

static uint16_t unicode_to_gbk_tbl[65536];
static uint16_t chs_remap_tbl[65536];
static bool chs_remap_loaded = false;

void chs_tbl_reset(void) {
	memset(chs_remap_tbl, 0, sizeof(chs_remap_tbl));
	chs_remap_loaded = false;
}

bool chs_tbl_is_loaded(void) {
	return chs_remap_loaded;
}

bool chs_tbl_load(const char *path) {
	if (!path || !*path)
		return false;
	FILE *fp = fopen(path, "rb");
	if (!fp)
		return false;
	if (fseek(fp, 0, SEEK_END) != 0) {
		fclose(fp);
		return false;
	}
	long size = ftell(fp);
	if (size <= 0 || size > (long)(65536 * 2) || (size & 1) != 0) {
		fclose(fp);
		return false;
	}
	rewind(fp);
	uint8_t *raw = malloc((size_t)size);
	if (!raw) {
		fclose(fp);
		return false;
	}
	size_t nread = fread(raw, 1, (size_t)size, fp);
	fclose(fp);
	if (nread != (size_t)size) {
		free(raw);
		return false;
	}
	chs_tbl_reset();
	size_t count = nread / 2;
	for (size_t i = 0; i < count; i++) {
		chs_remap_tbl[i] = (uint16_t)(raw[i * 2] | ((uint16_t)raw[i * 2 + 1] << 8));
	}
	free(raw);
	chs_remap_loaded = true;
	return true;
}

bool chs_tbl_load_from_dir(const char *dir_path) {
	chs_tbl_reset();
	DIR *dir = opendir(dir_path ? dir_path : ".");
	if (!dir)
		return false;
	struct dirent *d;
	bool loaded = false;
	while ((d = readdir(dir))) {
		if (strcasecmp(d->d_name, "CHS.TBL") == 0) {
			if (dir_path && strcmp(dir_path, ".") != 0) {
				size_t len = strlen(dir_path) + 1 + strlen(d->d_name) + 1;
				char *full = malloc(len);
				if (full) {
					snprintf(full, len, "%s/%s", dir_path, d->d_name);
					loaded = chs_tbl_load(full);
					free(full);
				}
			} else {
				loaded = chs_tbl_load(d->d_name);
			}
			break;
		}
	}
	closedir(dir);
	return loaded;
}

static uint8_t *append_utf8(uint8_t *dstp, int c) {
	if (c <= 0x7f) {
		*dstp++ = (uint8_t)c;
	} else if (c <= 0x7ff) {
		*dstp++ = (uint8_t)(0xc0 | (c >> 6));
		*dstp++ = (uint8_t)(0x80 | (c & 0x3f));
	} else {
		*dstp++ = (uint8_t)(0xe0 | (c >> 12));
		*dstp++ = (uint8_t)(0x80 | ((c >> 6) & 0x3f));
		*dstp++ = (uint8_t)(0x80 | (c & 0x3f));
	}
	return dstp;
}

static void ensure_unicode_to_gbk(void) {
	if (unicode_to_gbk_tbl[0x4e00])
		return;
	for (int lead = 0x81; lead <= 0xfe; lead++) {
		for (int trail = 0x40; trail <= 0xfe; trail++) {
			int code = gbk_to_unicode[lead - 0x81][trail - 0x40];
			if (code && code < 65536 && !unicode_to_gbk_tbl[code])
				unicode_to_gbk_tbl[code] = (uint16_t)((lead << 8) | trail);
		}
	}
}

char* codeconv(CharacterEncoding tocode,
			   CharacterEncoding fromcode,
			   const char *str) {
	if (tocode == fromcode)
		return strdup(str);
	if (tocode == UTF8 && fromcode == SHIFT_JIS)
		return (char *)sjis2utf((const uint8_t *)str);
	if (tocode == SHIFT_JIS && fromcode == UTF8)
		return (char *)utf2sjis((const uint8_t *)str);
	if (tocode == UTF8 && fromcode == GBK)
		return (char *)gbk2utf((const uint8_t *)str);
	if (tocode == GBK && fromcode == UTF8)
		return (char *)utf2gbk((const uint8_t *)str);
	if ((tocode == SHIFT_JIS || tocode == GBK) &&
	    (fromcode == SHIFT_JIS || fromcode == GBK)) {
		uint8_t *utf = fromcode == GBK ? gbk2utf((const uint8_t *)str) : sjis2utf((const uint8_t *)str);
		uint8_t *out = tocode == GBK ? utf2gbk(utf) : utf2sjis(utf);
		free(utf);
		return (char *)out;
	}
	return NULL;
}

uint8_t *sjis2utf(const uint8_t *src) {
	uint8_t* dst = malloc(strlen(src) * 3 + 1);
	uint8_t* dstp = dst;

	while (*src) {
		if (*src <= 0x7f) {
			*dstp++ = *src++;
			continue;
		}

		int c = 0;
		if (*src >= 0xa0 && *src <= 0xdf) {
			c = 0xff60 + *src - 0xa0;
			src++;
		} else {
			if (!src[1])
				break;
			if (chs_remap_loaded) {
				uint16_t mapped = chs_remap_tbl[((unsigned)src[0] << 8) | src[1]];
				if (mapped) {
					if (mapped <= 0x7f) {
						c = mapped;
					} else {
						uint8_t hi = (uint8_t)(mapped >> 8);
						uint8_t lo = (uint8_t)(mapped & 0xff);
						if (hi >= 0x81 && hi <= 0xfe && lo >= 0x40 && lo <= 0xfe)
							c = gbk_to_unicode[hi - 0x81][lo - 0x40];
					}
				}
			}
			if (!c) {
				if (src[1] >= 0x40)
					c = s2u[*src - 0x80][*(src+1) - 0x40];
				else
					c = '?';
			}
			src += 2;
		}

		dstp = append_utf8(dstp, c);
	}
	*dstp = '\0';
	return dst;
}

uint8_t *gbk2utf(const uint8_t *src) {
	uint8_t *dst = malloc(strlen((const char *)src) * 3 + 1);
	uint8_t *dstp = dst;

	while (*src) {
		if (*src <= 0x7f) {
			*dstp++ = *src++;
			continue;
		}
		if (*src == 0x80) {
			dstp = append_utf8(dstp, 0x20ac);
			src++;
			continue;
		}
		int c = '?';
		if (*src >= 0x81 && *src <= 0xfe && src[1] >= 0x40 && src[1] <= 0xfe) {
			c = gbk_to_unicode[*src - 0x81][src[1] - 0x40];
			if (!c)
				c = 0xfffd;
			src += 2;
		} else {
			src++;
		}
		dstp = append_utf8(dstp, c);
	}
	*dstp = '\0';
	return dst;
}

static int unicode_to_sjis(int u) {
	// U+30FB (KATAKANA MIDDLE DOT) is used as replacement character in
	// s2utbl.h, so needs special treatment.
	if (u == 0x30fb)
		return 0x8145;

	for (int b1 = 0x80; b1 <= 0xff; b1++) {
		if (b1 >= 0xa0 && b1 <= 0xdf)
			continue;
		for (int b2 = 0x40; b2 <= 0xff; b2++) {
			if (u == s2u[b1 - 0x80][b2 - 0x40])
				return b1 << 8 | b2;
		}
	}
	return 0;
}

uint8_t *utf2sjis(const uint8_t *src) {
	uint8_t* dst = malloc(strlen(src) + 1);
	uint8_t* dstp = dst;

	while (*src) {
		if (*src <= 0x7f) {
			*dstp++ = *src++;
			continue;
		}

		int u;
		if (*src <= 0xdf) {
			if ((src[1] & 0xc0) != 0x80) {
				*dstp++ = '?';
				src++;
				continue;
			}
			u = (src[0] & 0x1f) << 6 | (src[1] & 0x3f);
			src += 2;
		} else if (*src <= 0xef) {
			if ((src[1] & 0xc0) != 0x80 || (src[2] & 0xc0) != 0x80) {
				*dstp++ = '?';
				src++;
				continue;
			}
			u = (src[0] & 0xf) << 12 | (src[1] & 0x3f) << 6 | (src[2] & 0x3f);
			src += 3;
		} else {
			*dstp++ = '?';
			do src++; while ((*src & 0xc0) == 0x80);
			continue;
		}

		if (u > 0xff60 && u <= 0xff9f) {
			*dstp++ = u - 0xff60 + 0xa0;
		} else {
			int c = unicode_to_sjis(u);
			if (c) {
				*dstp++ = c >> 8;
				*dstp++ = c & 0xff;
			} else {
				*dstp++ = '?';
			}
		}
	}
	*dstp = '\0';
	return dst;
}

uint8_t *utf2gbk(const uint8_t *src) {
	ensure_unicode_to_gbk();
	uint8_t *dst = malloc(strlen((const char *)src) + 1);
	uint8_t *dstp = dst;

	while (*src) {
		if (*src <= 0x7f) {
			*dstp++ = *src++;
			continue;
		}
		int u = utf8_next_codepoint((const char **)&src);
		uint16_t gbk = (u > 0 && u < 65536) ? unicode_to_gbk_tbl[u] : 0;
		if (gbk) {
			*dstp++ = (uint8_t)(gbk >> 8);
			*dstp++ = (uint8_t)(gbk & 0xff);
		} else {
			*dstp++ = '?';
		}
	}
	*dstp = '\0';
	return dst;
}

void cjk_score_sample(const uint8_t *src, int *sjis_err, int *gbk_err, int *gbk_valid) {
	if (!src)
		return;
	const uint8_t *p = src;
	while (*p) {
		if (*p < 0x80) {
			p++;
		} else if ((*p >= 0x81 && *p <= 0x9f) || (*p >= 0xe0 && *p <= 0xfc)) {
			uint8_t b2 = p[1];
			if (b2 < 0x40 || b2 > 0xfc || b2 == 0x7f) {
				(*sjis_err)++;
			} else {
				uint16_t u = s2u[*p - 0x80][b2 - 0x40];
				bool is_sjis_gaiji = (*p >= 0xeb && *p <= 0xfc) && (b2 <= 0xa0);
				if (u == 0x30fb && !(*p == 0x81 && b2 == 0x45) && !is_sjis_gaiji)
					(*sjis_err)++;
			}
			p += b2 ? 2 : 1;
		} else if (*p >= 0xa1 && *p <= 0xdf) {
			p++;
		} else {
			(*sjis_err)++;
			p++;
		}
	}

	p = src;
	while (*p) {
		if (*p < 0x80) {
			p++;
		} else if (*p >= 0x81 && *p <= 0xfe) {
			uint8_t b2 = p[1];
			if (b2 < 0x40 || b2 > 0xfe || b2 == 0x7f || gbk_to_unicode[*p - 0x81][b2 - 0x40] == 0) {
				(*gbk_err)++;
				p++;
			} else {
				(*gbk_valid)++;
				p += 2;
			}
		} else {
			(*gbk_err)++;
			p++;
		}
	}
}

bool cjk_looks_like_gbk(const uint8_t *src) {
	if (!src)
		return false;
	int sjis_err = 0, gbk_err = 0, gbk_valid = 0;
	cjk_score_sample(src, &sjis_err, &gbk_err, &gbk_valid);
	if (gbk_valid <= 0 || sjis_err <= gbk_err)
		return false;

	int sjis_kana = 0, gb2312_valid = 0;
	const uint8_t *p = src;
	while (*p) {
		if (*p < 0x80) {
			p++;
		} else if ((*p >= 0x81 && *p <= 0x9f) || (*p >= 0xe0 && *p <= 0xfc)) {
			uint8_t b2 = p[1];
			if ((p[0] == 0x82 && b2 >= 0x9f && b2 <= 0xf1) ||
				(p[0] == 0x83 && b2 >= 0x40 && b2 <= 0x96)) {
				sjis_kana++;
			}
			p += b2 ? 2 : 1;
		} else {
			p++;
		}
	}
	p = src;
	while (*p) {
		if (*p < 0x80) {
			p++;
		} else if (*p >= 0x81 && *p <= 0xfe) {
			uint8_t b2 = p[1];
			if (b2 < 0x40 || b2 > 0xfe || b2 == 0x7f || gbk_to_unicode[*p - 0x81][b2 - 0x40] == 0) {
				p++;
			} else {
				if (*p >= 0xa1 && *p <= 0xf7 && b2 >= 0xa1 && b2 <= 0xfe)
					gb2312_valid++;
				p += 2;
			}
		} else {
			p++;
		}
	}
	return gb2312_valid > 0 && gb2312_valid > sjis_kana;
}

/* src 内に半角カナもしくはASCII文字があるかどうか */
bool sjis_has_hankaku(const uint8_t *src) {
	while(*src) {
		if (CHECKSJIS1BYTE(*src)) {
			src++;
		} else {
			return true;
		}
		src++;
	}
	return false;
}

/* src 内に 全角文字があるかどうか */
bool sjis_has_zenkaku(const uint8_t *src) {
	while(*src) {
		if (CHECKSJIS1BYTE(*src)) {
			return true;
		}
		src++;
	}
	return false;
}

bool cjk_has_hankaku(const uint8_t *src, CharacterEncoding enc) {
	while (*src) {
		if (CHECKCJK1BYTE(*src, enc)) {
			if (!src[1])
				return false;
			src += 2;
		} else if (enc == UTF8 && (*src & 0x80)) {
			while (*++src && UTF8_TRAIL_BYTE(*src))
				;
		} else {
			return true;
		}
	}
	return false;
}

bool cjk_has_zenkaku(const uint8_t *src, CharacterEncoding enc) {
	while (*src) {
		if (CHECKCJK1BYTE(*src, enc))
			return true;
		if (enc == UTF8 && (*src & 0x80))
			return true;
		src++;
	}
	return false;
}

int utf8_next_codepoint(const char **msg) {
	int code;
	const unsigned char *s = (const unsigned char *)*msg;

	if (*s <= 0x7f) {
		code = *s++;
	} else if (*s <= 0xbf) {
		// Invalid UTF-8 sequence
		code = '?';
		s++;
	} else if (*s <= 0xdf) {
		if ((s[1] & 0xc0) != 0x80) {
			code = 0xfffd;
			s++;
		} else {
			code = (s[0] & 0x1f) << 6 | (s[1] & 0x3f);
			s += 2;
		}
	} else if (*s <= 0xef) {
		if ((s[1] & 0xc0) != 0x80 || (s[2] & 0xc0) != 0x80) {
			code = 0xfffd;
			s++;
		} else {
			code = (s[0] & 0xf) << 12 | (s[1] & 0x3f) << 6 | (s[2] & 0x3f);
			s += 3;
		}
	} else if (*s <= 0xf7) {
		if ((s[1] & 0xc0) != 0x80 || (s[2] & 0xc0) != 0x80 || (s[3] & 0xc0) != 0x80) {
			code = 0xfffd;
			s++;
		} else {
			code = (s[0] & 0x7) << 18 | (s[1] & 0x3f) << 12 | (s[2] & 0x3f) << 6 | (s[3] & 0x3f);
			s += 4;
		}
	} else {
		code = 0xfffd;  // REPLACEMENT CHARACTER
		s++;
		while (0x80 <= *s && *s <= 0xbf)
			s++;
	}
	*msg = (const char *)s;
	return code;
}

int cjk_display_width(const char *s, CharacterEncoding enc) {
	if (!s)
		return 0;
	if (enc != UTF8)
		return (int)strlen(s);
	int width = 0;
	while (*s) {
		int cp = utf8_next_codepoint(&s);
		if (cp <= 0x7f || (cp >= 0xff61 && cp <= 0xff9f))
			width += 1;
		else
			width += 2;
	}
	return width;
}

char *advance_char(const char *s, CharacterEncoding e) {
	switch (e) {
	case SHIFT_JIS:
		return (char *)s + ((CHECKSJIS1BYTE(*s) && *(s + 1)) ? 2 : 1);
	case GBK:
		return (char *)s + ((CHECKGBK1BYTE(*s) && (unsigned char)s[1] >= 0x40) ? 2 : 1);
	case UTF8:
		while (UTF8_TRAIL_BYTE(*++s))
			;
		return (char *)s;
	}
	return NULL;
}
