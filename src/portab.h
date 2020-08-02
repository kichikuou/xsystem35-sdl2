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

#include "config.h"

#define OK		0
#define NG	      (-1)
#define true            1
#define false           0

#ifndef FALSE
#define FALSE           0
#undef  TRUE
#define TRUE            1
#endif

#define min(x,y) ((x) < (y) ? (x) : (y))
#define max(x,y) ((x) > (y) ? (x) : (y))

typedef	unsigned char  ___BYTE;
typedef	unsigned short ___WORD;
#ifdef _WIN32
typedef	unsigned long  ___DWORD;
#else
typedef	unsigned int   ___DWORD;
#endif
typedef _Bool          ___boolean;

#ifndef BYTE
#define BYTE ___BYTE
#endif

#ifndef WORD
#define WORD ___WORD
#endif

#ifndef DWORD
#define DWORD ___DWORD
#endif

#ifndef boolean
#define boolean ___boolean
#endif

/*
 * Standard gettext macros.
 */
#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,Message) (Message)
#  define dcgettext(Domain,Message,Type) (Message)
#  define bindtextdomain(Domain,Directory) (Domain)
#  define _(String) (String)
#  define N_(String) (String)
#endif

#define swap16(a) ( (((a) & 0xff) << 8) | ((unsigned short)(a) >> 8) )
#define swap32(a) ( ((a) << 24) | \
		   (((a) << 8) & 0x00ff0000) | \
		   (((a) >> 8) & 0x0000ff00) | \
		   ((unsigned int)(a) >>24) )


#endif /* !__PORTAB__ */
