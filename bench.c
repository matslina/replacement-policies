#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "lru.h"
#include "rnd.h"
#include "fifo.h"
#include "clk.h"
#include "gclk.h"
#include "slru.h"

/* Benchmarks the caches against some data set.
 *
 * The page-file is a list of 64 bit hex numbers, each of which
 * identifying a page to request. The optional nmemb argument specifys
 * the number of pages to keep in cache.
 */

#define BLOCK_SIZE 4096
#define DEFAULT_NMEMB 1024


typedef void* (*cache_new_fun)  (size_t, size_t);
typedef int   (*cache_fetch_fun)(void *, uint64_t, void**);
typedef void  (*cache_free_fun) (void **);

struct implementation_s {
  const char *name;
  cache_new_fun new_f;
  cache_fetch_fun fetch_f;
  cache_free_fun free_f;
};


struct implementation_s impls[] = {
  {.name    = "lru",
   .new_f   = (cache_new_fun)  lru_new,
   .fetch_f = (cache_fetch_fun)lru_fetch,
   .free_f  = (cache_free_fun) lru_free},
  {.name    = "rnd",
   .new_f   = (cache_new_fun)  rnd_new,
   .fetch_f = (cache_fetch_fun)rnd_fetch,
   .free_f  = (cache_free_fun) rnd_free},
  {.name    = "fifo",
   .new_f   = (cache_new_fun)  fifo_new,
   .fetch_f = (cache_fetch_fun)fifo_fetch,
   .free_f  = (cache_free_fun) fifo_free},
  {.name    = "clock",
   .new_f   = (cache_new_fun)  clk_new,
   .fetch_f = (cache_fetch_fun)clk_fetch,
   .free_f  = (cache_free_fun) clk_free},
  {.name    = "gclock",
   .new_f   = (cache_new_fun)  gclk_new,
   .fetch_f = (cache_fetch_fun)gclk_fetch,
   .free_f  = (cache_free_fun) gclk_free},
  {.name    = "slru",
   .new_f   = (cache_new_fun)  slru_new,
   .fetch_f = (cache_fetch_fun)slru_fetch,
   .free_f  = (cache_free_fun) slru_free},
};
int num_impls = sizeof(impls) / sizeof(struct implementation_s);

void usage_fail(char *prog) {
  fprintf(stderr, "Usage: %s <page-file> [nmemb]\n", prog);
  exit(1);
}

static void write_stuff(void *ptr, uint64_t key) {
  int i;
  uint64_t *p64;
  uint8_t  *p8;

  for (i=0, p64=(uint64_t *)ptr; i<BLOCK_SIZE/sizeof(uint64_t); i++, p64++)
    *p64 = key++;
  for (i=0, p8=(uint8_t *)p64; i<BLOCK_SIZE%sizeof(uint64_t); i++, p8++)
    *p8 = key++;
}

static int check_stuff(void *ptr, uint64_t key) {
  int i;
  uint64_t *p64;
  uint8_t  *p8;

  for (i=0, p64=(uint64_t *)ptr; i<BLOCK_SIZE/sizeof(uint64_t); i++, p64++)
    if (*p64 != key++)
      return 1;
  for (i=0, p8=(uint8_t *)p64; i<BLOCK_SIZE%sizeof(uint64_t); i++, p8++)
    if (*p8 != (uint8_t)key++)
      return 1;
  return 0;
}

int main(int argc, char *argv[]) {
  FILE *page_file;
  uint64_t *key;
  size_t keysize, keylen;
  size_t nmemb;
  int impl_i, i, hit, miss, fail, ecode;
  void *cache, *ptr;
  clock_t time_start, time_stop;

  /* cmd line args */
  if (!(2 <= argc && argc <= 3))
    usage_fail(argv[0]);

  nmemb = DEFAULT_NMEMB;
  if (argc >= 3)
    if (1 != sscanf(argv[2], "%d", &nmemb)) {
      fprintf(stderr, "bad lru-blocks: \'%s\'\n\n", argv[2]);
      usage_fail(argv[0]);
    }

  page_file = fopen(argv[1], "r");
  if (!page_file) {
    fprintf(stderr, "FAIL: could not open '%s' for reading\n", argv[1]);
    exit(1);
  }

  /* load block file */
  keysize = 100;
  keylen = 0;
  key = malloc(keysize * sizeof(uint64_t));
  while (1 == fscanf(page_file, "%llx", &key[keylen])) {
    keylen++;
    if (keylen >= keysize) {
      keysize *= 2;
      key = realloc(key, keysize * sizeof(uint64_t));
    }
  }

  /* and bench */
  for (impl_i=0; impl_i<num_impls; impl_i++) {
    cache = impls[impl_i].new_f(BLOCK_SIZE, nmemb);
    if (!cache) {
      printf("%s\t new() failed\n", impls[impl_i].name);
      continue;
    }

    time_start = clock();
    miss = hit = fail = 0;
    for (i=0; i<keylen; i++) {
      ecode = impls[impl_i].fetch_f(cache, key[i], &ptr);

      if (ecode == 0) {
        hit++;
        if (check_stuff(ptr, key[i]))
          fail++;
      } else if (ecode == 1) {
        miss++;
        write_stuff(ptr, key[i]);
      } else
        fail++;
    }
    time_stop = clock();

    printf("%s\t%.02f%% hit ratio (%d / %d)  time %.2f",
           impls[impl_i].name, 100*(float)hit/(miss+hit), hit, hit + miss,
           ((double)(time_stop - time_start))/CLOCKS_PER_SEC);

    if (fail)
      printf("  !!! %d fails", fail);
    printf("\n");

    impls[impl_i].free_f(&cache);
    if (cache)
      printf("%s\tfree is broken\n", impls[impl_i].name);
    cache = NULL;
  }

  return 0;
}
