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

/* pacct_rd.h
 *
 * protos & etc. for routines that read from pacct/acct files
 *
 */

#ifndef PACCT_RD_H
#define PACCT_RD_H

#include "config.h"

#include "files.h"

/* This looks strange, but it's real ANSI C. */

extern struct acct pacct_rd_never_used;

#define COMM_LEN ((int) sizeof (pacct_rd_never_used.ac_comm))

void pacct_init (int backwards);
void add_pacct_file (char *name);
struct acct *pacct_get_entry (void);
void print_pacct_record (struct acct *rec, FILE *out);
void pacct_print_file_and_line (FILE *out);

#ifdef HAVE_COMP_T
double comp_t_2_double (comp_t);
#endif

#ifdef LINUX_MULTIFORMAT
double comp2_t_2_double (comp2_t val);
size_t convert_acct_record_write (struct acct *rec_in,
                                  struct acct *rec_out,
                                  int version, int byteswap);
#endif
#endif /* PACCT_RD_H */
