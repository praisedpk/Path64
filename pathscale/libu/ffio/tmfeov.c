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


#pragma ident "@(#) libu/ffio/tmfeov.c	92.1	06/29/99 13:16:47"
 
#include "tmfio.h"
#include <stdlib.h>

/*
 * setsp: enable/disable eov processing
 *
 * Returns:
 *	0 on success 
 *	-1 on error with errno containing error code
 */
int
_tmf_setsp(struct tmfio *xf_info, int iflag)
{
	tmfeov_t	ctl;

	if (xf_info->tmf_speov) {
		/* require that we end special processing first */
		errno	= FDC_ERR_EOVALOW;
		return(ERR);
	}
	if (xf_info->tmf_eovhit) {
		errno	= FDC_ERR_REQ;
		return(ERR);
	}
	if (xf_info->tmf_tpos) {
		if (_tmf_tpwait (xf_info) < 0) {
			return(ERR);
		}
	}

	bzero(&ctl, sizeof(tmfreqhdr_t));

	if (iflag == 0) {
		/* Disable eov processing */
		xf_info->tmf_eovon	= 0;
		if (xf_info->eovbase != NULL) {
			free(xf_info->eovbase);
			xf_info->eovbase	= NULL;
			xf_info->eovbuf		= NULL;
		}
		xf_info->eovbytes	= 0;
		xf_info->spblocks	= 0;
		xf_info->tmf_tpmk	= 0;
		xf_info->tmf_partblk	= 0;
		ctl.select		= EOV_OFF;
	}
	else {	/* Enable eov processing */
		xf_info->tmf_eovon	= 1;
		ctl.select		= EOV_ON;
	}

	/* Issue the ioctl to enable/disable special processing */

	ctl.rh.request	= TR_EOV;
	ctl.rh.length	= sizeof(tmfeov_t) - sizeof(tmfreqhdr_t);

	if (ioctl(xf_info->tmf_fd, TMFC_DMNREQ, &ctl) < 0) {
		return(ERR);
	}
	if (ctl.rh.reply != 0) {
		errno	= ctl.rh.reply;
		return(ERR);
	}

	return(0);
}
/*
 * startsp - start special processing.
 *
 */
int
_tmf_startsp(struct tmfio *xf_info)
{
	tmfreqhdr_t	ctl;

	if (xf_info->tmf_eovon == 0) {
		errno	= FDC_ERR_EOVDIS;
		return(ERR);

	}
	if (xf_info->tmf_tpos) {
		if (_tmf_tpwait (xf_info) < 0) {
			return(ERR);
		}
	}
	xf_info->tmf_eovhit	= 0;
	xf_info->tmf_speov	= 1;

	return(0);
}
/*
 * endsp - end special processing.
 *
 */
int
_tmf_endsp(struct fdinfo *fio, struct ffsw *stat)
{
	struct tmfio	*xf_info;
	ssize_t		ret;
	tmfreqhdr_t	ctl;

	xf_info	= (struct tmfio *)fio->lyr_info;

	if (xf_info->tmf_tpos) {
		if (_tmf_tpwait (xf_info) < 0) {
			return(ERR);
		}
	}
	if (xf_info->tmf_eovon == 0) {
		ERETURN(stat, FDC_ERR_EOVDIS, 0);

	}
	if (xf_info->tmf_speov == 0) {
		ERETURN(stat, FDC_ERR_REQ, 0);
	}

	/* If anything is in buffer memory, write it out. */

	if (xf_info->tmf_tpmk) {
		_tmf_weof(fio, stat);
	}
	else {
		if (xf_info->eovbytes != 0) {
			if (xf_info->eovbase == xf_info->eovbuf) {
				ret	= write(xf_info->tmf_fd,
					  xf_info->eovbase, xf_info->eovbytes);
				if (ret != xf_info->eovbytes) {
					if (ret == ERR) {
						ERETURN(stat, errno, 0);
					}
					else{
						ERETURN(stat, FDC_ERR_WRTERR, 0);
					}
				}
			}
		}
	}

	xf_info->tmf_eovhit	= 0;
	xf_info->tmf_tpmk	= 0;
	xf_info->eovbytes	= 0;
	xf_info->eovbuf		= xf_info->eovbase;
	xf_info->spblocks	= 0;
	xf_info->tmf_speov	= 0;
	xf_info->tmf_partblk	= 0;

	return(0);
}
/*
 *	Returns info about whether we are at eov.
 *	bufbytes is the number of bytes from the first partial block.
 *	bufblk is the number of whole blocks.
 */
int
_tmf_checkev(struct tmfio *xf_info, struct ffc_chkev_s *arg)
{
	arg->bufblk	= 0;
	arg->bufbytes	= 0;

	if (xf_info->tmf_tpos) {
		if (_tmf_tpwait (xf_info) < 0) {
			return(ERR);
		}
	}
	if (xf_info->tmf_eovhit) {
		arg->stat	= 1;
		if (xf_info->tmf_partblk) {
			arg->bufbytes	= xf_info->eovbytes;
		}
		else {
			if (xf_info->eovbytes != 0)
				/* Without write-behind, the maximum */
				/* number of buffered blocks we will have */
				/* is 1 */
				arg->bufblk	= 1;
		}
	}
	else {
		arg->stat	= 0;
	}

	return(0);
}	
