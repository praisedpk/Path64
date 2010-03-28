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


#pragma ident "@(#) libfi/mathlb/hsqrt.c	92.1	07/09/99 11:00:36"


#include <fortran.h>
#include <math.h>

extern _f_real4 _HSQRT_( _f_real4 *x );

/*
 * HSQRT: real(kind=4) - pass by address
 */
_f_real4
_HSQRT_( _f_real4 *x )
{
	_f_real8 __sqrt(_f_real8 x);
	return ((_f_real4) __sqrt((_f_real8) *x));
}
