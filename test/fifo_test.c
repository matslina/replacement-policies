#include <stdio.h>
#include <string.h>
#include <check.h>
#include "fifo.h"

#include <stdio.h>

#define CACHED 0
#define FETCH(key, data, cached)                              \
  do {                                                        \
    void *p;                                                  \
    char tbuf[10000];                                         \
    fail_unless(cached == fifo_fetch(fifo, key, &p));         \
    if (cached == CACHED)                                     \
      fail_unless(!memcmp(p, data, strlen(data)));            \
    else                                                      \
      memcpy(p, data, strlen(data));                          \
  } while(0)

START_TEST(test_no_eviction) {
  fifo_t *fifo = fifo_new(10, 8);

  fail_unless(fifo != NULL);

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


  fifo_free(&fifo);
  fail_unless(fifo == NULL);
}
END_TEST

START_TEST(test_eviction_order) {
  fifo_t *fifo = fifo_new(10, 8);

  fail_unless(fifo != NULL);

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
  FETCH(0, "aaaaaaaaaa", CACHED);
  FETCH(4, "eeeeeeeeee", CACHED);
  FETCH(7, "hhhhhhhhhh", CACHED);

  /* fetching a new one only evicts 0, since the first in is the first
     out */
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

  /* bring in a few more */
  FETCH(9, "jjjjjjjjjj", !CACHED);
  FETCH(10, "kkkkkkkkkk", !CACHED);
  FETCH(11, "llllllllll", !CACHED);
  FETCH(5, "ffffffffff", CACHED);
  FETCH(6, "gggggggggg", CACHED);
  FETCH(7, "hhhhhhhhhh", CACHED);
  FETCH(8, "iiiiiiiiii", CACHED);
  FETCH(9, "jjjjjjjjjj", CACHED);
  FETCH(10, "kkkkkkkkkk", CACHED);
  FETCH(11, "llllllllll", CACHED);
  FETCH(0, "aaaaaaaaaa", CACHED);

  /* and the oldest three were evicted */
  FETCH(1, "bbbbbbbbbb", !CACHED);
  FETCH(2, "cccccccccc", !CACHED);
  FETCH(3, "dddddddddd", !CACHED);
  FETCH(4, "eeeeeeeeee", !CACHED);

  fifo_free(&fifo);
  fail_unless(fifo == NULL);
}
END_TEST


Suite *fifo_suite() {
  TCase *tc;
  Suite *s;

  s = suite_create ("fifo");

  tc = tcase_create ("foo");
  tcase_add_test (tc, test_eviction_order);
  suite_add_tcase (s, tc);

  return s;
}

int main(void) {
  int number_failed;
  Suite *s = fifo_suite();
  SRunner *sr = srunner_create(s);
  srunner_run_all (sr, CK_NORMAL);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return (number_failed == 0) ? 0 : 1;
}
