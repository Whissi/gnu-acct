/* last.c */

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

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <ctype.h>

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <sys/types.h>

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

#ifdef AMIX
#include </usr/ucbinclude/sys/signal.h>
#else
#include <signal.h>
#endif

#include "common.h"
#include "utmp_rd.h"
#include "getopt.h"
#include "hashtab.h"
#include "version.h"


/* globals */

char *program_name;		/* name of the program, for usage */

int debugging_enabled = 0;	/* Nonzero means print internal
				   information relating to entries
				   read, data structures, etc. */

long time_warp_leniency = 60;	/* Number of seconds that an entry can
				   be "before" the previous one -- to
				   deal with simultaneous init(8)
				   writes, or to overlook problems
				   with sun's wtmp files. */

long time_warp_suspicious = 31536000; /* Anything smaller than this
					 jump will be considered
					 normal.  A year is pretty
					 safe to assume... */

int print_file_problems = 0;	/* when there's a problem with the
				   file's format, scream! */

int print_year = 0;		/* Print the year when displaying
				   dates. */

int print_seconds = 0;		/* Print the seconds when displaying
				   dates. */

time_t last_time = 0;		/* time of last event (error checking)
				   needs to be a global so that the
				   signal handler can get at its value
				   when the program is interrupted */


char *last_event = NULL;	/* Set to NULL initially (means that a
				   user is still logged on) but
				   changed to "crash" or "down" as
				   appropriate. */

time_t last_event_time;		/* last time the machine rebooted/shutdown */

long num_lines_to_print = -1;	/* the -n option -- starts out at -1,
				   which is a signal to the program
				   not to limit the number of lines
				   printed. */

int truncate_ftp_entries = 1;	/* Nonzero means shorten "ftpxxx"
				   entries to "ftp" -- most progs do
				   this by default, but we should have
				   a flag for it */

#ifdef HAVE_UT_TYPE
int print_which_recs = 0;	/* If 0, only print the usual.  If 1,
				   print run level information,
				   shutdowns, and time changes.  If 2
				   or greater, print the whole, dirty
				   business. */
#endif

#ifdef HAVE_UT_ADDR
int print_addresses = 0;	/* should we print the ut_addr field? */
#endif


/* A table of names/ttys that the user wants to know about.  */

struct hashtab *names = NULL;


/* A table of logins/logouts. */

struct hashtab *login_table = NULL;

struct login_data {
  time_t time;
  short fake_entry;
};


/* prototypes */

void log_everyone_in PARAMS((time_t));
void give_usage PARAMS((void));
void parse_entries PARAMS((void));
void update_system_time PARAMS((time_t));
void log_out PARAMS((struct utmp *entry, short fake_flag));
void log_in PARAMS((struct utmp *));
void print_record PARAMS((struct utmp *, time_t logout_time,
			  char *, char *, char *));
void display_date PARAMS((time_t now));

#if defined (SVR4) && !defined (_POSIX_SOURCE)
RETSIGTYPE handler PARAMS((int, int, struct sigcontext *));
#else
RETSIGTYPE handler PARAMS((int));
#endif




/* code */

void
main (int argc, char *argv[])
{
  int c;
  int other_wtmp_file_specified = 0; /* nonzero if the user used the
					`-f' or `--file' flags */

  program_name = argv[0];
  
  /* Tell the utmp reader that we want to do things backwards! */

  utmp_init (1);

  /* Read options */

  /* Aaargh!  We've got to parse the arguments manually before we give
     them to getopt, since it can't handle `last -10' and things like
     it... */

  for (c = 1; c < argc; c++)
    {
      if (argv[c][0] == '-' && isdigit (argv[c][1]))
	{
	  /* This looks like an old-style option -- read the number! */
	  int d;

	  num_lines_to_print = atol (argv[c] + 1);
	  if (num_lines_to_print < 1)
	    fatal ("number of lines to print must be positive and non-zero");
	  
	  /* renumber the argv options */
	  for (d = c; d; d--)
	    argv[d] = argv[d - 1];
	  
	  argc--;
	  argv++;
	  c--;
	}
    }

  while (1)
    {
      int option_index = 0;
      
      static struct option long_options[] = {
	{ "complain", no_argument, NULL, 1 },
	{ "lines", required_argument, NULL, 3 },
	{ "debug", no_argument, NULL, 4 },
	{ "tw-leniency", required_argument, NULL, 5 },
	{ "version", no_argument, NULL, 6 },
	{ "help", no_argument, NULL, 7 },
	{ "file", required_argument, NULL, 8 },
	{ "no-truncate-ftp-entries", no_argument, NULL, 9 },
#ifdef HAVE_UT_TYPE
	{ "all-records", no_argument, NULL, 10 },
#endif
#ifdef HAVE_UT_ADDR
	{ "ip-address", no_argument, NULL, 11 },
#endif
	{ "print-year", no_argument, NULL, 12 },
#ifdef HAVE_UT_TYPE
	{ "more-records", no_argument, NULL, 13 },
#endif
	{ "tw-suspicious", required_argument, NULL, 14 },
	{ "print-seconds", no_argument, NULL, 15 },
	{ 0, 0, 0, 0 }
      };

      c = getopt_long (argc, argv,
#ifdef HAVE_UT_TYPE
		       "a"
#endif
#ifdef HAVE_UT_ADDR
		        "i"
#endif
                         "f:n:s"
#ifdef HAVE_UT_TYPE
		              "x"
#endif
		               "yV",
		       long_options, &option_index);
      
      if (c == EOF)
	break;

      switch (c)
	{
	case 1:
	  print_file_problems = 1;
	  break;
	case 'n':
	case 3:
	  num_lines_to_print = atol (optarg);
	  if (num_lines_to_print < 1)
	    fatal ("number of lines to print must be positive and non-zero");
	  break;
	case 4:
	  debugging_enabled = 1;
	  print_file_problems = 1;
	  break;
	case 5:
	  time_warp_leniency = atol (optarg);
	  if (time_warp_leniency < 0)
	    fatal ("time warp leniency value has to be non-negative");
	  break;
	case 'V':
	case 6:
	  printf ("%s: GNU Accounting Utilities (release %s)\n",
		  program_name, VERSION_STRING);
	  exit (0);
	  break;
	case 'f':
	case 8:
	  add_utmp_file (optarg);
	  other_wtmp_file_specified = 1;
	  break;
	case 9:
	  truncate_ftp_entries = 0;
	  break;
#ifdef HAVE_UT_TYPE
	case 'a':
	case 10:
	  print_which_recs = 2;
	  break;
#endif
#ifdef HAVE_UT_ADDR
	case 'i':
	case 11:
	  print_addresses = 1;
	  break;
#endif
	case 'y':
	case 12:
	  print_year = 1;
	  break;
#ifdef HAVE_UT_TYPE
	case 'x':
	case 13:
	  print_which_recs = 1;
	  break;
#endif
	case 14:
	  time_warp_suspicious = atol (optarg);
	  if (time_warp_suspicious < 0)
	    fatal ("time warp suspicious value has to be non-negative");
	  if (time_warp_suspicious <= time_warp_leniency)
	    fatal ("time warp suspicious value has to greater than the time warp leniency value");
	  break;
	case 's':
	case 15:
	  print_seconds = 1;
	  break;
	case 'h':
	case 7:
	  /* This should fall through to default! */
	default:
	  give_usage ();
	  exit (1);
	  break;
	}
    }

  /* Init the hash table for usernames.  Don't init it if we don't
     have any extra arguments so we can quickly check if we have
     anything in the list. */

  if (optind < argc)
    {
      names = hashtab_init (0);

      while (optind < argc)
	{
	  char *string;
      
	  /* add the username/tty as-is */

	  hashtab_create (names, argv[optind], 0);
      
	  /* add the name as "tty<name>" for u*x compatibilty */

	  string = (char *) xmalloc ((strlen (argv[optind]) + 4)
				     * sizeof (char));
	  sprintf (string, "tty%s", argv[optind]);
	  hashtab_create (names, string, 0);
	  free (string);

	  optind++;

#ifdef HAVE_UT_TYPE
	  if (!print_which_recs)
	    print_which_recs = 1;
#endif
	}
  
      if (debugging_enabled)
	hashtab_dump_keys (names, stddebug);
    }
  
  if (! other_wtmp_file_specified)
    add_utmp_file (WTMP_FILE_LOC);
  
  /* Create hash table for logins. */

  login_table = hashtab_init (0);
  
  /* Go! */

  parse_entries ();
  
  fputs ("\nwtmp begins ", stdout);
  display_date (last_time);
  fputc ('\n', stdout);

  exit (0);			/* a GUARANTEED return value to the env */
}



/* guess what this does... */
void
give_usage ()
{
  char *usage = "\
Usage: %s [-"
#ifdef HAVE_UT_TYPE
           "a"
#endif
#ifdef HAVE_UT_ADDR
            "i"
#endif
             "hs"
#ifdef HAVE_UT_TYPE
              "x"
#endif
               "yV] [-<lines>] [-n <lines>] [-f <file>] [people] [ttys] ...\n\
       [--lines <num>] [--file <file>] [--complain] [--debug]\n\
       [--version] [--tw-leniency <value>] [--tw-suspicious <value>]\n\
       [--no-truncate-ftp-entries] [--print-year] [--print-seconds]\n\
       "
#ifdef HAVE_UT_TYPE
      "[--more-records] [--all-records] "
#endif
#ifdef HAVE_UT_ADDR
                                       "[--ip-address] "
#endif
                                                      "[--help]\n"
;
  
  printf (usage, program_name);
  print_wtmp_file_location ();
}



/* Since the routines in ac & last are so similar, just include them
   from another file. */

#define BACKWARDS
#include "al_share.cpp"


/* since the sys clock has changed, each entry's login time has to be
 * adjusted...  */
void
update_system_time (time_t the_time)
{
  struct hashtab_order ho;
  struct hashtab_elem *he;
  
  for (he = hashtab_first (login_table, &ho);
       he != NULL;
       he = hashtab_next (&ho))
    {
      struct login_data *l = hashtab_get_value (he);
      l->time += the_time;
    }
}


/* Log all entries in at THE_TIME. */

void
log_everyone_in (time_t the_time)
{
  struct hashtab_order ho;
  struct hashtab_elem *he;
  
  for (he = hashtab_first (login_table, &ho);
       he != NULL;
       he = hashtab_next (&ho))
    {
      struct login_data *l = hashtab_get_value (he);

      /* If there was a real log out record present in the file
         without a corresponding login, we should print it. */

      if (debugging_enabled && (! l->fake_entry))
	{
	  struct utmp this;
	  char *ttyname = hashtab_get_key (he);

	  memset (&this, 0, sizeof (this));
	  strncpy (this.ut_line, ttyname, TTY_LEN);
	  this.ut_time = the_time;
	  
	  print_record (&this, l->time, NULL, NULL, "unpaired");
	}

      hashtab_delete (he);
    }
}


/* Put a terminal into the hash table. */

void
log_out (struct utmp *entry, short fake_flag)
{
  struct hashtab_elem *he;
  
  if (entry->ut_line[0] == '\0')
    {
      if (print_file_problems)
	{
	  utmp_print_file_and_line (stddebug);
	  fprintf (stddebug,
		   ": problem: trying to hash rec with ut_line == NULL\n");
	}
      return;
    }

  /* Match the most recent login on the terminal. */

  he = hashtab_find (login_table, entry->ut_line, TTY_LEN);

  if (he != NULL)
    {
      struct login_data *l = hashtab_get_value (he);

      if ((! l->fake_entry) && print_file_problems)
	{
	  char *ttyname = hashtab_get_key (he);
	  utmp_print_file_and_line (stddebug);
	  fprintf (stddebug, ": problem: duplicate record for line `%.*s'\n",
		   TTY_LEN, ttyname);
	}
	  
      /* Write over the old values. */

      l->time = entry->ut_time;
      l->fake_entry = fake_flag;

      return;
    }
  else
    {
      /* If we get here, we didn't find the entry in the list, so add
         a new one. */

      struct login_data l;

      l.time = entry->ut_time;
      l.fake_entry = fake_flag;

      he = hashtab_create (login_table, entry->ut_line, TTY_LEN);
      hashtab_set_value (he, &l, sizeof (l));
    }
}


/* Remove an entry from the hash table. */

void
log_in (struct utmp *entry)
{
  struct hashtab_elem *he;

  if (entry->ut_line[0] == '\0')
    {
      if (print_file_problems)
	{
	  utmp_print_file_and_line (stddebug);
	  fprintf (stddebug,
		   ": problem: trying to hash rec with ut_line == NULL\n");
	}
      return;
    }

  he = hashtab_find (login_table, entry->ut_line, TTY_LEN);

  if (he != NULL)
    {
      struct login_data *l = hashtab_get_value (he);

      if (l->fake_entry && print_file_problems)
	{
	  /* If this happens, we assume that the logout time for ENTRY
	     is the the same as the next login time on the tty. */

	  utmp_print_file_and_line (stddebug);
	  fprintf (stddebug, ": problem: missing logout record for `%.*s'\n",
		   TTY_LEN, entry->ut_line);
	}
	  
      print_record (entry, l->time, NULL, NULL, NULL);

      hashtab_delete (he);
    }
  else
    {
      /* We didn't find any corresponding logout, so either the person
	 it still logged in or there's a problem with the wtmp file
	 (logout without login). */

      print_record (entry, 0, NULL, NULL, NULL);
    }
  
  /* Make another logout entry at the current time (a fake one) so we
     can detect wtmp file errors (logins without logouts). */
	  
  log_out (entry, TRUE);
}


/* Print the record to the screen.  If LOGOUT_TIME is zero, print out
   the difference in time between LOGIN and the last recorded event
   (crash, down, etc.).  If other strings should be printed for the
   UT_NAME, UT_LINE, or end-of-line fields, they can be passed in as
   the last three arguments. */

void
print_record (struct utmp *login, time_t logout_time,
	      char *replacement_name, char *replacement_tty,
	      char *special_message)
{
  char *print_name;
  char *print_tty;
  char sanitized_tty[TTY_LEN];

  /* Do we want to print this entry?  Check our list of user-desired
     items. */

  if (! ((names == NULL)
	 || hashtab_find (names, login->ut_name, NAME_LEN)
	 || hashtab_find (names, login->ut_line, TTY_LEN)))
    return;			/* don't want to print this record */


  /* Choose replacement names, if provided.  Don't do this if we're on
     a BSD system. */

#ifdef HAVE_UT_TYPE
  if (replacement_tty)
    print_tty = replacement_tty;
  else
#endif
       if (truncate_ftp_entries && (strncmp (login->ut_line, "ftp", 3) == 0))
    print_tty = "ftp";		/* strip number from "ftpxxx" entry */
  else
    print_tty = login->ut_line;

  print_name = (replacement_name ? replacement_name : login->ut_name);

  
  /* Sanitize the tty name (get rid of control characters) */

  {
    int i;

    for (i = 0; (print_tty[i] != '\0') && (i < TTY_LEN); i++)
      sanitized_tty[i] = (isprint (print_tty[i]) && isascii (print_tty[i])
			  ? print_tty[i]
			  : '?');
    if (i < TTY_LEN)
      sanitized_tty[i] = '\0';
  }

  /* Print the name and tty. */

  printf ("%-*.*s %-*.*s ",
	  NAME_LEN, NAME_LEN, print_name,
	  TTY_LEN, TTY_LEN, sanitized_tty);


#ifdef HAVE_UT_ADDR
  if (print_addresses)
    {
      struct in_addr a;
      a.s_addr = login->ut_addr;
      printf ("%-15.15s ", (login->ut_addr) ? inet_ntoa (a) : "");
    }
#endif

#ifdef HAVE_UT_HOST
#ifdef HAVE_UT_ADDR
  /* If we have both HOST and ADDR, only print one, or else we go
         beyond 80 characters in width. */
  if (!print_addresses)
#endif
    printf ("%-*.*s ", HOST_LEN, HOST_LEN, login->ut_host);
#endif

  display_date (login->ut_time);

  if (special_message)
    printf ("  %s\n", special_message);
  else
    {
      char temp_str[15];
      int days, hours, minutes, seconds;
      int diff, use_last_event = 0;

      if (logout_time == 0)
	{
	  logout_time = last_event_time;
	  use_last_event = 1;
	}
	  
      diff = (int) logout_time - login->ut_time;
      days = (int) (diff / 86400);
      diff -= 86400 * days;
      hours = (int) (diff / 3600);
      diff -= 3600 * hours;
      minutes = (int) (diff / 60);
      diff -= 60 * minutes;
      seconds = diff;
	  
      if (days)
	{
	  if (print_seconds)
	    sprintf (temp_str, "(%d+%02d:%02d:%02d)",
		     days, hours, minutes, seconds);
	  else
	    sprintf (temp_str, "(%d+%02d:%02d)", days, hours, minutes);
	}
      else
	{
	  if (print_seconds)
	    sprintf (temp_str, " (%02d:%02d:%02d)",
		     hours, minutes, seconds);
	  else
	    sprintf (temp_str, " (%02d:%02d)", hours, minutes);
	}

      if (use_last_event)
	{
	  if (last_event == NULL)

	    fputs ("  still logged in\n", stdout);
	  else
	    printf ("- %-5.5s %s\n", last_event, temp_str);
	}
      else
	{
	  struct tm *tmptr;
	  time_t temp_time = logout_time;
	  tmptr = localtime (&temp_time);
	  if (print_seconds)
	    printf ("- %02d:%02d:%02d %s\n",
		    tmptr->tm_hour, tmptr->tm_min, tmptr->tm_sec, temp_str);
	  else
	    printf ("- %02d:%02d %s\n",
		    tmptr->tm_hour, tmptr->tm_min, temp_str);
	}
    }

  if (num_lines_to_print > -1)
    {
      num_lines_to_print--;
      if (num_lines_to_print == 0)    /* max lines printed */
	exit (0);
    }
}



/* Our signal handler */

RETSIGTYPE
#if defined (SVR4) && !defined (_POSIX_SOURCE)
handler (int sig, int code, struct sigcontext *scp)
#else
handler (int sig)
#endif
{
  fputs ("\ninterrupted at ", stdout);
  display_date (last_time);
  putchar ('\n');
  exit (0);
}



/* Display the date, paying attention to the PRINT_YEAR flag. */

void
display_date (time_t now)
{
  char *ct = ctime (&now);
  
  printf ("%-10.10s ", ct);

  if (print_year)
    printf ("%-4.4s ", ct + 20);

  if (print_seconds)
    printf ("%-8.8s ", ct + 11);
  else
    printf ("%-5.5s ", ct + 11);
}
