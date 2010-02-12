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

/* hashtab.h
 *
 * generic hash table routines
 *
 */

#ifndef HASHTAB_H
#define HASHTAB_H

struct hashtab_order
{
  int which;
  struct hashtab_elem *elem;
  struct hashtab *ht;
};

struct hashtab_elem
{
  void *key;
  unsigned int key_len;
  unsigned int hashval;
  void *data;
  struct hashtab_elem *prev, *next;
  struct hashtab *ht;		/* so we don't have to pass the table
				   for all of those data manipulation
				   calls... */
};

struct hashtab
{
  unsigned long table_size;	/* number of buckets */
  unsigned long items_hashed;	/* number of items in the table */
  unsigned long resize_size;	/* num items hashed after which we
				   should increase the number of
				   buckets */
  struct hashtab_elem **table;
  int numeric;			/* If nonzero, this is a numeric hash
				   table, so the length of items
				   passed shouldn't be truncated.
				   This is also doubles as the size of
				   the data to be added. */
};

struct hashtab *hashtab_init (int numeric);
struct hashtab_elem *hashtab_create (struct hashtab *ht, void *key,
                                     unsigned int len);
struct hashtab_elem *hashtab_find (struct hashtab *ht, void *key,
                                   unsigned int len);
void *hashtab_get_key (struct hashtab_elem *he);
void *hashtab_get_value (struct hashtab_elem *he);
void hashtab_set_value (struct hashtab_elem *he,
                        void *v, unsigned int len);
struct hashtab_elem *hashtab_first (struct hashtab *ht,
                                    struct hashtab_order *ho);
struct hashtab_elem *hashtab_next (struct hashtab_order *ho);
void hashtab_dump_keys (struct hashtab *ht, FILE *out);
void hashtab_delete (struct hashtab_elem *he);

#endif /* HASHTAB_H */
