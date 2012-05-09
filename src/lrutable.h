#ifndef LRUTABLE_H_80f7cf7ab604837c37e8007aaaf9e273
#define LRUTABLE_H_80f7cf7ab604837c37e8007aaaf9e273

/* Linked hash table mapping unsigned 64 bit integer to void pointer.
 *
 * Entries with identical keys are stacked, i.e. get(), del(), etc,
 * will operate on the latest set() entry.
 *
 * New entries are prepended to a linked list, thereby preserving the
 * order of insertion (although in some sense reversed).
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

/* Deletes entry by key
 *
 * Returns 0 on success
 *         1 if entry was not found
 */
int lrutable_del(lrutable_t *t, uint64_t key);

/* Retrieves and deletes entry by key
 *
 * Returns 0 on success
 *         1 if entry was not found
 */
int lrutable_pop(lrutable_t *t, uint64_t key, void **val);

/* Retrieves the most recently set entry's key and value
 *
 * Returns 0 on success
 *        -1 if lrutable is empty
 */
int lrutable_get_newest(lrutable_t *t, uint64_t *key, void **val);

/* Retrieves the least recently set entry's key and value
 *
 * Returns 0 on success
 *        -1 if lrutable is empty
 */
int lrutable_get_oldest(lrutable_t *t, uint64_t *key, void **val);

/* Retrieves and deletes the most recently set entry's key and value
 *
 * Returns 0 on success
 *        -1 if lrutable is empty
 */
int lrutable_pop_newest(lrutable_t *t, uint64_t *key, void **val);

/* Retrieves and deletes the least recently set entry's key and value
 *
 * Returns 0 on success
 *        -1 if lrutable is empty
 */
int lrutable_pop_oldest(lrutable_t *t, uint64_t *key, void **val);

/* Promotes entry by key to newest in list
 *
 * Returns 0 on success
 *         1 if entry was not found
 */
int lrutable_make_newest(lrutable_t *t, uint64_t key);

/* Demotes entry by key to oldest in list
 *
 * Returns 0 on success
 *         1 if entry was not found
 */
int lrutable_make_oldest(lrutable_t *t, uint64_t key);

/* Deletes the newest entry by key
 *
 * Returns 0 on success
 *        -1 if lrutable is empty
 */
int lrutable_del_newest(lrutable_t *t);

/* Deletes the oldest entry by key
 *
 * Returns 0 on success
 *        -1 if lrutable is empty
 */
int lrutable_del_oldest(lrutable_t *t);


void lrutable_pt(lrutable_t *t);
void lrutable_pl(lrutable_t *t);


#endif
