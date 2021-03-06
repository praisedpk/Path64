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


#pragma ident "@(#) libfi/matrix/matmul_i2d.c	92.1	07/09/99 15:18:08"
#include "matmul.h"

/*
 * Name of this entry point
 */
#define NAME _MATMUL_I2D
/*
 * Name of routine called do computation (if any)
 */
#define SUBNAME _I2DGEMMX
/*
 * Type of constants alpha and beta
 */
#define RESULTTYPE   _f_real16

void
NAME(DopeVectorType *RESULT, DopeVectorType *MATRIX_A,
     DopeVectorType *MATRIX_B)
{
    void    SUBNAME();

    MatrixDimenType matdimdata, *MATDIM;

#if !defined(_CRAYMPP) && !defined(_ABSOFT)
    const RESULTTYPE   one =  (RESULTTYPE) 1.0;
    const RESULTTYPE   zero = (RESULTTYPE) 0.0;
#else
    struct {
        _f_int4 ireal1;         /* first 32 bits of real part */
        _f_int4 irest2[3];      /* rest of 128-bit double precision */
        } one, zero;

    memset(&zero, 0, sizeof(zero));

    one = zero;
    one.ireal1 =  017777600000;
#endif

        MATDIM = (MatrixDimenType *) &matdimdata;

    /*
     * Parse dope vectors, and perform error checking.
     */

    _premult(RESULT, MATRIX_A, MATRIX_B, MATDIM);

    /*
     * Perform the matrix multiplication.
     */

    SUBNAME(&MATDIM->m, &MATDIM->n, &MATDIM->k, &one, MATDIM->A,
         &MATDIM->inc1a, &MATDIM->inc2a, MATDIM->B, &MATDIM->inc1b,
         &MATDIM->inc2b, &zero, MATDIM->C, &MATDIM->inc1c, &MATDIM->inc2c);
    return;
}
