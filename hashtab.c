/* hashtab.c
 *
 * generic hash table routines */

#include "config.h"

#include <stdio.h>

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include "common.h"
#include "hashtab.h"

#define INITIAL_TABLE_SIZE 7	/* number of buckets */
#define RESIZE_MULTIPLIER 4	/* grow table by this factor */
#define RESIZE_SIZE_MULTIPLIER 0.6 /* grow table when it's 60% full */

/* A decent string hash function */

static
unsigned long
hash (const char *s, unsigned int len, unsigned long hashsize)
{
  unsigned long hashval;

  for (hashval = 0; len; s++, len--)
    hashval = *s + 31 * hashval;
  return hashval % hashsize;
}


/* Create and return a new hash table.  If NUMERIC is non-zero, items
   to be hashed are numeric and shouldn't be truncated like
   strings. */

struct hashtab *
hashtab_init (int numeric)
{
  struct hashtab *new;

  new = (struct hashtab *) xmalloc (sizeof (struct hashtab));
  memset (new, 0, sizeof (struct hashtab));
  new->numeric = numeric;
  
  return new;
}


/* Return the smaller of the length of a string or the number passed
   in.  If 0 is passed, return the string length unconditionally. */

static
unsigned int
get_key_len (char *s, unsigned int len, int numeric)
{
  if (numeric)
    {
      if (len == 0)
	fatal ("hashtab.c (get_key_len): hashing a num with len == 0");
      if (len != numeric)
	fatal ("hashtab.c (get_key_len): hashing a num with len == numeric");
      return len;
    }
  else
    {
      if (!len)
	return strlen (s);
      else
	{
	  unsigned int i;
	  for (i = len; i && (*s != '\0'); s++, i--)
	    ;
	  return len - i;
	}
    }
}


/* When the number of entries gets too big for efficient use of the
   hash table, resize it. */

static
void
hashtab_resize (struct hashtab *ht)
{
  unsigned long old_size, bucket;
  struct hashtab_elem **old_table, *elt;

  /* Save the old values. */

  old_size = ht->table_size;
  old_table = ht->table;

  /* Resize and allocate a new table. */

  ht->table_size *= RESIZE_MULTIPLIER;
  ht->resize_size = ht->table_size * RESIZE_SIZE_MULTIPLIER;
  ht->table = (struct hashtab_elem **)
    xmalloc (sizeof (struct hashtab_elem *) * ht->table_size);
  memset (ht->table, 0, sizeof (struct hashtab_elem *) * ht->table_size);
  
  /* Step through each key in the old table, re-hashing and inserting
     into the new table.  We don't use the HASHTAB_FIRST and
     HASHTAB_CREATE routines because we can do this more efficiently,
     knowing that each element is unique (don't need check the old
     ones in a bin on insert). */

  for (bucket = 0; bucket < old_size; bucket++)
    for (elt = old_table[bucket]; elt != NULL;)
      {
	struct hashtab_elem *old_next = elt->next;

	elt->hashval = hash (elt->key, elt->key_len, ht->table_size);
	elt->prev = NULL;
	elt->next = ht->table[elt->hashval];
	if (ht->table[elt->hashval])
	  ht->table[elt->hashval]->prev = elt;
	ht->table[elt->hashval] = elt;

	elt = old_next;
      }
  
  /* Remove the old table. */

  free (old_table);
}


/* Create an entry for the given key.  If the key already exists,
   return the existing entry with the data cleared.  If LEN is 0,
   assume that the string is null-terminated.  Otherwise, only use LEN
   bytes. */

struct hashtab_elem *
hashtab_create (struct hashtab *ht, void *key, unsigned int len)
{
  unsigned long hashval;
  unsigned int key_len;
  struct hashtab_elem *he;
  
  if (ht->table == NULL)
    {
      /* Create a fresh table. */
      ht->table_size = INITIAL_TABLE_SIZE;
      ht->items_hashed = 0;
      ht->resize_size = ht->table_size * RESIZE_SIZE_MULTIPLIER;
      ht->table = (struct hashtab_elem **)
	xmalloc (sizeof (struct hashtab_elem *) * ht->table_size);
      memset (ht->table, 0, sizeof (struct hashtab_elem *) * ht->table_size);
    }

  /* Hash the key. */

  key_len = get_key_len (key, len, ht->numeric);
  hashval = hash (key, key_len, ht->table_size);

  /* We could use HASHTAB_FIND, but then we'd have to call STRLEN
     twice & etc. */

  for (he = ht->table[hashval]; he != NULL; he = he->next)
    {
      if ((he->key_len == key_len) && (memcmp (he->key, key, key_len) == 0))
	{
	  /* Clear the existing data and return the entry. */
	  he->data = NULL;
	  return he;
	}
    }

  /* Create a new entry, since we didn't find one. */

  he = (struct hashtab_elem *) xmalloc (sizeof (struct hashtab_elem));
  he->key_len = key_len;
  he->key = (char *) xmalloc (sizeof (char) * (he->key_len + 1));
  memcpy (he->key, key, key_len);

  /* Make sure the key is null-terminated -- this won't hurt if we're
     using a numeric hash value, since the extra zero will never get
     considered. */

  ((char *) he->key)[he->key_len] = '\0';
  he->ht = ht;
  he->prev = NULL;
  he->hashval = hashval;
  he->next = ht->table[hashval];
  if (ht->table[hashval])
    ht->table[hashval]->prev = he;
  ht->table[hashval] = he;

  ht->items_hashed++;

  /* Are there too many items in the table? */

  if (ht->items_hashed > ht->resize_size)
    hashtab_resize (ht);

  return he;
}


/* Find KEY in HT and return the entry associated with it.  If LEN is
   0, assume that the string is null-terminated.  Otherwise, only use
   LEN bytes. */

struct hashtab_elem *
hashtab_find (struct hashtab *ht, void *key, unsigned int len)
{
  unsigned long hashval;
  unsigned int key_len;
  struct hashtab_elem *he;
  
  if (ht->table == NULL)
    return NULL;

  key_len = get_key_len (key, len, ht->numeric);
  hashval = hash (key, key_len, ht->table_size);
  
  for (he = ht->table[hashval]; he != NULL; he = he->next)
    if ((he->key_len == key_len) && (memcmp (he->key, key, key_len) == 0))
      return he;

  return NULL;
}


/* Return the key associated with HE. */

void *
hashtab_get_key (struct hashtab_elem *he)
{
  return he->key;
}


/* Return the data associated with HE. */

void *
hashtab_get_value (struct hashtab_elem *he)
{
  return he->data;
}


/* Set the data for HE. */

void
hashtab_set_value (struct hashtab_elem *he, void *v, unsigned int len)
{
  he->data = (void *) xmalloc (len);
  memcpy (he->data, v, len);
}


/* Return the first thing in the table. */

struct hashtab_elem *
hashtab_first (struct hashtab *ht, struct hashtab_order *ho)
{
  ho->which = 0;
  ho->elem = NULL;
  ho->ht = ht;

  return hashtab_next (ho);
}


/* Given HO, return the next entry in the hash table. */

struct hashtab_elem *
hashtab_next (struct hashtab_order *ho)
{
  unsigned long i;
  struct hashtab_elem *he;

  if (ho->elem)
    {
      he = ho->elem;
      ho->elem = he->next;
      if (he->next == NULL)
	ho->which++;
      return he;
    }

  for (i = ho->which; i < ho->ht->table_size; i++)
    for (he = ho->ht->table[i]; he != NULL; he = he->next)
      {
	ho->which = (he->next ? i : i + 1);
	ho->elem = he->next;
	return he;
      }

  return NULL;			/* nothing in the table */
}


void
hashtab_dump_keys (struct hashtab *ht, FILE *out)
{
  struct hashtab_order ho;
  struct hashtab_elem *he;
  
  for (he = hashtab_first (ht, &ho);
       he != NULL;
       he = hashtab_next (&ho))
    fprintf (stddebug, "%s (%d)\n", (char *) he->key, he->key_len);
}


/* Delete the given element from the hash table.  We want to be able
   to call this in between calls to HASHTAB_NEXT so we can use the
   obvious way to delete items from the table. */

void
hashtab_delete (struct hashtab_elem *he)
{
  he->ht->items_hashed--;

  if (he->prev)
    he->prev->next = he->next;
  else
    he->ht->table[he->hashval] = he->next;

  if (he->next)
    he->next->prev = he->prev;

  if (he->data)
    free (he->data);
  free (he->key);
  free (he);
}
