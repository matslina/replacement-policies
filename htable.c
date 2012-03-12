#include <stdlib.h>
#include <assert.h>
#include "htable.h"

typedef uint64_t htable_key_t;
typedef void  *  htable_val_t;

#define STATE_DELETED  1
#define STATE_OCCUPIED 2

#define HASH_TO_BLOCK(h) (h >> 2)
#define HASH_TO_RECORD(h) (h & 3)
#define BLOCKSTATE_GET(blockstate, record) \
  (((blockstate) >> ((record) << 1)) & 3)
#define BLOCKSTATE_SET(blockstate, record, value) \
  (blockstate) = ((blockstate) & ~(3 << (record << 1))) |\
                 (((value) & 3) << (record << 1))
#define NEXT(htable, h) ((h) + 1 >= htable->numrecords ? 0 : (h) + 1)

struct htable_record {
  htable_key_t key;
  htable_val_t val;
};

struct htable_block {
  uint8_t state;
  struct htable_record record[4];
};

struct htable_s {
  struct htable_block *block;
  size_t numblocks;
  size_t numrecords;
};

/* Thomas Wang's hash64shift()
 */
static int hash64shift(uint64_t k, int mod) {
  k = (~k) + (k << 21);
  k = k ^ (k >> 24);
  k = (k + (k << 3)) + (k << 8);
  k = k ^ (k >> 14);
  k = (k + (k << 2)) + (k << 4);
  k = k ^ (k >> 28);
  k = k + (k << 31);

  return k % mod;
}

htable_t *htable_new(size_t entries) {
  htable_t *htable;

  htable = calloc(1, sizeof(htable_t));
  if (!htable)
    return NULL;

  htable->numblocks = ((int)(entries * 1.4)) / 4 + 1;
  htable->numrecords = htable->numblocks * 4;

  htable->block = calloc(htable->numblocks, sizeof(struct htable_block));
  if (!htable->block)
    return NULL;

  return htable;
}

int htable_set(htable_t *htable, uint64_t key, void *val) {
  int h, b, r, rs, probes;

  h = hash64shift(key, htable->numrecords);

  for (probes=0; probes < htable->numrecords; probes++) {
    b = HASH_TO_BLOCK(h);
    r = HASH_TO_RECORD(h);
    rs = BLOCKSTATE_GET(htable->block[b].state, r);

    if (!(rs & STATE_OCCUPIED)) {
      htable->block[b].record[r].key = key;
      htable->block[b].record[r].val = val;
      BLOCKSTATE_SET(htable->block[b].state, r, STATE_OCCUPIED);
      return 0;
    }

    if (htable->block[b].record[r].key == key) {
      htable->block[b].record[r].val = val;
      return 0;
    }

    h = NEXT(htable, h);
  }

  return -1;
}

int htable_get(htable_t *htable, uint64_t key, void **val) {
  int h, b, r, rs, probes;

  h = hash64shift(key, htable->numblocks * 4);

  for (probes=0; probes < htable->numrecords; probes++) {
    b = HASH_TO_BLOCK(h);
    r = HASH_TO_RECORD(h);
    rs = BLOCKSTATE_GET(htable->block[b].state, r);

    if (!rs)
      return 1;

    if (rs & STATE_OCCUPIED && htable->block[b].record[r].key == key) {
      *val = htable->block[b].record[r].val;
      return 0;
    }

    h = NEXT(htable, h);
  }

  return 1;
}

int htable_del(htable_t *htable, uint64_t key) {
  int h, b, r, rs, probes;

  h = hash64shift(key, htable->numblocks * 4);

  for (probes=0; probes < htable->numrecords; probes++) {
    b = HASH_TO_BLOCK(h);
    r = HASH_TO_RECORD(h);
    rs = BLOCKSTATE_GET(htable->block[b].state, r);

    if (!rs)
      return 1;

    if (rs & STATE_OCCUPIED && htable->block[b].record[r].key == key) {
      BLOCKSTATE_SET(htable->block[b].state, r, STATE_DELETED);
      return 0;
    }

    h = NEXT(htable, h);
  }

  return 1;
}

 void htable_free(htable_t **htable) {
   free((*htable)->block);
   (*htable)->block = NULL;
   free(*htable);
   *htable = NULL;
 }
