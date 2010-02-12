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

/* dev_hash.h
 *
 * protos for functions that lookup device names
 *
 */


#ifndef DEV_HASH_H
#define DEV_HASH_H

char *dev_gnu_name (long dev_num);

struct pts_params
{
  char *utsname;		/* os name */
  int base;			/* base major number */
  int max;			/* max # of devices */
  int mod;			/* number of minors per major */
};

#endif /* DEV_HASH_H */
