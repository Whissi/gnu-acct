/* uid_hash.c
 *
 * routines used by lastcomm and sa to hash data by uids
 */

#include "config.h"

#include <stdio.h>
#include <pwd.h>

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include "common.h"
#include "hashtab.h"
#include "uid_hash.h"


/* globals */

struct hashtab *uid_table = NULL;

struct uid_data {
  char *name;			/* name of the user */
};


/* code */

/* look up UID in the hash table of uids.  If it's not there, get it
 * from the password database and add it to the hash table.
 */
char *
uid_name (int uid)
{
  struct hashtab_elem *he;

  if (uid_table == NULL)
    uid_table = hashtab_init (sizeof (int));

  he = hashtab_find (uid_table, (void *) &uid, sizeof (uid));

  if (he == NULL)
    {
      /* It wasn't there, so add it. */

      struct passwd *thispw = getpwuid (uid);
      struct uid_data ud;

      if (thispw != NULL)
	{
	  ud.name = (char *) xmalloc (sizeof (char)
				       * (strlen (thispw->pw_name) + 1));
	  strcpy (ud.name, thispw->pw_name);
	}
      else
	{
	  int digits, uid_copy = uid;

	  /* Count the number of digits we'll need. */
	  for (digits = 2; uid_copy; digits++, uid_copy /= 10)
	    ;

	  ud.name = (char *) xmalloc (sizeof (char) * digits);
	  sprintf (ud.name, "%d", uid);
	}
      
      he = hashtab_create (uid_table, (void *) &uid, sizeof (uid));
      hashtab_set_value (he, &ud, sizeof (ud));
    }

  {
    struct uid_data *ud = hashtab_get_value (he);
    return ud->name;
  }
}
