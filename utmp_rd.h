/* utmp_rd.h
 *
 * protos & etc. for routines that read from utmp files */

#include "config.h"		/* for HAVE_UT_HOST */

#include "files.h"

#ifdef HAVE_UT_ADDR
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>		/* for INET_NTOA */
#endif

/* This looks strange, but it's real ANSI C. */

static struct utmp utmp_rd_never_used;

#define NAME_LEN ((int) sizeof (utmp_rd_never_used.ut_name))
#define TTY_LEN ((int) sizeof (utmp_rd_never_used.ut_line))

#ifdef HAVE_UT_HOST
#define HOST_LEN ((int) sizeof (utmp_rd_never_used.ut_host))
#endif

#ifdef HAVE_UT_ID
#define ID_LEN ((int) sizeof (utmp_rd_never_used.ut_id))
#endif

#if 0
#ifdef __linux__		/* Debian bug#1651 reported by */
#define NAME_LEN 9		/* Austin Donnelly <and1000@cam.ac.uk> */
#else
#define NAME_LEN 8		/* length for usernames */
#endif

#define TTY_LEN 8		/* length for ttys */
#endif

#define OLD_TIME_CHAR '|'
#define NEW_TIME_CHAR '{'
#define BOOT_TIME_CHAR '~'

#ifdef HAVE_UT_TYPE

#define RECORD_TYPE ut_type

#ifndef UTMAXTYPE
#define UTMAXTYPE 8		/* FIXME: is there a better way to do
				   this? */
#endif

#else

#define RECORD_TYPE ut_line[0]

#define NEW_TIME NEW_TIME_CHAR
#define OLD_TIME OLD_TIME_CHAR
#define BOOT_TIME BOOT_TIME_CHAR

#endif

void utmp_init PARAMS((int backwards));
void add_utmp_file PARAMS((char *name));
struct utmp *utmp_get_entry PARAMS((void));
void print_utmp_record PARAMS((struct utmp *rec, FILE *out));
void utmp_print_file_and_line PARAMS((FILE *out));

#ifdef HAVE_UT_TYPE
int fix_ut_type_field PARAMS((struct utmp *rec));
#endif

int bad_utmp_record PARAMS((struct utmp *rec));
