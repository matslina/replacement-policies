#ifndef LRUTABLE_H_80f7cf7ab604837c37e8007aaaf9e273
#define LRUTABLE_H_80f7cf7ab604837c37e8007aaaf9e273

/* Linked hash table mapping unsigned 64 bit integer to void pointer.
 *
 * This is a superset of the functionality provided by htable.c,
 * providing the ability to operate on the least recently used (LRU)
 * entry.
 *
 * Note that lrutable_set() and lrutable_get() both promote the entry
 * to MRU. No other operation will affect this ordering.
 *
 * Entries with identical keys are stacked, i.e. get(), del(), etc,
 * will operate on the latest set() entry.
 */

#include <stdint.h>

typedef struct lrutable_s lrutable_t;

/* Allocates a new linked table of the given capacity.
 *
 * Returns NULL if out of memory.
 */
lrutable_t *lrutable_new(size_t entries);

/* Destroys a table and releases all associated resources
 *
 * The htable pointer at *h will be set to NULL
 */
void lrutable_free(lrutable_t **t);

/* Returns the number of entries in t */
size_t lrutable_size(lrutable_t *t);

/* Sets value for key
 *
 * Returns 0 on sucess
 *        -1 if the table is full
 */
int lrutable_set(lrutable_t *t, uint64_t key, void *val);

/* Retrieves value by key
 *
 * Returns 0 on success
 *         1 if key was not found
 */
int lrutable_get(lrutable_t *t, uint64_t key, void **val);

/* Retrieves and deletes entry by key
 *
 * Returns 0 on success
 *         1 if entry was not found
 */
int lrutable_pop(lrutable_t *t, uint64_t key, void **val);

/* Deletes entry by key
 *
 * Returns 0 on success
 *         1 if entry was not found
 */
int lrutable_del(lrutable_t *t, uint64_t key);

/* Retrieves the least recently used entry
 *
 * Returns 0 on success
 *        -1 if lrutable is empty
 */
int lrutable_get_lru(lrutable_t *t, uint64_t *key, void **val);

/* Retrieves and deletes the least recently used entry
 *
 * Returns 0 on success
 *        -1 if lrutable is empty
 */
int lrutable_pop_lru(lrutable_t *t, uint64_t *key, void **val);

/* Deletes the least recently used entry
 *
 * Returns 0 on success
 *        -1 if lrutable is empty
 */
int lrutable_del_lru(lrutable_t *t);

/* Prints the table component of t to stderr
 */
void lrutable_pt(lrutable_t *t);

/* Prints the list component of t to stderr
 */
void lrutable_pl(lrutable_t *t);


#endif
