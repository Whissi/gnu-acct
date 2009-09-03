/* Copyright Free Software Foundation 1997, 2003, 2005
   This file is released under the terms of the GNU General Public License */

/* print an acct/pacct file in human-readable format. */

#include "config.h"

#include <stdio.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include "common.h"
#include "pacct_rd.h"
#ifdef HAVE_GETOPT_LONG
#include <getopt.h>
#else
#include "getopt_long.h"
#endif

char *program_name;
int debugging_enabled = 0;	/* no -- we don't care about bad
				   records or the file-reading
				   algorithms. */
int ahz = AHZ;


static
void
give_usage (void)
{
  printf ("Usage: %s [-hrR] [-n <recs>] <files>\n\
          [--num <recs>] [--raw] [--reverse] [--help]\n",
          program_name);
#ifdef LINUX_MULTIFORMAT
  printf ("       [--format <version>] [--byteswap]\n");
#endif
  print_acct_file_location ();
}


int
main (int argc, char *argv[])
{
  int c;
  int backwards_flag = 0;	/* start forwards */
  long num_lines_to_print = -1;	/* -1 means all */
  int raw = 0;
#ifdef LINUX_MULTIFORMAT
  int version = -1;
  int byteswap = 0;
  size_t size;
  static struct acct buf;
#endif

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
          { "ahz", required_argument, NULL, 5 },
#ifdef LINUX_MULTIFORMAT
          { "format", required_argument, NULL, 6 },
          { "byteswap", no_argument, NULL, 7 },
#endif
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
        case 5:
          ahz = atoi (optarg);
          if (ahz < 1)
            {
              printf ("%s: AHZ must be one or greater\n", program_name);
              exit (1);
            }
          break;
#ifdef LINUX_MULTIFORMAT
        case 6:
          if (optarg[0]=='v')
            optarg++;
          version = atoi (optarg);
          break;
        case 7:
          byteswap = 1;
          break;
#endif
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
          {
#ifdef LINUX_MULTIFORMAT
            size = convert_acct_record_write (rec, &buf, version, byteswap);
            fwrite (&buf, size, 1, stdout);
#else
            fwrite (rec, sizeof (struct acct), 1, stdout);
#endif
          }
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

