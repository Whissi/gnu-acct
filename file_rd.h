/* file_rd.h
 *
 * data structures and routines for reading/writing binary
 * record-oriented files. */

struct file_list {
  char *name;
  struct file_list *next;
};

struct file_rd_info {
  struct file_list *the_files;
  FILE *fp;			/* current file pointer */
  char *name;			/* current file name -- we need this
				   because we close files as soon as
				   we find their ends and free their
				   entries from the linked list */
  int recs_read;		/* how many records are in our buffer? */
  int recs_left;		/* how many records are left in the
				   buffer to be used? */
  void *buffer;			/* the buffer for the data */
  int buffered_records;		/* how many records to buffer */
  int record_size;		/* how big is each record? */
  int backwards;		/* non-zero if we're reading backwards */
  long rec_number;		/* record number in the currently open
				   file, for error-reporting purposes */
};

struct file_rd_info *file_reader_init PARAMS((int record_size,
					      int buffered_records,
					      int backwards));
void file_reader_add_file PARAMS((struct file_rd_info *fri, char *name));
char *file_reader_get_entry PARAMS((struct file_rd_info *fri));
void file_reader_print_file_and_line PARAMS((FILE *out,
					     struct file_rd_info *fri));
