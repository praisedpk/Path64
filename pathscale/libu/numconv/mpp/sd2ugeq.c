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


#pragma ident "@(#) libu/numconv/mpp/sd2ugeq.c	92.4	09/01/99 08:50:08"

#include <math.h>
#include <stdio.h>

#if defined(_UNICOS)
#include <fp.h>
#endif

#include <cray/fmtconv.h>
#include "qq_routines.h"

#define DEBUGSW 0

#if defined(_WORD32) || defined(__mips) || defined(_LITTLE_ENDIAN)
#define PORTABLE_INFINITY	0x7ff0000000000000LL
#define	NOT_SIGN_MASK	0x7fffffffffffffffLL
#define	SIGN_MASK	0x8000000000000000LL

#else
#define PORTABLE_INFINITY	0x7ff000000000000
#define	NOT_SIGN_MASK	0x7fffffffffffffff
#define	SIGN_MASK	0x8000000000000000
#endif

#define	ZERO		'0'
#define	BLANK		' '
#define	POSITIVE	0
#define	NEGATIVE	1
#define SAFE_DENORM	4503599627370496.0

/*
	General:
		Convert the value to the formatted ASCII character field 
		specified.  Using the Fortran "Gw.dEe" format.

	Inputs:
		value - value to output
		fca - first character address of ASCII output field
		mode - flags to determine sign behavior
			0 = only minus sign required
			1 = sign required
			0200 = skip write of minus sign for -0.0
		w - total width of character field
		d - width of field after decimal point
		e - field width of exponent
		p - scale factor

	Outputs:
		value - unchanged
		fca - unchanged
		mode - unchanged
		w - unchanged
		d - unchanged
		e - unchanged
		p - unchanged

        Examples:
		With value=0.105e10, mode=1, w=10, d=2, e=0, p=0 we will end up
		with a field filled with " +0.11E+10"

		With value=-9.995e-10, mode=0, w=10, d=2, e=2, p=1 we will end
		up with a field filled with " -1.00E-09"

		With value=-0.005e-5, mode=0, w=12, d=2, e=4, p=0 we will end up
		with a field filled with " -0.50E-0007"

		With value=0.55, mode=0, w=8, d=2, e=0, p=-1 we will end up
		with a field filled with "0.55    "

		With value=-4.2455e+1, mode=0, w=12, d=2, e=4, p=0 we will end 
		up with a field filled with "-42.46      "

*/
long
*_sd2ugeq(	const void	*value,
		long		*fca,
		const long	*mode,
		const long	*w,
		const long	*d,
		const long	*e,
		const long	*p)
{
	long	*position_pointer;
	long	*retval;
	long	field_width;
	long	flags;
	long	decimal_places;
	long	exponent_size;
	long	scale_factor;
	long	filler;
	long	g_as_f;
	long	*starting_integer_position;
	long	*starting_fraction_position;
	long	*starting_exponent_position;
	long	*end_pointer;
	FLOAT64         qq_as64[5];
	FLOAT128	datum;
	FLOAT128	datumh;
	FLOAT128	datuml;
	FLOAT128	delta;
	char	sign;
	char	zsign;
	long	integer_digits;
	long	exponent_field_size;
	long	digit;
	long	power10;
	long	_OUTPUT_NAN_INF();
	long	power_of_ten;
	int32	denorm_2_adjust;
	long	digits_left;
	int32	roundby;
	int32	scaleoverten ; 
	int32	roundto10 ; 
	union	{FLOAT128 q[2]; uint64 i64[4]; uint32 i32[8];
		FLOAT128 d; uint64 i; FLOAT64 dbl; }	tmp,
					qpower,
					datumi,
					upper,
					lower,
					sign_bit;
	uint32	signexp;
/*
	set-up initial state
*/
#ifdef __mips
	/* convert double-double float to IEEE 128-bit float. */
	(void) _m128toi128(&datum, (FLOAT128 *)value);
#else
	datum = *(FLOAT128 *)value;
#endif
        if(DEBUGSW) printf("_sd2ugeq:V:'%50.40Le' %d P f %d .%d e %d\n",
		datum,(int32)*p,(int32)*w,(int32)*d,(int32)*e );

	position_pointer = (long *)fca;
	field_width = *w;
	retval = position_pointer+field_width;
	flags = *mode;
	decimal_places = *d;
	exponent_size = *e;
	scale_factor = *p;
	sign = BLANK;

/*
	perform some argument checking
*/
	if (decimal_places < 0) goto STARFILL;
	if (field_width < 0) goto STARFILL;
	if (exponent_size < 0) goto STARFILL;
	if (exponent_size == 0) exponent_size = 2; /* possible "Ew.d" format*/

/*
	handle special case NaN and Infinity input data
*/
	tmp.d = datum;
	upper.i = tmp.i & SIGN_MASK ;
	tmp.i &= NOT_SIGN_MASK ;
	sign_bit.i = upper.i;
	if ( tmp.i >= QINFINITY) { /* if NaN or Inf, process */
		upper.i = upper.i | PORTABLE_INFINITY;
		if (ISNANL(datum) ) upper.i = upper.i | 1 ;  /* make it a NaN */
		(void)_OUTPUT_NAN_INF(upper.dbl,position_pointer,
				flags,field_width);
		goto DONE;
	}

/*
	convert denormals to normals and preserve the sign such that
	denormal = (2**-denorm_2_adjust)*normal. 
        We cannot count on hardware multiply to do this, since denormals
        aren't always allowed and can be treated as zero.
        Insert exponent(1.0) creating 1.xyyyyy (Don't trust scalbl for this)
        then while we know it's positive, convert to 0.xyyyyy .
        If x is 1 the exponent will be exp(0.5), if not, it will be less.
        In either case we have implicitly multiplied by 2^exp(0.5).
        Only after this is done can we sign the scaled datum.
        THIS WAS: denormal = (10**-denorm_adjust)*normal.

	_m128toi128 should never create an IEEE128 denormal, but
	test for it just the same.  The algorithm here depends on IEEE128
	arithmetic. Denormal support is not required.
*/
        denorm_2_adjust = 0;
        if ( ( tmp.i64[0] != 0 || tmp.i64[1] != 0) &&
                ( tmp.i64[0] >> QMANTISSA_TOPBITS ) == 0 ) {
                        denorm_2_adjust = QEXPONE-1 ;
                        tmp.i += ( ((int64) QEXPONE) << QMANTISSA_TOPBITS );
                        tmp.d = tmp.d - 1.0;
                        tmp.i += upper.i;
                        datum = tmp.d;
                        tmp.i -= upper.i;
#if defined(__mips)
			datum = 0.0;	/* IEEE128 underflow => 0.0 */
#endif
	}

/*
	determine sign
*/
	if ((flags & MODESN) != 0) sign = '+';
	zsign	= sign;
/*
 *	Use the unsigned integer sign value to check for sign, since
 *	the check for less than zero does not apply to a signed zero.
 *	For IRIX, the default is to skip the write of the minus sign
 *	for -0.0.  The mode bit MODEMSN indicates this skip.  If the
 *	assign option -Z on/off is present for external files, the
 *	handling of the minus sign for -0.0 can be switched for
 *	external files.
 *	if (datum < 0.0) {
 */
	if (sign_bit.i != 0) {
		if (((flags & MODEMSN) != 0) && (datum < 0.0) ||
		    ((flags & MODEMSN) == 0)) {
			sign = '-';
			datum = tmp.d ; /* remove sign. Don't use -datum on MIPS */
		}
	}

/*
	check for Zero input data

	Fortran 90 output gives: F(w-n).(d-1),n(' ')
	Fortran 77 output gives: Ew.dEe

*/
	if (datum == 0.0) {
		if ((flags & MODE77) == 0) {
			decimal_places--; 
			if (decimal_places < 0) goto STARFILL;
		}
		if (field_width-exponent_size-2 <= decimal_places+1)
			if ((field_width-exponent_size-2 < decimal_places+1) ||
				(sign != BLANK)) goto STARFILL;
		if (field_width-exponent_size-2 < decimal_places+3) {
			filler = ZERO;
			if (sign != BLANK) filler = sign;
			if (field_width-exponent_size-2 == decimal_places+2)
				*position_pointer++ = filler;
		}
		else {
			starting_integer_position = position_pointer+
				field_width-exponent_size-2-decimal_places-3;
			while (position_pointer < starting_integer_position)
				*position_pointer++ = BLANK;
			*position_pointer++ = sign;
			*position_pointer++ = ZERO;
		};
		*position_pointer++ = '.';
		for ( ; decimal_places > 0; decimal_places--)
			*position_pointer++ = ZERO;
		if ((flags & MODE77) != 0) {
			*position_pointer++ = 'E';
			*position_pointer++ = '+';
			filler = ZERO;
		}
		else {
			exponent_size += 2;
			filler = BLANK;
		}
		for ( ; exponent_size > 0; exponent_size--)
			*position_pointer++ = filler;
		goto DONE;
	};

        if(DEBUGSW) printf(
	"_sd2ugeq:scale in:'%50.40Le'/2^%d %d P f %d .%d e %d \n",
		datum, denorm_2_adjust, (int32)*p,(int32)*w,(int32)*d,(int32)*e );

/*
	scale the value such that (1.0 <= value < 20.0)
*/
	_qqscale20( &datum, &tmp.q[0], &tmp.q[1], &denorm_2_adjust,
		&scaleoverten, &power_of_ten ) ;

	/****************************************************
	 *  _qqscale20 result will not be < 1.0 .
	 *  The digit creator can handle numbers in [1,20).
	 ****************************************************/

	if (tmp.i64[0] < IEEE_128_ONE_HI64 ) { /* .lt. 1.0 */
		if(DEBUGSW)
			printf( "\n_qqscale <1 ERROR\n");
		tmp.i64[0] = IEEE_128_ONE_HI64;
		tmp.i64[1] = 0;
		tmp.q[1] = 0.0;
	}
        if(DEBUGSW) printf("_sd2ugeq:scale out:'%50.40Le' %x *10^%d \n",
		tmp.q[0], tmp.i32[0], (int32) power_of_ten );
/*
	round the value according to decimal_places and scale_factor
*/
	g_as_f = NEGATIVE;
	filler = decimal_places;
	if (power_of_ten < decimal_places) {
		if (power_of_ten >= -1) {
			filler -= 1;
			g_as_f = POSITIVE;
		}
	};
	if (g_as_f != POSITIVE) {
		if (scale_factor <= 0) filler = decimal_places-1+scale_factor;
	}
	if (filler >= IEEE_128_DEC_OUT_DIGITS) filler = IEEE_128_DEC_OUT_DIGITS-1;
	/****************************************************
	 *  10^-filler/2 = 5^-filler * 2^-(filler+1) is the round,
	 *  if the scaled number is < 10; if >=10 use 10* the round.
	 ****************************************************/
	if (tmp.i32[0] >= IEEE_128_TEN_HI32 ) roundby = 1-filler; /* .ge. 10. */
	else			roundby = -filler ;
	/***************************************************
	 * _qq_power5 returns 5.0^roundby as a 256-bit variable,
	 * the result of multiplying coarse and fine powers.
	 * (5^roundby)*(2^roundby)/2 = (10^roundby)/2 .
	 ****************************************************/
	_qq_power5( &roundby, &qpower.q[0], &qpower.q[1] );
	qpower.i64[0] += ((int64)roundby -1) << QMANTISSA_TOPBITS ;

        if(DEBUGSW) printf("_sd2ugeq:round:'%50.40Le' %x *10^%d \n",
		qpower.q[0], qpower.i32[0], roundby );

	_qq_addunsigned( &qpower.q[0], &qpower.q[1],&tmp.q[0], &tmp.q[1], 
		&qpower.q[0], &qpower.q[1] );

        if(DEBUGSW) printf("_sd2ugeq:rounded:'%50.40Le' %x *10^%d \n",
		qpower.q[0], qpower.i32[0], (int32) power_of_ten );

        /****************************************************
         *  If the rounded number ended up >= 10, the digit output
         *  will compensate, so increment power_of_ten and continue.
         *  G as F needs to know if rounding made it over 10.
         *  Convert to fixed point in qq_as64[].
         ****************************************************/
        QQCONVERT2AS64( qpower, qq_as64, power_of_ten, scaleoverten, roundto10)

	digits_left = IEEE_128_DEC_OUT_DIGITS;
	if ( roundto10 != 0 ) {
		if (g_as_f == POSITIVE) {
			if (power_of_ten == decimal_places) g_as_f = NEGATIVE;
		}
		else {
			if (power_of_ten == -1 && (flags&MODE77)!=0 )
				g_as_f = POSITIVE;
		};
	};

/*
	if (g_as_f is positive) then
		output the value in "Fw.d"
	else
		output the value in "Gw.dEe" if exponent_size > 0 or "Gw.d" if 
		exponent_size == 0 on routine entry
*/

	if (g_as_f == POSITIVE) {
		integer_digits = 1+power_of_ten;
		decimal_places = decimal_places-integer_digits;
		if (integer_digits > 0) { /* number > 1.0 */
			if (integer_digits >= (field_width-decimal_places-
				3-exponent_size))
				if ((integer_digits > (field_width-
					decimal_places-3-exponent_size)) ||
					(sign != BLANK)) goto STARFILL;
			starting_integer_position = position_pointer+
						(field_width-decimal_places-
						3-integer_digits-exponent_size);
			while (position_pointer < (starting_integer_position-1))
				*position_pointer++ = BLANK; /* space fill */
			if (position_pointer < starting_integer_position)
				*position_pointer++ = sign;
               	PUTDIGITLOOP( qq_as64,
                   	starting_integer_position+ integer_digits,
                        position_pointer, digits_left )

		*position_pointer++ = '.';
		starting_fraction_position = position_pointer;
		}
		else { /* number < 1.0 */
			if (-integer_digits >= decimal_places) {
				integer_digits = -decimal_places;
				/* for small numbers, check flag for
				 * write of minus sign for -0.0.
				 */
				if ((flags & MODEMSN) != 0)
					sign	= zsign;
                        }
			if (field_width-exponent_size-2 <= (decimal_places+1))
				if ((field_width-exponent_size-2 <
					(decimal_places+1)) ||
					(sign != BLANK)) goto STARFILL;
			starting_fraction_position = position_pointer+
				(field_width-decimal_places-exponent_size-2);
			while (position_pointer < (starting_fraction_position-3)
				) *position_pointer++ = BLANK; /* space fill */
			if (position_pointer == (starting_fraction_position-3)) 
				{
				*position_pointer++ = sign;
				*position_pointer++ = ZERO;
			}
			else {
				if (position_pointer ==
					(starting_fraction_position-2)) {
					filler = ZERO;
					if (sign != BLANK) filler = sign;
					*position_pointer++ = filler;
				};
			};
			*position_pointer++ = '.';
			for (; integer_digits < 0; integer_digits++)
				*position_pointer++ = ZERO;
		};
/*********************** ELIMINATED CODE *********************
		while (position_pointer < (starting_fraction_position+
			decimal_places)) {
				GETONEDIGIT( position_pointer, datumh, datuml )
		};
*************************************************************/
               		PUTDIGITLOOP( qq_as64,
                       		starting_fraction_position+ decimal_places,
                        	position_pointer, digits_left )

		for (integer_digits = 0; integer_digits < exponent_size+3;
			integer_digits++) *position_pointer++ = BLANK;
		goto DONE;
	}
	else {
		power10 = power_of_ten-scale_factor+1;
/*
        If e=0 and |exp|>99, use snnn, omitting the E.
        exponent_size in this case is the exponent field width minus 2.
        If e=0 and |exp|>999, use snnnn so exponent_size = 3.
*/
        	if( *e == 0 ){
                  if( ((-power10) >999) || (power10 >999) ) exponent_size += 1;
        	}
		exponent_field_size = exponent_size+2;
		starting_integer_position = position_pointer+field_width-2-
					decimal_places-exponent_field_size;
		starting_exponent_position = position_pointer+field_width-
						exponent_field_size;
		if (scale_factor > 0) { /* mantissa >= 1.0 */
			if (scale_factor >= (decimal_places+2)) goto STARFILL;
			if (decimal_places+2 >= field_width-exponent_field_size)
				if (decimal_places+2 > field_width-
					exponent_field_size
					|| sign != BLANK) goto STARFILL;
			while (position_pointer < (starting_integer_position-1))
				*position_pointer++ = BLANK; /* space fill */
			if (position_pointer < starting_integer_position)
				*position_pointer++ = sign;
/*********************** ELIMINATED CODE *********************
			while (position_pointer < starting_integer_position+
				scale_factor) {
				GETONEDIGIT( position_pointer, datumh, datuml )
			};
*************************************************************/
               		PUTDIGITLOOP( qq_as64,
                       		starting_integer_position+ scale_factor,
                        	position_pointer, digits_left )

			*position_pointer++ = '.';
			starting_fraction_position = position_pointer;
		}
		else { /* number < 1.0 */
			if (scale_factor <= (-decimal_places)) goto STARFILL;
			if (field_width-exponent_field_size <= decimal_places+1)
				if ((field_width-exponent_field_size <
					decimal_places+1)
					|| (sign != BLANK)) goto STARFILL;
			starting_fraction_position = position_pointer+
				field_width-decimal_places-exponent_field_size;
			while (position_pointer < starting_fraction_position-3)
				*position_pointer++ = BLANK; /* space fill */
			if (position_pointer == (starting_fraction_position-3)) 
				{
				*position_pointer++ = sign;
				*position_pointer++ = ZERO;
			}
			else {
				if (position_pointer ==
					(starting_fraction_position-2)) {
					filler = ZERO;
					if (sign != BLANK) filler = sign;
					*position_pointer++ = filler;
				};
			};
			*position_pointer++ = '.';
			for (integer_digits = scale_factor; integer_digits < 0;
				integer_digits++) *position_pointer++ = ZERO;
			starting_fraction_position = position_pointer;
		};
/*********************** ELIMINATED CODE *********************
		while (position_pointer < starting_exponent_position) {
				GETONEDIGIT( position_pointer, datumh, datuml )
		};
*/
               		PUTDIGITLOOP( qq_as64,
                       		starting_exponent_position,
                        	position_pointer, digits_left )

	};

/*
	output the exponent in "Esnn" or "sn...n" format
*/
	if( _outputexponent( power10, position_pointer, exponent_field_size, e))
		goto DONE;
STARFILL0:
	field_width = *w;
	position_pointer = (long *)fca;
STARFILL:
	for ( ; field_width > 0; field_width--)
		*position_pointer++ = '*';
DONE:
	return(retval);
}
