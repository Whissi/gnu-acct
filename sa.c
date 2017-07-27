/* sa.c -- everyone's favorite hairball */

/*
Copyright (C) 1993, 1996, 1997, 2008, 2009, 2010, 2013, 2014
 Free Software Foundation, Inc.

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
#include "pacct_rd.h"
#include "utmp_rd.h"
#ifdef HAVE_GETOPT_LONG_ONLY
#include <getopt.h>
#else
#include "getopt.h"
#endif
#include "uid_hash.h"
#include "hashtab.h"
#include "version.h"


/* defines, structures, and the like */

/* This is the struct we read/write from/to usracct and savacct. */

struct stats
{
  unsigned long num_calls;	/* how many times this command called */
#ifdef HAVE_ACUTIME
  double user_time;		/* user time */
#endif
#ifdef HAVE_ACSTIME
  double sys_time;		/* system time */
#endif
#ifdef HAVE_ACETIME
  double elapsed_time;		/* real time */
#endif
#ifdef HAVE_ACMEM
  double mem_usage;		/* total memory usage */
#endif
#ifdef HAVE_ACIO
  double disk_io;		/* how many disk I/O operations */
#endif
#ifdef HAVE_PAGING
  double minor_faults;
  double major_faults;
  double swap_count;
#endif
};

struct usracct
{
  char name[NAME_LEN];		/* should we be using the val from utmp_rd? */
  struct stats s;
};

#ifndef HAVE_GETPAGESIZE
#define getpagesize() 4096
#endif


/* globals */

char *program_name;		/* name of the program, for usage & errs */

int debugging_enabled = 0;	/* print internal information relating
				   to entries read, data structures,
				   etc. */

/* Hash table of entries broken down by user */

struct hashtab *user_table;

struct user_data
{
  struct stats s;
};

/* Hash table of entries broken down by command */

struct hashtab *command_table;

struct command_key
{
  char comm[COMM_LEN];		/* command name */
  short fork_flag;		/* nonzero if fork but no exec */
};

struct command_data
{
  struct stats s;
  short junked;
};

struct savacct
{
  struct command_key c;
  struct stats s;
};


struct stats stats_totals;	/* total of all commands */

#define PAGES_TO_KB(x) ((double) (x) / system_page_size)

double system_page_size;	/* size in kbytes of each page in
				   memory */

char default_acct_file[] = ACCT_FILE_LOC;
char default_savacct_file[] = SAVACCT_FILE_LOC;
char default_usracct_file[] = USRACCT_FILE_LOC;

char *acct_file_name = default_acct_file; /* the file name */
char *savacct_file_name = default_savacct_file;
char *usracct_file_name = default_usracct_file;


/* command line flags */

int print_seconds = 0;		/* print seconds instead of minutes */
int dont_read_summary_files = 0;
int print_users = 0;
int percentages = 0;		/* include percentages in printout */
int user_summary_flag = 0;	/* are we printing a user summary? */
int group_summary_flag = 0;	/* are we printing a group summary? */
int ahz = AHZ;			/* for viewing logs from another system */


#if defined(HAVE_ACUTIME) && defined(HAVE_ACSTIME)
int separate_times = 0;		/* print separate user and sys times */
#endif

#if defined(HAVE_ACSTIME) && defined(HAVE_ACUTIME) && defined(HAVE_ACMEM)
int print_ksec = 0;
#endif

#if defined(HAVE_ACUTIME) && defined(HAVE_ACSTIME) && defined(HAVE_ACETIME)
int print_ratio = 0;
#endif

#ifdef HAVE_ACIO
int total_io = 0;		/* print total disk io operations */
#endif

int show_paging = 0;            /* #ifdef makes the print function untidy ;-) */
int paging_as_avg = 0;

/* which field to sort by */

enum
{
  sort_num_calls
#if defined(HAVE_ACSTIME) && defined(HAVE_ACUTIME)
  , sort_sys_plus_user
#endif
#if defined(HAVE_ACSTIME) && defined(HAVE_ACUTIME)
  , sort_sys_plus_user_div_calls
#endif
#ifdef HAVE_ACIO
  , sort_avio
  , sort_tio
#endif
#ifdef HAVE_ACMEM
  , sort_cpu_mem_average
#endif
#if defined(HAVE_ACSTIME) && defined(HAVE_ACUTIME) && defined(HAVE_ACMEM)
  , sort_cpu_storage
#endif
#ifdef HAVE_ACETIME
  , sort_real_time
#endif
} sort_type =
#if defined(HAVE_ACSTIME) && defined(HAVE_ACUTIME)
  sort_sys_plus_user;
#else
  sort_num_calls;		/* fallback default */
#endif

int reverse_sort = 0;		/* nonzero if the sort is reversed */
int merge_files;		/* nonzero if we want to write new
				   savacct and usracct files */
int separate_forks = 0;	        /* nonzero if we're supposed to pay
				   attention to the fork-no-exec flag
				   when grouping command names */
int junk_threshold = 0;		/* if a command has this number of
				   calls or less, add it to "***junk" */
int print_all_records = 0;	/* don't lump calls into the
				   "***other" category */
int always_yes = 0;		/* nonzero means always answer yes to
				   a query */
static unsigned int hzval;

void write_savacct_file (char *);
void write_usracct_file (char *);
void parse_savacct_entries (char *);
void parse_usracct_entries (char *);
void parse_acct_entries (void);
void init_flags_and_data (void);
unsigned long hash_name (char *);
void update_command_list (char *, struct stats *, short fork_flag);
void update_user_list (char *, struct stats *);
int compare_sum_entry (struct hashtab_elem **, struct hashtab_elem **);
int compare_user_entry (struct hashtab_elem **,
                        struct hashtab_elem **);
int compare_stats_entry (struct stats *, struct stats *);
void print_command_list (void);
void print_user_list (void);
int non_printable (char *, int);
int ask_if_junkable (char *, int);

/* code */

int main(int argc, char *argv[])
{
  int c;

  hzval = sysconf(_SC_CLK_TCK);
  program_name = argv[0];

  /* Cache the page size of the machine for the PAGES_TO_KB macro */
  system_page_size = (double) getpagesize () / 1024.0;

  /* init the summary record */
  memset (&stats_totals, 0, sizeof (stats_totals));

  /* Init the pacct file reader, telling it to read forwards in the
     files. */

  user_table = hashtab_init (0);
  command_table = hashtab_init (sizeof (struct command_key));

  pacct_init (0);

  while (1)
    {
      int option_index = 0;

      static struct option long_options[] =
      {
        { "debug", no_argument, NULL, 1
        },
        { "version", no_argument, NULL, 2 },
        { "help", no_argument, NULL, 3 },
        { "other-acct-file", required_argument, NULL, 4 },
        { "print-seconds", no_argument, NULL, 5 },
        { "dont-read-summary-files", no_argument, NULL, 6 },
        { "list-all-names", no_argument, NULL, 7 },
        { "other-savacct-file", 1, NULL, 9 },
        { "print-users", no_argument, NULL, 12 },
        { "percentages", no_argument, NULL, 14 },
        { "not-interactive", no_argument, NULL, 19 },
        { "user-summary", no_argument, NULL, 20 },
        { "group-summary", no_argument, NULL, 30 },
        { "reverse-sort", no_argument, NULL, 21 },
        { "merge", no_argument, NULL, 22 },
        { "threshold", required_argument, NULL, 23 },
        { "separate-forks", no_argument, NULL, 24 },
        { "other-usracct-file", required_argument, NULL, 25 },

#if defined(HAVE_ACUTIME) && defined(HAVE_ACSTIME)
        { "separate-times", no_argument, NULL, 8
        },
#endif

#if defined(HAVE_ACSTIME) && defined(HAVE_ACUTIME) && defined(HAVE_ACMEM)
        { "sort-ksec", no_argument, NULL, 10
        },
#endif

#if defined(HAVE_ACUTIME) && defined(HAVE_ACSTIME) && defined(HAVE_ACETIME)
        { "print-ratio", no_argument, NULL, 11
        },
#endif

#ifdef HAVE_ACIO
        { "sort-tio", no_argument, NULL, 13 },
#endif

#if defined(HAVE_ACSTIME) && defined(HAVE_ACUTIME)
        { "sort-sys-user-div-calls", no_argument, NULL, 15
        },
#endif

#ifdef HAVE_ACIO
        { "sort-avio", no_argument, NULL, 16 },
#endif

#ifdef HAVE_ACMEM
        { "sort-cpu-avmem", no_argument, NULL, 17 },
#endif

#if defined(HAVE_ACSTIME) && defined(HAVE_ACUTIME)
        /* Don't want this if it's the default */
        { "sort-num-calls", no_argument, NULL, 18
        },
#endif
#ifdef HAVE_ACETIME
        { "sort-real-time", no_argument, NULL, 26 },
#endif
        { "ahz", required_argument, NULL, 27 },
#ifdef HAVE_PAGING
        { "show-paging", no_argument, NULL, 28 },
        { "show-paging-avg", no_argument, NULL, 29 },
#endif
        { 0, 0, 0, 0 }
      };

      c = getopt_long (argc, argv,
                       "ahV"
#if defined(HAVE_ACSTIME) && defined(HAVE_ACUTIME)
                       "b"
#endif
                       "c"
#ifdef HAVE_ACIO
                       "d"
#endif
                       "fi"
#if defined(HAVE_ACUTIME) && defined(HAVE_ACSTIME)
                       "l"
#endif
                       "j"
#ifdef HAVE_ACMEM
                       "k"
#endif
                       "mMnrs"
#if defined(HAVE_ACUTIME) && defined(HAVE_ACSTIME) && defined(HAVE_ACETIME)
                       "t"
#endif
                       "uv:"
#ifdef HAVE_ACIO
                       "D"
#endif
#if defined(HAVE_ACSTIME) && defined(HAVE_ACUTIME) && defined(HAVE_ACMEM)
                       "K"
#endif
#ifdef HAVE_PAGING
                       "pP"
#endif
                       ,
                       long_options, &option_index);

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
        case 4:
          acct_file_name = optarg;
          break;
        case 'j':
        case 5:
          print_seconds = 1;
          break;
        case 'i':
        case 6:
          dont_read_summary_files = 1;
          break;
        case 'a':
        case 7:
          print_all_records = 1;
          break;
#if defined(HAVE_ACUTIME) && defined(HAVE_ACSTIME)
        case 'l':
        case 8:
          separate_times = 1;
          break;
#endif
        case 9:
          savacct_file_name = optarg;
          break;
#if defined(HAVE_ACSTIME) && defined(HAVE_ACUTIME) && defined(HAVE_ACMEM)
        case 'K':
        case 10:
          sort_type = sort_cpu_storage;
          print_ksec = 1;
          break;
#endif
#if defined(HAVE_ACUTIME) && defined(HAVE_ACSTIME) && defined(HAVE_ACETIME)
        case 't':
        case 11:
          print_ratio = 1;
          break;
#endif
        case 'u':
        case 12:
          print_users = 1;
          break;
#ifdef HAVE_ACIO
        case 'D':
        case 13:
          sort_type = sort_tio;
          total_io = 1;
          break;
#endif
        case 'c':
        case 14:
          percentages = 1;
          break;
#if defined(HAVE_ACSTIME) && defined(HAVE_ACUTIME)
        case 'b':
        case 15:
          sort_type = sort_sys_plus_user_div_calls;
          break;
#endif
#ifdef HAVE_ACIO
        case 'd':
        case 16:
          sort_type = sort_avio;
          break;
#endif
#ifdef HAVE_ACMEM
        case 'k':
        case 17:
          sort_type = sort_cpu_mem_average;
          break;
#endif
        case 'n':
#if defined(HAVE_ACSTIME) && defined(HAVE_ACUTIME)
        case 18:
          sort_type = sort_num_calls;
          break;
#endif
        case 'f':
        case 19:
          always_yes = 1;
          break;
        case 'm':
        case 20:
          user_summary_flag = 1;
          break;
        case 'M':
        case 30:
          user_summary_flag = 1;
          group_summary_flag = 1;
          break;
        case 'r':
        case 21:
          reverse_sort = 1;
          break;
        case 's':
        case 22:
          merge_files = 1;
          break;
        case 'v':
        case 23:
          junk_threshold = strtol(optarg, (char **)NULL, 10);

          if (junk_threshold < 1)
            {
              (void)printf("%s: threshold must be one or greater\n", program_name);
              exit(EXIT_FAILURE);
            }

          break;
        case 24:
          separate_forks = 1;
          break;
        case 25:
          usracct_file_name = optarg;
          break;
#ifdef HAVE_ACETIME
        case 26:
          sort_type = sort_real_time;
          break;
#endif
        case 27:
          ahz = strtol(optarg, (char **)NULL, 10);

          if (ahz < 1)
            {
              (void)printf("%s: AHZ must be one or greater\n", program_name);
              exit(EXIT_FAILURE);
            }

          break;
#ifdef HAVE_PAGING
        case 'P':
        case 29:
          paging_as_avg = 1;
        case 'p':
        case 28:
          show_paging = 1;
          break;
#endif
        case 'h':
        case 3:
          /* This should fall through to default! */
        default:
          give_usage();
          exit(EXIT_FAILURE);
        }
    }

  /* check options */

  if (print_users && (merge_files || user_summary_flag))
    {
      (void)printf("%s: can't specify `--merge' or `--user-summary' or `--group-summary' with `--print-users'\n", program_name);
      exit(EXIT_FAILURE);
    }

  if (merge_files && user_summary_flag)
    {
      (void)printf("%s: can't specify `--user-summary' or `--group-summary' with `--merge'\n",
                   program_name);
      exit(EXIT_FAILURE);
    }

  if (optind == (argc - 1))
    {
      /* if we get here, we're expecting a filename */
      acct_file_name = argv[optind++];
    }
  else if (optind < argc)
    {
      printf ("%s: extra arguments ", program_name);

      while (optind < argc)
        {
          printf ("`%s'", argv[optind++]);
          if (optind < argc)
            printf (", ");
        }

      printf (" -- aborting\n");
      exit(EXIT_FAILURE);
    }

  /* Print out some debugging information. */

  if (debugging_enabled)
    {
      (void)fprintf (stddebug, "hzval -> %d\n", hzval);
      (void)fprintf (stddebug, "getpagesize() -> %d\n", getpagesize ());
      (void)fprintf (stddebug, "system_page_size == %.2f\n", system_page_size);
    }

  /* see if the summary file is there -- if so, load it and parse it */

  if (!dont_read_summary_files)
    {
      struct stat st;

#define NEED_SAVACCT_INFO (!print_users && !user_summary_flag)

      if (NEED_SAVACCT_INFO)
        if (stat (savacct_file_name, &st) == 0)
          parse_savacct_entries (savacct_file_name);

#define NEED_USRACCT_INFO (!print_users && (merge_files || user_summary_flag))

      if (NEED_USRACCT_INFO)
        if (stat (usracct_file_name, &st) == 0)
          parse_usracct_entries (usracct_file_name);
    }

  /* now, do the acct file */

  add_pacct_file (acct_file_name);

  parse_acct_entries ();

  if (print_users)
    exit(EXIT_SUCCESS);

  if (merge_files)
    {
      write_savacct_file (savacct_file_name);
      write_usracct_file (usracct_file_name);
    }
  else
    {
      if (user_summary_flag)
        print_user_list ();
      else
        print_command_list ();
    }

  exit(EXIT_SUCCESS);			/* guarantee the proper return value */
}


/* guess what this does... */
void
give_usage (void)
{
  (void)printf("\n\
                Usage: %s [ options ] [ file ]\n\
                \n\
                options: [-a"
#if defined(HAVE_ACSTIME) && defined(HAVE_ACUTIME)
               "b"
#endif
               "c"
#ifdef HAVE_ACIO
               "d"
#endif
               "fi"
#if defined(HAVE_ACUTIME) && defined(HAVE_ACSTIME)
               "l"
#endif
               "j"
#ifdef HAVE_ACMEM
               "k"
#endif
               "mn"
#ifdef HAVE_PAGING
               "p"
#endif
               "rs"
#if defined(HAVE_ACUTIME) && defined(HAVE_ACSTIME) && defined(HAVE_ACETIME)
               "t"
#endif
               "u"
#ifdef HAVE_ACIO
               "D"
#endif
#if defined(HAVE_ACSTIME) && defined(HAVE_ACUTIME) && defined(HAVE_ACMEM)
               "K"
#endif
#ifdef HAVE_PAGING
               "P"
#endif
               "] [-v <num>] [--version] [--help]\n\
                [--other-acct-file <name>] [--other-usracct-file <name>]\n\
                [--print-seconds] [--dont-read-summary-files] [--debug]\n\
                "
#if defined(HAVE_ACUTIME) && defined(HAVE_ACSTIME)
               " [--separate-times]"
#endif
               " [--other-savacct-file <name>] [--percentages]\n\
                "
#if defined(HAVE_ACUTIME) && defined(HAVE_ACSTIME) && defined(HAVE_ACETIME)
               " [--print-ratio]"
#endif
               " [--print-users] [--merge] [--user-summary] [--group-summary]\n\
                [--list-all-names] [--not-interactive] [--threshold <num>]\n\
                "
#if defined(HAVE_ACSTIME) && defined(HAVE_ACUTIME) && defined(HAVE_ACMEM)
               " [--sort-ksec]"
#endif
#ifdef HAVE_ACIO
               " [--sort-tio]"
#endif
#if defined(HAVE_ACSTIME) && defined(HAVE_ACUTIME)
               " [--sort-sys-user-div-calls]"
#endif
#ifdef HAVE_ACIO
               " [--sort-avio]"
#endif
               "\n\
                "
#ifdef HAVE_ACMEM
               " [--sort-cpu-avmem]"
#endif
#if defined(HAVE_ACSTIME) && defined(HAVE_ACUTIME)
               " [--sort-num-calls]"
#endif
#ifdef HAVE_ACETIME
               " [--sort-real-time]"
#endif
               " [--ahz hz]\n"
#ifdef HAVE_PAGING
               "       [--show-paging] [--show-paging-avg]\n"
#endif
               "\n", program_name);
  print_acct_file_locations ();
}


static void add_stats (struct stats *accum, struct stats *s)
{
#define ADDIT(x); accum->x += s->x;

#ifdef HAVE_ACUTIME
  ADDIT (user_time);
#endif
#ifdef HAVE_ACSTIME
  ADDIT (sys_time);
#endif
#ifdef HAVE_ACETIME
  ADDIT (elapsed_time);
#endif
#ifdef HAVE_ACIO
  ADDIT (disk_io);
#endif
#ifdef HAVE_ACMEM
  ADDIT (mem_usage);
#endif
#ifdef HAVE_PAGING
  ADDIT (minor_faults);
  ADDIT (major_faults);
  ADDIT (swap_count);
#endif
  ADDIT (num_calls);
}

static void print_stats_raw (struct stats *s, FILE *out)
{
  (void)fprintf (out,
                 "%10ld"
#ifdef HAVE_ACUTIME
                 " %10.2fu"
#endif
#ifdef HAVE_ACSTIME
                 " %10.2fs"
#endif
#ifdef HAVE_ACETIME
                 " %10.2fe"
#endif
#ifdef HAVE_ACIO
                 " %10.2fio"
#endif
#ifdef HAVE_ACMEM
                 " %10.2fmem"
#endif
                 "\n",
                 s->num_calls
#ifdef HAVE_ACUTIME
                 , s->user_time
#endif
#ifdef HAVE_ACSTIME
                 , s->sys_time
#endif
#ifdef HAVE_ACETIME
                 , s->elapsed_time
#endif
#ifdef HAVE_ACIO
                 , s->disk_io
#endif
#ifdef HAVE_ACMEM
                 , s->mem_usage
#endif
                );
}


static void print_stats_nicely (struct stats *s)
{

#define NC s->num_calls
#define DNC ((double) NC)

#ifdef HAVE_ACETIME
#define RE s->elapsed_time
#define S_RE stats_totals.elapsed_time
#endif

#ifdef HAVE_ACUTIME
#define U s->user_time
#define S_U stats_totals.user_time
#endif

#ifdef HAVE_ACSTIME
#define S s->sys_time
#define S_S stats_totals.sys_time
#endif

#if defined(HAVE_ACUTIME) && defined(HAVE_ACSTIME)
#define CP (U + S)
#define S_CP (S_U + S_S)
#endif

#ifdef HAVE_ACIO
#define IO s->disk_io
#endif

#ifdef HAVE_ACMEM
#define K s->mem_usage
#endif

#ifdef HAVE_PAGING
#define MIN s->minor_faults
#define MAJ s->major_faults
#define SWP s->swap_count
#endif

#define S_NC stats_totals.num_calls
#define S_DNC ((double) S_NC)


  if (debugging_enabled)
    {
      (void)fprintf (stddebug, "raw:");
      print_stats_raw (s, stddebug);
    }

  /* FIXME should we check to see if (S_NC == 0) ? */

  if (NC == 0)
    {
      (void)printf("%s: ERROR -- print_stats_nicely called with num_calls == 0\n", program_name);
      exit(EXIT_FAILURE);
    }

  /* print it out */

  printf ("%8ld ", NC);

  if (percentages)
    printf ("%7.2f%% ", (S_NC
                         ? (DNC / S_DNC) * 100.0
                         : 0.0));

  /* We don't need to check to see that DNC != 0 since it is simply NC
  cast to a double, which we have checked before to be nonzero. */

#ifdef HAVE_ACETIME
  printf ("%10.2fre ", (RE / (print_seconds
                              ? DNC
                              : 60.0)));

  if (percentages)
    printf ("%7.2f%% ", (S_RE
                         ? (RE / S_RE) * 100.0
                         : 0.0));
#endif


#if defined(HAVE_ACUTIME) && defined(HAVE_ACSTIME)
  if (separate_times)
    {
#endif
#ifdef HAVE_ACUTIME
      printf ("%10.2fu ", (U / (print_seconds
                                ? DNC
                                : 60.0)));

      if (percentages)
        printf ("%7.2f%% ", (S_U
                             ? (U / S_U) * 100.0
                             : 0.0));
#endif

#ifdef HAVE_ACSTIME
      printf ("%10.2fs", (S / (print_seconds
                               ? DNC
                               : 60.0)));

      if (percentages)
        printf (" %7.2f%%", (S_S
                             ? (S / S_S) * 100.0
                             : 0.0));
#endif

#if defined(HAVE_ACUTIME) && defined(HAVE_ACSTIME)
    }
  else
    {
      printf ("%10.2fcp", (CP / (print_seconds
                                 ? DNC
                                 : 60.0)));

      if (percentages)
        printf (" %7.2f%%", (S_CP
                             ? (CP / S_CP) * 100.0
                             : 0.0));
    }
#endif

#if defined(HAVE_ACUTIME) && defined(HAVE_ACSTIME) && defined(HAVE_ACETIME)
  if (print_ratio)
    {
      if (CP == 0.0)
        (void)fputs("*ignore*", stdout);
      else
        printf("%8.1f", (CP ? RE / CP : 0.0));

      (void)fputs("re/cp", stdout);
    }
#endif

  if ( show_paging == 0 )
    {
#ifdef HAVE_ACIO
      if (total_io)
        printf ("%10.0ftio ", IO);
      else
        printf ("%10.0favio ", IO / DNC);
#endif

#if 0
      /* Old code, to which Christoph says: BAD. */
      if (print_ksec)
        printf ("%9.0fk*sec", K);
      else
        printf ("%9ldk", (long) (CP ? K / CP : 0.0));
#endif

#if defined(HAVE_ACSTIME) && defined(HAVE_ACUTIME) && defined(HAVE_ACMEM)

      /* Christoph Badura <bad@flatlin.ka.sub.org> says:
      *
      * The k*sec statistic is computed as
      * ((ac_utime+ac_stime)*pages_to_kbytes(ac_mem))/hzval.  Of course you
      * need to expand the comp_t values.
      *
      * PAGES_TO_KBYTES(x) simply divides x by (getpagesize()/1024).  Of
      * course, getpagesize()/1024 is precomputed and stored in a
      * variable.  There are no special cases for other machines, but
      * then my version is from the original VAX distribution so there
      * weren't any other machines.
      *
      * I don't understand why this works.  ac_mem is in pages, isn't it,
      * and dividing that by the number of KB per page can't give the
      * right number, i.e ac_mem=10 and getpagesize=4096 then
      * 10/(4096/1024) == 10/4 = 2.5 and not 40 KB.  Or am I missing
      * something? */

      if (print_ksec)
        printf ("%9.0fk*sec", (CP ? ((K * CP) / DNC) : K));
      else
        printf ("%9.0fk", K / DNC);
#endif
    }
  else
    {
#ifdef HAVE_PAGING
      if ( paging_as_avg == 0 )
        {
          printf ("%10.0fmin ", MIN);
          printf ("%10.0fmaj ", MAJ);
          printf ("%10.0fswp ", SWP);
        }
      else
        {
          printf ("%5.0fmin/c ", (NC==0)?0:MIN/NC);
          printf ("%5.0fmaj/c ", (NC==0)?0:MAJ/NC);
          printf ("%5.0fswp/c ", (NC==0)?0:SWP/NC);
        }
#else
      printf ("Paging report not enabled!");
#endif
    }

}

void write_savacct_file(char *filename)
{
  struct hashtab_order ho;
  struct hashtab_elem *he;
  FILE *fp = NULL;

  /* Write to a temporary file in case we get interrupted. */

  char *s = (char *) alloca (sizeof (char) * (strlen (filename) + 2));
  (void)sprintf(s, "%s~", filename);

  if ((fp = file_open (s, 1)) == NULL)
    exit(EXIT_FAILURE);

  /* Write each record. */

  for (he = hashtab_first (command_table, &ho);
       he != NULL;
       he = hashtab_next (&ho))
    {
      struct command_key *ck = hashtab_get_key (he);
      struct command_data *cd = hashtab_get_value (he);
      struct savacct sa;

      memcpy (&(sa.c), ck, sizeof (sa.c));
      memcpy (&(sa.s), &(cd->s), sizeof (sa.s));

      if (fwrite (&sa, sizeof (sa), 1, fp) == 0)
        {
          printf ("%s (write_savacct_file): probs writing to file `%s'\n",
                  program_name, s);
          exit(EXIT_FAILURE);
        }
    }

  if (rename (s, filename) != 0)
    {
      perror ("sa");
      exit(EXIT_FAILURE);
    }
}

void write_usracct_file(char *filename)
{
  struct hashtab_order ho;
  struct hashtab_elem *he;
  FILE *fp = NULL;

  /* Write to a temporary file in case we get interrupted. */

  char *s = (char *) alloca (sizeof (char) * (strlen (filename) + 2));
  (void)sprintf (s, "%s~", filename);

  if ((fp = file_open (s, 1)) == NULL)
    exit(EXIT_FAILURE);

  /* Write each record. */

  for (he = hashtab_first (user_table, &ho);
       he != NULL;
       he = hashtab_next (&ho))
    {
      char *name = hashtab_get_key (he);
      struct user_data *ud = hashtab_get_value (he);
      struct usracct ua;

      strncpy (ua.name, name, NAME_LEN);
      memcpy (&(ua.s), &(ud->s), sizeof (ua.s));

      if (fwrite (&ua, sizeof (ua), 1, fp) == 0)
        {
          printf ("%s (write_usracct_file): probs writing to file `%s'\n",
                  program_name, s);
          exit(EXIT_FAILURE);
        }
    }

  if (rename (s, filename) != 0)
    {
      perror ("sa");
      exit(EXIT_FAILURE);
    }
}

static void update_totals(struct stats *s)
{
  add_stats (&stats_totals, s);
}

/* parse the entries in a savacct file */

void parse_savacct_entries(char *filename)
{
  struct savacct rec;
  FILE *fp = NULL;

  if ((fp = file_open (filename, 0)) == NULL)
    exit(EXIT_FAILURE);

  if (debugging_enabled)
    fprintf (stddebug, "\
             ----------------------------------------------------------------------\n\
             savacct entries in `%s'\n\
             ----------------------------------------------------------------------\n\
             ", filename);

  while (fread (&rec, sizeof (struct savacct), 1, fp))
    {
      update_command_list (rec.c.comm, &(rec.s), rec.c.fork_flag);
      update_totals (&(rec.s));
    }

  (void)fclose (fp);
}

/* parse the entries in a usracct file */

void parse_usracct_entries(char *filename)
{
  struct usracct rec;
  FILE *fp = NULL;

  if ((fp = file_open (filename, 0)) == NULL)
    exit(EXIT_FAILURE);

  if (debugging_enabled)
    fprintf (stddebug, "\
             ----------------------------------------------------------------------\n\
             usracct entries in `%s'\n\
             ----------------------------------------------------------------------\n\
             ", filename);

  while (fread (&rec, sizeof (struct usracct), 1, fp))
    {
      update_user_list (rec.name, &(rec.s));

      /* only add these records to the total when we're doing a
      per-user summary of the data, because it will only screw up
      the percentages if we view a breakdown by command name */

      if (user_summary_flag)
        update_totals (&(rec.s));
    }

  (void)fclose (fp);
}


/* parse the entries in an acct file */

void parse_acct_entries (void)
{
  struct acct *rec;             /* the current record */
#if defined(LINUX_MULTIFORMAT)
# define CURR_AHZ ((double)(rec->ac_ahz))
#else
# define CURR_AHZ ((double)(ahz))
#endif

  if (debugging_enabled)
    fprintf (stddebug, "\
             ----------------------------------------------------------------------\n\
             acct entries\n\
             ----------------------------------------------------------------------\n\
             ");
             
  /* loop while there are entries to be had */
  while ((rec = pacct_get_entry ()) != NULL)
    {
#ifdef HAVE_ACUTIME
      double ut = comp_t_2_double (rec->ac_utime) / CURR_AHZ;
#endif

#ifdef HAVE_ACSTIME
      double st = comp_t_2_double (rec->ac_stime) / CURR_AHZ;
#endif

#ifdef HAVE_ACETIME
      double et = ACETIME_2_DOUBLE (rec->ac_etime) / CURR_AHZ;
#endif

#ifdef HAVE_ACIO
      double di = comp_t_2_double (rec->ac_io) / CURR_AHZ;
#endif

#ifdef HAVE_ACMEM
      double mu = PAGES_TO_KB (ACMEM_2_DOUBLE (rec->ac_mem));
#endif

#ifdef HAVE_PAGING
      double minflt = ACMINFLT_2_DOUBLE (rec->ac_minflt);
      double majflt = ACMAJFLT_2_DOUBLE (rec->ac_majflt);
      double swaps  = ACSWAPS_2_DOUBLE (rec->ac_swaps);
#endif

      if (print_users)
        {
          printf ("%-8.8s"
#if defined(HAVE_ACUTIME) && defined(HAVE_ACSTIME)
                  " %6.2f cpu"
#endif
#if defined(HAVE_ACMEM)
                  " %8.0fk mem"
#endif
#ifdef HAVE_ACIO
                  " %6.0f io"
#endif
                  " %-*.*s%s\n",
                  (!group_summary_flag) ? uid_name (rec->ac_uid) : gid_name (rec->ac_gid),
#if defined(HAVE_ACUTIME) && defined(HAVE_ACSTIME)
                  ut + st,
#endif
#if defined(HAVE_ACMEM)
                  mu,
#endif
#ifdef HAVE_ACIO
                  di,
#endif
                  COMM_LEN, COMM_LEN, rec->ac_comm,
                  (rec->ac_flag & AFORK) ? "*" : "");
        }
      else
        {
          struct stats s;

          s.num_calls = 1;
#ifdef HAVE_ACUTIME
          s.user_time = ut;
#endif
#ifdef HAVE_ACSTIME
          s.sys_time = st;
#endif
#ifdef HAVE_ACETIME
          s.elapsed_time = et;
#endif
#ifdef HAVE_ACIO
          s.disk_io = di;
#endif
#ifdef HAVE_ACMEM
          s.mem_usage = mu;
#endif
#ifdef HAVE_PAGING
          s.minor_faults = minflt;
          s.major_faults = majflt;
          s.swap_count   = swaps;
#endif

          if (NEED_SAVACCT_INFO)
            update_command_list (rec->ac_comm, &s, (rec->ac_flag & AFORK));

          if (NEED_USRACCT_INFO)
            update_user_list ((!group_summary_flag) ? uid_name (rec->ac_uid) : gid_name (rec->ac_gid), &s);

          update_totals (&s);
        }
    }
}


void update_command_list(char *comm, struct stats *s, short fork_flag)
{
  /* Look for the command in the list.  If found, add the stats.  If
  not found, create a new entry and add the stats. */

  struct hashtab_elem *he;
  struct command_key ck;

  if (debugging_enabled)
    {
      fprintf (stddebug, "+:%-*.*s%s ",
               COMM_LEN, COMM_LEN, comm, (fork_flag) ? "*" : " ");
      print_stats_raw (s, stddebug);
    }

  memset (&ck, 0, sizeof (ck));
  strncpy (ck.comm, comm, COMM_LEN);
  ck.fork_flag = fork_flag;

  he = hashtab_find (command_table, &ck, (unsigned int)sizeof (ck));
  if (he == NULL)
    {
      struct command_data cd;
      memset (&cd, 0, sizeof (cd));
      he = hashtab_create (command_table, &ck, sizeof (ck));
      hashtab_set_value (he, &cd, sizeof (cd));
    }

  {
    struct command_data *cd = hashtab_get_value (he);

    add_stats (&(cd->s), s);

#if 0
    /* we want to OR the old value of AP->FORK_FLAG because we might
    not be separating statistics based on whether or not the
    command forked.  In that case, we want to record the fact that
    some of the commands forked without calling exec. */

    cd->fork_flag |= fork_flag;
#endif
  }
}

/* Add the amount of time and resources used for the per-user
summary. */

void update_user_list(char *name, struct stats *s)
{
  /* Look for the user in the list.  If found, add the stats.  If not
  found, create a new entry and add the stats. */

  struct hashtab_elem *he;

  if (debugging_enabled)
    {
      fprintf (stddebug, "+u:%*s ", NAME_LEN, name);
      print_stats_raw (s, stddebug);
    }

  he = hashtab_find (user_table, name, (unsigned int)NAME_LEN);

  if (he == NULL)
    {
      struct user_data ud;
      memset (&ud, 0, sizeof (ud));
      he = hashtab_create (user_table, name, NAME_LEN);
      hashtab_set_value (he, &ud, sizeof (ud));
    }

  {
    struct user_data *ud = hashtab_get_value (he);
    add_stats (&(ud->s), s);
  }
}

/* compare two entries, returning an int less than, equal to, or
* greater than zero reflecting whether s1 is less than, equal to, or
* greater than s2. */

int compare_stats_entry(struct stats *s1, struct stats *s2)
{
  double v1 = 0, v2 = 0;

  switch (sort_type)
    {
#if defined(HAVE_ACSTIME) && defined(HAVE_ACUTIME)
    case sort_sys_plus_user:
      v1 = s1->user_time + s1->sys_time;
      v2 = s2->user_time + s2->sys_time;
      break;
#endif
#if defined(HAVE_ACSTIME) && defined(HAVE_ACUTIME)
    case sort_sys_plus_user_div_calls:
      v1 = (s1->user_time + s1->sys_time) / (double) s1->num_calls;
      v2 = (s2->user_time + s2->sys_time) / (double) s2->num_calls;
      break;
#endif
#ifdef HAVE_ACIO
    case sort_avio:
      v1 = s1->disk_io / (double) s1->num_calls;
      v2 = s2->disk_io / (double) s2->num_calls;
      break;
    case sort_tio:
      v1 = s1->disk_io;
      v2 = s2->disk_io;
      break;
#endif
#ifdef HAVE_ACMEM
    case sort_cpu_mem_average:
      v1 = s1->mem_usage / (double) s1->num_calls;
      v2 = s2->mem_usage / (double) s2->num_calls;
      break;
#endif
#if defined(HAVE_ACSTIME) && defined(HAVE_ACUTIME) && defined(HAVE_ACMEM)
    case sort_cpu_storage:
      v1 = ((s1->user_time + s1->sys_time)
            ? (s1->user_time + s1->sys_time) * s1->mem_usage
            : s1->mem_usage);
      v2 = ((s2->user_time + s2->sys_time)
            ? (s2->user_time + s2->sys_time) * s2->mem_usage
            : s2->mem_usage);
      break;
#endif
#ifdef HAVE_ACETIME
    case sort_real_time:
      if (print_seconds)
        {
          v1 = s1->elapsed_time / (double) s1->num_calls;
          v2 = s2->elapsed_time / (double) s2->num_calls;
        }
      else
        {
          v1 = s1->elapsed_time;
          v2 = s2->elapsed_time;
        }
      break;
#endif
    case sort_num_calls:
      v1 = (double) s1->num_calls;
      v2 = (double) s2->num_calls;
      break;
    default:
      fatal ("unknown sort type");
      break;
    }
  if (v1 < v2) return ((reverse_sort) ? -1 : 1);
  if (v1 > v2) return ((reverse_sort) ? 1 : -1);

#define compareit(x); \
  if (s1->x < s2->x) return ((reverse_sort) ? -1 : 1); \
  if (s1->x > s2->x) return ((reverse_sort) ? 1 : -1);

  compareit (num_calls);
#ifdef HAVE_ACUTIME
  compareit (user_time);
#endif
#ifdef HAVE_ACSTIME
  compareit (sys_time);
#endif
#ifdef HAVE_ACETIME
  compareit (elapsed_time);
#endif
#ifdef HAVE_ACIO
  compareit (disk_io);
#endif
#ifdef HAVE_ACMEM
  compareit (mem_usage);
#endif

#undef compareit

  return 0;
}

int compare_user_entry(struct hashtab_elem **s1, struct hashtab_elem **s2)
{
  struct user_data *ud1 = hashtab_get_value (*s1);
  struct user_data *ud2 = hashtab_get_value (*s2);

  return compare_stats_entry (&(ud1->s), &(ud2->s));
}

int compare_sum_entry(struct hashtab_elem **s1, struct hashtab_elem **s2)
{
  struct command_data *ud1 = hashtab_get_value (*s1);
  struct command_data *ud2 = hashtab_get_value (*s2);

  return compare_stats_entry (&(ud1->s), &(ud2->s));
}

void print_user_list(void)
{
  /* even though the BSD version of sa doesn't support sorting of the
  user summary list, why can't we? */

  struct hashtab_order ho;
  struct hashtab_elem *he, **entry_array, user_totals;
  struct user_data user_totals_ud;
  long num_users, which, temp;
  char * const empty_string = "";

  /* Make the summary record.  FIXME -- we need some functions so we
  can fabricate these entries without playing with the structure
  internals. */

  user_totals_ud.s = stats_totals;
  user_totals.key = empty_string;
  user_totals.data = &user_totals_ud;

  /* Count the number of users in the hash table. */

  for (he = hashtab_first (user_table, &ho), num_users = 0;
       he != NULL;
       he = hashtab_next (&ho), num_users++)
    ;

  num_users++;			/* one for the summary entry */

  entry_array = (struct hashtab_elem **)
                xmalloc (sizeof (struct hashtab_elem *) * num_users);

  which = 0;
  entry_array[which++] = &user_totals;

  for (he = hashtab_first (user_table, &ho);
       he != NULL;
       he = hashtab_next (&ho))
    {
      entry_array[which++] = he;
    }

  /* The summary entry should always be first, so don't sort it.
  Remember to correct the number of elements to adjust... */

  qsort (entry_array + 1, (size_t) num_users - 1,
         sizeof (struct hashtab_elem *), (int (*)()) compare_user_entry);

  /* Now we've got a sorted list of user entries. */

  for (temp = 0; temp < num_users; temp++)
    {
      char *name;
      struct stats *s;
      struct user_data *ud;

      he = entry_array[temp];
      name = hashtab_get_key (he);
      ud = hashtab_get_value (he);
      s = &(ud->s);

      if (debugging_enabled)
        {
          fprintf (stddebug, "t:%-10.10s ", name);
          print_stats_raw (s, stddebug);
        }

      printf ("%-*.*s ", NAME_LEN, NAME_LEN, name);
      print_stats_nicely (s);
      (void)putchar('\n');
    }

  free (entry_array);
}

int non_printable(char *s, int len)
{
  int a;

  for (a = 0; (a < len) && (s[a] != '\0'); a++)
    {
      if (!isprint (s[a]))
        return 1;
    }

  return 0;
}

int ask_if_junkable(char *s, int len)
{
  char line[1000];
  char word[1000];

  if (always_yes)
    return 1;

  (void)printf ("Junk `%*s'? ", len, s);
  (void)fflush (stdout);

  if (fgets (line, 1000, stdin) &&
      (sscanf (line, " %s ", word) > 0) &&
      ((word[0] == 'y') || (word[0] == 'Y')))
    return 1;

  return 0;
}

void print_command_list(void)
{
  struct hashtab_order ho;
  struct hashtab_elem **entry_array, *he;
  struct hashtab_elem sum_totals, other_totals, junk_totals;
  struct command_data sum_totals_cd, other_totals_cd, junk_totals_cd;
  struct command_key sum_totals_ck, other_totals_ck, junk_totals_ck;
  long num_commands, temp, which;
  int num_in_other_category, num_in_junk_category;

  /* Make the summary records.  FIXME -- we need some functions so we
  can fabricate these entries without playing with the structure
  internals. */

  memset (&sum_totals_cd, 0, sizeof (sum_totals_cd));
  memset (&sum_totals_ck, 0, sizeof (sum_totals_ck));
  sum_totals_cd.s = stats_totals;
  sum_totals.key = &sum_totals_ck;
  sum_totals.data = &sum_totals_cd;

  (void)memset (&other_totals_cd, 0, sizeof (other_totals_cd));
  (void)memset (&other_totals_ck, 0, sizeof (other_totals_ck));
  (void)strcpy (other_totals_ck.comm, "***other");
  other_totals.key = &other_totals_ck;
  other_totals.data = &other_totals_cd;

  (void)memset (&junk_totals_cd, 0, sizeof (junk_totals_cd));
  (void)memset (&junk_totals_ck, 0, sizeof (junk_totals_ck));
  (void)strcpy (junk_totals_ck.comm, "**junk**");
  junk_totals.key = &junk_totals_ck;
  junk_totals.data = &junk_totals_cd;

  /* Count the number of commands in the table. */

  for (he = hashtab_first (command_table, &ho), num_commands = 0,num_in_other_category = 0, num_in_junk_category = 0;
       he != NULL; he = hashtab_next (&ho))
    {
      if (print_all_records)
        num_commands++;
      else
        {
          struct command_key *ck = hashtab_get_key (he);
          struct command_data *cd = hashtab_get_value (he);

          /* if we're junking stuff and the number of calls is
          under the threshold, do it... */

          if (junk_threshold
              && (cd->s.num_calls <= junk_threshold)
              && ask_if_junkable (ck->comm, COMM_LEN))
            {
              num_in_junk_category++;
              cd->junked = 1;
              add_stats (&(junk_totals_cd.s), &(cd->s));
              junk_totals_ck.fork_flag |= ck->fork_flag;
            }

          /* if the number of calls is one or the characters in
                       the filename are non-printing, we have to add this to
                       the "***other" category */

          else if ((cd->s.num_calls == 1)
                   || non_printable (ck->comm, COMM_LEN))
            {
              num_in_other_category++;
              cd->junked = 1;
              add_stats (&(other_totals_cd.s), &(cd->s));
              other_totals_ck.fork_flag |= ck->fork_flag;
            }
          else
            num_commands++;
        }
    }

  if (num_commands == 0)
    exit (EXIT_SUCCESS);			/* no calls -- outta here! */

  num_commands++;		/* one for the summary entry */
  if (num_in_other_category)
    num_commands++;		/* one for the `***other' category */
  if (num_in_junk_category)
    num_commands++;		/* one for the `***junk' category */

  entry_array = (struct hashtab_elem **)
                xmalloc (sizeof (struct hashtab_elem *) * num_commands);

  which = 0;
  entry_array[which++] = &sum_totals;

  if (num_in_other_category)
    entry_array[which++] = &other_totals;

  if (num_in_junk_category)
    entry_array[which++] = &junk_totals;

  for (he = hashtab_first (command_table, &ho);
       he != NULL;
       he = hashtab_next (&ho))
    {
      if (print_all_records)
        entry_array[which++] = he;
      else
        {
          struct command_data *cd = hashtab_get_value (he);

          if (cd->junked)
            {
              /* skip this one */
            }
          else
            entry_array[which++] = he;
        }
    }

  /* the summary entry should always be first, so don't sort it --
     therefore, pass location 1 of the array to qsort and one less
     than the number of commands */

  qsort (entry_array + 1, (size_t) num_commands - 1,
         sizeof (struct hashtab_entry *), (int (*)()) compare_sum_entry);

  for (temp = 0; temp < num_commands; temp++)
    {
      struct command_data *cd;
      struct command_key *ck;

      he = entry_array[temp];
      cd = hashtab_get_value (he);
      ck = hashtab_get_key (he);

      print_stats_nicely (&(cd->s));

      if (ck->comm[0] != '\0')
        {
          printf ("   %-.*s", COMM_LEN, ck->comm);

          if (ck->fork_flag)
            (void)putchar('*');
        }
      (void)putchar('\n');
    }
  free (entry_array);
}
