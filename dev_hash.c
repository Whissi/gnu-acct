/* dev_hash.c
 *
 * code that looks up device names from numbers */

#include "config.h"

/* AIX requires this to be the first thing in the file.  */
#ifdef __GNUC__
# define alloca __builtin_alloca
#else
# if HAVE_ALLOCA_H
#  include <alloca.h>
# else
#  ifdef _AIX
 #pragma alloca
#  else
#   ifndef alloca /* predefined by HP cc +Olibcalls */
char *alloca ();
#   endif
#  endif
# endif
#endif

#include <stdio.h>

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>

#include "common.h"
#include "hashtab.h"
#include "dev_hash.h"

#if HAVE_DIRENT_H
# include <dirent.h>
# define NAMLEN(dirent) strlen((dirent)->d_name)
#else
# define dirent direct
# define NAMLEN(dirent) (dirent)->d_namlen
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# if HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif


/* globals */

struct hashtab *dev_table = NULL;

struct dev_data {
  char *name;			/* name of the device */
};


/* Read the DIRNAME directory entries and make a linked list of ttys
   (so we can search through it later) */

static
void
setup_devices (char *dirname)
{
  DIR *dirp;			/* the directory itself */
  struct dirent *dp;		/* directory entry structure */
  struct stat sp;		/* file information structure */
  extern int debugging_enabled;

  if (debugging_enabled)
    fprintf (stddebug, "device     devnum\n-----------------\n");

  if ((dirp = opendir (dirname)) == NULL)
    return;			/* skip it silently */
  
  for (dp = readdir (dirp); dp != NULL; dp = readdir (dirp))
    {
      char *fullname = (char *) alloca ((strlen (dirname)
					 + NAMLEN (dp)
					 + 1) * sizeof (char));

      sprintf (fullname, "%s/%s", dirname, dp->d_name);
      stat (fullname, &sp);
      
      if ((sp.st_mode & S_IFMT) != S_IFCHR)     /* skip if not a tty */
	continue;
      
      if (debugging_enabled)
	fprintf (stddebug, "%-8.8d %s\n", (int) sp.st_rdev, dp->d_name);

      /* entry OK, so put it into the list */

      {
	struct hashtab_elem *he;
	struct dev_data dd;
	long dev_num;

	dev_num = sp.st_rdev;
	dd.name = (char *) xmalloc (sizeof (char) * (NAMLEN (dp) + 1));
	strcpy (dd.name, dp->d_name);
	
	he = hashtab_create (dev_table, (void *) &dev_num, sizeof (dev_num));
	hashtab_set_value (he, &dd, sizeof (dd));
      }
    }
  closedir (dirp);
}


/* Return the name of the device associated with DEV_NUM.  The
   argument passed was originally a dev_t, but that data type is too
   limited on some systems (won't let us pass -1 because it's an
   unsigned short or other). */

char *
devname (long dev_num)
{
  struct hashtab_elem *he;

  /* special case */

  if (dev_num == -1)
    return "__";

  if (dev_table == NULL)
    {
      dev_table = hashtab_init (sizeof (long));
      setup_devices ("/dev");	   /* most certainly */
      setup_devices ("/dev/pty");  /* perhaps */
      setup_devices ("/dev/ptym"); /* perhaps */
    }
  
  he = hashtab_find (dev_table, (void *) &dev_num, sizeof (dev_num));
  
  if (he != NULL)
    {
      struct dev_data *dd = hashtab_get_value (he);
      return dd->name;
    }
  
  /* didn't find it */
  
  return "??";
}

