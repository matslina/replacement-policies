#include <stdio.h>
#include <string.h>
#include <check.h>
#include "slru.h"


#define CACHED 0
#define FETCH(key, data, cached)                            \
  do {                                                      \
    void *p;                                                \
    fail_unless(cached == slru_fetch(slru, key, &p));         \
    if (cached == CACHED)                                   \
      fail_unless(!memcmp(p, data, strlen(data)));          \
    else                                                    \
      memcpy(p, data, strlen(data));                        \
  } while(0)

START_TEST(test_no_eviction) {
  slru_t *slru = slru_new(10, 8);

  fail_unless(slru != NULL);

  /* first fetch is not cached, second is */
  FETCH(0, "aaaaaaaaaa", !CACHED);
  FETCH(0, "aaaaaaaaaa", CACHED);

  /* a new page doesn't evict the previous */
  FETCH(1, "bbbbbbbbbb", !CACHED);
  FETCH(1, "bbbbbbbbbb", CACHED);
  FETCH(0, "aaaaaaaaaa", CACHED);

  /* and the same applies while slru is not full */
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

  slru_free(&slru);
  fail_unless(slru == NULL);
}
END_TEST

START_TEST(test_eviction_order) {
  slru_t *slru = slru_new(10, 8);

  fail_unless(slru != NULL);

  /* fill up */
  FETCH(0, "aaaaaaaaaa", !CACHED);
  FETCH(1, "bbbbbbbbbb", !CACHED);
  FETCH(2, "cccccccccc", !CACHED);
  FETCH(3, "dddddddddd", !CACHED);
  FETCH(4, "eeeeeeeeee", !CACHED);
  FETCH(5, "ffffffffff", !CACHED);
  FETCH(6, "gggggggggg", !CACHED);
  FETCH(7, "hhhhhhhhhh", !CACHED);

  /* at this point, 7/h is MRU and 0/a is LRU, so both are cached */
  FETCH(0, "aaaaaaaaaa", CACHED);
  FETCH(7, "hhhhhhhhhh", CACHED);

  /* refetching these should prevent them both from eviction. instead
     1/b should've been thrown out. */
  FETCH(8, "iiiiiiiiii", !CACHED);
  FETCH(1, "bbbbbbbbbb", !CACHED);

  /* bringing back 1/b should've evicted 2/c */

  slru_free(&slru);
  fail_unless(slru == NULL);
}
END_TEST


Suite *slru_suite() {
  TCase *tc;
  Suite *s;

  s = suite_create ("slru");

  tc = tcase_create ("foo");
  tcase_add_test (tc, test_no_eviction);
  tcase_add_test (tc, test_eviction_order);
  suite_add_tcase (s, tc);

  return s;
}

int main(void) {
  int number_failed;
  Suite *s = slru_suite();
  SRunner *sr = srunner_create(s);
  srunner_run_all (sr, CK_NORMAL);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return (number_failed == 0) ? 0 : 1;
}
