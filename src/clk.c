#include <stdlib.h>
#include "htable.h"
#include "clk.h"

#include <stdio.h>

struct clk_page {
  uint64_t key;
  uint8_t referenced;
  void *data;
};

struct clk_s {
  size_t size;
  size_t nmemb;
  size_t active;
  int hand;
  htable_t *t;
  struct clk_page *page;
  void *data;
};

clk_t *clk_new(size_t size, size_t nmemb) {
  clk_t *r;

  r = malloc(sizeof(clk_t));
  if (!r)
    goto fail;

  r->page = malloc(nmemb * sizeof(struct clk_page));
  if (!r->page)
    goto fail_page;

  r->data = malloc(nmemb * size);
  if (!r->data)
    goto fail_data;

  r->t = htable_new(nmemb);
  if (!r->t)
    goto fail_htable;

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

int clk_fetch(clk_t *clk, uint64_t key, void **ptr) {
  struct clk_page *page;

  /* if cached, tick the referenced box and return */
  if (!htable_get(clk->t, key, (void **)&page)) {
    page->referenced = 1;
    *ptr = page->data;
    return 0;
  }

  /* otherwise, check if there's an unused page available */
  if (clk->active < clk->nmemb) {
    page = clk->page + clk->active;
    page->data = clk->data + clk->active * clk->size;
    page->key = key;
    page->referenced = 0;
    htable_set(clk->t, key, (void *)page);
    clk->active++;
    *ptr = page->data;
    return 1;
  }

  /* otherwise, do eviction according to the clock algorithm */
  while (clk->page[clk->hand].referenced) {
    clk->page[clk->hand].referenced = 0;
    if (++clk->hand >= clk->nmemb)
      clk->hand = 0;
  }
  page = clk->page + clk->hand;
  if (++clk->hand >= clk->nmemb)
    clk->hand = 0;

  /* and finally reuse the evicted page */
  htable_del(clk->t, page->key);
  htable_set(clk->t, key, page);
  page->key = key;
  *ptr = page->data;

  return 1;
}

void clk_free(clk_t **clk) {
  free((*clk)->data);
  free((*clk)->page);
  htable_free(&(*clk)->t);
  free(*clk);
  *clk = NULL;
}
