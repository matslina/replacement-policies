#include <stdio.h>
#include <string.h>
#include <check.h>
#include "rnd.h"

#include <stdio.h>

#define CACHED 0
#define FETCH(key, data, count)                                 \
  do {                                                          \
    void *p;                                                    \
    char tbuf[10000];                                           \
    int ret;                                                    \
    ret = rnd_fetch(rnd, key, &p);                              \
    count += ret;                                               \
    if (ret == CACHED)                                          \
      fail_unless(!memcmp(p, data, strlen(data)));              \
    else                                                        \
      memcpy(p, data, strlen(data));                            \
  } while(0)

START_TEST(test_no_eviction) {
  rnd_t *rnd = rnd_new(10, 8);
  int count;

  fail_unless(rnd != NULL);

  /* no evictions as long as everything fits in cache */
  count = 0;
  FETCH(0, "aaaaaaaaaa", count);
  FETCH(0, "aaaaaaaaaa", count);

  fail_unless(count == 1);

  FETCH(1, "bbbbbbbbbb", count);
  FETCH(0, "aaaaaaaaaa", count);
  FETCH(1, "bbbbbbbbbb", count);

  fail_unless(count == 2);

  FETCH(2, "cccccccccc", count);
  FETCH(3, "dddddddddd", count);
  FETCH(4, "eeeeeeeeee", count);
  FETCH(5, "ffffffffff", count);
  FETCH(6, "gggggggggg", count);
  FETCH(7, "hhhhhhhhhh", count);

  fail_unless(count == 8);

  FETCH(3, "dddddddddd", count);
  FETCH(0, "aaaaaaaaaa", count);
  FETCH(2, "cccccccccc", count);
  FETCH(6, "gggggggggg", count);
  FETCH(1, "bbbbbbbbbb", count);
  FETCH(7, "hhhhhhhhhh", count);
  FETCH(4, "eeeeeeeeee", count);
  FETCH(5, "ffffffffff", count);

  fail_unless(count == 8);

  rnd_free(&rnd);
  fail_unless(rnd == NULL);
}
END_TEST

START_TEST(test_eviction_order) {
  rnd_t *rnd = rnd_new(10, 8);
  int count;

  fail_unless(rnd != NULL);

  count = 0;

  /* no evictions for the first 8 */
  FETCH(0, "aaaaaaaaaa", count);
  FETCH(1, "bbbbbbbbbb", count);
  FETCH(2, "cccccccccc", count);
  FETCH(3, "dddddddddd", count);
  FETCH(4, "eeeeeeeeee", count);
  FETCH(5, "ffffffffff", count);
  FETCH(6, "gggggggggg", count);
  FETCH(7, "hhhhhhhhhh", count);

  fail_unless(count == 8);

  FETCH(0, "aaaaaaaaaa", count);
  FETCH(1, "bbbbbbbbbb", count);
  FETCH(2, "cccccccccc", count);
  FETCH(3, "dddddddddd", count);

  fail_unless(count == 8);

  /* adding more will evict something, but we don't know what */
  FETCH(8, "iiiiiiiiii", count);
  fail_unless(count == 9);
  FETCH(8, "iiiiiiiiii", count);
  fail_unless(count == 9);
  FETCH(8, "iiiiiiiiii", count);
  fail_unless(count == 9);

  rnd_free(&rnd);
  fail_unless(rnd == NULL);
}
END_TEST


Suite *rnd_suite() {
  TCase *tc;
  Suite *s;

  s = suite_create ("rnd");

  tc = tcase_create ("foo");
  tcase_add_test (tc, test_no_eviction);
  tcase_add_test (tc, test_eviction_order);
  suite_add_tcase (s, tc);

  return s;
}

int main(void) {
  int number_failed;
  Suite *s = rnd_suite();
  SRunner *sr = srunner_create(s);
  srunner_run_all (sr, CK_NORMAL);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return (number_failed == 0) ? 0 : 1;
}
