#include <stdlib.h>
#include <assert.h>
#include "htable.h"
#include "slru.h"

#include <stdio.h>

#define PROTECTED_RATIO 0.5

typedef uint64_t slru_key_t;

struct slru_page {
  slru_key_t key;
  struct slru_page *next;
  struct slru_page *prev;
  void *data;
};

struct slru_s {
  htable_t *A_t;
  htable_t *B_t;
  size_t A_size;
  size_t A_max;
  size_t size;
  size_t nmemb;
  size_t active;
  struct slru_page *free;
  struct slru_page *A_first;
  struct slru_page *A_last;
  struct slru_page *B_first;
  struct slru_page *B_last;
  struct slru_page *page;
  void *data;
};

slru_t *slru_new(size_t size, size_t nmemb) {
  slru_t *slru;
  int i;

  assert(nmemb >= 2);

  slru = malloc(sizeof(slru_t));
  if (!slru) goto fail;

  slru->page = malloc(nmemb * sizeof(struct slru_page));
  if (!slru->page) goto fail_page;

  slru->data = malloc(nmemb * size);
  if (!slru->data) goto fail_data;

  slru->A_t = htable_new(nmemb);
  if (!slru->A_t) goto fail_htable_A;

  slru->B_t = htable_new(nmemb);
  if (!slru->B_t) goto fail_htable_B;

  for (i=0; i<nmemb; i++) {
    slru->page[i].next = &slru->page[i+1];
    slru->page[i].data = slru->data + i * size;
  }
  slru->page[nmemb-1].next = NULL;

  slru->A_max = PROTECTED_RATIO * nmemb;
  if (slru->A_max < 1)
    slru->A_max = 1;
  if (slru->A_max > nmemb)
    slru->A_max = nmemb;

  slru->A_size = 0;
  slru->size = size;
  slru->nmemb = nmemb;
  slru->active = 0;
  slru->free = &slru->page[0];
  slru->A_first = NULL;
  slru->A_last = NULL;
  slru->B_first = NULL;
  slru->B_last = NULL;

  return slru;

 fail_htable_B:
  free(slru->A_t);
 fail_htable_A:
  free(slru->data);
 fail_data:
  free(slru->page);
 fail_page:
  free(slru);
 fail:
  return NULL;
}

static void list_remove(struct slru_page *page,
                        struct slru_page **first,
                        struct slru_page **last) {
  if (page->prev) {
    page->prev->next = page->next;
  } else {
    assert (*first == page);
    *first = page->next;
  }

  if (page->next) {
    page->next->prev = page->prev;
  } else {
    assert (*last == page);
    *last = page->prev;
  }
}

static void list_prepend(struct slru_page *page,
                         struct slru_page **first,
                         struct slru_page **last) {
  page->next = *first;
  page->prev = NULL;
  *first = page;

  if (page->next) {
    page->next->prev = page;
  } else {
    *last = page;
  }
}

int slru_fetch(slru_t *slru, slru_key_t key, void **ptr) {
  struct slru_page *page;

  /* hit A */
  if (!htable_get(slru->A_t, key, (void *)&page)) {
    /* move to head */
    if (page != slru->A_first) {
      list_remove(page, &slru->A_first, &slru->A_last);
      list_prepend(page, &slru->A_first, &slru->A_last);
    }
    *ptr = page->data;
    return 0;
  }

  /* hit B */
  if (!htable_get(slru->B_t, key, (void *)&page)) {
    /* promote to A */
    list_remove(page, &slru->B_first, &slru->B_last);
    list_prepend(page, &slru->A_first, &slru->A_last);
    slru->A_size ++;
    htable_del(slru->B_t, key);
    htable_set(slru->A_t, key, page);
    *ptr = page->data;

    /* demote A LRU to B if A is too large */
    if (slru->A_size > slru->A_max) {
      page = slru->A_last;
      list_remove(page, &slru->A_first, &slru->A_last);
      list_prepend(page, &slru->B_first, &slru->B_last);
      htable_del(slru->A_t, page->key);
      htable_set(slru->B_t, page->key, page);
    }

    return 0;
  }

  /* free pages available? */
  if (slru->free) {
    page = slru->free;
    slru->free = slru->free->next;
    list_prepend(page, &slru->B_first, &slru->B_last);
    htable_set(slru->B_t, key, page);
    page->key = key;
    *ptr = page->data;
    return 1;
  }

  /* evict from B if possible, else fall back to A */
  if (slru->B_last) {
    page = slru->B_last;
    list_remove(page, &slru->B_first, &slru->B_last);
    htable_del(slru->B_t, page->key);
  } else {
    page = slru->A_last;
    list_remove(page, &slru->A_first, &slru->A_last);
    htable_del(slru->A_t, page->key);
  }

  page->key = key;
  list_prepend(page, &slru->B_first, &slru->B_last);
  htable_set(slru->B_t, key, page);
  *ptr = page->data;

  return 1;
}

void slru_free(slru_t **slru) {
  free((*slru)->data);
  free((*slru)->page);
  htable_free(&(*slru)->A_t);
  htable_free(&(*slru)->B_t);
  free(*slru);
  *slru = NULL;
}
