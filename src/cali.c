/*
 * cali.c  計算式の評価
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
/* $Id: cali.c,v 1.16 2002/12/31 04:11:19 chikama Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include "portab.h"
#include "variable.h"
#include "scenario.h"
#include "nact.h"
#include "xsystem35.h"

#define OP_AND 0x74
#define OP_OR 0x75
#define OP_XOR 0x76
#define OP_MUL 0x77
#define OP_DIV 0x78
#define OP_ADD 0x79
#define OP_SUB 0x7a
#define OP_EQUAL 0x7b
#define OP_LT 0x7c
#define OP_GT 0x7d
#define OP_NEQUAL 0x7e
#define OP_END 0x7f

#define OP_C0_INDEX 1
#define OP_C0_MOD 2
#define OP_C0_LE 3
#define OP_C0_GE 4

#define CALI_DEPTH_MAX 256

static int *getVar(int c0, struct VarRef *ref) {
	int addr = sl_getIndex();
	int var;
	if ((c0 & 0x40) == 0) {
		var = c0 & 0x3f;  // 0 - 0x3f
	} else {
		int c1 = sl_getc();
		if (c0 != 0xc0) {
			var = (c0 & 0x3f) * 256 + c1;  // 0x100 - 0x3fff
		} else if (c1 == 1) {
			c0 = sl_getc();
			c1 = sl_getc();
			var = c0 << 8 | c1;
			int index = getCaliValue();
			int *store = v_ref_indexed(var, index, ref);
			if (!store)
				WARNING("%03d:%05x: Out of bounds index access: %s[%d]", sl_getPage(), addr, v_name(var), index);
			return store;
		} else if (c1 >= 0x40) {
			var = c1;  // 0x40 - 0xff
		} else {
			SYSERROR("Invalid variable reference at %d:0x%x", sl_getPage(), addr);
			return NULL;
		}
	}
	int *store = v_ref(var, ref);
	if (!store)
		WARNING("%03d:%05x: Out of bounds array access: %s", sl_getPage(), addr, v_name(var));
	return store;
}

// Returns a pointer to the variable
int *getCaliVariable(void) {
	int *c0 = getVar(sl_getc(), NULL);
	if (sl_getc() != OP_END) {
		SYSERROR("Invalid variable expression at %03d:%05x", sl_getPage(), sl_getIndex());
	}
	return c0;
}

bool getCaliArray(struct VarRef *ref) {
	bool ok = getVar(sl_getc(), ref) != NULL;
	if (sl_getc() != OP_END) {
		SYSERROR("Invalid variable expression at %03d:%05x", sl_getPage(), sl_getIndex());
	}
	return ok;
}

// For variable assignment commands
int *getVariable(void) {
	return getVar(sl_getc(), NULL);
}

// Returns the result of the calculation
int getCaliValue(void) {
	uint16_t stack[CALI_DEPTH_MAX];
	uint16_t *sp = stack;
	int rhs, lhs, result;
	int c0,c1;

	while ((c0 = sl_getc()) != OP_END) {
		if (c0 & 0x80) { // variable
			int c1 = sl_getcAt(sl_getIndex());
			int *t;
			if (c0 == 0xc0) {
				if (c1 == OP_C0_INDEX || c1 >= 0x34) goto l_var;
				c1 = sl_getc();
				if (c1 == OP_C0_MOD) {
					rhs = *--sp;
					lhs = *--sp;
					*sp++ = rhs ? lhs % rhs : 0;
					continue;
				} else if (c1 == OP_C0_LE) {
					rhs = *--sp;
					lhs = *--sp;
					*sp++ = (lhs <= rhs);
					continue;
				} else if (c1 == OP_C0_GE) {
					rhs = *--sp;
					lhs = *--sp;
					*sp++ = (lhs >= rhs);
					continue;
				} else if (c1 < 0x34) {
					SYSERROR("Unknown operator at %03d:%05x", sl_getPage(), sl_getIndex());
				}
			}
		l_var:
			t = getVar(c0, NULL);
			*sp++ = t ? *t : 0;
		} else {
			switch(c0) {
			case OP_NEQUAL:
				rhs = *--sp;
				lhs = *--sp;
				*sp++ = (lhs != rhs);
				break;
			case OP_GT:
				rhs = *--sp;
				lhs = *--sp;
				*sp++ = (lhs > rhs);
				break;
			case OP_LT:
				rhs = *--sp;
				lhs = *--sp;
				*sp++ = (lhs < rhs);
				break;
			case OP_EQUAL:
				rhs = *--sp;
				lhs = *--sp;
				*sp++ = (lhs == rhs);
				break;
			case OP_SUB:
				rhs = *--sp;
				lhs = *--sp;
				result = lhs - rhs;
				*sp++ = result < 0 ? 0 : result;
				break;
			case OP_ADD:
				rhs = *--sp;
				lhs = *--sp;
				result = lhs + rhs;
				*sp++ = result > 0xffff ? 0xffff : result;
				break;
			case OP_DIV:
				rhs = *--sp;
				lhs = *--sp;
				*sp++ = rhs ? lhs / rhs : 0;
				break;
			case OP_MUL:
				rhs = *--sp;
				lhs = *--sp;
				result = lhs * rhs;
				*sp++ = result > 0xffff ? 0xffff : result;
				break;
			case OP_XOR:
				rhs = *--sp;
				lhs = *--sp;
				*sp++ = (lhs ^ rhs);
				break;
			case OP_OR:
				rhs = *--sp;
				lhs = *--sp;
				*sp++ = (lhs | rhs);
				break;
			case OP_AND:
				rhs = *--sp;
				lhs = *--sp;
				*sp++ = (lhs & rhs);
				break;
			default:  // immediate value
				if (c0 & 0x40) {  // 0x0 - 0x33
					*sp++ = (c0 & 0x3f);
				} else {  // 0x34 - 0x3fff
					c1 = sl_getc();
					if (c0 == 0 && c1 <= 0x33)
						SYSERROR("Invalid cali at %03d:%05x", sl_getPage(), sl_getIndex());
					*sp++ = (c0 & 0x3f) << 8 | c1;
				}
			}
		}
	}

	if (sp != stack + 1) {
		WARNING("Unexpected end of expression at %03d:%05x", sl_getPage(), sl_getIndex());
		return 0;
	}
	return *--sp;
}
