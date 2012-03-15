#ifndef HTABLE_H_3f4c3193b823d6177b8337c512d3455f
#define HTABLE_H_3f4c3193b823d6177b8337c512d3455f

#include <stdint.h>

typedef struct htable_s htable_t;

htable_t *htable_new(size_t entries);
int htable_set(htable_t *hash, uint64_t key, void *val);
int htable_get(htable_t *hash, uint64_t key, void **val);
int htable_del(htable_t *hash, uint64_t key);
void htable_free(htable_t **hash);


#endif
