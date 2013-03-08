/* file_rd.c
 *
 * generic routines for reading and writing binary record-oriented
 * files.
 *
 */

#include "config.h"

#include <stdio.h>

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include "common.h"
#include "files.h"
#include "file_rd.h"

/* Set up a file reader. */

struct file_rd_info *file_reader_init (int record_size, int buffered_records, int backwards)
{
  struct file_rd_info *new;

  new = (struct file_rd_info *) xmalloc (sizeof (struct file_rd_info));
  memset (new, 0, sizeof (struct file_rd_info));
  new->record_size = record_size;
  new->buffered_records = buffered_records;
  new->buffer = (char *) xmalloc (record_size * buffered_records);
  new->backwards = backwards;

  return new;
}

/* Add a file to the list of files to process */

void file_reader_add_file(struct file_rd_info *fri, char *name)
{
  /* SNOC this file onto our list so that we process them in order. */

  struct file_list *np, *new;

  new = (struct file_list *) xmalloc (sizeof (struct file_list));
  new->name = name;
  new->next = NULL;

  for (np = fri->the_files; (np != NULL) && (np->next != NULL); np = np->next)
    /* spin */;

  if (np == NULL)
    {
      /* This must be the first thing in the list! */
      fri->the_files = new;
    }
  else
    {
      /* In every other case, np->next is NULL, so put NEW in its
         place. */
      np->next = new;
    }
}

/* Do a buffered read of the file and return the next record in REC.
   Return 0 if no more entries. */

char *file_reader_get_entry(struct file_rd_info *fri)
{
  extern int debugging_enabled;

  /* If RECS_LEFT == 0, there's nothing in the buffer, so read another
     block of records from the file. */

  if (fri->recs_left == 0)
    {
      while (1)
        {
          /* If there's no file open, we should do so. */

          if (fri->fp == NULL)
            {
              /* Open up the next file!  First, check to see that there IS
              another file... */

              if (fri->the_files == NULL)
                return 0;		/* no more files == no more records */

              fri->fp = file_open (fri->the_files->name, 0);
              if (fri->fp == NULL)
                {
                  /* make sure to free this list entry */
                  goto no_more_records_no_close;
                }

              if (fri->backwards)
                (void)fseek(fri->fp, 0, SEEK_END);	/* go to end of file */

              fri->rec_number = 0;	/* start over! */
              fri->name = fri->the_files->name;
            }

          /* We have to do things differently depending on our
                   direction of reading */

          if (fri->backwards)
            {
              off_t offset, max_recs, recs_to_read;

              if ((offset = ftello (fri->fp)) <= 0)
                goto no_more_records;

              /* Read as many records as possible, up to
                       FRI->BUFFERED_RECORDS */

              max_recs = offset / (long) fri->record_size;

              recs_to_read = ((max_recs < fri->buffered_records)
                              ? max_recs
                              : fri->buffered_records);

              /* Move back in the file */

              (void)fseek(fri->fp, -fri->record_size * recs_to_read,
                          SEEK_CUR);

              if (debugging_enabled)
                {
                  off_t new_offset = ftello (fri->fp);
                  (void)fprintf (stddebug, "Did seek in file %ld --> %ld\n",
                                 (long int)offset, (long int)new_offset);
                }

              if (fread ((void *) fri->buffer, fri->record_size,
                         recs_to_read, fri->fp) == -1)
                fatal ("get_entry: couldn't read from file");

              if (debugging_enabled)
                (void)fprintf (stddebug, "Got %ld records from file\n",
                               (long int)recs_to_read);

              /* don't need to check this, because the above read was fine */

              (void)fseek(fri->fp, -fri->record_size * recs_to_read, SEEK_CUR);

              /* Set up buffer position variables */

              fri->recs_read = recs_to_read;
              fri->recs_left = recs_to_read;

              break;		/* get out of this WHILE loop */
            }
          else
            {
              /* Reading forwards in the file */

              int retval;

              retval = fread ((void *) fri->buffer, fri->record_size,
                              fri->buffered_records, fri->fp);

              if (retval == -1)
                fatal ("get_entry: couldn't read from file");
              else if (retval != 0)
                {
                  fri->recs_read = retval;
                  fri->recs_left = retval;
                  break;	/* get out of this loop */
                }
            }

          /* If we got here, there were no more records in the file.
                   Close it, delete it from our list, and try again. */

no_more_records:
          (void)fclose (fri->fp);
no_more_records_no_close:
          fri->fp = NULL;
          fri->the_files = fri->the_files->next;
        }
    }

  {
    char *rec;

    if (fri->backwards)
      rec = (char *) fri->buffer + (--fri->recs_left * fri->record_size);
    else
      rec = (char *) fri->buffer + ((fri->recs_read - fri->recs_left--)
                                    * fri->record_size);

    fri->rec_number++;

    return rec;
  }
}

void file_reader_print_file_and_line(FILE *out, struct file_rd_info *fri)
{
  (void)fprintf(out, "%s:%ld", fri->name ? fri->name : "NULL?", fri->rec_number);
}

