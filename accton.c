/* accton.c */

/* Copyright (C) 1993, 1996, 1997 Free Software Foundation, Inc.

This file contains the code for GNU accton.

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

#include <stdio.h>
#include <sys/types.h>
#include <errno.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "common.h"
#include "getopt.h"
#include "version.h"


/* globals */

char *program_name;		/* name of this executable */


/* protos */

void main PARAMS((int argc, char *argv[]));
void usage PARAMS((void));


/* code */

void
usage (void)
{
  printf ("\
Usage: %s [-hV] [file]\n\
       [--help] [--version]\n",
	  program_name);
  print_acct_file_location ();
}

void
main (int argc, char *argv[])
{
  int c;
  char *acct_file = NULL;

  program_name = argv[0];

  while (1)
    {
      int option_index = 0;
      
      static struct option long_options[] = {
	{ "version", no_argument, NULL, 1 },
	{ "help", no_argument, NULL, 2 },
	{ 0, 0, 0, 0 }
      };

      c = getopt_long (argc, argv, "hV", long_options, &option_index);
      
      if (c == EOF)
	break;

      switch (c)
	{
	case 'V':
	case 1:
	  printf ("%s: GNU Accounting Utilities (release %s)\n",
		  program_name, VERSION_STRING);
	  exit (0);
	  break;
	case 'h':
	case 2:
	  /* This should fall through to default! */
	default:
	  usage ();
	  exit (1);
	  break;
	}
    }

  if ((argc - optind) > 1)
    {
      usage ();
      exit (1);
    }
  else if ((argc - optind) == 1)
    {
      acct_file = argv[optind];
    }
  
  if (acct (acct_file) == -1)
    {
      int err;
      extern int errno;

      err = errno;
      perror ("accton");
      exit (err);
    }

  exit(0);
}

