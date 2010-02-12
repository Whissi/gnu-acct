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

/* utmp_rd.h
 *
 * protos & etc. for routines that read from utmp files
 *
 */

#ifndef UTMP_RD_H
#define UTMP_RD_H

#include "config.h"		/* for HAVE_STRUCT_UTMP_UT_HOST */

#include "files.h"

#ifdef HAVE_STRUCT_UTMP_UT_ADDR
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>		/* for INET_NTOA */
#endif

/* This looks strange, but it's real ANSI C. */

extern struct utmp utmp_rd_never_used;

#define NAME_LEN ((int) sizeof (utmp_rd_never_used.ut_name))
#define TTY_LEN ((int) sizeof (utmp_rd_never_used.ut_line))

#ifdef HAVE_STRUCT_UTMP_UT_HOST
#define HOST_LEN ((int) sizeof (utmp_rd_never_used.ut_host))
#else
#define HOST_LEN 0
#endif

#ifdef HAVE_STRUCT_UTMP_UT_ID
#define ID_LEN ((int) sizeof (utmp_rd_never_used.ut_id))
#endif

#if 0
#ifdef __linux__		/* Debian bug#1651 reported by */
#define NAME_LEN 9		/* Austin Donnelly <and1000@cam.ac.uk> */
#else
#define NAME_LEN 8		/* length for usernames */
#endif

#define TTY_LEN 8		/* length for ttys */
#endif

#define OLD_TIME_CHAR '|'
#define NEW_TIME_CHAR '{'
#define BOOT_TIME_CHAR '~'

#ifdef HAVE_STRUCT_UTMP_UT_TYPE

#define RECORD_TYPE ut_type

#ifndef UTMAXTYPE
#define UTMAXTYPE 8		/* FIXME: is there a better way to do
this? */
#endif

#else

#define RECORD_TYPE ut_line[0]

#define NEW_TIME NEW_TIME_CHAR
#define OLD_TIME OLD_TIME_CHAR
#define BOOT_TIME BOOT_TIME_CHAR

#endif

void utmp_init (int backwards);
void add_utmp_file (char *name);
struct utmp *utmp_get_entry (void);
void print_utmp_record (struct utmp *rec, FILE *out);
void utmp_print_file_and_line (FILE *out);

#ifdef HAVE_STRUCT_UTMP_UT_TYPE
int fix_ut_type_field (struct utmp *rec);
#endif

int bad_utmp_record (struct utmp *rec);
#endif /* UTMP_RD_H */
