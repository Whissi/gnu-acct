/* common.c */

/* Copyright (C) 1993, 1996, 1997 Free Software Foundation, Inc.

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

#include "config.h"

/* AIX requires this to be the first thing in the file.  */
#ifdef __GNUC__
# define alloca __builtin_alloca
#else
# if HAVE_ALLOCA_H
#  include <alloca.h>
# else
#  ifdef _AIX
 #pragma alloca
#  else
#   ifndef alloca /* predefined by HP cc +Olibcalls */
char *alloca ();
#   endif
#  endif
# endif
#endif

#include "files.h"

#include <stdio.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include "common.h"

extern char *program_name;

/* Same as `malloc' but report error if no memory available.  */

char *
xmalloc (unsigned size)
{
  register char *value = (char *) malloc (size);
  if (value == 0)
    fatal ("virtual memory exhausted");
  return value;
}

/* Exit the program with error message S */

void
fatal (char *s)
{
  fprintf (stderr, "%s: %s\n", program_name, s);
  exit (1);
}

/* open file FILE_NAME & return a pointer to it */

FILE *
file_open (char *file_name, int write_flag)
{
  FILE *fp;

  if (file_name == NULL)
    fatal ("file_open: FILE_NAME is NULL");

  if (strcmp (file_name, "-") == 0)
    return write_flag ? stdout : stdin;

  if ((fp = fopen (file_name, write_flag ? "wb" : "rb")) == NULL)
    {
      char *tmp = (char *) alloca (sizeof (char) * (strlen (file_name) + 30));
      sprintf (tmp, "couldn't open file '%s'", file_name);
      perror (tmp);
    }
  return fp;
}

#ifndef HAVE_RENAME

/*
 * rename  -- BSD rename(2) clone
 *
 * This code is in the public domain.
 *
 * Bugs: Doesn't fail with EACCES if from can't be removed.
 *	 Might leave "to" nonexistent if the system crashes at an unfortunate
 *	 moment.
 *	 It might return unexpected errno values.
 *
 * Author: Christoph Badura <bad@flatlin.ka.sub.org>
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

int
rename (char *from, char *to)
{
	struct stat sbfrom, sbto;

	/*
	 * just try the link(2), if it fails with EEXIST, we'll try plan B.
	 */
	if (link(from, to) == 0) {
		(void) unlink(from);	/* XXX what if we can't remove it? */
		return 0;
	}
	if (errno != EEXIST)
		return -1;

	/*
	 * Careful now!  The following is full of race conditions.
	 * However, we only want to check for a cross device link and
	 * should be resonably safe.
	 */
	if (stat(from, &sbfrom) < 0)
		return -1;
	if (stat(to, &sbto) < 0)
		return -1;
	if (sbfrom.st_dev != sbto.st_dev) {
		errno = EXDEV;
		return -1;
	}

	/*
	 * OK, this isn't going to be across file systems,
	 * therefore its safe to try to remove "to."
	 */
	if (unlink(to) < 0)
		return -1;
	/*
	 * Bug: if the system crashes, "to" is nonexistent.  That's
	 * not compatible, but there's nothing we can do about it.
	 */
	if (link(from, to) == 0) {
		(void) unlink(from);	/* XXX what if we can't remove it? */
		return 0;
	}
	return -1;
}

#endif


/* Print the default wtmp file (for ac, last). */

void
print_wtmp_file_location (void)
{
  printf ("\nThe system's default login accounting file is %s.\n",
	  WTMP_FILE_LOC);
}


/* Print the default acct file (for accton, lastcomm). */

void
print_acct_file_location (void)
{
  printf ("\nThe system's default process accounting file is %s.\n",
	  ACCT_FILE_LOC);
}


/* Print the default acct file (for sa). */

void
print_acct_file_locations (void)
{
  printf ("The system's default process accounting files are:\n\n");
  printf ("  raw process accounting data: %s\n", ACCT_FILE_LOC);
  printf ("      summary by command name: %s\n", SAVACCT_FILE_LOC);
  printf ("          summary by username: %s\n\n", USRACCT_FILE_LOC);
}

