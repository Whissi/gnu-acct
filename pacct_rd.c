/* pacct_rd.c
 *
 * routines that read acct/pacct files */

#include "config.h"

#include <stdio.h>
#include <math.h>

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
#include "pacct_rd.h"

/* globals */

static struct file_rd_info *pacct_info = NULL;

#define BUFFERED_RECS 256


/* Set up pacct handling routines */

void
pacct_init (int backwards)
{
  pacct_info = file_reader_init (sizeof (struct acct), BUFFERED_RECS,
				 backwards);
}


/* Add a pacct/acct file to the list of files to process */

void
add_pacct_file (char *name)
{
  file_reader_add_file (pacct_info, name);
}


/* Do a buffered read of the file and return the next record in REC.
   Return 0 if no more entries. */

struct acct *
pacct_get_entry (void)
{
  return (struct acct *) file_reader_get_entry (pacct_info);
}


void
print_pacct_record (struct acct *rec, FILE *out)
{
  fprintf (stddebug, "%-*.*s|", COMM_LEN, COMM_LEN, rec->ac_comm);

#define NUM_FORMAT "%6.1f"

#ifdef HAVE_ACUTIME
  fprintf (stddebug, NUM_FORMAT "|",
#ifdef ACUTIME_COMPT
	   comp_t_2_double (rec->ac_utime)
#else
	   (double) rec->ac_utime
#endif
	   );
#endif

#ifdef HAVE_ACSTIME
  fprintf (stddebug, NUM_FORMAT "|",
#ifdef ACSTIME_COMPT
	   comp_t_2_double (rec->ac_stime)
#else
	   (double) rec->ac_stime
#endif
	   );
#endif

#ifdef HAVE_ACETIME
  fprintf (stddebug, NUM_FORMAT "|",
#ifdef ACETIME_COMPT
	   comp_t_2_double (rec->ac_etime)
#else
	   (double) rec->ac_etime
#endif
	   );
#endif

  fprintf (stddebug, "%5d|%5d|", rec->ac_uid, rec->ac_gid);

#ifdef HAVE_ACMEM
  fprintf (stddebug, NUM_FORMAT "|",
#ifdef ACMEM_COMPT
	   comp_t_2_double (rec->ac_mem)
#else
	   (double) rec->ac_mem
#endif
	   );
#endif

#ifdef HAVE_ACIO
  fprintf (stddebug, NUM_FORMAT "|",
#ifdef ACIO_COMPT
	   comp_t_2_double (rec->ac_io)
#else
	   (double) rec->ac_io
#endif
	   );
#endif

  fprintf (stddebug, "%s", ctime ((time_t *) &(rec->ac_btime)));
}


void
pacct_print_file_and_line (FILE *out)
{
  file_reader_print_file_and_line (out, pacct_info);
}


#ifdef HAVE_COMP_T

/* convert a comp_t to a double */

double
comp_t_2_double (comp_t c_num)
{
  unsigned long in = 0;		/* the number part */
  
  /* comp_t are strange numbers -- of 16 bits, the first three are are
     the exponent and the last 13 are the number.  The exp is base 8
     (yuck)! */

  in = c_num & 017777;
  c_num >>= 13;

  while (c_num) {
    c_num--;
    in <<= 3;
  }
  
  /* printf ("%x -> %d %d -> %f\n",
     (int) c_num, (int) ex, (int) in, fl); */
  
  return (double) in;
}

#endif /* HAVE_COMP_T */
