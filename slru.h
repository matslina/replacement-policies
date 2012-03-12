#ifndef SLRU_H_437f5c202a3fb1ebbb77e7fb1c9f8718
#define SLRU_H_437f5c202a3fb1ebbb77e7fb1c9f8718

#include <stdint.h>

typedef struct slru_s slru_t;

slru_t *slru_new(size_t size, size_t nmemb);
int slru_fetch(slru_t *slru, uint64_t key, void **ptr);
void slru_free(slru_t **slru);

#endif
