/* utmp_rd.c
 *
 * routines that read utmp files */

#include "config.h"

#include <stdio.h>

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <ctype.h>
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

#include "common.h"
#include "file_rd.h"
#include "utmp_rd.h"

/* globals */

static struct file_rd_info *utmp_info = NULL;

#define BUFFERED_RECS 256


/* Set up utmp handling routines */

void
utmp_init (int backwards)
{
  utmp_info = file_reader_init (sizeof (struct utmp), BUFFERED_RECS,
				backwards);
}


/* Add a utmp/wtmp file to the list of files to process */

void
add_utmp_file (char *name)
{
  file_reader_add_file (utmp_info, name);
}


/* Do a buffered read of the file and return the next record in REC.
   Return 0 if no more entries. */

struct utmp *
utmp_get_entry (void)
{
  return (struct utmp *) file_reader_get_entry (utmp_info);
}


void
print_utmp_record (struct utmp *rec, FILE *out)
{
  if (rec)
    {
      fprintf (out, "%-*.*s|%-*.*s|",
	       NAME_LEN, NAME_LEN, rec->ut_name,
	       TTY_LEN, TTY_LEN, rec->ut_line);

#ifdef HAVE_UT_TYPE
      fprintf (out, "%1d|", rec->ut_type);
#endif

#ifdef HAVE_UT_ID
      {
	int i;
	for (i = 0; i < ID_LEN; i++)
	  {
	    char c = rec->ut_id[i];
	    if (c == '\0')
	      {
		fprintf (out, "%-*.*s", ID_LEN - i, ID_LEN - i, "");
		break;
	      }
	    else if (! isprint (c))
	      {
		fputc ('?', out);
	      }
	    else
	      {
		fputc (c, out);
	      }
	  }
	fputc ('|', out);
      }
#endif

#ifdef HAVE_UT_PID
      fprintf (out, "%5d|", rec->ut_pid);
#endif

#ifdef HAVE_UT_ADDR
      {
	struct in_addr a;
	a.s_addr = rec->ut_addr;
	fprintf (out, "%-15.15s|",
		 (rec->ut_addr) ? inet_ntoa (a) : "");
      }
#endif

#ifdef HAVE_UT_HOST
      fprintf (out, "%-*.*s|", HOST_LEN, HOST_LEN, rec->ut_host);
#endif

      fputs (ctime ((time_t *) &(rec->ut_time)), out);
    }
}

void
utmp_print_file_and_line (FILE *out)
{
  file_reader_print_file_and_line (out, utmp_info);
}


#ifdef HAVE_UT_TYPE

/* Some machines have a combination of bsd and sysv stuff writing to
   their /etc/wtmp file.  If we can handle ut_type fields, let's do
   it.  Munge BSD-style records into to real sysv records (with
   UT_TYPE set correctly) so we don't have to have so many special
   cases for them in our processing loop.  Return non-zero if the
   field was modified. */

int
fix_ut_type_field (struct utmp *rec)
{
  int mods = 0;

  /* Munge the various BSD-type records. */

  if (rec->ut_line[0] == OLD_TIME_CHAR)
    mods = 1, rec->ut_type = OLD_TIME;
  else if (rec->ut_line[0] == NEW_TIME_CHAR)
    mods = 1, rec->ut_type = NEW_TIME;
  else if (rec->ut_line[0] == BOOT_TIME_CHAR)
    {
      if ((strncmp (rec->ut_name, "reboot", NAME_LEN) == 0)
	  && (rec->ut_type != BOOT_TIME))
	mods = 1, rec->ut_type = BOOT_TIME;
#ifdef RUN_LVL
      else if ((strncmp (rec->ut_name, "shutdown", NAME_LEN) == 0)
	       && (rec->ut_type != RUN_LVL))
	mods = 1, rec->ut_type = RUN_LVL;
#else
      /* If we don't have RUN_LVL, set the type to BOOT_TIME and the
         bsd side will catch it. */
      else if ((strncmp (rec->ut_name, "shutdown", NAME_LEN) == 0)
	       && (rec->ut_type != BOOT_TIME))
        mods = 1, rec->ut_type = BOOT_TIME;
#endif
    }

  /* Munge screwed-up sysv records. */

#ifdef RUN_LVL
  if ((rec->ut_type == RUN_LVL)
      && (strncmp (rec->ut_name, "reboot", NAME_LEN) == 0)
      && (rec->ut_type != BOOT_TIME))
    mods = 1, rec->ut_type = BOOT_TIME;

  if ((rec->ut_type == BOOT_TIME)
      && (strncmp (rec->ut_name, "shutdown", NAME_LEN) == 0)
      && (rec->ut_type != RUN_LVL))
    mods = 1, rec->ut_type = RUN_LVL;
#endif

#if defined (USER_PROCESS) && defined (DEAD_PROCESS)
  if ((rec->ut_type == USER_PROCESS)
      && (rec->ut_name[0] == '\0'))
    mods = 1, rec->ut_type = DEAD_PROCESS;
#endif

  /* Often EMPTY or UT_UNKNOWN is 0, so we might modify some of those
     records here.  The chances of finding a real record with UT_TYPE
     == 0 are low, however.  We're trying to catch records that were
     written that don't have UT_TYPE set correctly... */

#if defined (USER_PROCESS) && defined (DEAD_PROCESS)
  if ((rec->ut_type < 1) || (rec->ut_type > UTMAXTYPE))
    {
      if (rec->ut_line[0] != '\0')
	{
	  if (rec->ut_name[0] != '\0')
	    mods = 1, rec->ut_type = USER_PROCESS;
	  else
	    mods = 1, rec->ut_type = DEAD_PROCESS;
	}
    }
#endif

  return mods;
}
#endif


/* If a record is obviously bad, return non-zero. */

int
bad_utmp_record (struct utmp *rec)
{
#ifndef HAVE_UT_TYPE

  /* If we have UT_TYPE, all bets are off, since all of these tests
     might be valid for one record type or another.  We can only do
     this for bsd-ish records. */

  if ((rec->ut_line[0] == '\0')
      && (rec->ut_line[0] == '\0')
      && (rec->ut_time == 0))
    return 1;
  
#endif

  return 0;
}
