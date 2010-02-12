/* Copyright (C) 1993, 1996, 1997, 2003, 2005 Free Software Foundation, Inc.

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
#include <stdio.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <ctype.h>

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <pwd.h>

#include "common.h"
#include "files.h"
#include "uid_hash.h"
#include "dev_hash.h"
#include "pacct_rd.h"
#include "hashtab.h"
#ifdef HAVE_GETOPT_LONG_ONLY
#include <getopt.h>
#else
#include "getopt.h"
#endif
#include "version.h"


/* globals */

int debugging_enabled = 0;	/* Nonzero means print internal
				   information relating to entries
				   read, data structures, etc.  */
int strict_match_flag = 0;	/* Nonzero if each record has to match
				   all items on the command line */
int print_controls = 0;		/* don't print control characters */
int ahz = AHZ;			/* for viewing logs from another system */

char *program_name;		/* name of the program, for usage & errs */

int show_paging = 0;		/* If they want paging stats, print 'em */

static unsigned int hzval;

/* Here are various lists for the user to specify entries that they
   want to see.  */

struct hashtab *user_list = NULL;    /* lookup table for usernames */
struct hashtab *tty_list = NULL;     /* lookup table for ttys */
struct hashtab *command_list = NULL; /* lookup table for commands */
struct hashtab *all_list = NULL;     /* lookup table for users/ttys/cmds */

char *dev_gnu_name (long);
int get_entry (struct acct **);
int desired_entry (char *, char *, char *);

/* code */

int main(int argc, char *argv[])
{
  int c;
  int other_pacct_file_specified = 0; /* nonzero if the user used the
  					 `-f' or `--file' flag */
  int backwards = 1; /* default to reading backwards */

  hzval = sysconf(_SC_CLK_TCK);

  program_name = argv[0];

  while (1)
    {
      int option_index = 0;

      static struct option long_options[] =
      {
        { "debug", no_argument, NULL, 1
        },
        { "version", no_argument, NULL, 2 },
        { "help", no_argument, NULL, 3 },
        { "file", required_argument, NULL, 4 },
        { "strict-match", no_argument, NULL, 5 },
        { "print-controls", no_argument, NULL, 6 },
        { "user", required_argument, NULL, 7 },
        { "tty", required_argument, NULL, 8 },
        { "command", required_argument, NULL, 9 },
        { "ahz", required_argument, NULL, 10 },
#ifdef HAVE_PAGING
        { "show-paging", no_argument, NULL, 11 },
#endif
        { "forwards", no_argument, NULL, 12 },
        { 0, 0, 0, 0 }
      };

      c = getopt_long (argc, argv, "f:hV"
#ifdef HAVE_PAGING
                       "p"
#endif
                       , long_options, &option_index);

      if (c == EOF)
        break;

      switch (c)
        {
        case 1:
          debugging_enabled = 1;
          break;
        case 'V':
        case 2:
          (void)printf("%s: GNU Accounting Utilities (release %s)\n",
                       program_name, VERSION_STRING);
          exit(EXIT_SUCCESS);
        case 'f':
        case 4:
          if (strcmp(optarg, "-") == 0)
            {
              if (backwards && other_pacct_file_specified)
                {
                  (void)printf("%s: -f - must be specified first, or --forwards must precede the first -f\n", program_name);
                  exit(EXIT_FAILURE);
                }
              backwards = 0;
              optarg = "/proc/self/fd/0";
            }
          if (other_pacct_file_specified == 0)
            pacct_init(backwards);
          add_pacct_file (optarg);
          other_pacct_file_specified = 1;
          break;
        case 5:
          strict_match_flag = 1;
          break;
        case 6:
          print_controls = 1;
          break;
        case 7:
          if (user_list == NULL)
            user_list = hashtab_init (0);
          hashtab_create (user_list, optarg, 0);
          break;
        case 8:
          if (tty_list == NULL)
            tty_list = hashtab_init (0);
          hashtab_create (tty_list, optarg, 0);
          break;
        case 9:
          if (command_list == NULL)
            command_list = hashtab_init (0);
          hashtab_create (command_list, optarg, 0);
          break;
        case 10:
          ahz = strtol(optarg, (char **)NULL, 10);

          if (ahz < 1)
            {
              (void)printf("%s: AHZ must be one or greater\n", program_name);
              exit(EXIT_FAILURE);
            }

          break;
#ifdef HAVE_PAGING
        case 'p':
        case 11:
          show_paging = 1;
          break;
#endif
        case 12:
          if (other_pacct_file_specified)
            {
              (void)printf("%s: --forwards must come before -f\n", program_name);
              exit(EXIT_FAILURE);
            }
          backwards = 0;
          break;
        case 'h':
        case 3:
          /* This should fall through to default! */
        default:
          give_usage();
          exit(EXIT_FAILURE);
        }
    }

  if (other_pacct_file_specified == 0)
    pacct_init(backwards);

  /* if we get here, we expect everything else to be a username,
     terminal name, or command name */

  /* Init the hash table for usernames.  Don't init it if we don't
     have any extra arguments so we can quickly check if we have
     anything in the list. */

  if (optind < argc)
    {
      all_list = hashtab_init (0);

      while (optind < argc)
        hashtab_create (all_list, argv[optind++], 0);
    }

  /* If the "strict-match" flag was selected, make sure the user
     hasn't specified multiple usernames, ttys, or command names. */

  if (strict_match_flag)
    {
      if ((tty_list && (tty_list->items_hashed > 1))
          || (user_list && (user_list->items_hashed > 1))
          || (command_list && (command_list->items_hashed > 1)))
        {
          (void)printf("%s: ERROR: when using the `--strict-match' flag,\n"
                       "   it is nonsensical to specify more than one item\n"
                       "   to match in a single category (using the `--tty',\n"
                       "   `--user', or `--command' flags).\n",
                       program_name);
          exit(EXIT_FAILURE);
        }
    }

  if (debugging_enabled)
    {
      if (all_list)
        {
          (void)fputs("all_list\n--------\n", stddebug);
          hashtab_dump_keys (all_list, stddebug);
        }

      if (tty_list)
        {
          (void)fputs("tty_list\n--------\n", stddebug);
          hashtab_dump_keys (tty_list, stddebug);
        }

      if (user_list)
        {
          (void)fputs("user_list\n---------\n", stddebug);
          hashtab_dump_keys (user_list, stddebug);
        }

      if (command_list)
        {
          (void)fputs("command_list\n------------\n", stddebug);
          hashtab_dump_keys (command_list, stddebug);
        }
    }

  if (! other_pacct_file_specified)
    add_pacct_file (ACCT_FILE_LOC);

  parse_entries ();

  exit(EXIT_SUCCESS);			/* guarantee the proper return value */
}

/* guess what this does... */

void give_usage(void)
{
  (void)printf("Usage: %s [-h"
#ifdef HAVE_PAGING
               "p"
#endif
               "V] [-f file] [command] ... [user] ... [terminal] ...\n"
               "       [--forwards] [--file <file>] [--strict-match] [--print-controls]\n"
               "       [--user <name>] [--tty <name>] [--command <name>] [--debug]\n"
               "       "
#ifdef HAVE_PAGING
               "[--show-paging] "
#endif
               "[--version] [--help]\n", program_name);
  print_acct_file_location ();
}

/* parse the entries in an acct file */

void parse_entries(void)
{
  struct acct *rec;             /* the current record */
  time_t btime;

  /* loop while there are entries to be had */

  while ((rec = pacct_get_entry ()) != NULL)
    {
      char *this_uid = uid_name (rec->ac_uid);
      char *this_dev = dev_gnu_name ((long) rec->ac_tty);

      if (debugging_enabled)
        {
          fprintf (stddebug, "---------------------------------------------------------------------------\nCURRENT REC: ");
          print_pacct_record (rec, stddebug);
        }

      if (desired_entry (this_uid, this_dev, rec->ac_comm))
        {
          double ut = ACUTIME_2_DOUBLE (rec->ac_utime);
          double st = ACSTIME_2_DOUBLE (rec->ac_stime);

#ifdef HAVE_PAGING
          double minf = ACMINFLT_2_DOUBLE (rec->ac_minflt);
          double majf = ACMAJFLT_2_DOUBLE (rec->ac_majflt);
          double swap = ACSWAPS_2_DOUBLE (rec->ac_swaps);
#endif

          /* Make sure there are no unprintable characters in the
                   command name string. */

          if (! print_controls)
            {
              int i;
              for (i = 0; i < COMM_LEN; i++)
                {
                  if (rec->ac_comm[i] == '\0')
                    break;
                  if (! isprint (rec->ac_comm[i]))
                    rec->ac_comm[i] = '?';
                }
            }

          (void)printf("%-*.*s ", COMM_LEN, COMM_LEN, rec->ac_comm);

#ifdef ASU
          if (rec->ac_flag & ASU)
            (void)putchar('S');
          else
#endif
            (void)putchar(' ');

#ifdef AFORK
          if (rec->ac_flag & AFORK)
            (void)putchar('F');
          else
#endif
            (void)putchar(' ');

#ifdef ACOMPAT
          if (rec->ac_flag & ACOMPAT)
            (void)putchar('C');
          else
#endif
            (void)putchar(' ');

#ifdef ACORE
          if (rec->ac_flag & ACORE)
            (void)putchar('D');
          else
#endif
            (void)putchar(' ');

#ifdef AXSIG
          if (rec->ac_flag & AXSIG)
            (void)putchar('X');
          else
#endif
            (void)putchar(' ');

          btime = (time_t) rec->ac_btime;
          if ( show_paging == 0 )
            (void)printf(" %-8.8s %-8.8s %6.2f secs %-16.16s\n",
                         this_uid, this_dev,
#ifdef LINUX_MULTIFORMAT
                         ((ut + st) / (double) rec->ac_ahz),
#else
                         ((ut + st) / (double) hzval),
#endif
                         ctime (&btime));
          else
#ifdef HAVE_PAGING
            (void)printf(" %6.0fmin %6.0fmaj %4.0fswp %6.2f secs %-16.16s\n",
                         minf, majf, swap,
#ifdef LINUX_MULTIFORMAT
                         ((ut + st) / (double) rec->ac_ahz),
#else
                         ((ut + st) / (double) hzval),
#endif
                         ctime (&btime));
#else
            (void)printf("  --- No paging statistics! --- \n" );
#endif
        }
    }
}


/* Decide whether or not to print an entry. */

int desired_entry(char *uid, char *dev, char *comm)
{
  /* UID and DEV are already null-terminated, but COMM isn't. */

  if ((all_list == NULL)
      && (user_list == NULL)
      && (tty_list == NULL)
      && (command_list == NULL))
    return 1;

  if (strict_match_flag)
    {
      /* We must match each element in all_list to the current record.
      This makes the intuitive usage

      lastcomm --strict-match noel cat ttyp2

      work correctly. */

      if (all_list)
        {
          struct hashtab_order ho;
          struct hashtab_elem *he;

          for (he = hashtab_first (all_list, &ho);
               he != NULL;
               he = hashtab_next (&ho))
            {
              /* While both the uid and dev will be null-terminated,
              comm may not be (especially if it contains COMM_LEN
              characters).  We therefore need to figure out the
              length of comm (which is a maxiumum of COMM_LEN) and
              use strncmp below instead of strcmp. */

              int commlen = strlen (comm);
              if (commlen > COMM_LEN)
                commlen = COMM_LEN;

              /* We must match at least one of these items. */

              if (strcmp (he->key, uid)
                  || strcmp (he->key, dev)
                  || strncmp (he->key, comm, commlen))
                return 0;
            }
        }

      /* Now handle matches for individual categories (with `--tty' &
      c.).  Since we've guaranteed above that each of the following
      lists will only contain a single item, we can use
      hashtab_find to see if that item matches the given string. */

      if (tty_list && hashtab_find (tty_list, dev, (unsigned int)0) == NULL)
        return 0;

      if (user_list && hashtab_find (user_list, uid, (unsigned int)0) == NULL)
        return 0;

      if (command_list && hashtab_find (command_list, comm, (unsigned int)COMM_LEN) == NULL)
        return 0;

      /* found all of them */
      return 1;
    }
  else
    {
      /* Return 1 for any match. */

      if (all_list
          && (hashtab_find (all_list, uid, (unsigned int)0)
              || hashtab_find (all_list, dev, (unsigned int)0)
              || hashtab_find (all_list, comm, (unsigned int)COMM_LEN)))
        return 1;

      if (tty_list && hashtab_find (tty_list, dev, (unsigned int)0))
        return 1;

      if (user_list && hashtab_find (user_list, uid, (unsigned int)0))
        return 1;

      if (command_list && hashtab_find (command_list, comm, (unsigned int)COMM_LEN))
        return 1;

      /* didn't match anything */
      return 0;
    }
}

