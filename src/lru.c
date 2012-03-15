#include <stdlib.h>
#include <assert.h>
#include "htable.h"
#include "lru.h"

#include <stdio.h>

typedef uint64_t lru_key_t;

struct lru_page {
  lru_key_t key;
  struct lru_page *next;
  struct lru_page *prev;
  void *data;
};

struct lru_s {
  htable_t *t;
  size_t size;
  size_t nmemb;
  size_t active;
  struct lru_page *first;
  struct lru_page *last;
  struct lru_page *page;
  void *data;
};

lru_t *lru_new(size_t size, size_t nmemb) {
  lru_t *lru;

  assert(nmemb >= 2);

  lru = malloc(sizeof(lru_t));
  if (!lru) goto fail;

  lru->page = malloc(nmemb * sizeof(struct lru_page));
  if (!lru->page) goto fail_page;

  lru->data = malloc(nmemb * size);
  if (!lru->data) goto fail_data;

  lru->t = htable_new(nmemb);
  if (!lru->t) goto fail_htable;

  lru->size = size;
  lru->nmemb = nmemb;
  lru->active = 0;
  lru->first = NULL;
  lru->last = NULL;

  return lru;

 fail_htable:
  free(lru->data);
 fail_data:
  free(lru->page);
 fail_page:
  free(lru);
 fail:
  return NULL;
}

static void move_to_head(lru_t *lru, struct lru_page *page) {
  if (page == lru->first)
    return;

  assert(page->prev != NULL);
  assert(lru->active >= 2);

  if (page->next)
    page->next->prev = page->prev;
  else
    lru->last = page->prev;
  page->prev->next = page->next;

  page->next = lru->first;
  page->next->prev = page;
  page->prev = NULL;
  lru->first = page;
}

int lru_fetch(lru_t *lru, lru_key_t key, void **ptr) {
  struct lru_page *page;

  /* hit cache */
  if (!htable_get(lru->t, key, (void *)&page)) {
    move_to_head(lru, page);
    *ptr = page->data;
    return 0;
  }

  /* still some free pages left? */
  if (lru->active < lru->nmemb) {
    page = lru->page + lru->active;
    page->data = lru->data + lru->active * lru->size;
    page->key = key;
    htable_set(lru->t, key, (void *)page);
    lru->active ++;

    if (lru->first)
      lru->first->prev = page;
    else
      lru->last = page;
    page->prev = NULL;
    page->next = lru->first;
    lru->first = page;

    *ptr = page->data;

    return 1;
  }

  /* otherwise we evict LRU and move it to head */
  move_to_head(lru, lru->last);
  htable_del(lru->t, lru->first->key);
  lru->first->key = key;
  htable_set(lru->t, key, (void *)lru->first);
  *ptr = lru->first->data;

  return 1;
}

void lru_free(lru_t **lru) {
  free((*lru)->data);
  free((*lru)->page);
  htable_free(&(*lru)->t);
  free(*lru);
  *lru = NULL;
}
