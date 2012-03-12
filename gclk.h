#ifndef GCLK_H_87d4dc3995db41633e314d8179ffe74b
#define GCLK_H_87d4dc3995db41633e314d8179ffe74b

#include <stdint.h>

typedef struct gclk_s gclk_t;

gclk_t *gclk_new(size_t size, size_t nmemb);
int gclk_fetch(gclk_t *clock, uint64_t key, void **ptr);
void gclk_free(gclk_t **clock);

#endif
