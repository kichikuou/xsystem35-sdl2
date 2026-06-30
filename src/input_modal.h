/*
 * input_modal.h  string- and number-input dialogs
 *
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

#ifndef __INPUT_MODAL__
#define __INPUT_MODAL__

#include "portab.h"

typedef struct inputstring_param {
	char *title;
	char *oldstring;
	const char *newstring;
	int   max;
	/* for MJ cmd */
	bool need_window;
	int     x, y, h;
} INPUTSTRING_PARAM;

typedef struct inputnum_param {
	char *title;
	int value;
	int def;
	int max;
	int min;
} INPUTNUM_PARAM;

bool input_modal_string(INPUTSTRING_PARAM *);
bool input_modal_string_inline(INPUTSTRING_PARAM *);
bool input_modal_number(INPUTNUM_PARAM *);

#endif /* !__INPUT_MODAL__ */
