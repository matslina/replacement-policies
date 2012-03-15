#ifndef FIFO_H_b3d1cfd36c988a4d2797b7f86bbe4bce
#define FIFO_H_b3d1cfd36c988a4d2797b7f86bbe4bce

#include <stdint.h>

typedef struct fifo_s fifo_t;

fifo_t *fifo_new(size_t size, size_t nmemb);
int fifo_fetch(fifo_t *fifo, uint64_t key, void **ptr);
void fifo_free(fifo_t **fifo);

#endif
