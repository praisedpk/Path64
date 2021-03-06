/*
 * Copyright (C) 2007. PathScale, LLC. All Rights Reserved.
 */
/*
 * Copyright 2004, 2005, 2006 PathScale, Inc.  All Rights Reserved.
 */

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


#pragma ident "@(#) libu/util/i3e/ieee_remainder_h_h.c	92.1	07/07/99 14:37:09"

#include <fenv.h>
#include <fp.h>
#include "i3eintrin.h"

extern float _IEEE_REMAINDER_H_H(float x, float y);

#define IEEE_32_MANT_BITS_H1    11
#define IEEE_32_MANT_BITS_H2    12

/*
 * _IEEE_REMAINDER_H_H(X,Y) - calculate remainder of two real(4)
 *                            arguments.
 *
 *              The standard definition for real is:
 *                 If y = 0, remainder(x,y) = NaN and raise the INVALID
 *                           exception.
 *                 else if x = INFINITY, remainder(x,y) = NaN and raise
 *                           the INVALID exception.
 *                 else remainder(x,y) = x - (REAL(NINT(x/y))) * y
 *
 *              The algorithm for real is:
 *                x - (REAL(NINT(x/y))) * y.
 *
 * Use the following algorithm for x/y rounded quantity:
 *   1.  x/y
 *   2. if(fraction(x/y) = .5 exactly, round to next EVEN number.
 *           0.5 = 0.0, 1.5 + 2.0, 10.5 = 10, etc.
 * Use the following algorithm for the multiply:
 *   ((((x - tdivU *yU) - tdivU * yL) -tdivL * yU) - tdivL * yL)
 * Th calculation is done in double (64-bit) real precision so the
 * upper and lower split is not required.
 */

#pragma _CRI duplicate _IEEE_REMAINDER_H_H as _IEEE_REMAINDER_4_4
#pragma _CRI duplicate _IEEE_REMAINDER_H_H as _IEEE_REMAINDER_4
#pragma _CRI duplicate _IEEE_REMAINDER_H_H as _IEEE_REMAINDER_H
#pragma _CRI duplicate _IEEE_REMAINDER_H_H as _REMAINDER_4
#pragma _CRI duplicate _IEEE_REMAINDER_H_H as _REMAINDER_H
float
_IEEE_REMAINDER_H_H(float argx, float argy)
{
	union _ieee_float {
		float		fpword;
		unsigned int	usword;
		int		int32;
		struct {
#if defined(_LITTLE_ENDIAN)
			unsigned int mantissa	: IEEE_32_MANT_BITS;
			unsigned int exponent	: IEEE_32_EXPO_BITS;
			unsigned int sign	: 1;
#else
			unsigned int sign	: 1;
			unsigned int exponent	: IEEE_32_EXPO_BITS;
			unsigned int mantissa	: IEEE_32_MANT_BITS;
#endif
		} parts;
	};
	union _ieee_float x_val, y_val, nearint, tdiv, evenchk, tmp, res;
	unsigned int	even_x = 0X00000001;
	int xfpclas	= _fpclassifyf(argx);
	int yfpclas	= _fpclassifyf(argy);
	x_val.fpword	= argx;
	y_val.fpword	= argy;

	if ((xfpclas == FP_INFINITE) || yfpclas == FP_ZERO) {
		union _ieee_float x_val;
		int	j;

		x_val.fpword	= _HALF_NaN;

		/* need to emit invalid exception */
		j	= FE_INVALID;
		feraiseexcept(j);
		return(x_val.fpword);
	}
	tdiv.fpword	= argx / argy;
	tmp.usword	= tdiv.usword & (~IEEE_32_SIGN_BIT);

	/* check for 2**23 or greater = already integer */
	if (tmp.fpword < 8388608) {

		/* calculate fraction */
                evenchk.fpword =
                        tdiv.fpword - (float)((int)tdiv.fpword);

                if (tdiv.fpword < 0.0) {
                        nearint.int32 = (int) (tdiv.fpword - 0.5);
                        if ((evenchk.fpword == -0.5) &&
                           ((nearint.usword & even_x) != 0))
                                nearint.int32 += 1;
                } else {
                        nearint.int32 = (int) (tdiv.fpword + 0.5);
                        if ((evenchk.fpword == 0.5) &&
                           ((nearint.usword & even_x) != 0))
                                nearint.int32 -= 1;
                }
                tdiv.fpword = (float) nearint.int32;
        }

	/* algorithm for ieee in 64-bits for x - (x/y)*y. */
	res.fpword	= (float) ((double) x_val.fpword -
		((double) tdiv.fpword * (double) y_val.fpword));
	if (res.fpword == 0.0)
		res.usword= res.usword | (x_val.usword & IEEE_32_SIGN_BIT);
	return(res.fpword);
}
