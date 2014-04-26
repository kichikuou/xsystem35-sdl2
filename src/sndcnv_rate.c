/*
 * sndcnv_rate.c  PCM ¼þÇÈ¿ôÊÑ´¹
 *
 * Copyright (C) 1997-1998 Masaki Chikama (Wren) <chikama@kasumi.ipl.mech.nagoya-u.ac.jp>
 *               1998-                           <masaki-c@is.aist-nara.ac.jp>
 *
 * based on sox/rate.c Copyright 1998 Fabrice Bellard.
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
/* $Id: sndcnv_rate.c,v 1.1 2002/08/18 09:35:29 chikama Exp $ */

#include <glib.h>

#define LONG    gint32
#define ULONG   guint32

#define FRAC_BITS 16

/* Private data */
typedef struct ratestuff {
	ULONG opos_frac;  /* fractional position of the output stream in input stream unit */
	ULONG opos;
	ULONG opos_inc_frac;  /* fractional position increment in the output stream */
	ULONG opos_inc; 
	ULONG ipos;      /* position in the input stream (integer) */
	LONG ilast; /* last sample in the input stream */
} *rate_t;

/*
 * Prepare processing.
 */
int st_rate_start(sndcnv_t *effp) {
	rate_t rate = (rate_t) effp->priv;
	ULONG incr;
	
	// printf("inrate = %d, outrate = %d\n", effp->ininfo.rate, effp->outinfo.rate);
	if (effp->ifmt.rate >= 65535 || effp->ofmt.rate >= 65535) {
		// st_fail("rate effect can only handle rates <= 65535");
		return NG;
        }
	
	rate->opos_frac = 0;
	rate->opos = 0;
	
        /* increment */
	incr = (ULONG)((double)effp->ifmt.rate / (double)effp->ofmt.rate * 
		       (double) ((unsigned long)1 << FRAC_BITS));
	
	rate->opos_inc_frac = incr & (((unsigned long)1 << FRAC_BITS) -1);
	rate->opos_inc = incr >> FRAC_BITS;
	
	// printf("inc_frac = %d\n", rate->opos_inc_frac);
	// printf("inc = %d\n", rate->opos_inc);
	
	rate->ipos = 0;
	rate->ilast = 0;
	
	return OK;
}

int st_rate_flow(sndcnv_t *effp, LONG *ibuf, LONG *obuf, LONG *isamp, LONG *osamp) {
	rate_t rate = (rate_t) effp->priv;
	LONG *istart, *iend;
	LONG *ostart, *oend;
	LONG ilast, icur, out;
	ULONG tmp;
	double t;
	
	ilast = rate->ilast;
	
	istart = ibuf;
	iend   = ibuf + *isamp;
	
	ostart = obuf;
	oend   = obuf + *osamp;
	
	while (obuf < oend) {
		// printf("%p, %p\n", obuf, oend);
		
		/* Safety catch to make sure we have input samples.  */
		if (ibuf >= iend) goto the_end;
		
		/* read as many input samples so that ipos > opos */
		
		while (rate->ipos <= rate->opos) {
			ilast = *ibuf++;
			rate->ipos++;
			/* See if we finished the input buffer yet */
			if (ibuf >= iend) goto the_end;
		}
		
		icur = *ibuf;
		
		/* interpolate */
		t = (double)rate->opos_frac / ((unsigned long)1 << FRAC_BITS);
		out = (double)ilast * (1.0 - t) + (double)icur * t;

		/* output sample & increment position */
		
		*obuf++ = (LONG)out;
		
		tmp = rate->opos_frac + rate->opos_inc_frac;
		rate->opos = rate->opos + rate->opos_inc + (tmp >> FRAC_BITS);
		rate->opos_frac = tmp & (((unsigned long)1 << FRAC_BITS) - 1);
	}
 the_end:
	*isamp = ibuf - istart;
	*osamp = obuf - ostart;
	rate->ilast = ilast;
	return OK;
}
