/*
 * Copyright (C) 2026 <KichikuouChrome@gmail.com>
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
#ifndef __NLS_H__
#define __NLS_H__

// Built-in message catalog used on platforms without libintl.

struct nls_entry {
	const char *msgid;   // NULL marks the end of a table
	const char *msgstr;
};

// Maps a language code to its message table.
struct nls_catalog {
	const char *lang;                 // e.g. "ja"; NULL marks the end
	const struct nls_entry *table;
};

// Selects the catalog matching the user's locale. Call once at startup.
void builtin_nls_init(void);

// Returns the translation of msgid, or msgid itself if untranslated.
const char *builtin_gettext(const char *msgid);

#endif /* __NLS_H__ */
