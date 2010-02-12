/* common.h */

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

#ifndef COMMON_H
#define COMMON_H

#include "config.h"		/* can't hurt!  work that CPP! */
#include "xalloc.h"

/* defines */

#define stddebug stdout

#define TRUE 1
#define FALSE 0

/* for those machines that don't have this stuff in stdio.h */
#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

void fatal (char *);
FILE *file_open (char *, int);

void give_usage (void);
void parse_entries (void);

void print_wtmp_file_location (void);
void print_acct_file_location (void);
void print_acct_file_locations (void);

#ifdef ACUTIME_COMPT
# define ACUTIME_2_DOUBLE(x) (comp_t_2_double(x))
#else
# define ACUTIME_2_DOUBLE(x) ((double)(x))
#endif

#ifdef ACSTIME_COMPT
# define ACSTIME_2_DOUBLE(x) (comp_t_2_double(x))
#else
# define ACSTIME_2_DOUBLE(x) ((double)(x))
#endif

#ifdef ACETIME_COMPT
# define ACETIME_2_DOUBLE(x) (comp_t_2_double(x))
#else
# define ACETIME_2_DOUBLE(x) ((double)(x))
#endif

#ifdef ACIO_COMPT
# define ACIO_2_DOUBLE(x) (comp_t_2_double(x))
#else
# define ACIO_2_DOUBLE(x) ((double)(x))
#endif

#ifdef ACMEM_COMPT
# define ACMEM_2_DOUBLE(x) (comp_t_2_double(x))
#else
# define ACMEM_2_DOUBLE(x) ((double)(x))
#endif

#ifdef ACMINFLT_COMPT
# define ACMINFLT_2_DOUBLE(x) (comp_t_2_double(x))
#else
# define ACMINFLT_2_DOUBLE(x) ((double)(x))
#endif

#ifdef ACMAJFLT_COMPT
# define ACMAJFLT_2_DOUBLE(x) (comp_t_2_double(x))
#else
# define ACMAJFLT_2_DOUBLE(x) ((double)(x))
#endif

#ifdef ACSWAPS_COMPT
# define ACSWAPS_2_DOUBLE(x) (comp_t_2_double(x))
#else
# define ACSWAPS_2_DOUBLE(x) ((double)(x))
#endif

#endif /* COMMON_H */
