/* pacct_rd.h
 *
 * protos & etc. for routines that read from pacct/acct files */

#include "config.h"

#include "files.h"

/* This looks strange, but it's real ANSI C. */

static struct acct pacct_rd_never_used;

#define COMM_LEN ((int) sizeof (pacct_rd_never_used.ac_comm))

void pacct_init (int backwards);
void add_pacct_file (char *name);
struct acct *pacct_get_entry (void);
void print_pacct_record (struct acct *rec, FILE *out);
void pacct_print_file_and_line (FILE *out);

#ifdef HAVE_COMP_T
double comp_t_2_double (comp_t);
#endif

