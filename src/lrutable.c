#include <stdlib.h>
#include <assert.h>
#include "lrutable.h"

#include <stdio.h>

struct lrutable_record {
  uint64_t key;
  void *val;
  struct lrutable_record *tnext;
  struct lrutable_record *lnext;
  struct lrutable_record *lprev;
};

struct lrutable_s {
  struct lrutable_record **table;
  struct lrutable_record *record;
  struct lrutable_record *lfirst;
  struct lrutable_record *llast;
  struct lrutable_record *free;
  size_t capacity;
};

void lrutable_pt(lrutable_t *t) {
  struct lrutable_record *rec;
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

void lrutable_pl(lrutable_t *t) {
  struct lrutable_record *rec;
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

lrutable_t *lrutable_new(size_t capacity) {
  int i;
  lrutable_t *t;

  t = calloc(1, sizeof(lrutable_t));
  if (!t)
    return NULL;

  t->record = malloc(capacity * sizeof(struct lrutable_record));
  if (!t->record) {
    free(t);
    return NULL;
  }

  t->table = calloc(capacity, sizeof(struct lrutable_record *));
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

void lrutable_free(lrutable_t **t) {
  free((*t)->record);
  free((*t)->table);
  free(*t);
  *t = NULL;
}

int lrutable_set(lrutable_t *t, uint64_t key, void *val) {
  int h;
  struct lrutable_record *rec;

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
static int lookup_record(lrutable_t *t, uint64_t key,
                         int *tpos,
                         struct lrutable_record **rec,
                         struct lrutable_record **tprev) {
  int h;
  struct lrutable_record *r, *p;

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
static void list_disconnect(lrutable_t *t, struct lrutable_record *rec) {
  if (rec->lprev)
    rec->lprev->lnext = rec->lnext;
  else
    t->lfirst = rec->lnext;
  if (rec->lnext)
    rec->lnext->lprev = rec->lprev;
  else
    t->llast = rec->lprev;
}

int lrutable_get(lrutable_t *t, uint64_t key, void **val) {
  int rc;
  struct lrutable_record *rec;

  rc = lookup_record(t, key, NULL, &rec, NULL);
  if (rc)
    return rc;

  /* make record MRU */
  list_disconnect(t, rec);
  rec->lprev = NULL;
  rec->lnext = t->lfirst;
  if (t->lfirst)
    t->lfirst->lprev = rec;
  else
    t->llast = rec;
  t->lfirst = rec;

  *val = rec->val;

  return 0;
}

int lrutable_pop(lrutable_t *t, uint64_t key, void **val) {
  int rc, h;
  struct lrutable_record *rec, *tprev;

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

int lrutable_del(lrutable_t *t, uint64_t key) {
  void *val;
  return lrutable_pop(t, key, &val);
}

int lrutable_get_lru(lrutable_t *t, uint64_t *key, void **val) {
  if (!t->llast)
    return -1;

  *key = t->llast->key;
  *val = t->llast->val;

  return 0;
}

int lrutable_pop_lru(lrutable_t *t, uint64_t *key, void **val) {
  if (!t->llast)
    return -1;
  *key = t->llast->key;
  return lrutable_pop(t, *key, val);
}

int lrutable_del_lru(lrutable_t *t) {
  if (!t->llast)
    return -1;
  return lrutable_del(t, t->llast->key);
}
