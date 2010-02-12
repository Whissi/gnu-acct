/*
Copyright (C) 1993, 1996, 1997, 2008, 2009, 2010 Free Software Foundation, Inc.

This file is part of the GNU Accounting Utilities

The GNU Accounting Utilities are free software; you can redistribute
them and/or modify them under the terms of the GNU General Public
License as published by the Free Software Foundation; either version
3, or (at your option) any later version.

The GNU Accounting Utilities are distributed in the hope that they will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with the GNU Accounting Utilities; see the file COPYING.  If
not, write to the Free Software Foundation, 675 Mass Ave, Cambridge,
MA 02139, USA.
*/

#ifndef _LINUX_ACCT_H
#define _LINUX_ACCT_H

#include <linux/types.h>

/*
 *  comp_t is a 16-bit "floating" point number with a 3-bit base 8
 *  exponent and a 13-bit fraction.
 *  comp2_t is 24-bit with 5-bit base 2 exponent and 20 bit fraction
 *  (leading 1 not stored).
 *  See linux/kernel/acct.c for the specific encoding systems used.
 */

typedef __u16	comp_t;
typedef __u32	comp2_t;

/*
 *   accounting file record
 *
 *   This structure contains all of the information written out to the
 *   process accounting file whenever a process exits.
 */

#define ACCT_COMM	16

struct acct
{
  char		ac_flag;		/* Flags */
  char		ac_version;		/* Always set to ACCT_VERSION */
  __u16		ac_tty;			/* Control Terminal */
  __u32		ac_exitcode;		/* Exitcode */
  __u32		ac_uid;			/* Real User ID */
  __u32		ac_gid;			/* Real Group ID */
  __u32		ac_pid;			/* Process ID */
  __u32		ac_ppid;		/* Parent Process ID */
  __u32		ac_btime;		/* Process Creation Time */
  float		ac_etime;		/* Elapsed Time */
  comp_t		ac_utime;		/* User Time */
  comp_t		ac_stime;		/* System Time */
  comp_t		ac_mem;			/* Average Memory Usage */
  comp_t		ac_io;			/* Chars Transferred */
  comp_t		ac_rw;			/* Blocks Read or Written */
  comp_t		ac_minflt;		/* Minor Pagefaults */
  comp_t		ac_majflt;		/* Major Pagefaults */
  comp_t		ac_swaps;		/* Number of Swaps */
  char		ac_comm[ACCT_COMM];	/* Command Name */
  __u16		ac_ahz;			/* AHZ */
  __u16		ac_uid16;		/* LSB of Real User ID */
  __u16		ac_gid16;		/* LSB of Real Group ID */
  __u16		ac_etime_16;		/* Elapsed Time (as comp_t) */
  __u16		ac_etime_lo;		/* Elapsed Time LSB */
  __u8		ac_etime_hi;		/* Elapsed Time MSB */
  __u8		ac_pad;			/* padding to 76 bytes */
};

struct acct_v0
{
  char		ac_flag;		/* Flags */
  __u16		ac_uid;			/* Real User ID */
  __u16		ac_gid;			/* Real Group ID */
  __u16		ac_tty;			/* Control Terminal */
  __u32		ac_btime;		/* Process Creation Time */
  comp_t		ac_utime;		/* User Time */
  comp_t		ac_stime;		/* System Time */
  comp_t		ac_etime;		/* Elapsed Time */
  comp_t		ac_mem;			/* Average Memory Usage */
  comp_t		ac_io;			/* Chars Transferred */
  comp_t		ac_rw;			/* Blocks Read or Written */
  comp_t		ac_minflt;		/* Minor Pagefaults */
  comp_t		ac_majflt;		/* Major Pagefaults */
  comp_t		ac_swaps;		/* Number of Swaps */
  __u32		ac_exitcode;		/* Exitcode */
  char		ac_comm[ACCT_COMM + 1];	/* Command Name */
  char		ac_pad[10];		/* Padding Bytes */
};

/*
 *  accounting flags
 */
/* bit set when the process ... */
#define AFORK		0x01	/* ... executed fork, but did not exec */
#define ASU		0x02	/* ... used super-user privileges */
#define ACOMPAT		0x04	/* ... used compatibility mode (VAX only not used) */
#define ACORE		0x08	/* ... dumped core */
#define AXSIG		0x10	/* ... was killed by a signal */

#define AHZ		100

#endif	/* _LINUX_ACCT_H */
