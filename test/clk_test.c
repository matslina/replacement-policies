#include <stdio.h>
#include <string.h>
#include <check.h>
#include "clk.h"

#include <stdio.h>

#define CACHED 0
#define FETCH(key, data, cached)                              \
  do {                                                        \
    void *p;                                                  \
    char tbuf[10000];                                         \
    fail_unless(cached == clk_fetch(clk, key, &p));         \
    if (cached == CACHED)                                     \
      fail_unless(!memcmp(p, data, strlen(data)));            \
    else                                                      \
      memcpy(p, data, strlen(data));                          \
  } while(0)

START_TEST(test_no_eviction) {
  clk_t *clk = clk_new(10, 8);

  fail_unless(clk != NULL);

  FETCH(0, "aaaaaaaaaa", !CACHED);
  FETCH(0, "aaaaaaaaaa", CACHED);
  FETCH(0, "aaaaaaaaaa", CACHED);

  FETCH(1, "bbbbbbbbbb", !CACHED);
  FETCH(0, "aaaaaaaaaa", CACHED);
  FETCH(1, "bbbbbbbbbb", CACHED);

  FETCH(2, "cccccccccc", !CACHED);
  FETCH(2, "cccccccccc", CACHED);
  FETCH(0, "aaaaaaaaaa", CACHED);
  FETCH(1, "bbbbbbbbbb", CACHED);
  FETCH(2, "cccccccccc", CACHED);

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


  clk_free(&clk);
  fail_unless(clk == NULL);
}
END_TEST

START_TEST(test_eviction_order) {
  clk_t *clk = clk_new(10, 8);

  fail_unless(clk != NULL);

  /* fill up */
  FETCH(0, "aaaaaaaaaa", !CACHED);
  FETCH(1, "bbbbbbbbbb", !CACHED);
  FETCH(2, "cccccccccc", !CACHED);
  FETCH(3, "dddddddddd", !CACHED);
  FETCH(4, "eeeeeeeeee", !CACHED);
  FETCH(5, "ffffffffff", !CACHED);
  FETCH(6, "gggggggggg", !CACHED);
  FETCH(7, "hhhhhhhhhh", !CACHED);

  /* refetching does not evict */
  FETCH(1, "bbbbbbbbbb", CACHED);

  /* trigger an eviction by bringing in a new page */
  FETCH(8, "iiiiiiiiii", !CACHED);

  /* now, since the hand is initally at the 0th element in the buffer,
     page 0/a should have been evicted */
  FETCH(0, "aaaaaaaaaa", !CACHED);

  /* bringing back 0/a triggered another eviction, but the clock hand
     was at 1/b and 1/b has been referenced so it should've evicted
     the next page instead, i.e. 2/c, and 1/b should still be in
     cache */
  FETCH(1, "bbbbbbbbbb", CACHED);

  /* and let's verify that 2/c was in fact thrown out by bringing it
     in via a cache miss */
  FETCH(2, "cccccccccc", !CACHED);

  /* obviously, that miss evicted 3/d */
  FETCH(3, "dddddddddd", !CACHED);
  /* and for the sake of clarity, since we just brought back 3/d, 4/e
     is now evicted */

  /* let's reference all but the currently "first" element (and that's
     of course 8/i */
  FETCH(1, "bbbbbbbbbb", CACHED);
  FETCH(0, "aaaaaaaaaa", CACHED);
  FETCH(2, "cccccccccc", CACHED);
  FETCH(3, "dddddddddd", CACHED);
  FETCH(5, "ffffffffff", CACHED);
  FETCH(6, "gggggggggg", CACHED);
  FETCH(7, "hhhhhhhhhh", CACHED);
  /* note that these were referenced in the same order as they're
     currently situated in the clock buffer. not that it matters, but
     good to know. */

  /* since all but the first entry (8/i) have been referenced,
     bringing in a new page should move the hand all the way around to
     8/i and evict it . */
  FETCH(9, "jjjjjjjjjj", !CACHED);

  /* bringing back 8/i to make sure it was kicked out properly */
  FETCH(8, "iiiiiiiiii", !CACHED);

  /* and that's probably enough */

  clk_free(&clk);
  fail_unless(clk == NULL);
}
END_TEST


Suite *clk_suite() {
  TCase *tc;
  Suite *s;

  s = suite_create ("clk");

  tc = tcase_create ("foo");
  tcase_add_test (tc, test_no_eviction);
  tcase_add_test (tc, test_eviction_order);
  suite_add_tcase (s, tc);

  return s;
}

int main(void) {
  int number_failed;
  Suite *s = clk_suite();
  SRunner *sr = srunner_create(s);
  srunner_run_all (sr, CK_NORMAL);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return (number_failed == 0) ? 0 : 1;
}
