/* common.h */

/* Copyright (C) 1993 Free Software Foundation, Inc.

This file is part of the GNU Accounting Utilities

The GNU Accounting Utilities are free software; you can redistribute
them and/or modify them under the terms of the GNU General Public
License as published by the Free Software Foundation; either version
2, or (at your option) any later version.

The GNU Accounting Utilities are distributed in the hope that they will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with the GNU Accounting Utilities; see the file COPYING.  If
not, write to the Free Software Foundation, 675 Mass Ave, Cambridge,
MA 02139, USA.  */

#ifndef COMMON_H
#define COMMON_H

#include "config.h"		/* can't hurt!  work that compiler! */

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

#ifndef AHZ
#define AHZ 64
#endif

/* prototypes */

char *xmalloc (unsigned);
void fatal (char *);
FILE *file_open (char *, int);

#ifndef HAVE_RENAME
int rename (char *from, char *to);
#endif

#endif /* COMMON_H */
