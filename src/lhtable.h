#ifndef LHTABLE_H_80f7cf7ab604837c37e8007aaaf9e273
#define LHTABLE_H_80f7cf7ab604837c37e8007aaaf9e273

/* Linked hash table mapping unsigned 64 bit integer to void pointer.
 *
 * Entries with identical keys are stacked, i.e. get(), del(), etc,
 * will operate on the latest set() entry.
 *
 * New entries are prepended to a linked list, thereby preserving the
 * order of insertion (although in some sense reversed).
 */

#include <stdint.h>

typedef struct lhtable_s lhtable_t;

/* Allocates a new linked table of the given capacity.
 *
 * Returns NULL if out of memory.
 */
lhtable_t *lhtable_new(size_t entries);

/* Destroys a table and releases all associated resources
 *
 * The htable pointer at *h will be set to NULL
 */
void lhtable_free(lhtable_t **t);

/* Sets value for key
 *
 * Returns 0 on sucess
 *        -1 if the table is full
 */
int lhtable_set(lhtable_t *t, uint64_t key, void *val);

/* Retrieves value by key
 *
 * Returns 0 on success
 *         1 if key was not found
 */
int lhtable_get(lhtable_t *t, uint64_t key, void **val);

/* Deletes entry by key
 *
 * Returns 0 on success
 *         1 if entry was not found
 */
int lhtable_del(lhtable_t *t, uint64_t key);

/* Retrieves and deletes entry by key
 *
 * Returns 0 on success
 *         1 if entry was not found
 */
int lhtable_pop(lhtable_t *t, uint64_t key, void **val);

/* Retrieves the most recently set entry's key and value
 *
 * Returns 0 on success
 *        -1 if lhtable is empty
 */
int lhtable_get_newest(lhtable_t *t, uint64_t *key, void **val);

/* Retrieves the least recently set entry's key and value
 *
 * Returns 0 on success
 *        -1 if lhtable is empty
 */
int lhtable_get_oldest(lhtable_t *t, uint64_t *key, void **val);

/* Promotes entry by key to newest in list
 *
 * Returns 0 on success
 *         1 if entry was not found
 */
int lhtable_make_newest(lhtable_t *t, uint64_t key);

/* Demotes entry by key to oldest in list
 *
 * Returns 0 on success
 *         1 if entry was not found
 */
int lhtable_make_oldest(lhtable_t *t, uint64_t key);

void lhtable_pt(lhtable_t *t);
void lhtable_pl(lhtable_t *t);


#endif
