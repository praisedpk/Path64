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


#ifndef __LIO_H__
#define __LIO_H__
/* $Header$ */
/*	3.0 SID #	1.2	*/

#define	LINTW	12
#define	LINE	80
#define	LLOGW	2
#define	LLOW	1.0
#define	LHIGH	10.0
#define	LFW	12
#define	LFD	8
#define	LEW	17
#define	LED	9
#define	LEE	2

/* PROTOTYPES */
extern int s_wsle(cilist *);
extern int s_wsle_mp(cilist *, unit **);
extern int s_wsli(icilist *);
extern int s_wsli_mp(icilist *, unit **);
extern int e_wsle(void);
extern int e_wsle_mp(unit **);
extern int e_xsle(void);
extern int e_xsle_mp(unit **);
extern int e_wsli(void);
extern int e_wsli_mp(void);
extern int t_putc(unit *, XINT, char, char *);
extern int lwrt_I(unit *, uinteger *, int, int, int);
extern int lwrt_L(unit *, uinteger *, int, int, int);
extern int lwrt_A(unit *, char *, ftnlen, int);
extern int lwrt_G(unit *, ufloat *, int, int, int, int, int, int, int);
extern int lwrt_C(unit *, ufloat *, ufloat *, int, int, int, int, int, int);
extern int l_write(unit *, XINT *, flex *, ftnlen, ftnint);
extern int do_Lio(ftnint *, ftnint *, flex *, ftnlen);
extern int do_Lio_1dim(ftnint *, flex *, flex *, ftnint *, ftnint *, ftnint *, ftnlen, ftnlen);
extern int __kai_do_lio(ftnint *, ftnint *, flex *, ftnlen);
extern int __kai_do_lio_mp(ftnint *, ftnint *, flex *, unit **, ftnlen);
extern int do_Lio_mp(ftnint *, ftnint *, flex *, unit **, ftnlen);
extern int do_lio (ftnint *type, ftnint *number, flex *ptr, ftnlen len);
extern int __kai_do_lio_1dim(ftnint *, flex *, flex *, ftnint *,
		ftnint *, ftnint *, ftnlen, ftnlen);
extern char    *icptr, *icend;

extern int c_le(cilist64 *, unit**);
extern void c_li(icilist64 *);

extern int s_wsle64 (cilist64 *a);
extern int s_wsle64_mp (cilist64 *a, unit **fu);
extern int s_wsli64 (icilist64 *a);
extern int s_wsli64_mp (icilist64 *a, unit **fu);
extern int do_Lio64 (ftnint *type, XINT *number, flex *ptr, ftnlen len);
extern int do_Lio64_1dim(ftnint *type, flex *ptr,
                flex *do_idx, XINT *lb,
                XINT *ub, XINT *step,
                ftnlen len, ftnlen idxlen);
extern int do_Lio64_mp (ftnint *type, XINT *number, flex *ptr,
		unit **fu, ftnlen len);
extern int __kai_do_lio64(ftnint *type, XINT *number, flex *ptr, ftnlen len);
extern int __kai_do_lio64_1dim(ftnint *type, flex *ptr,
                flex *do_idx, XINT *lb,
                XINT *ub, XINT *step,
                ftnlen len, ftnlen idxlen);
extern int do_Lio64_mp_1dim(ftnint *type, flex *ptr,
                flex *do_idx, XINT *lb,
                XINT *ub, XINT *step,
                unit **fu,
                ftnlen len, ftnlen idxlen);
extern int __kai_do_lio64_mp(ftnint *type, XINT *number, flex *ptr,
		unit **fu, ftnlen len);
extern int e_wsle64(void);
extern int e_wsle64_mp(unit **);
extern int e_xsle64(void);
extern int e_xsle64_mp(unit **);
extern int e_wsli64(void);
extern int e_wsli64_mp(void);

extern int   do_lioxa4(char *, XINT);
extern int   do_lioxa8(char *, XINT);
extern int   do_lioxh1(char *, XINT, XINT);
extern int   do_lioxi1(char *, XINT);
extern int   do_lioxi2(char *, XINT);
extern int   do_lioxi4(char *, XINT);
extern int   do_lioxi8(char *, XINT);
extern int   do_lioxl1(char *, XINT);
extern int   do_lioxl2(char *, XINT);
extern int   do_lioxl4(char *, XINT);
extern int   do_lioxl8(char *, XINT);
extern int   do_lioxr4(char *, XINT);
extern int   do_lioxr8(char *, XINT);
extern int   do_lioxr16(char *, XINT);
extern int   do_lioxc4(char *, XINT);
extern int   do_lioxc8(char *, XINT);
extern int   do_lioxc16(char *, XINT);
extern int   do_lioxa4v(ftnint);
extern int   do_lioxa8v(ftnll);
extern int   do_lioxh1v(char);
extern int   do_lioxi1v(char);
extern int   do_lioxi2v(short);
extern int   do_lioxi4v(ftnint);
extern int   do_lioxi8v(ftnll);
extern int   do_lioxl1v(char);
extern int   do_lioxl2v(short);
extern int   do_lioxl4v(ftnint);
extern int   do_lioxl8v(ftnll);
extern int   do_lioxr4v(float);
extern int   do_lioxr8v(double);
extern int   do_lioxr16v(long double);
extern int   do_lioxc4v(float, float);
extern int   do_lioxc8v(double, double);
extern int   do_lioxc16v(long double, long double);

extern int   do_lioxa4_mp(char *, XINT, unit **);
extern int   do_lioxa8_mp(char *, XINT, unit **);
extern int   do_lioxh1_mp(char *, XINT, XINT, unit **);
extern int   do_lioxi1_mp(char *, XINT, unit **);
extern int   do_lioxi2_mp(char *, XINT, unit **);
extern int   do_lioxi4_mp(char *, XINT, unit **);
extern int   do_lioxi8_mp(char *, XINT, unit **);
extern int   do_lioxl1_mp(char *, XINT, unit **);
extern int   do_lioxl2_mp(char *, XINT, unit **);
extern int   do_lioxl4_mp(char *, XINT, unit **);
extern int   do_lioxl8_mp(char *, XINT, unit **);
extern int   do_lioxr4_mp(char *, XINT, unit **);
extern int   do_lioxr8_mp(char *, XINT, unit **);
extern int   do_lioxr16_mp(char *, XINT, unit **);
extern int   do_lioxc4_mp(char *, XINT, unit **);
extern int   do_lioxc8_mp(char *, XINT, unit **);
extern int   do_lioxc16_mp(char *, XINT, unit **);
extern int   do_lioxa4v_mp(ftnint, unit **);
extern int   do_lioxa8v_mp(ftnll, unit **);
extern int   do_lioxh1v_mp(char, unit **);
extern int   do_lioxi1v_mp(char, unit **);
extern int   do_lioxi2v_mp(short, unit **);
extern int   do_lioxi4v_mp(ftnint, unit **);
extern int   do_lioxi8v_mp(ftnll, unit **);
extern int   do_lioxl1v_mp(char, unit **);
extern int   do_lioxl2v_mp(short, unit **);
extern int   do_lioxl4v_mp(ftnint, unit **);
extern int   do_lioxl8v_mp(ftnll, unit **);
extern int   do_lioxr4v_mp(float, unit **);
extern int   do_lioxr8v_mp(double, unit **);
extern int   do_lioxr16v_mp(long double, unit **);
extern int   do_lioxc4v_mp(float, float, unit **);
extern int   do_lioxc8v_mp(double, double, unit **);
extern int   do_lioxc16v_mp(long double, long double, unit **);

#endif /*__LIO_H__*/
