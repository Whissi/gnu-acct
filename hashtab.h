/* hashtab.h
 *
 * generic hash table routines */

struct hashtab_order {
  int which;
  struct hashtab_elem *elem;
  struct hashtab *ht;
};

struct hashtab_elem {
  void *key;
  unsigned int key_len;
  unsigned int hashval;
  void *data;
  struct hashtab_elem *prev, *next;
  struct hashtab *ht;		/* so we don't have to pass the table
				   for all of those data manipulation
				   calls... */
};

struct hashtab {
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
void hashtab_set_value (struct hashtab_elem *he, void *v, unsigned int len);
struct hashtab_elem *hashtab_first (struct hashtab *ht,
				    struct hashtab_order *ho);
struct hashtab_elem *hashtab_next (struct hashtab_order *ho);
void hashtab_dump_keys (struct hashtab *ht, FILE *out);
void hashtab_delete (struct hashtab_elem *he);