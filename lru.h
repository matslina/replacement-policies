#ifndef LRU_H_deda7aa66f370ad032e54beb92f95493
#define LRU_H_deda7aa66f370ad032e54beb92f95493

#include <stdint.h>

typedef struct lru_s lru_t;

lru_t *lru_new(size_t size, size_t nmemb);
int lru_fetch(lru_t *lru, uint64_t key, void **ptr);
void lru_free(lru_t **lru);

#endif
