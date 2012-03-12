#include <stdlib.h>
#include "htable.h"
#include "fifo.h"

#include <stdio.h>

struct fifo_page {
  uint64_t key;
  void *data;
};

struct fifo_s {
  size_t size;
  size_t nmemb;
  size_t active;
  int head;
  htable_t *t;
  struct fifo_page *page;
  void *data;
};

fifo_t *fifo_new(size_t size, size_t nmemb) {
  fifo_t *r;

  r = malloc(sizeof(fifo_t));
  if (!r) goto fail;

  r->page = malloc(nmemb * sizeof(struct fifo_page));
  if (!r->page) goto fail_page;

  r->data = malloc(nmemb * size);
  if (!r->data) goto fail_data;

  r->t = htable_new(nmemb);
  if (!r->t) goto fail_htable;

  r->size = size;
  r->nmemb = nmemb;
  r->active = 0;
  r->head = 0;

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

int fifo_fetch(fifo_t *fifo, uint64_t key, void **ptr) {
  struct fifo_page *page;

  if (!htable_get(fifo->t, key, (void **)&page)) {
    *ptr = page->data;
    return 0;
  }

  if (fifo->active < fifo->nmemb) {
    page = fifo->page + fifo->active;
    page->data = fifo->data + fifo->active * fifo->size;
    page->key = key;
    fifo->active++;
    htable_set(fifo->t, key, (void *)page);
    *ptr = page->data;
    return 1;
  }

  page = fifo->page + fifo->head;
  if (++fifo->head >= fifo->nmemb)
    fifo->head = 0;
  htable_del(fifo->t, page->key);
  htable_set(fifo->t, key, page);
  page->key = key;
  *ptr = page->data;
  return 1;
}

void fifo_free(fifo_t **fifo) {
  free((*fifo)->data);
  free((*fifo)->page);
  htable_free(&(*fifo)->t);
  free(*fifo);
  *fifo = NULL;
}
