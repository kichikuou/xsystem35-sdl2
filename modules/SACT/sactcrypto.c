/*
 * sactcrypt.c: SACTの暗号化関連
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
/* $Id: sactcrypto.c,v 1.1 2003/04/22 16:29:52 chikama Exp $ */

#include <stdio.h>
#include <glib.h>
#include "portab.h"
#include "system.h"
#include "variable.h"

/*
  多分、セーブ時に暗号化して、ロード時に復号化するので
  適当でいいとおもう
*/

int scryp_encrypt_word(int *array, int num, int key) {
	WARNING("NOT IMPLEMENTED\n");
	return OK;
}

int scryp_decrypt_word(int *array, int num, int key) {
	WARNING("NOT IMPLEMENTED\n");
	return OK;
}

int scryp_encrypt_str(int strno, int key) {
	WARNING("NOT IMPLEMENTED\n");
	return OK;
}

int scryp_decrypt_str(int strno, int key) {
	WARNING("str = '%s'\n", v_str(strno -1));
	WARNING("NOT IMPLEMENTED\n");
	return OK;
}
