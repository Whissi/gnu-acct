/*
Copyright (C) 1993, 1996, 1997, 2008, 2009, 2010 Free Software Foundation, Inc.

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

/* file_rd.h
 *
 * data structures and routines for reading/writing binary
 * record-oriented files.
 *
 */

#ifndef FILE_RD_H
#define FILE_RD_H

#include <unistd.h>

struct file_list
{
  char *name;
  struct file_list *next;
};

struct file_rd_info
{
  struct file_list *the_files;
  FILE *fp;			/* current file pointer */
  char *name;			/* current file name -- we need this
				   because we close files as soon as
				   we find their ends and free their
				   entries from the linked list */
  off_t recs_read;		/* how many records are in our buffer? */
  off_t recs_left;		/* how many records are left in the
				   buffer to be used? */
  void *buffer;			/* the buffer for the data */
  int buffered_records;		/* how many records to buffer */
  int record_size;		/* how big is each record? */
  int backwards;		/* non-zero if we're reading backwards */
  long rec_number;		/* record number in the currently open
				   file, for error-reporting purposes */
};

struct file_rd_info *file_reader_init (int record_size,
                                       int buffered_records,
                                       int backwards);
void file_reader_add_file (struct file_rd_info *fri, char *name);
char *file_reader_get_entry (struct file_rd_info *fri);
void file_reader_print_file_and_line (FILE *out, struct file_rd_info *fri);

#endif /* FILE_RD_H */
