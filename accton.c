/* Copyright (C) 1993, 1996, 1997, 2003, 2008 Free Software Foundation, Inc.

This file contains the code for GNU accton.

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
MA 02139, USA.  */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "common.h"
#include "files.h"
#ifdef HAVE_GETOPT_LONG
#include <getopt.h>
#else
#include "getopt_long.h"
#endif
#include "version.h"


/* globals */

char *program_name;		/* name of this executable */


/* protos */

void print_option_error PARAMS((void));
void print_version PARAMS((void));
void print_usage PARAMS((void));
void print_help PARAMS((void));


/* code */

/*
 * to print when there are errors related to options
 */
void
print_option_error (void)
{
  printf ("Try '%s --help' for more information.\n", program_name);
}


/*
 * to print version
 */
void
print_version (void)
{
  printf ("%s: GNU Accounting Utilities (release %s)\n",
          program_name, VERSION_STRING);
}


/*
 * to print usage (kind of very brief help)
 */
void
print_usage (void)
{
  printf ("Usage: %s [OPTION] on|off|ACCOUNTING_FILE\n", program_name);
}


/*
 * to print help
 */
void
print_help (void)
{
  print_usage ();

  printf ("\n\
          Turns process accounting on or off, or changes the file where this\n\
          info is saved.\n\n\
          OPTIONS:\n\
          -h, --help       Show help and exit\n\
          -V, --version    Show version and exit\n\n\
          ARGUMENTS:\n\
          on               Activate process accounting and use default file\n\
          off              Deactivate process accounting\n\
          ACCOUNTING_FILE  Activate (if not active) and save information in\n\
          this file\n\n\
          The system's default process accounting file is '%s'.\n\n\
          Report bugs to <bug-acct@gnu.org>\n",
          ACCT_FILE_LOC);
}


/*
 * does all the real work
 */
int
main (int argc, char *argv[])
{
  int c, status_acct, number_args;
  char *acct_file = NULL;


  program_name = argv[0];


  /* deal with options */
  while (1)
    {
      int option_index = 0;

      static struct option long_options[] =
        {
          { "version", no_argument, NULL, 1
          },
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
          print_version ();
          exit (EXIT_SUCCESS);
          break;
        case 'h':
        case 2:
          print_help ();
          exit (EXIT_SUCCESS);
          break;
        default:
          print_usage ();
          print_option_error ();
          exit (EXIT_FAILURE);
          break;
        }
    }


  /* get the number of real arguments -- not options nor prog. name */
  number_args = argc - optind;

  /* more than one arg -> error, too many args */
  if (1 < number_args)
    {
      printf ("%s: too many arguments\n", program_name);
      print_usage ();
      print_option_error ();
      exit (EXIT_FAILURE);
    }
  /* no arguments -> error, no arguments */
  else if (0 == number_args)
    {
      printf ("%s: no arguments\n", program_name);
      print_usage ();
      print_option_error ();
      exit (EXIT_FAILURE);
    }
  /* exactly one arg -> on|off|ACCOUNTING_FILE */
  else
    {
      acct_file = argv[optind];

      /* if the argument was "on" then pass the default file to activate.. */
      if (0 == strcasecmp (acct_file, "on"))
        {
          status_acct  = acct (ACCT_FILE_LOC);
          printf ("Turning on process accounting, file set to the default '%s'.\n",
                  ACCT_FILE_LOC);
        }
      /* ..else if was "off", pass NULL to deactivate.. */
      else if (0 == strcasecmp (acct_file, "off"))
        {
          status_acct  = acct (NULL);
          printf ("Turning off process accounting.\n");
        }
      /* ..else, the argument will be used as filename */
      else
        {
          status_acct  = acct (acct_file);
          printf ("Turning on process accounting, file set to '%s'.\n", acct_file);
        }


      /* did the system call return an error? */
      if (-1 == status_acct)
        {
          int err;
          extern int errno;

          err = errno;
          perror ("accton");
          exit (err);
        }
    }


  return EXIT_SUCCESS;
}

