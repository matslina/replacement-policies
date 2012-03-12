#ifndef CLK_H_403dd41a1efbdc2bfb0ea2638ba7ccf5
#define CLK_H_403dd41a1efbdc2bfb0ea2638ba7ccf5

#include <stdint.h>

typedef struct clk_s clk_t;

clk_t *clk_new(size_t size, size_t nmemb);
int clk_fetch(clk_t *clock, uint64_t key, void **ptr);
void clk_free(clk_t **clock);

#endif
