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


#pragma ident "@(#) libfi/mpp_shift/eoshift_p_wd1_2.c	92.1	07/13/99 10:44:06"


#include <stdlib.h>
#include <liberrno.h>
#include <cray/dopevec.h>
#include "f90_macros.h"

#define RANK		2

/*
 *	Compiler generated call:
 *		CALL _EOSHIFT_JP2(RESULT, SOURCE, SHIFT, BOUNDARY, DIM)
 *		CALL _EOSHIFT_LP2(RESULT, SOURCE, SHIFT, BOUNDARY, DIM)
 *		CALL _EOSHIFT_SP2(RESULT, SOURCE, SHIFT, BOUNDARY, DIM)
 *
 *	Purpose: Perform an end-off shift of the SOURCE array along the DIM
 *		 dimension.  The magnitude of the shift is found in SHIFT.
 *		 The value to be shifted in is contained in BOUNDARY
 *
 *	Arguments:
 *		RESULT   - Dope vector for result temporary array
 *		SOURCE   - Dope vector for user source array
 *		SHIFT    - Dope vector for shift count
 *		BOUNDARY - Dope vector for boundary (optional)
 *		DIM	 - Dimension along which to shift (optional)
 *
 *	Description:
 *		This is the MPP version of EOSHIFT.  This particular file
 *		contains the intermediate routines.  These routines parse
 *		and update the dope vectors, allocate either shared or
 *		private space for the result temporary, and possibly update
 *		the shared data descriptor (sdd) for the result temporary.
 *		Once this set-up work is complete, a Fortran subroutine is
 *		called which uses features from the Fortran Programming
 *		Model to perform the actual shift.
 *
 *		Include file eoshift_p.h contains the rank independent
 *		source code for this routine.
 */

#pragma duplicate _EOSHIFT_JP2 as _EOSHIFT_LP2
#pragma duplicate _EOSHIFT_JP2 as _EOSHIFT_SP2

void
_EOSHIFT_JP2 (
		DopeVectorType  *result,
		DopeVectorType  *source,
		DopeVectorType  *shift,
		DopeVectorType  *boundary,
		int		*dim)

{
#include "eoshift_p.h"

/*
 *      Call the Fortran work routine
 */

	special = 0;
	if (shflag == _btol(1) && bndflag == _btol(1)) {
	    if (shftval >= -1 && shftval <= 1) {
		if (_blkct (source_sdd_ptr, 1, 0) == 1 &&
		    _blkct (source_sdd_ptr, 2, 0) == 1) {
		    special = 1;
		}
	    }
	}

	if (special) {
	    EOSHIFT_SPEC_WD1_P2@ ( result_sdd_ptr, source_sdd_ptr,
		&dim_val, src_extents, &shftval, &bndval);
	} else {
	    EOSHIFT_WD1_P2@ ( result_sdd_ptr, source_sdd_ptr, shift_sdd_ptr,
		bound_sdd_ptr, &dim_val, src_extents, shft_extents,
		bnd_extents, &shflag, &shftval, &bndflag, &bndval);
	}
}
