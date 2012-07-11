#ifndef LINKMAP_H_80f7cf7ab604837c37e8007aaaf9e273
#define LINKMAP_H_80f7cf7ab604837c37e8007aaaf9e273

/* Linked hash table.
 *
 * A linkmap_t is a doubly linked list coupled with a hash table. Each
 * entry consists of a uint64_t key and a void* value. Entries can be
 * manipulated by key.
 */

#include <stdint.h>

typedef struct linkmap_s linkmap_t;

/* Allocates a new linked table of the given capacity.
 *
 * If capacity < 1, a capacity of 1 will be used.
 *
 * Returns NULL if out of memory.
 */
linkmap_t *linkmap_new(size_t capacity);

/* Destroys a table and releases all associated resources
 *
 * The htable pointer at *h will be set to NULL
 */
void linkmap_free(linkmap_t **lm);

/* Returns the number of entries in t
 */
size_t linkmap_size(linkmap_t *lm);

/* Sets value for key.
 *
 * This will overwrite any existing entry with this key. If no such
 * entry exists, then a new entry will be created and positioned as
 * head of the list.
 *
 * Returns 0 on sucess
 *         1 if the table is full
 */
int linkmap_set(linkmap_t *lm, uint64_t key, void *val);

/* These retrieve entries.
 *
 * _get() retrieves value by key,
 * _get_head/tail() retrieves key and value for list head/tail
 *
 * Returns 0 on success
 *         1 if key was not found or if linkmap is empty
 */
int linkmap_get(linkmap_t *lm, uint64_t key, void **val);
int linkmap_get_head(linkmap_t *lm, uint64_t *key, void **val);
int linkmap_get_tail(linkmap_t *lm, uint64_t *key, void **val);

/* These retrieves and deletes entries
 *
 * _pop() operate on entry identified by key,
 * _get_head/tail() operate on list head/tail
 *
 * Returns 0 on success
 *         1 if entry was not found or if linkmap is empty
 */
int linkmap_pop(linkmap_t *lm, uint64_t key, void **val);
int linkmap_pop_head(linkmap_t *lm, uint64_t *key, void **val);
int linkmap_pop_tail(linkmap_t *lm, uint64_t *key, void **val);

/* These delete entries.
 *
 * _del() deletes entry by key
 * _del_head/tail() deletes list head/tail
 *
 * Returns 0 on success
 *         1 if entry was not found or if linkmap is empty
 */
int linkmap_del(linkmap_t *lm, uint64_t key);
int linkmap_del_head(linkmap_t *lm);
int linkmap_del_tail(linkmap_t *lm);

#endif
