/* lastcomm.c */

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
#include <stdio.h>
#include <ctype.h>

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include <pwd.h>

#include "common.h"
#include "uid_hash.h"
#include "dev_hash.h"
#include "pacct_rd.h"
#include "hashtab.h"
#include "getopt.h"
#include "version.h"


/* globals */

int debugging_enabled = 0;	/* Nonzero means print internal
				   information relating to entries
				   read, data structures, etc.  */
int strict_match_flag = 0;	/* Nonzero if each record has to match
				   all items on the command line */
int print_controls = 0;		/* don't print control characters */

char *program_name;		/* name of the program, for usage & errs */


/* Here are various lists for the user to specify entries that they
   want to see.  */

struct hashtab *user_list = NULL;    /* lookup table for usernames */
struct hashtab *tty_list = NULL;     /* lookup table for ttys */
struct hashtab *command_list = NULL; /* lookup table for commands */
struct hashtab *all_list = NULL;     /* lookup table for users/ttys/cmds */


/* protos */

void main PARAMS((int, char *[]));
void give_usage PARAMS((void));
void parse_entries PARAMS((void));
char *devname PARAMS((long));
int get_entry PARAMS((struct acct **));
int desired_entry PARAMS((char *, char *, char *));


/* code */

void
main (int argc, char *argv[])
{
  int c;
  int other_pacct_file_specified = 0; /* nonzero if the user used the
					 `-f' or `--file' flag */

  program_name = argv[0];
  
  /* Tell the pacct reader that we want to do things backwards! */

  pacct_init (1);

  while (1)
    {
      int option_index = 0;
      
      static struct option long_options[] = {
	{ "debug", no_argument, NULL, 1 },
	{ "version", no_argument, NULL, 2 },
	{ "help", no_argument, NULL, 3 },
	{ "file", required_argument, NULL, 4 },
	{ "strict-match", no_argument, NULL, 5 },
	{ "print-controls", no_argument, NULL, 6 },
	{ "user", required_argument, NULL, 7 },
	{ "tty", required_argument, NULL, 8 },
	{ "command", required_argument, NULL, 9 },
	{ 0, 0, 0, 0 }
      };
      
      c = getopt_long (argc, argv, "f:hV", long_options, &option_index);
      
      if (c == EOF)
	break;

      switch (c)
	{
	case 1:
	  debugging_enabled = 1;
	  break;
	case 'V':
	case 2:
	  printf ("%s: GNU Accounting Utilities (release %s)\n",
		  program_name, VERSION_STRING);
	  exit (0);
	  break;
	case 'f':
	case 4:
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
	case 'h':
	case 3:
	  /* This should fall through to default! */
	default:
	  give_usage ();
	  exit (1);
	  break;
	}
    }

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
  
  if (debugging_enabled)
    {
      if (all_list)
	{
	  fputs ("all_list\n--------\n", stddebug);
	  hashtab_dump_keys (all_list, stddebug);
	}
	
      if (tty_list)
	{
	  fputs ("tty_list\n--------\n", stddebug);
	  hashtab_dump_keys (tty_list, stddebug);
	}

      if (user_list)
	{
	  fputs ("user_list\n---------\n", stddebug);
	  hashtab_dump_keys (user_list, stddebug);
	}

      if (command_list)
	{
	  fputs ("command_list\n------------\n", stddebug);
	  hashtab_dump_keys (command_list, stddebug);
	}
    }
  
  if (! other_pacct_file_specified)
    add_pacct_file (ACCT_FILE_LOC);

  parse_entries ();

  exit (0);			/* guarantee the proper return value */
}


/* guess what this does... */
void
give_usage (void)
{
  char *usage = "\
Usage: %s [-hV] [-f file] [command] ... [user] ... [terminal] ...\n\
       [--file <file>] [--strict-match] [--print-controls]\n\
       [--user <name>] [--tty <name>] [--command <name>] [--debug]\n\
       [--version] [--help]\n\
";
  
  printf (usage, program_name);
  print_acct_file_location ();
}

/* parse the entries in an acct file */
void
parse_entries (void)
{
  struct acct *rec;             /* the current record */

  /* loop while there are entries to be had */

  while ((rec = pacct_get_entry ()) != NULL)
    {
      char *this_uid = uid_name (rec->ac_uid);
      char *this_dev = devname ((long) rec->ac_tty);

      if (debugging_enabled)
	{
	  fprintf (stddebug, "---------------------------------------------------------------------------\nCURRENT REC: ");
	  print_pacct_record (rec, stddebug);
	}

      if (desired_entry (this_uid, this_dev, rec->ac_comm))
	{
#ifdef HAVE_COMP_T
	  double ut = comp_t_2_double (rec->ac_utime);
	  double st = comp_t_2_double (rec->ac_stime);
#else
	  double ut = (double) rec->ac_utime;
	  double st = (double) rec->ac_stime;
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

	  printf ("%-*.*s ", COMM_LEN, COMM_LEN, rec->ac_comm);

#ifdef ASU
	  if (rec->ac_flag & ASU)
	    putchar ('S');
	  else
#endif
	    putchar (' ');
	  
#ifdef AFORK
	  if (rec->ac_flag & AFORK)
	    putchar ('F');
	  else
#endif
	    putchar (' ');
	  
#ifdef ACOMPAT
	  if (rec->ac_flag & ACOMPAT)
	    putchar ('C');
	  else
#endif
	    putchar (' ');
	  
#ifdef ACORE
	  if (rec->ac_flag & ACORE)
	    putchar ('D');
	  else
#endif
	    putchar (' ');
	  
#ifdef AXSIG
	  if (rec->ac_flag & AXSIG)
	    putchar ('X');
	  else
#endif
	    putchar (' ');
	  
	  printf (" %-8.8s %-8.8s %6.2f secs %-16.16s\n",
		  this_uid, this_dev,
		  ((ut + st) / (double) AHZ),
		  ctime (&rec->ac_btime));
	}
    }
}



/* Decide whether or not to print an entry. */

int
desired_entry (char *uid, char *dev, char *comm)
{
  /* UID and DEV are already null-terminated, but COMM isn't. */

  if ((all_list == NULL)
      && (user_list == NULL)
      && (tty_list == NULL)
      && (command_list == NULL))
    return 1;

  if (strict_match_flag)
    {
      /* All categories must have one match. */

      if (all_list
	  && hashtab_find (all_list, uid, 0) == NULL
	  && hashtab_find (all_list, dev, 0) == NULL
	  && hashtab_find (all_list, comm, COMM_LEN) == NULL)
	return 0;
      
      if (tty_list && hashtab_find (tty_list, dev, 0) == NULL)
	return 0;

      if (user_list && hashtab_find (user_list, uid, 0) == NULL)
	return 0;

      if (command_list && hashtab_find (command_list, comm, COMM_LEN) == NULL)
	return 0;
      
      /* found all of them */
      return 1;
    }
  else
    {
      /* Return 1 for any match. */
      
      if (all_list
	  && (hashtab_find (all_list, uid, 0)
	      || hashtab_find (all_list, dev, 0)
	      || hashtab_find (all_list, comm, COMM_LEN)))
	  return 1;

      if (tty_list && hashtab_find (tty_list, dev, 0))
	return 1;

      if (user_list && hashtab_find (user_list, uid, 0))
	return 1;

      if (command_list && hashtab_find (command_list, comm, COMM_LEN))
	return 1;

      /* didn't match anything */
      return 0;
    }
}

