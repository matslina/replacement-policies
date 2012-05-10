#include <stdlib.h>
#include <assert.h>
#include "htable.h"

#include <stdio.h>

typedef uint64_t htable_key_t;
typedef void  *  htable_val_t;

struct htable_record {
  htable_key_t key;
  htable_val_t val;
  struct htable_record *next;
};

struct htable_s {
  struct htable_record **table;
  struct htable_record *record;
  struct htable_record *free;
  size_t capacity;
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

void pt(htable_t *t) {
  struct htable_record *rec;
  int i;

  for (i=0; i<t->capacity; i++) {
    fprintf(stderr, "table[%d]: ", i);
    rec = t->table[i];
    while(rec) {
      fprintf(stderr, "%lld ",rec->key);
      rec = rec->next;
    }
    fprintf(stderr, "\n");
  }
}

htable_t *htable_new(size_t capacity) {
  int i;
  htable_t *htable;

  htable = calloc(1, sizeof(htable_t));
  if (!htable)
    return NULL;

  htable->record = malloc(capacity * sizeof(struct htable_record));
  if (!htable->record) {
    free(htable);
    return NULL;
  }

  htable->table = calloc(capacity, sizeof(struct htable_record *));
  if (!htable->table) {
    free(htable->record);
    free(htable);
    return NULL;
  }

  for (i=0; i<capacity; i++)
    htable->record[i].next = &htable->record[i+1];
  htable->record[capacity-1].next = NULL;

  htable->free = &htable->record[0];
  htable->capacity = capacity;

  return htable;
}

int htable_set(htable_t *htable, uint64_t key, void *val) {
  int h;
  struct htable_record *rec;

  rec = htable->free;
  if (!rec)
    return -1;

  htable->free = rec->next;

  h = hash64shift(key, htable->capacity);

  rec->next = htable->table[h];
  htable->table[h] = rec;
  rec->key = key;
  rec->val = val;

  return 0;
}

int htable_get(htable_t *htable, uint64_t key, void **val) {
  int h;
  struct htable_record *rec;

  h = hash64shift(key, htable->capacity);

  rec = htable->table[h];

  while (rec) {
    if (rec->key == key) {
      *val = rec->val;
      return 0;
    }
    rec = rec->next;
  }

  return 1;
}

int htable_pop(htable_t *htable, uint64_t key, void **val) {
  int h;
  struct htable_record *rec, *prev;

  h = hash64shift(key, htable->capacity);

  prev = NULL;
  rec = htable->table[h];

  while (rec) {
    if (rec->key == key) {
      if (prev)
        prev->next = rec->next;
      else
        htable->table[h] = rec->next;
      rec->next = htable->free;
      htable->free = rec;

      *val = rec->val;
      return 0;
    }

    prev = rec;
    rec = rec->next;
  }

  return 1;
}

int htable_del(htable_t *htable, uint64_t key) {
  void *val;

  return htable_pop(htable, key, &val);
}

 void htable_free(htable_t **htable) {
   free((*htable)->record);
   free((*htable)->table);
   free(*htable);
   *htable = NULL;
 }
