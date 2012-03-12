#include <stdio.h>
#include <string.h>
#include <check.h>
#include "lru.h"


#define CACHED 0
#define FETCH(key, data, cached)                            \
  do {                                                      \
    void *p;                                                \
    fail_unless(cached == lru_fetch(lru, key, &p));         \
    if (cached == CACHED)                                   \
      fail_unless(!memcmp(p, data, strlen(data)));          \
    else                                                    \
      memcpy(p, data, strlen(data));                        \
  } while(0)

START_TEST(test_no_eviction) {
  lru_t *lru = lru_new(10, 8);

  fail_unless(lru != NULL);

  /* first fetch is not cached, second is */
  FETCH(0, "aaaaaaaaaa", !CACHED);
  FETCH(0, "aaaaaaaaaa", CACHED);

  /* a new page doesn't evict the previous */
  FETCH(1, "bbbbbbbbbb", !CACHED);
  FETCH(1, "bbbbbbbbbb", CACHED);
  FETCH(0, "aaaaaaaaaa", CACHED);

  /* and the same applies while lru is not full */
  FETCH(2, "cccccccccc", !CACHED);
  FETCH(3, "dddddddddd", !CACHED);
  FETCH(4, "eeeeeeeeee", !CACHED);
  FETCH(5, "ffffffffff", !CACHED);
  FETCH(6, "gggggggggg", !CACHED);
  FETCH(7, "hhhhhhhhhh", !CACHED);
  FETCH(0, "aaaaaaaaaa", CACHED);
  FETCH(1, "bbbbbbbbbb", CACHED);
  FETCH(2, "cccccccccc", CACHED);
  FETCH(3, "dddddddddd", CACHED);
  FETCH(4, "eeeeeeeeee", CACHED);
  FETCH(5, "ffffffffff", CACHED);
  FETCH(6, "gggggggggg", CACHED);
  FETCH(7, "hhhhhhhhhh", CACHED);

  lru_free(&lru);
  fail_unless(lru == NULL);
}
END_TEST

START_TEST(test_eviction_order) {
  lru_t *lru = lru_new(10, 8);

  fail_unless(lru != NULL);

  /* fill up lru and check */
  FETCH(0, "aaaaaaaaaa", !CACHED);
  FETCH(1, "bbbbbbbbbb", !CACHED);
  FETCH(2, "cccccccccc", !CACHED);
  FETCH(3, "dddddddddd", !CACHED);
  FETCH(4, "eeeeeeeeee", !CACHED);
  FETCH(5, "ffffffffff", !CACHED);
  FETCH(6, "gggggggggg", !CACHED);
  FETCH(7, "hhhhhhhhhh", !CACHED);

  /* another entry evicts 0 */
  FETCH(8, "iiiiiiiiii", !CACHED);
  FETCH(1, "bbbbbbbbbb", CACHED);
  FETCH(2, "cccccccccc", CACHED);
  FETCH(3, "dddddddddd", CACHED);
  FETCH(4, "eeeeeeeeee", CACHED);
  FETCH(5, "ffffffffff", CACHED);
  FETCH(6, "gggggggggg", CACHED);
  FETCH(7, "hhhhhhhhhh", CACHED);
  FETCH(8, "iiiiiiiiii", CACHED);
  FETCH(0, "aaaaaaaaaa", !CACHED);

  /* refetching does not evict */
  FETCH(5, "ffffffffff", CACHED);
  FETCH(5, "ffffffffff", CACHED);
  FETCH(5, "ffffffffff", CACHED);
  FETCH(5, "ffffffffff", CACHED);
  FETCH(5, "ffffffffff", CACHED);
  FETCH(5, "ffffffffff", CACHED);
  FETCH(5, "ffffffffff", CACHED);
  FETCH(5, "ffffffffff", CACHED);
  FETCH(5, "ffffffffff", CACHED);
  FETCH(5, "ffffffffff", CACHED);
  FETCH(5, "ffffffffff", CACHED);

  /* and doing that didn't change 2 from being LRU  */
  FETCH(2, "cccccccccc", CACHED);

  /* but 1 was evicted */
  FETCH(1, "bbbbbbbbbb", !CACHED);
}
END_TEST


Suite *lru_suite() {
  TCase *tc;
  Suite *s;

  s = suite_create ("lru");

  tc = tcase_create ("foo");
  tcase_add_test (tc, test_no_eviction);
  tcase_add_test (tc, test_eviction_order);
  suite_add_tcase (s, tc);

  return s;
}

int main(void) {
  int number_failed;
  Suite *s = lru_suite();
  SRunner *sr = srunner_create(s);
  srunner_run_all (sr, CK_NORMAL);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return (number_failed == 0) ? 0 : 1;
}
