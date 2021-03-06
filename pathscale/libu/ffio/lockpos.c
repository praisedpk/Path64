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


#pragma ident "@(#) libu/ffio/lockpos.c	92.1	06/29/99 13:16:47"


#include <ffio.h>
#include "locklyr.h"


_ffseek_t
_lock_pos(struct fdinfo *fio, int cmd, void *arg, int len, struct ffsw *stat)
{
	struct fdinfo *llfio;
	_ffseek_t ret;

	llfio = fio->fioptr;

	LYR_LOCK(fio);
	ret = XRCALL(llfio, posrtn) llfio, cmd, arg, len, stat);
	LYR_UNLOCK(fio);
	return(ret);
}

int
_lock_bksp(struct fdinfo *fio, struct ffsw *stat)
{
	struct fdinfo *llfio;
	int ret;

	llfio = fio->fioptr;
	LYR_LOCK(fio);
	ret = XRCALL(llfio, backrtn) llfio, stat);
	LYR_UNLOCK(fio);
	return(ret);
}
