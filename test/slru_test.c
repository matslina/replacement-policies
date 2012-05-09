#include <stdio.h>
#include <string.h>
#include <check.h>
#include "slru.h"

/* NOTE: these tests assume that the protected segment will hold
 * nmemb>>1 entries (with the remaining nmemb-(nmemb>>1) slots for the
 * probationary segment) */

#define CACHED 0
#define FETCH(key, data, cached)                              \
  do {                                                        \
    void *p;                                                  \
    fail_unless(cached == slru_fetch(slru, key, &p));         \
    if (cached == CACHED)                                     \
      fail_unless(!memcmp(p, data, strlen(data)));            \
    else                                                      \
      memcpy(p, data, strlen(data));                          \
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

  /* 0/a and 1/b should now be protected, leaving 4 slots in the
     probationary segment. fill the probationary and check that the
     protected entries are still cached. */
  FETCH(2, "cccccccccc", !CACHED);
  FETCH(3, "dddddddddd", !CACHED);
  FETCH(4, "eeeeeeeeee", !CACHED);
  FETCH(5, "ffffffffff", !CACHED);
  FETCH(1, "bbbbbbbbbb", CACHED);
  FETCH(0, "aaaaaaaaaa", CACHED);

  /* refetching from the probationary promotes to protected */
  FETCH(3, "dddddddddd", CACHED);
  FETCH(4, "eeeeeeeeee", CACHED);

  /* so now we should have 2 slots left in the probationary, while the
     protected is full */
  FETCH(6, "gggggggggg", !CACHED);
  FETCH(7, "hhhhhhhhhh", !CACHED);

  /* all 8 entries are now in cache */
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


START_TEST(test_probationary_eviction) {
  slru_t *slru;

  /* fill up probationary segment, then add a new element and try to
   * verify that the LRU (0/a in this case) is evicted */
  slru = slru_new(10, 8);
  FETCH(0, "aaaaaaaaaa", !CACHED);
  FETCH(1, "bbbbbbbbbb", !CACHED);
  FETCH(2, "cccccccccc", !CACHED);
  FETCH(3, "dddddddddd", !CACHED);
  /* segment full at this point */
  FETCH(4, "eeeeeeeeee", !CACHED);
  /* try to verify */
  FETCH(1, "bbbbbbbbbb", CACHED);
  FETCH(2, "cccccccccc", CACHED);
  FETCH(3, "dddddddddd", CACHED);
  FETCH(4, "eeeeeeeeee", CACHED);
  FETCH(0, "aaaaaaaaaa", !CACHED);
  slru_free(&slru);
  fail_unless(slru == NULL);

  /* repeat, but with more entries */
  slru = slru_new(10, 8);
  FETCH(0, "aaaaaaaaaa", !CACHED);
  FETCH(1, "bbbbbbbbbb", !CACHED);
  FETCH(2, "cccccccccc", !CACHED);
  FETCH(3, "dddddddddd", !CACHED);
  FETCH(4, "eeeeeeeeee", !CACHED);
  FETCH(5, "ffffffffff", !CACHED);
  FETCH(6, "gggggggggg", !CACHED);
  FETCH(7, "hhhhhhhhhh", !CACHED);
  FETCH(8, "iiiiiiiiii", !CACHED);
  /* only the 4 most recent entries should remain in cache */
  FETCH(5, "ffffffffff", CACHED);
  FETCH(6, "gggggggggg", CACHED);
  FETCH(7, "hhhhhhhhhh", CACHED);
  FETCH(8, "iiiiiiiiii", CACHED);
  FETCH(0, "aaaaaaaaaa", !CACHED);
  FETCH(1, "bbbbbbbbbb", !CACHED);
  FETCH(2, "cccccccccc", !CACHED);
  FETCH(3, "dddddddddd", !CACHED);
  FETCH(4, "eeeeeeeeee", !CACHED);
  slru_free(&slru);
  fail_unless(slru == NULL);

}
END_TEST

START_TEST(test_promotion) {
  slru_t *slru;

  slru = slru_new(10, 8);

  /* fetch and refetch two entries so that they get promoted to the
   * protected segment */
  FETCH(0, "aaaaaaaaaa", !CACHED);
  FETCH(1, "bbbbbbbbbb", !CACHED);
  FETCH(0, "aaaaaaaaaa", CACHED);
  FETCH(1, "bbbbbbbbbb", CACHED);

  /* adding 4 new entries will not affect the 2 protected */
  FETCH(2, "cccccccccc", !CACHED);
  FETCH(3, "dddddddddd", !CACHED);
  FETCH(4, "eeeeeeeeee", !CACHED);
  FETCH(5, "ffffffffff", !CACHED);
  FETCH(0, "aaaaaaaaaa", CACHED);
  FETCH(1, "bbbbbbbbbb", CACHED);

  /* refetching any of the 4 probationary entries will promote it */
  FETCH(3, "dddddddddd", CACHED);
  FETCH(0, "aaaaaaaaaa", CACHED);
  FETCH(1, "bbbbbbbbbb", CACHED);
  FETCH(3, "dddddddddd", CACHED);

  slru_free(&slru);
  fail_unless(slru == NULL);
}
END_TEST

START_TEST(test_demotion) {
  slru_t *slru = slru_new(10, 8);

  /* fill up the protected segment */
  FETCH(0, "aaaaaaaaaa", !CACHED);
  FETCH(1, "bbbbbbbbbb", !CACHED);
  FETCH(2, "cccccccccc", !CACHED);
  FETCH(3, "dddddddddd", !CACHED);
  FETCH(0, "aaaaaaaaaa", CACHED);
  FETCH(1, "bbbbbbbbbb", CACHED);
  FETCH(2, "cccccccccc", CACHED);
  FETCH(3, "dddddddddd", CACHED);

  /* another promoted entry should demote 0/a to probationary, since
   * that is the protected LRU */
  FETCH(4, "eeeeeeeeee", !CACHED);
  FETCH(4, "eeeeeeeeee", CACHED);

  /* to verify that 0/a truly is in the probationary segment, we add 4
   * new entries and check that 0/a got evicted completely */
  FETCH(5, "ffffffffff", !CACHED);
  FETCH(6, "gggggggggg", !CACHED);
  FETCH(7, "hhhhhhhhhh", !CACHED);
  FETCH(8, "iiiiiiiiii", !CACHED);
  FETCH(0, "aaaaaaaaaa", !CACHED);

  /* to verify that 0/a wasn't just dropped entirely when we tried to
     trigger demotion, we repeat the process for 1/b (the current
     protected LRU) but only add 3 new records. that should keep 1/b
     in cache. */

  /* promote 5/f to protected, thereby demoting 1/b */
  FETCH(5, "ffffffffff", !CACHED);
  /* 3 new entries in probationary */
  FETCH(9, "jjjjjjjjjj", !CACHED);
  FETCH(10, "kkkkkkkkkk", !CACHED);
  FETCH(11, "llllllllll", !CACHED);
  /* and 1/b should be probationary LRU, but still not evicted */
  FETCH(1, "bbbbbbbbbb", CACHED);

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
  tcase_add_test (tc, test_promotion);
  tcase_add_test (tc, test_demotion);
  tcase_add_test (tc, test_probationary_eviction);
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
