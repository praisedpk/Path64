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


#pragma ident "@(#) libf/pxf/pxfchroot.c	92.1	06/29/99 11:36:06"
#include <errno.h>
#include <fortran.h>
#include <liberrno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern char *_fc_acopy(_fcd f);

/*
 *      PXFCHROOT       Change the root directory to the
 *                      specified directory.
 *
 *      Call from Fortran:
 *
 *              SUBROUTINE PXFCHROOT (PATH, ILEN, IERROR)
 *              CHARACTER * (*) PATH
 *              INTEGER ILEN, IERROR
 *
 *      Where:
 *
 *      PATH    is an input character variable or array element
 *              containing the name of the new directory.
 *
 *      ILEN    is an input integer variable containing the length
 *              of PATH in characters.  If ILEN is zero, all
 *              trailing blanks are removed before calling chroot().
 *
 *      IERROR  is an output integer variable that will contain the
 *              status:
 *
 *                 Zero if PXFCHROOT is successful; otherwise nonzero.
 *
 *              In addition to the errors returned by the chroot(2)
 *              system call, PXFCHROOT may return the following errors:
 *
 *              EINVAL  If ILEN < 0 or ILEN > LEN(PATH)
 *
 *              ENOMEM  If PXFCHROOT is unable to obtain memory to
 *                      copy PATH.
 */

#ifdef _UNICOS
void
PXFCHROOT(
#else
void
_PXFCHROOT(
#endif
	_fcd	PATH,		/* Character variable containing path */
				/* of new directory */
	_f_int	*ILEN,		/* Length of pathname */
	_f_int	*IERROR)	/* Error Status */
{
	_f_int	arglen, errsts, length;
	char	*argstr, *pthstr;
	errsts	= 0;
	argstr	= _fcdtocp(PATH);
	arglen	= (_f_int) _fcdlen (PATH);
	length  = *ILEN;
	if (length < 0 || length > arglen)
		errsts  = EINVAL;
	else {
		/*
		 * If length is zero, strip trailing blanks. Otherwise,
		 * malloc memory and copy the string; adding a NULL
		 * terminator.
		 */
		if (length == 0)
			pthstr  = _fc_acopy(PATH);
		else
			pthstr  = (char *) malloc(length + 1);
		if (pthstr == NULL)     /* If no memory allocated */
			errsts  = ENOMEM;
		else {
			if (length != 0) {      /* Copy argument */
				(void) memcpy(pthstr, argstr, length);
				pthstr[length]  = '\0';
			}
			/* Change directories through chdir() to path. */
			if (chroot(pthstr) == -1)
				errsts  = errno;
			free(pthstr);
		}
	}
	*IERROR = errsts;
	return;
}



#ifndef _UNICOS
void
pxfchroot_(
	   char *PATH,		        /* Character variable containing path */
				        /* of new directory */
	   _f_int	*ILEN,		/* Length of pathname */
	   _f_int	*IERROR,	/* Error Status */
	   _f_int	pathlen)
{
  _PXFCHROOT(_cptofcd(PATH, pathlen), ILEN, IERROR);
}
#endif
