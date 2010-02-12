/*
Copyright (C) 1997, 2003, 2008, 2009, 2010 Free Software Foundation, Inc.

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

/* print a utmp file in human-readable format. */

#include "config.h"

#include <stdio.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include "common.h"
#include "files.h"
#include "utmp_rd.h"
#ifdef HAVE_GETOPT_LONG_ONLY
#include <getopt.h>
#else
#include "getopt.h"
#endif

char *program_name;
int debugging_enabled = 0;	/* no -- we don't care about bad
				   records or the file-reading
				   algorithms. */

void give_usage(void)
{
  printf ("Usage: %s [-hrR] [-n <recs>] <files>\n\
          [--num <recs>] [--raw] [--reverse] [--help]\n",
          program_name);
  print_wtmp_file_location ();
}

int main(int argc, char *argv[])
{
  int c;
  int backwards_flag = 0;	/* nonzero means read from the end */
  long num_lines_to_print = -1;	/* -1 means all */
  int raw = 0;

  program_name = argv[0];

  while (1)
    {
      int option_index = 0;

      static struct option long_options[] =
      {
        { "reverse", no_argument, NULL, 1
        },
        { "help", no_argument, NULL, 2 },
        { "num", required_argument, NULL, 3 },
        { "raw", no_argument, NULL, 4 },
        { 0, 0, 0, 0 },
      };

      c = getopt_long (argc, argv, "rhn:R", long_options, &option_index);

      if (c == EOF)
        break;

      switch (c)
        {
        case 'r':
        case 1:
          backwards_flag = 1;
          break;
        case 'h':
        case 2:
          give_usage();
          exit(EXIT_FAILURE);
        case 'n':
        case 3:
          num_lines_to_print = strtol(optarg, (char **)NULL, 10);
          if (num_lines_to_print < 1)
            fatal ("number of lines to print must be positive and non-zero");
          break;
        case 'R':
        case 4:
          raw = 1;
          break;
        }
    }

  if (optind >= argc)
    {
      /* User didn't pass any filenames -- give them the usage
         message. */
      give_usage();
      exit(EXIT_FAILURE);
    }

  /* Init the file reader */

  utmp_init (backwards_flag);

  /* Add all files that we want to dump. */

  while (optind < argc)
    add_utmp_file (argv[optind++]);

  /* Dump the records. */

  {
    struct utmp *rec;
    while ((rec = utmp_get_entry ()) != NULL)
      {
        if (raw)
          (void)fwrite (rec, sizeof (struct utmp), 1, stdout);
        else
          print_utmp_record (rec, stdout);

        if (num_lines_to_print > -1)
          {
            num_lines_to_print--;
            if (num_lines_to_print == 0)    /* max lines printed */
              exit(EXIT_SUCCESS);
          }
      }
  }
  exit(EXIT_SUCCESS);
}

