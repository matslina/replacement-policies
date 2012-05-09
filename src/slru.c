#include <stdlib.h>
#include <assert.h>
#include "lrutable.h"
#include "slru.h"

#include <stdio.h>

#define PROTECTED_SIZE(total) (total >> 1)
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

struct slru_s {
  lrutable_t *A_t;
  lrutable_t *B_t;
  void *data;
  size_t A_max;
  size_t A_size;
  size_t B_max;
  size_t B_size;
  size_t active;
  size_t size;
  size_t nmemb;
};

slru_t *slru_new(size_t size, size_t nmemb) {
  slru_t *slru;
  int i;

  assert(nmemb >= 2);

  slru = malloc(sizeof(slru_t));
  if (!slru)
    return NULL;

  slru->A_max = MAX(1, MIN(nmemb, PROTECTED_SIZE(nmemb)));
  slru->B_max = nmemb - slru->A_max;
  slru->A_size = 0;
  slru->B_size = 0;
  slru->size = size;
  slru->nmemb = nmemb;
  slru->active = 0;

  slru->data = malloc(nmemb * size);
  if (!slru->data){
    free(slru);
    return NULL;
  }

  slru->A_t = lrutable_new(slru->A_max);
  if (!slru->A_t) {
    free(slru->data);
    free(slru);
    return NULL;
  }

  slru->B_t = lrutable_new(slru->B_max);
  if (!slru->B_t) {
    lrutable_free(&slru->B_t);
    free(slru->data);
    free(slru);
    return NULL;
  }

  return slru;
}

/* Retrieves entry by key from cache.
 *
 * The entry will be promoted to A MRU if found.
 *
 * Returns 0 if the key was found
 *         1 if the key was not found
 */
static int slru_get(slru_t *slru, uint64_t key, void **ptr) {
  void *data, *v;
  uint64_t k;

  /* hit A */
  if (!lrutable_get(slru->A_t, key, &data)) {
    lrutable_make_newest(slru->A_t, key);
    *ptr = data;
    return 0;
  }

  /* else check B, promote to A MRU if found */
  if (!lrutable_pop(slru->B_t, key, &data)) {
    slru->B_size--;

    /* if A is full, we demote A's LRU to B */
    if (slru->A_size >= slru->A_max) {
      lrutable_pop_oldest(slru->A_t, &k, &v);
      lrutable_set(slru->B_t, k, v);
      slru->A_size--;
      slru->B_size++;
    }

    lrutable_set(slru->A_t, key, data);
    slru->A_size++;
    *ptr = data;

    return 0;
  }

  return 1;
}

static int slru_evict(slru_t *slru, uint64_t *key, void **ptr) {
  return 0;
}

int slru_fetch(slru_t *slru, uint64_t key, void **ptr) {
  int rc;
  void *data, *v;
  uint64_t k;

  /* try to get from cache */
  if (!slru_get(slru, key, ptr))
    return 0;

  /* if that fails, we either create a new page and insert that into
     B, or we evict the LRU entry from B to make room. */

  if (slru->active < slru->nmemb && slru->B_size < slru->B_max) {
    data = slru->data + slru->active * slru->size;
    slru->active++;
  } else {
    lrutable_pop_oldest(slru->B_t, &k, &data);
    slru->B_size--;
  }

  lrutable_set(slru->B_t, key, data);
  slru->B_size++;
  *ptr = data;

  return 1;
}

void slru_free(slru_t **slru) {
  free((*slru)->data);
  lrutable_free(&(*slru)->A_t);
  lrutable_free(&(*slru)->B_t);
  free(*slru);
  *slru = NULL;
}
