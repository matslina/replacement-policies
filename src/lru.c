#include <stdlib.h>
#include <assert.h>
#include "lrutable.h"
#include "lru.h"

#include <stdio.h>

struct lru_s {
  lrutable_t *t;
  size_t size;
  size_t nmemb;
  size_t active;
  void *data;
};

lru_t *lru_new(size_t size, size_t nmemb) {
  lru_t *lru;

  assert(nmemb >= 2);

  lru = malloc(sizeof(lru_t));
  if (!lru)
    return NULL;

  lru->t = lrutable_new(nmemb);
  if (!lru->t) {
    free(lru);
    return NULL;
  }

  lru->data = malloc(nmemb * size);
  if (!lru->data) {
    free(lru->t);
    free(lru);
    return NULL;
  }

  lru->size = size;
  lru->nmemb = nmemb;
  lru->active = 0;

  return lru;
}

int lru_fetch(lru_t *lru, uint64_t key, void **ptr) {
  void *data, *val;
  uint64_t k;

  /* hit cache */
  if (!lrutable_get(lru->t, key, ptr))
    return 0;

  /* create new page if possible, evict LRU otherwise */
  if (lru->active < lru->nmemb) {
    val = lru->data + lru->active * lru->size;
    lru->active++;
  } else {
    lrutable_pop_lru(lru->t, &k, &val);
  }

  /* insert as LRU */
  lrutable_set(lru->t, key, val);

  *ptr = val;

  return 1;
}

void lru_free(lru_t **lru) {
  free((*lru)->data);
  lrutable_free(&(*lru)->t);
  free(*lru);
  *lru = NULL;
}
