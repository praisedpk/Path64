/*

  Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.

   Path64 is free software; you can redistribute it and/or modify it
   under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation version 2.1

   Path64 is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with Path64; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

   Special thanks goes to SGI for their continued support to open source

*/


static const char USMID[] = "@(#) libu/ffio/posix/chknset.c	92.0	10/08/98 14:57:41";

#include <stdio.h>
#include <ffio.h>
#include "../f77io.h"

/*
 *	_chk_n_set()
 *
 *	Validates the string parameters (for example record format and record 
 *	subtype) and the numeric parameters (for example recsize and mbs)
 *	for a particular FFIO layer class.
 *
 *	Bounds checking is done, and default rfc (record formats) and rfc_subc 
 *	(record subtype) values are inserted for some layers.
 *
 *	A return value of non-zero indicates failure, and the returned value
 *	indicates the failure mode.
 *
 *	an input value of < 0 indicates an unrecognized token for that
 *		parameter.
 *	an input value of zero indicates a parameter spec that was not
 *		specified
 */


int
_chk_n_set( class, rfc_p, rfc_subc_p, par_count, n_param)
int class;
int *rfc_p;
int *rfc_subc_p;
int par_count;		/* number of numeric parameters in n_param array */
int n_param[];
	{
	int rfc, rfc_subc;
	int rsz, mbs, num3;
	int bdwlen;
	
	rsz = mbs = num3 = 0;
	switch(par_count) {
		default:	num3	= n_param[2];
		case 2:		mbs	= n_param[1];
		case 1:		rsz	= n_param[0];
		case 0:		break;
	}

	rfc = *rfc_p;
	rfc_subc = *rfc_subc_p;

	if (class <= 0) return(FDC_PERR_CLASS);
	if (rfc < 0) return(FDC_PERR_RFC);
	if (rfc_subc < 0) return(FDC_PERR_RFCS);

	switch (class)
		{
		default:
			if (rsz < 0) return(FDC_PERR_RSZ);
			if (mbs < 0) return(FDC_PERR_MBS);
			break;

		case CLASS_SITE:
		case CLASS_USER:
		case CLASS_USER0:
		case CLASS_USER1:
		case CLASS_USER2:
		case CLASS_USER3:
		case CLASS_USER4:
		case CLASS_USER5:
		case CLASS_USER6:
		case CLASS_USER7:
		case CLASS_USER8:
		case CLASS_USER9:
			/* anything goes for site and user layers */
			break;
		}

/*
 *	Perform class-specific error checking of any string parameters or 
 *	numeric parameters which were passed by the user.
 */
	switch(class)
		{
		case  CLASS_SYSCALL:
		case  CLASS_ERROR:
		case  CLASS_SYSTEM:
		case  CLASS_NULL:
		case  CLASS_FD:
			if ((rfc | rfc_subc | mbs) != 0)
				return(FDC_PERR_RFC);
			break;
		case CLASS_F77:
			switch(rfc)
				{
				case TR_UX_SUN:
				case TR_UX_VAX:
					if (rfc_subc != 0)
						return(FDC_PERR_RFCS);
					break;
				default:
					return(FDC_PERR_RFC);
				}
			break;
		case CLASS_USER:
		case CLASS_USER0:
		case CLASS_USER1:
		case CLASS_USER2:
		case CLASS_USER3:
		case CLASS_USER4:
		case CLASS_USER5:
		case CLASS_USER6:
		case CLASS_USER7:
		case CLASS_USER8:
		case CLASS_USER9:
			/* anything is legal */
			break;
		case  CLASS_SITE:
			/* anything is legal */
			break;
		default:
			return(FDC_PERR_CLASS);
		}
	*rfc_p = rfc;
	*rfc_subc_p = rfc_subc;
	return(0);
	}
