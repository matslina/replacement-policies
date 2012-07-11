#include <stdlib.h>
#include <assert.h>
#include "linkmap.h"
#include "lru.h"

#include <stdio.h>

struct lru_s {
  linkmap_t *lm;
  size_t size;
  size_t nmemb;
  size_t active;
  void *data;
};

lru_t *lru_new(size_t size, size_t nmemb) {
  lru_t *lru;
  linkmap_t *lm;
  void *data;

  assert(nmemb >= 2);

  lru = malloc(sizeof(lru_t));
  lm = linkmap_new(nmemb);
  data = malloc(nmemb * size);

  if (!lru || !lm || !data) {
    free(lru);
    free(lm);
    free(data);
    return NULL;
  }

  lru->lm = lm;
  lru->data = data;
  lru->size = size;
  lru->nmemb = nmemb;
  lru->active = 0;

  return lru;
}

int lru_fetch(lru_t *lru, uint64_t key, void **ptr) {
  void *data, *val;
  uint64_t k;

  /* hit cache */
  if (!linkmap_pop(lru->lm, key, ptr)) {
    /* reinsert as head, i.e. MRU, if found */
    linkmap_set(lru->lm, key, *ptr);
    return 0;
  }

  /* create new page if possible, evict LRU otherwise */
  if (lru->active < lru->nmemb) {
    val = lru->data + lru->active * lru->size;
    lru->active++;
  } else {
    linkmap_pop_tail(lru->lm, &k, &val);
  }

  /* insert as LRU */
  linkmap_set(lru->lm, key, val);

  *ptr = val;

  return 1;
}

void lru_free(lru_t **lru) {
  free((*lru)->data);
  linkmap_free(&(*lru)->lm);
  free(*lru);
  *lru = NULL;
}
