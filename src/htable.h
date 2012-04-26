#ifndef HTABLE_H_3f4c3193b823d6177b8337c512d3455f
#define HTABLE_H_3f4c3193b823d6177b8337c512d3455f

/* Hash table mapping unsigned 64 bit integer to void pointer.
 *
 * Entries with identical keys are stacked, i.e. get() and del() will
 * operate on the latest set() entry.
 */

#include <stdint.h>

typedef struct htable_s htable_t;

/* Allocates a new table of the given capacity.
 *
 * Returns NULL if out of memory.
 */
htable_t *htable_new(size_t entries);

/* Sets value for key
 *
 * Returns 0 on sucess
 *        -1 if the table is full
 */
int htable_set(htable_t *h, uint64_t key, void *val);

/* Retrieves value by key
 *
 * Returns 0 on success
 *         1 if key was not found
 */
int htable_get(htable_t *h, uint64_t key, void **val);

/* Deletes entry by key
 *
 * Returns 0 on success
 *         1 if entry was not found
 */
int htable_del(htable_t *h, uint64_t key);

/* Destroys a table and releases all associated resources
 *
 * The htable pointer at *h will be set to NULL
 */
void htable_free(htable_t **h);

#endif
