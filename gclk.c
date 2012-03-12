#include <stdlib.h>
#include "htable.h"
#include "gclk.h"

#include <stdio.h>

struct gclk_page {
  uint64_t key;
  uint8_t references;
  void *data;
};

struct gclk_s {
  size_t size;
  size_t nmemb;
  size_t active;
  int hand;
  htable_t *t;
  struct gclk_page *page;
  void *data;
};

gclk_t *gclk_new(size_t size, size_t nmemb) {
  gclk_t *r;

  r = malloc(sizeof(gclk_t));
  if (!r) goto fail;

  r->page = malloc(nmemb * sizeof(struct gclk_page));
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

int gclk_fetch(gclk_t *gclk, uint64_t key, void **ptr) {
  struct gclk_page *page;

  if (!htable_get(gclk->t, key, (void **)&page)) {
    if (page->references < 1)
      page->references++;
    *ptr = page->data;
    return 0;
  }

  if (gclk->active < gclk->nmemb) {
    page = gclk->page + gclk->active;
    page->data = gclk->data + gclk->active * gclk->size;
    page->key = key;
    page->references = 0;
    htable_set(gclk->t, key, (void *)page);
    gclk->active++;
    *ptr = page->data;
    return 1;
  }

  while (gclk->page[gclk->hand].references) {
    gclk->page[gclk->hand].references--;
    if (++gclk->hand >= gclk->nmemb)
      gclk->hand = 0;
  }

  page = gclk->page + gclk->hand;
  if (++gclk->hand >= gclk->nmemb)
    gclk->hand = 0;
  htable_del(gclk->t, page->key);
  htable_set(gclk->t, key, page);
  page->key = key;
  *ptr = page->data;

  return 1;
}

void gclk_free(gclk_t **gclk) {
  free((*gclk)->data);
  free((*gclk)->page);
  htable_free(&(*gclk)->t);
  free(*gclk);
  *gclk = NULL;
}
