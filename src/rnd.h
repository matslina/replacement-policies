#ifndef RND_H_c97b0748be1dee4c84cc90a0ffce5142
#define RND_H_c97b0748be1dee4c84cc90a0ffce5142

#include <stdint.h>

typedef struct rnd_s rnd_t;

rnd_t *rnd_new(size_t size, size_t nmemb);
int rnd_fetch(rnd_t *rnd, uint64_t key, void **ptr);
void rnd_free(rnd_t **rnd);

#endif
