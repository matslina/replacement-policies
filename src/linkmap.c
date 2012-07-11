#include <stdlib.h>
#include "linkmap.h"

#define MAX(a,b) ((a) > (b) ? (a) : (b))


typedef struct linkmap_entry_s linkmap_entry_t;

struct linkmap_entry_s {
  uint64_t key;
  void *val;
  linkmap_entry_t *tnext;
  linkmap_entry_t *lnext;
  linkmap_entry_t *lprev;
};

struct linkmap_s {
  linkmap_entry_t **table;
  linkmap_entry_t *entry;
  linkmap_entry_t *lfirst;
  linkmap_entry_t *llast;
  linkmap_entry_t *free;
  size_t capacity;
  size_t size;
};


/* Thomas Wang's hash64shift()
 */
static int hash64shift(uint64_t k, int mod) {
  k = (~k) + (k << 21);
  k = k ^ (k >> 24);
  k = (k + (k << 3)) + (k << 8);
  k = k ^ (k >> 14);
  k = (k + (k << 2)) + (k << 4);
  k = k ^ (k >> 28);
  k = k + (k << 31);

  return k % mod;
}

/* Scans an entry's table list for a key. Returns 0 if found, 1
 * otherwise. The entry and its previous entry in the table list are
 * writtenback to entry and prev on success.
 */
static int table_scan(linkmap_entry_t *list, uint64_t key,
                      linkmap_entry_t **entry,
                      linkmap_entry_t **prev) {
  linkmap_entry_t *c = list, *p = NULL;

  while (c) {
    if (c->key == key) {
      *entry = c;
      *prev = p;
      return 0;
    }
    p = c;
    c = c->tnext;
  }

  return 1;
}

/* Removes entry from the linked list. Note that the entry will _not_
 * be removed from the hash table.
 */
static void unlink(linkmap_t *lm, linkmap_entry_t *entry) {
  if (entry->lprev)
    entry->lprev->lnext = entry->lnext;
  else
    lm->lfirst = entry->lnext;
  if (entry->lnext)
    entry->lnext->lprev = entry->lprev;
  else
    lm->llast = entry->lprev;
}

linkmap_t *linkmap_new(size_t capacity) {
  int i;
  linkmap_t *lm;
  linkmap_entry_t *entry;
  linkmap_entry_t **table;

  capacity = MAX(capacity, 1);

  lm = calloc(1, sizeof(linkmap_t));
  entry = malloc(capacity * sizeof(linkmap_entry_t));
  table = calloc(capacity, sizeof(linkmap_entry_t *));

  if (!lm || !entry || !table) {
    free(lm);
    free(entry);
    free(table);
    return NULL;
  }

  lm->entry = entry;
  lm->table = table;
  lm->capacity = capacity;
  lm->size = 0;

  /* create the list of unused entries */
  lm->free = lm->entry;
  for (i=0; i<capacity; i++)
    lm->entry[i].tnext = &lm->entry[i+1];
  lm->entry[capacity-1].tnext = NULL;

  return lm;
}

void linkmap_free(linkmap_t **lm) {
  if (!lm || !*lm)
    return;
  free((*lm)->entry);
  free((*lm)->table);
  free(*lm);
  *lm = NULL;
}

size_t linkmap_size(linkmap_t *lm) {
  return lm->size;
}

int linkmap_set(linkmap_t *lm, uint64_t key, void *val) {
  int h;
  linkmap_entry_t *entry, *prev;

  h = hash64shift(key, lm->capacity);

  /* replace value if key already exists*/
  if (!table_scan(lm->table[h], key, &entry, &prev)) {
    entry->val = val;
    return 0;
  }

  /* otherwise grab an entry from the free list */
  if (!lm->free)
    return 1;
  entry = lm->free;
  lm->free = entry->tnext;

  /* insert it into the hash table */
  entry->tnext = lm->table[h];
  lm->table[h] = entry;

  /* insert as head in list */
  if (lm->lfirst)
    lm->lfirst->lprev = entry;
  else
    lm->llast = entry;
  entry->lprev = NULL;
  entry->lnext = lm->lfirst;
  lm->lfirst = entry;

  lm->size++;
  entry->key = key;
  entry->val = val;

  return 0;
}

int linkmap_get(linkmap_t *lm, uint64_t key, void **val) {
  int h;
  linkmap_entry_t *entry, *prev;

  h = hash64shift(key, lm->capacity);
  if (!table_scan(lm->table[h], key, &entry, &prev)) {
    *val = entry->val;
    return 0;
  }

  return 1;
}

int linkmap_get_head(linkmap_t *lm, uint64_t *key, void **val) {
  if (!lm->lfirst)
    return 1;
  *key = lm->lfirst->key;
  *val = lm->lfirst->val;
  return 0;
}

int linkmap_get_tail(linkmap_t *lm, uint64_t *key, void **val) {
  if (!lm->llast)
    return 1;
  *key = lm->llast->key;
  *val = lm->llast->val;
  return 0;
}

int linkmap_pop(linkmap_t *lm, uint64_t key, void **val) {
  int h;
  linkmap_entry_t *entry, *tprev;

  /* find the entry */
  h = hash64shift(key, lm->capacity);
  if (table_scan(lm->table[h], key, &entry, &tprev))
    return 1;

  /* disconnect from table and list */
  if (tprev)
    tprev->tnext = entry->tnext;
  else
    lm->table[h] = entry->tnext;
  unlink(lm, entry);

  /* put back the entry on the free list */
  entry->tnext = lm->free;
  lm->free = entry;

  lm->size--;
  *val = entry->val;

  return 0;
}

int linkmap_pop_head(linkmap_t *lm, uint64_t *key, void **val) {
  if (!lm->lfirst)
    return 1;
  *key = lm->lfirst->key;
  return linkmap_pop(lm, *key, val);
}

int linkmap_pop_tail(linkmap_t *lm, uint64_t *key, void **val) {
  if (!lm->llast)
    return 1;
  *key = lm->llast->key;
  return linkmap_pop(lm, *key, val);
}

int linkmap_del(linkmap_t *lm, uint64_t key) {
  void *ignval;
  return linkmap_pop(lm, key, &ignval);
}

int linkmap_del_head(linkmap_t *lm) {
  void *ignval;
  uint64_t ignkey;
  return linkmap_pop_head(lm, &ignkey, &ignval);
}

int linkmap_del_tail(linkmap_t *lm) {
  void *ignval;
  uint64_t ignkey;
  return linkmap_pop_tail(lm, &ignkey, &ignval);
}
