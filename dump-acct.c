/* dump-acct.c
 *
 * print an acct/pacct file in human-readable format. */

#include "config.h"

#include <stdio.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include "common.h"
#include "pacct_rd.h"
#include "getopt.h"


char *program_name;
int debugging_enabled = 0;	/* no -- we don't care about bad
				   records or the file-reading
				   algorithms. */

void main PARAMS((int argc, char *argv[]));

static
void
give_usage (void)
{
  printf ("Usage: %s [-hrR] [-n <recs>] <files>\n\
       [--num <recs>] [--raw] [--reverse] [--help]\n",
	  program_name);
  print_acct_file_location ();
}


void
main (int argc, char *argv[])
{
  int c;
  int backwards_flag = 0;	/* start forwards */
  long num_lines_to_print = -1;	/* -1 means all */
  int raw = 0;

  program_name = argv[0];

  while (1)
    {
      int option_index = 0;
      
      static struct option long_options[] = {
	{ "reverse", no_argument, NULL, 1 },
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
	  give_usage ();
	  exit (1);
	  break;
	case 'n':
	case 3:
	  num_lines_to_print = atol (optarg);
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
      give_usage ();
      exit (1);
    }
  
  pacct_init (backwards_flag);

  /* Add all files that we want to dump. */

  while (optind < argc)
    add_pacct_file (argv[optind++]);

  /* Dump the records. */

  {
    struct acct *rec;
    while ((rec = pacct_get_entry ()) != NULL)
      {
	if (raw)
	  fwrite (rec, sizeof (struct acct), 1, stdout);
	else
	  print_pacct_record (rec, stdout);

	if (num_lines_to_print > -1)
	  {
	    num_lines_to_print--;
	    if (num_lines_to_print == 0)    /* max lines printed */
	      exit (0);
	  }
      }
	
  }

  exit (0);
}

