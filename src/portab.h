/*
 * portab.h 汎用定義
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
/* $Id: portab.h,v 1.20 2003/08/30 21:29:16 chikama Exp $ */

#ifndef __PORTAB__
#define __PORTAB__

#include <stdbool.h>
#include <stdint.h>
#include "config.h"

typedef uint16_t vmvar_t;

#ifdef _WIN32
# ifdef min
#  undef min
# endif
# ifdef max
#  undef max
# endif
# define NOMINMAX
#endif
#define min(x,y) ((x) < (y) ? (x) : (y))
#define max(x,y) ((x) > (y) ? (x) : (y))

/*
 * Standard gettext macros.
 */
#ifdef HAVE_LIBINTL
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  include "nls.h"
#  define _(String) builtin_gettext (String)
#  define N_(String) (String)
#  define textdomain(String) (String)
#  define bindtextdomain(Domain,Directory) (Domain)
#endif

#ifndef __EMSCRIPTEN__
#define EMSCRIPTEN_KEEPALIVE
#endif

#endif /* !__PORTAB__ */
