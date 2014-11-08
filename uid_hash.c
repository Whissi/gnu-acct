/* uid_hash.c
 *
 * routines used by lastcomm and sa to hash data by uids
 *
 */

#include "config.h"

#include <stdio.h>
#include <pwd.h>
#include <grp.h>

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include "common.h"
#include "files.h"
#include "hashtab.h"
#include "uid_hash.h"

/* globals */

struct hashtab *uid_table = NULL;
struct hashtab *gid_table = NULL;

struct uid_data
{
  char *name;			/* name of the user */
};

struct gid_data
{
  char *name;			/* name of the user */
};


/* code */

/* look up UID in the hash table of uids.  If it's not there, get it
 * from the password database and add it to the hash table.
 */

char *uid_name(int uid)
{
  struct hashtab_elem *he;

  if (uid_table == NULL)
    uid_table = hashtab_init(sizeof(int));

  he = hashtab_find(uid_table, (void *) &uid, sizeof(uid));

  if (he == NULL)
    {
      /* It wasn't there, so add it. */

      struct passwd *thispw = getpwuid (uid);
      struct uid_data ud;

      if (thispw != NULL)
        {
          ud.name = (char *) xmalloc(sizeof (char)* (strlen (thispw->pw_name) + 1));
          (void)strcpy(ud.name, thispw->pw_name);
        }
      else
        {
          int digits = 2, uid_copy = uid;

          /* Count the number of digits we'll need. */
          for (; uid_copy; digits++, uid_copy /= 10);

          ud.name = (char *) xmalloc (sizeof (char) * digits);
          (void)sprintf(ud.name, "%d", uid);
        }

      he = hashtab_create(uid_table, (void *) &uid, sizeof (uid));
      hashtab_set_value(he, &ud, sizeof (ud));
    }

  struct uid_data *ud = hashtab_get_value(he);
  return ud->name;
}

char *gid_name(int gid)
{
  struct hashtab_elem *he;

  if (gid_table == NULL)
    gid_table = hashtab_init(sizeof(int));

  he = hashtab_find(gid_table, (void *) &gid, sizeof(gid));

  if (he == NULL)
    {
      /* It wasn't there, so add it. */
      struct group *grp;
      struct gid_data gd;

      if ((grp = getgrgid(gid)) != NULL)
      {
          gd.name = (char *) xmalloc(sizeof (char)* (strlen (grp->gr_name) + 1));
          (void)strcpy(gd.name, grp->gr_name);
      }
      else
        {
          int digits = 2, gid_copy = gid;

          /* Count the number of digits we'll need. */
          for (; gid_copy; digits++, gid_copy /= 10);

          gd.name = (char *) xmalloc (sizeof (char) * digits);
          (void)sprintf(gd.name, "%d", gid);
        }

      he = hashtab_create(gid_table, (void *) &gid, sizeof (gid));
      hashtab_set_value(he, &gd, sizeof (gd));
    }

  struct gid_data *gd = hashtab_get_value(he);
  return gd->name;
}


