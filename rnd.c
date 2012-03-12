#include <stdlib.h>
#include "htable.h"
#include "rnd.h"

struct rnd_page {
  uint64_t key;
  void *data;
};

struct rnd_s {
  size_t size;
  size_t nmemb;
  size_t active;
  htable_t *t;
  struct rnd_page *page;
  void *data;
};

rnd_t *rnd_new(size_t size, size_t nmemb) {
  rnd_t *r;

  r = malloc(sizeof(rnd_t));
  if (!r) goto fail;

  r->page = malloc(nmemb * sizeof(struct rnd_page));
  if (!r->page) goto fail_page;

  r->data = malloc(nmemb * size);
  if (!r->data) goto fail_data;

  r->t = htable_new(nmemb);
  if (!r->t) goto fail_htable;

  r->size = size;
  r->nmemb = nmemb;
  r->active = 0;

  return r;

 fail_htable:
  free(r->data);
 fail_data:
  free(r->page);
 fail_page:
  free(r);
 fail:
  return NULL;
}

int rnd_fetch(rnd_t *rnd, uint64_t key, void **ptr) {
  struct rnd_page *page;

  if (!htable_get(rnd->t, key, (void **)&page)) {
    *ptr = page->data;
    return 0;
  }

  if (rnd->active < rnd->nmemb) {
    page = rnd->page + rnd->active;
    page->data = rnd->data + rnd->active * rnd->size;
    page->key = key;
    htable_set(rnd->t, key, (void *)page);
    rnd->active++;
    *ptr = page->data;
    return 1;
  }

  page = rnd->page + (random() % rnd->nmemb);
  htable_del(rnd->t, page->key);
  htable_set(rnd->t, key, page);
  page->key = key;
  *ptr = page->data;
  return 1;
}

void rnd_free(rnd_t **rnd) {
  free((*rnd)->page);
  htable_free(&(*rnd)->t);
  free(*rnd);
  *rnd = NULL;
}
