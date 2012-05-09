#include <stdlib.h>
#include <assert.h>
#include "lhtable.h"

#include <stdio.h>

struct lhtable_record {
  uint64_t key;
  void *val;
  struct lhtable_record *tnext;
  struct lhtable_record *lnext;
  struct lhtable_record *lprev;
};

struct lhtable_s {
  struct lhtable_record **table;
  struct lhtable_record *record;
  struct lhtable_record *lfirst;
  struct lhtable_record *llast;
  struct lhtable_record *free;
  size_t capacity;
};

void lhtable_pt(lhtable_t *t) {
  struct lhtable_record *rec;
  int i;

  for (i=0; i<t->capacity; i++) {
    fprintf(stderr, "table[%d]: ", i);
    rec = t->table[i];
    while(rec) {
      fprintf(stderr, "%lld ",rec->key);
      rec = rec->tnext;
    }
    fprintf(stderr, "\n");
  }
}

void lhtable_pl(lhtable_t *t) {
  struct lhtable_record *rec;
  int i;

  for (i=0, rec=t->lfirst;
       i < t->capacity && rec;
       i++, rec = rec->lnext)
    fprintf(stderr, "%d<-|%d|->%d  ",
            rec->lprev ? rec->lprev->key : -1,
            rec->key,
            rec->lnext ? rec->lnext->key : -1);
  fprintf(stderr, "\n");
  if (rec)
    fprintf(stderr, "WARNING: cycle?\n");
}

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

lhtable_t *lhtable_new(size_t capacity) {
  int i;
  lhtable_t *t;

  t = calloc(1, sizeof(lhtable_t));
  if (!t)
    return NULL;

  t->record = malloc(capacity * sizeof(struct lhtable_record));
  if (!t->record) {
    free(t);
    return NULL;
  }

  t->table = calloc(capacity, sizeof(struct lhtable_record *));
  if (!t->table) {
    free(t->record);
    free(t);
    return NULL;
  }

  for (i=0; i<capacity; i++)
    t->record[i].tnext = &t->record[i+1];
  t->record[capacity-1].tnext = NULL;

  t->capacity = capacity;
  t->free = t->record;

  return t;
}

void lhtable_free(lhtable_t **t) {
  free((*t)->record);
  free((*t)->table);
  free(*t);
  *t = NULL;
}

int lhtable_set(lhtable_t *t, uint64_t key, void *val) {
  int h;
  struct lhtable_record *rec;

  /* grab a record from the free list */
  rec = t->free;
  if (!rec)
    return -1;
  t->free = rec->tnext;

  /* insert into hash table */
  h = hash64shift(key, t->capacity);
  rec->tnext = t->table[h];
  t->table[h] = rec;

  /* insert as head in list */
  if (t->lfirst)
    t->lfirst->lprev = rec;
  else
    t->llast = rec;
  rec->lprev = NULL;
  rec->lnext = t->lfirst;
  t->lfirst = rec;

  /* done */
  rec->key = key;
  rec->val = val;
  return 0;
}

/* Looks up record by key
 *
 * Sets tpos to table postion, rec to address of record and tprev to
 * address of the previous record in the table (i.e. _not_ previous in
 * list).
 *
 * Returns 0 on success,
 *         1 if key is not found
 */
static int lookup_record(lhtable_t *t, uint64_t key,
                         int *tpos,
                         struct lhtable_record **rec,
                         struct lhtable_record **tprev) {
  int h;
  struct lhtable_record *r, *p;

  h = hash64shift(key, t->capacity);
  r = t->table[h];
  p = NULL;

  while (r) {
    if (r->key == key) {
      if (tpos)  *tpos = h;
      if (rec)   *rec = r;
      if (tprev) *tprev = p;
      return 0;
    }

    p = r;
    r = r->tnext;
  }

  return 1;
}

/* Removes record from list
 *
 * The record will still remain the hash table. It will not be placed
 * on the free list.
 */
static void list_disconnect(lhtable_t *t, struct lhtable_record *rec) {
  if (rec->lprev)
    rec->lprev->lnext = rec->lnext;
  else
    t->lfirst = rec->lnext;
  if (rec->lnext)
    rec->lnext->lprev = rec->lprev;
  else
    t->llast = rec->lprev;
}

int lhtable_get(lhtable_t *t, uint64_t key, void **val) {
  int rc;
  struct lhtable_record *rec;

  rc = lookup_record(t, key, NULL, &rec, NULL);
  if (rc)
    return rc;

  *val = rec->val;

  return 0;
}

/* Retrieves and deletes entry by key
 *
 * Returns 0 on success
 *         1 if entry was not found
 */
int lhtable_pop(lhtable_t *t, uint64_t key, void **val) {
  int rc, h;
  struct lhtable_record *rec, *tprev;

  rc = lookup_record(t, key, &h, &rec, &tprev);
  if (rc)
    return rc;

  *val = rec->val;

  /* remove from table and list */
  if (tprev)
    tprev->tnext = rec->tnext;
  else
    t->table[h] = rec->tnext;
  list_disconnect(t, rec);

  /* add to free list */
  rec->tnext = t->free;
  t->free = rec;

  return 0;
}

int lhtable_del(lhtable_t *t, uint64_t key) {
  void *val;
  return lhtable_pop(t, key, &val);
}

int lhtable_get_newest(lhtable_t *t, uint64_t *key, void **val) {
  if (!t->lfirst)
    return -1;

  *key = t->lfirst->key;
  *val = t->lfirst->val;

  return 0;
}

int lhtable_get_oldest(lhtable_t *t, uint64_t *key, void **val) {
  if (!t->llast)
    return -1;

  *key = t->llast->key;
  *val = t->llast->val;

  return 0;
}

int lhtable_pop_newest(lhtable_t *t, uint64_t *key, void **val) {
  if (!t->lfirst)
    return -1;
  *key = t->lfirst->key;
  return lhtable_pop(t, *key, val);
}

int lhtable_pop_oldest(lhtable_t *t, uint64_t *key, void **val) {
  if (!t->llast)
    return -1;
  *key = t->llast->key;
  return lhtable_pop(t, *key, val);
}

int lhtable_make_newest(lhtable_t *t, uint64_t key) {
  int rc;
  struct lhtable_record *rec;

  rc = lookup_record(t, key, NULL, &rec, NULL);
  if (rc)
    return rc;

  list_disconnect(t, rec);

  /* set as first */
  rec->lprev = NULL;
  rec->lnext = t->lfirst;
  if (t->lfirst)
    t->lfirst->lprev = rec;
  else
    t->llast = rec;
  t->lfirst = rec;

  return 0;
}

int lhtable_make_oldest(lhtable_t *t, uint64_t key) {
  int rc;
  struct lhtable_record *rec;

  rc = lookup_record(t, key, NULL, &rec, NULL);
  if (rc)
    return rc;

  list_disconnect(t, rec);

  /* set as last */
  rec->lnext = NULL;
  rec->lprev = t->llast;
  if (t->llast)
    t->llast->lnext = rec;
  else
    t->lfirst = rec;
  t->llast = rec;

  return 0;
}

int lhtable_del_newest(lhtable_t *t) {
  if (!t->lfirst)
    return -1;
  return lhtable_del(t, t->lfirst->key);
}

int lhtable_del_oldest(lhtable_t *t) {
  if (!t->llast)
    return -1;
  return lhtable_del(t, t->llast->key);
}
