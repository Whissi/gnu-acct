/* dev_hash.c
 *
 * code that looks up device names from numbers
 *
 */

#include "config.h"

/* AIX requires this to be the first thing in the file.  */
#ifdef __GNUC__
# define alloca __builtin_alloca
#else
# ifdef HAVE_ALLOCA_H
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
#include <sys/utsname.h>
#include <pwd.h>

#include "common.h"
#include "files.h"
#include "hashtab.h"
#include "dev_hash.h"

#ifdef HAVE_DIRENT_H
# include <dirent.h>
# define NAMLEN(dirent) strlen((dirent)->d_name)
#else
# define dirent direct
# define NAMLEN(dirent) (dirent)->d_namlen
# ifdef HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# ifdef HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# ifdef HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif

#ifdef __linux__
#define NULLDEV 0
#else
#define NULLDEV -1
#endif

/* globals */

struct hashtab *dev_table = NULL;

struct dev_data
{
  char *name;			/* name of the device */
};

/* hash all possible /dev/pts devices as they only appear
 *  in the filesystem when the device is active.
 */

static void
setup_pts_devices ()
{

  struct utsname uts;
  struct dev_data dd;
  int i;
  struct pts_params *pts_ent, pts_table[] =
  {
    {"Linux", 136, 2048, 256},
    {NULL}
  };

  if ( uname (&uts) )
    {
      /* silent error */
      return;
    }

  for (pts_ent = &(pts_table[0]); pts_ent != NULL; ++pts_ent)
    {
      if (!strcmp (uts.sysname, pts_ent->utsname))
        break;
    }
  if (pts_ent == NULL) return; /* unsupported OS */

  for (i = 0; i < pts_ent->max; ++i)
    {
      long dev_num;
      struct hashtab_elem *he;
      int major, minor;

      major = pts_ent->base + (i/pts_ent->mod);
      minor = i % pts_ent->mod;
      dev_num = ((major << 8) + minor);

      dd.name = xmalloc (sizeof(char) * (strlen("pts/xxxx") + 1));
      sprintf (dd.name, "pts/%d", i);

      he = hashtab_create (dev_table, (void *) & dev_num, sizeof(dev_num));
      hashtab_set_value (he, &dd, sizeof (dd));
    }
}

/* Read the DIRNAME directory entries and make a linked list of ttys
   (so we can search through it later) */

static void setup_devices(char *dirname)
{
  DIR *dirp;			/* the directory itself */
  struct dirent *dp;		/* directory entry structure */
  struct stat sp;		/* file information structure */
  extern int debugging_enabled;

  if (debugging_enabled)
    (void)fprintf (stddebug, "device     devnum\n-----------------\n");

  if ((dirp = opendir (dirname)) == NULL)
    return;			/* skip it silently */

  if (!strcmp (dirname, "/dev/pts"))
    {
      /* assuming SysV, these are dynamically allocated */
      closedir (dirp);
      setup_pts_devices ();
      return;
    }

  for (dp = readdir (dirp); dp != NULL; dp = readdir (dirp))
    {
      char *fullname = (char *) alloca ((strlen (dirname)
                                         + NAMLEN (dp)
                                         + 1) * sizeof (char));

      (void)sprintf (fullname, "%s/%s", dirname, dp->d_name);
      if (stat (fullname, &sp))
        continue;

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
        (void)strcpy (dd.name, dp->d_name);

        he = hashtab_create (dev_table, (void *) &dev_num, sizeof (dev_num));
        hashtab_set_value (he, &dd, sizeof (dd));
      }
    }
  (void)closedir (dirp);
}


/* Return the name of the device associated with DEV_NUM.  The
   argument passed was originally a dev_t, but that data type is too
   limited on some systems (won't let us pass -1 because it's an
   unsigned short or other). */

char *dev_gnu_name(long dev_num)
{
  struct hashtab_elem *he;
  static char devstr [20];

  /* special case */

  if (dev_num == NULLDEV)
    return "__";

  if (dev_table == NULL)
    {
      dev_table = hashtab_init (sizeof (long));
      setup_devices ("/dev");	   /* most certainly */
      setup_devices ("/dev/pty");  /* perhaps */
      setup_devices ("/dev/ptym"); /* perhaps */
      /* This will catch only currently active devices, so we prefer
       * to calculate the device name manually if we have the major. */
      setup_devices ("/dev/pts");  /* perhaps */
    }

  he = hashtab_find (dev_table, (void *) &dev_num, (unsigned int)sizeof (dev_num));

  if (he != NULL)
    {
      struct dev_data *dd = hashtab_get_value (he);
      return dd->name;
    }
#ifdef UNIX98_PTY_SLAVE_MAJOR
  if ((dev_num >> 8) == UNIX98_PTY_SLAVE_MAJOR)
    {
      snprintf(dev_name, sizeof(dev_name), "pts/%ld", dev_num & 0xff);
      return dev_name;
    }
#endif


  /* didn't find it, return it as [maj,min] */

  sprintf (devstr, "[%d,%d]",  /* is this portable? */
           (int) ((dev_num & 0xFF00) >> 8), (int) (dev_num & 0x00FF));

  return devstr;
}
