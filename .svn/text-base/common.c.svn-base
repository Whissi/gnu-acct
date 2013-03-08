/* common.c */

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

#include "config.h"

/* AIX requires this to be the first thing in the file.  */
#ifdef __GNUC__
# define alloca __builtin_alloca
#else
# ifdef HAVE_ALLOCA_H
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
#include "xalloc.h"

#include <stdio.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include "common.h"

extern char *program_name;

/* Exit the program with error message S */

void fatal(char *s)
{
  (void)fprintf(stderr, "%s: %s\n", program_name, s);
  exit(EXIT_FAILURE);
}

/* open file FILE_NAME & return a pointer to it */

FILE *file_open(char *file_name, int write_flag)
{
  FILE *fp = NULL;

  if (file_name == NULL)
    fatal("file_open: FILE_NAME is NULL");

  if (strcmp(file_name, "-") == 0)
    return write_flag ? stdout : stdin;

  if ((fp = fopen(file_name, write_flag ? "wb" : "rb")) == NULL)
    {
      char *tmp = (char *)alloca(sizeof (char) *(strlen(file_name) + 30));
      (void)sprintf(tmp, "couldn't open file '%s'", file_name);
      perror(tmp);
    }
  return fp;
}

/* Print the default wtmp file (for ac, last). */

void print_wtmp_file_location(void)
{
  (void)printf("\nThe system's default login accounting file is %s.\n", WTMP_FILE_LOC);
}


/* Print the default acct file (for accton, lastcomm). */

void print_acct_file_location(void)
{
  (void)printf("\nThe system's default process accounting file is %s.\n", ACCT_FILE_LOC);
}

/* Print the default acct file (for sa). */

void print_acct_file_locations(void)
{
  (void)printf("The system's default process accounting files are:\n\n");
  (void)printf("  raw process accounting data: %s\n", ACCT_FILE_LOC);
  (void)printf("      summary by command name: %s\n", SAVACCT_FILE_LOC);
  (void)printf("          summary by username: %s\n\n", USRACCT_FILE_LOC);
}

