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


#pragma ident "@(#) libfi/matrix/matmul_zc4.c	92.1	07/09/99 15:18:08"

#include "matmul.h"

#define NAME _MATMUL_ZC4

/*
 * Subroutine name
 */

#if defined(UNICOS) || defined(UNIX_NAMING_CONVENTION)
#define SUBNAME	_S4DGEMMX
#elif defined(BUILD_COMPILER_GNU) && defined(BUILD_OS_DARWIN)
#define SUBNAME	underscore_s4dgemmx__
#else
#define SUBNAME	s4dgemmx__
#endif

/*
 * Type of constants alpha and beta
 */
#define RESULTTYPE	_f_real16

void
NAME(DopeVectorType * RESULT, DopeVectorType * MATRIX_A,
     DopeVectorType * MATRIX_B)
{
    void    SUBNAME();

    RESULTTYPE	*Ar, *Ai;
    _f_real4	*Br, *Bi;
    RESULTTYPE	*Cr, *Ci;
    MatrixDimenType matdimdata, *MATDIM;

#if !defined(_SOLARIS) && !defined(_CRAYMPP) && !defined(_ABSOFT)
    const RESULTTYPE   neg_one =  (RESULTTYPE) (-1.0);
    const RESULTTYPE   one =  (RESULTTYPE) 1.0;
    const RESULTTYPE   zero = (RESULTTYPE) 0.0;
#else
    struct {
        _f_int4 ireal1;         /* first 32 bits of real part */
        _f_int4 irest2[7];      /* rest of 256-bit double complex */
        } neg_one, one, zero;

    memset(&zero, 0, sizeof(zero));

    one = zero;
    one.ireal1 =  017777600000;

    neg_one = zero;
    neg_one.ireal1=  037777600000;
#endif

        MATDIM = (MatrixDimenType *) &matdimdata;

    /*
     * Parse dope vectors, and perform error checking.
     */

    _premult(RESULT, MATRIX_A, MATRIX_B, MATDIM);

    /*
     * Do the real and imaginary parts separately.
     */

    Ar = (RESULTTYPE *) MATDIM->A;
    Ai = Ar + 1;
    MATDIM->inc1a *=2;
    MATDIM->inc2a *=2;

    Br = (_f_real4 *) MATDIM->B;
    Bi = Br + 1;
    MATDIM->inc1b *= 2;
    MATDIM->inc2b *= 2;

    Cr = (RESULTTYPE *) MATDIM->C;
    Ci = Cr + 1;
    MATDIM->inc1c *= 2;
    MATDIM->inc2c *= 2;

    /*
     * Perform the matrix multiplication.
     * Note:
     * (Cr + Ci*i) = (Ar + Ai*i)*(Br + Bi*i)
     *             = (Ar*Br - Ai*Bi) + (Ar*Bi + Ai*Br)*i
     * Do the transposed problem:  C' = B'*A'
     */

    /* real part */
    SUBNAME(&MATDIM->n,&MATDIM->m,&MATDIM->k,&one,Br,&MATDIM->inc2b,
    	    &MATDIM->inc1b,Ar,&MATDIM->inc2a,&MATDIM->inc1a,&zero,Cr,
    	    &MATDIM->inc1c,&MATDIM->inc2c);
    SUBNAME(&MATDIM->n,&MATDIM->m,&MATDIM->k,&neg_one,Bi,&MATDIM->inc2b,
    	    &MATDIM->inc1b,Ai,&MATDIM->inc2a,&MATDIM->inc1a,&one,Cr,
    	    &MATDIM->inc1c,&MATDIM->inc2c);

    /* imaginary part */
    SUBNAME(&MATDIM->n,&MATDIM->m,&MATDIM->k,&one,Br,&MATDIM->inc2b,
    	    &MATDIM->inc1b,Ai,&MATDIM->inc2a,&MATDIM->inc1a,&zero,Ci,
    	    &MATDIM->inc1c,&MATDIM->inc2c);
    SUBNAME(&MATDIM->n,&MATDIM->m,&MATDIM->k,&one,Bi,&MATDIM->inc2b,
    	    &MATDIM->inc1b,Ar,&MATDIM->inc2a,&MATDIM->inc1a,&one,Ci,
    	    &MATDIM->inc1c,&MATDIM->inc2c);
}
