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
#include "nls.h"

#include <string.h>
#include <SDL_locale.h>

// Defines the translation tables and the nls_catalogs registry.
#include "nls_catalog.h"

static const struct nls_entry *active_table;

static const struct nls_entry *find_catalog(const char *lang) {
	for (const struct nls_catalog *c = nls_catalogs; c->lang; c++) {
		if (!strcmp(c->lang, lang))
			return c->table;
	}
	return NULL;
}

void builtin_nls_init(void) {
	SDL_Locale *locales = SDL_GetPreferredLocales();
	if (!locales)
		return;
	// Pick the catalog for the highest-priority locale that has one.
	for (const SDL_Locale *l = locales; l->language && !active_table; l++)
		active_table = find_catalog(l->language);
	SDL_free(locales);
}

const char *builtin_gettext(const char *msgid) {
	if (active_table) {
		for (const struct nls_entry *e = active_table; e->msgid; e++) {
			if (strcmp(e->msgid, msgid) == 0)
				return e->msgstr;
		}
	}
	return msgid;
}
