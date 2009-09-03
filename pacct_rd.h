/* pacct_rd.h
 *
 * protos & etc. for routines that read from pacct/acct files */

#include "config.h"

#include "files.h"

/* This looks strange, but it's real ANSI C. */

extern struct acct pacct_rd_never_used;

#define COMM_LEN ((int) sizeof (pacct_rd_never_used.ac_comm))

void pacct_init PARAMS((int backwards));
void add_pacct_file PARAMS((char *name));
struct acct *pacct_get_entry PARAMS((void));
void print_pacct_record PARAMS((struct acct *rec, FILE *out));
void pacct_print_file_and_line PARAMS((FILE *out));

#ifdef HAVE_COMP_T
double comp_t_2_double PARAMS((comp_t));
#endif

#ifdef LINUX_MULTIFORMAT
double comp2_t_2_double PARAMS((comp2_t val));
size_t convert_acct_record_write PARAMS((struct acct *rec_in,
                                        struct acct *rec_out,
                                        int version, int byteswap));
#endif
