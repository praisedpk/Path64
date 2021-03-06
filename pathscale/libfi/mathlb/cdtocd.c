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


#pragma ident "@(#) libfi/mathlb/cdtocd.c	92.1	07/09/99 11:00:36"
#include <fortran.h>
#include <fp.h>
#include <math.h>
#include "inline.h"
#include "mathdefs.h"

extern void _CDTOCD(d_complex_t *ret_val, d_complex_t x, d_complex_t y);

/*
 *	complex(kind=16) raised to a complex(kind=16) = _CDTOCD
 *	x = a+b*i
 *	y = c+d*i
 *
 *	if ((x == 0+0*i) && (y == 0+0*i)) return(NAN)
 *	if (x == 0+0*i) then return(0+0*i)
 *	if (y == 0+0*i) then return(1+0*i)
 */

void
_CDTOCD(d_complex_t *ret_val,
	d_complex_t x,
	d_complex_t y)
{
	_f_real16 __atan2l(_f_real16 ax, _f_real16 bx);
	_f_real16 __cosl(_f_real16 ax);
	_f_real16 __expl(_f_real16 ax);
	_f_real16 __logl(_f_real16 ax);
	_f_real16 __sinl(_f_real16 ax);
	_f_real16 __sqrtl(_f_real16 ax);
	_f_real16 a;
	_f_real16 b;
	_f_real16 c;
	_f_real16 d;
	_f_real16 one;
	_f_real16 two;
	REGISTER_16 realx;
	REGISTER_16 imagx;
	_f_real16 loglabsx, atn2l, expltwo;
	if (x.real == 0.0 && x.imag == 0.0) {
		if (y.real == 0.0 && y.imag == 0.0) {
			ret_val->real = _DBL_NaN;
			ret_val->imag = _DBL_NaN;
		}
		else {
			ret_val->real = (_f_real16) 0.0;
			ret_val->imag = (_f_real16) 0.0;
		}
		return;
	}
	realx.f = x.real;
	imagx.f = x.imag;
	a = x.real;
	b = x.imag;
	c = y.real;
	d = y.imag;

	/* clear sign bit */
	realx.ui[0] &= ~IEEE_128_64_SIGN_BIT;
	imagx.ui[0] &= ~IEEE_128_64_SIGN_BIT;

	atn2l = __atan2l(b,a);

	if (realx.f > imagx.f)
		loglabsx = __logl(realx.f *
		  __sqrtl(1.0 + (imagx.f/realx.f) * (imagx.f/realx.f)));
	else
		loglabsx = __logl(imagx.f *
		  __sqrtl(1.0 + (realx.f/imagx.f) * (realx.f/imagx.f)));

	one = d * loglabsx + c * atn2l;
	two = c * loglabsx - d * atn2l;
	expltwo = __expl(two);
	ret_val->real = expltwo * __cosl(one);
	ret_val->imag = expltwo * __sinl(one);
}
