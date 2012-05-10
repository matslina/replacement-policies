#include <check.h>
#include "lrutable.h"

#include <stdio.h>

#define GET_AND_CHECK(t, k, v)                          \
  do {                                                  \
    void *val;                                          \
    fail_unless(!lrutable_get(t, k, (void **)&val));      \
    fail_unless(val == (void *)v,                       \
                "expected %x got %x", v, val);          \
  } while (0)

/*********************************
 *  Hash table tests             *
 *  (copied from htable_test.c)  *
 ********************************/

START_TEST(test_set_get) {
  lrutable_t *t = lrutable_new(10);

  fail_unless(!lrutable_set(t, 42, (void *)1337));
  GET_AND_CHECK(t, 42, 1337);

  fail_unless(!lrutable_set(t, 1, (void *)101));
  fail_unless(!lrutable_set(t, 2, (void *)102));
  GET_AND_CHECK(t, 42, 1337);
  GET_AND_CHECK(t, 1, 101);
  GET_AND_CHECK(t, 2, 102);

  fail_unless(!lrutable_set(t, 0xdeadbeefdeadbeefLL, (void *)0xcafebabe));
  GET_AND_CHECK(t, 42, 1337);
  GET_AND_CHECK(t, 1, 101);
  GET_AND_CHECK(t, 2, 102);
  GET_AND_CHECK(t, 0xdeadbeefdeadbeefLL, 0xcafebabe);

  fail_unless(!lrutable_set(t, 4711, NULL));
  GET_AND_CHECK(t, 42, 1337);
  GET_AND_CHECK(t, 1, 101);
  GET_AND_CHECK(t, 2, 102);
  GET_AND_CHECK(t, 0xdeadbeefdeadbeefLL, 0xcafebabe);
  GET_AND_CHECK(t, 4711, NULL);

  lrutable_free(&t);
  fail_unless(t == NULL);
}
END_TEST

START_TEST(test_set_full) {
  lrutable_t *t = lrutable_new(5);

  fail_unless(!lrutable_set(t, 1, (void *)101));
  fail_unless(!lrutable_set(t, 2, (void *)102));
  fail_unless(!lrutable_set(t, 3, (void *)103));
  fail_unless(!lrutable_set(t, 4, (void *)104));
  fail_unless(!lrutable_set(t, 5, (void *)105));
  GET_AND_CHECK(t, 1, 101);
  GET_AND_CHECK(t, 2, 102);
  GET_AND_CHECK(t, 3, 103);
  GET_AND_CHECK(t, 4, 104);
  GET_AND_CHECK(t, 5, 105);

  fail_unless(-1 == lrutable_set(t, 1, (void *)1231));
  fail_unless(-1 == lrutable_set(t, 6, (void *)1231));
}
END_TEST

START_TEST(test_stacking) {
  lrutable_t *t = lrutable_new(10);

  /* push */
  fail_unless(!lrutable_set(t, 42, (void *)1337));
  GET_AND_CHECK(t, 42, 1337);

  /* push */
  fail_unless(!lrutable_set(t, 42, (void *)12));
  GET_AND_CHECK(t, 42, 12);

  /* push */
  fail_unless(!lrutable_set(t, 42, (void *)NULL));
  GET_AND_CHECK(t, 42, NULL);

  /* pop */
  fail_unless(!lrutable_del(t, 42));
  GET_AND_CHECK(t, 42, 12);

  /* pop */
  fail_unless(!lrutable_del(t, 42));
  GET_AND_CHECK(t, 42, 1337);

  /* push */
  fail_unless(!lrutable_set(t, 42, (void *)44444));
  GET_AND_CHECK(t, 42, 44444);

  /* pop */
  fail_unless(!lrutable_del(t, 42));
  GET_AND_CHECK(t, 42, 1337);

  /* pop */
  fail_unless(!lrutable_del(t, 42));
  fail_unless(1 == lrutable_get(t, 42, NULL));

  lrutable_free(&t);
  fail_unless(t == NULL);
}
END_TEST

START_TEST(test_get_missing) {
  void *v;
  lrutable_t *t = lrutable_new(10);

  fail_unless(1 == lrutable_get(t, 42, &v));
  fail_unless(1 == lrutable_get(t, 4711, &v));

  fail_unless(!lrutable_set(t, 42, (void *)1337));
  GET_AND_CHECK(t, 42, 1337);
  fail_unless(1 == lrutable_get(t, 4711, &v));
  fail_unless(1 == lrutable_get(t, 1, &v));

  fail_unless(!lrutable_set(t, 4711, (void *)4711));
  GET_AND_CHECK(t, 4711, 4711);
  GET_AND_CHECK(t, 42, 1337);

  fail_unless(1 == lrutable_get(t, 1, &v));

  lrutable_free(&t);
  fail_unless(t == NULL);
}
END_TEST

START_TEST(test_deletion) {
  void *v;
  lrutable_t *t = lrutable_new(10);

  fail_unless(1 == lrutable_get(t, 125, &v));

  fail_unless(!lrutable_set(t, 125, (void *)521));
  GET_AND_CHECK(t, 125, 521);

  fail_unless(!lrutable_del(t, 125));
  fail_unless(1 == lrutable_get(t, 125, &v));

  fail_unless(!lrutable_set(t, 125, (void *)521));
  fail_unless(!lrutable_set(t, 126, (void *)621));
  fail_unless(!lrutable_set(t, 127, (void *)721));
  fail_unless(!lrutable_set(t, 128, (void *)821));
  fail_unless(!lrutable_set(t, 129, (void *)921));
  GET_AND_CHECK(t, 125, 521);
  GET_AND_CHECK(t, 126, 621);
  GET_AND_CHECK(t, 127, 721);
  GET_AND_CHECK(t, 128, 821);
  GET_AND_CHECK(t, 129, 921);

  fail_unless(!lrutable_del(t, 125));
  fail_unless(!lrutable_del(t, 127));
  fail_unless(!lrutable_del(t, 129));

  fail_unless(1 == lrutable_get(t, 125, &v));
  GET_AND_CHECK(t, 126, 621);
  fail_unless(1 == lrutable_get(t, 127, &v));
  GET_AND_CHECK(t, 128, 821);
  fail_unless(1 == lrutable_get(t, 129, &v));

  fail_unless(!lrutable_set(t, 125, (void *)521));
  fail_unless(!lrutable_set(t, 127, (void *)721));
  fail_unless(!lrutable_set(t, 129, (void *)921));
  GET_AND_CHECK(t, 125, 521);
  GET_AND_CHECK(t, 126, 621);
  GET_AND_CHECK(t, 127, 721);
  GET_AND_CHECK(t, 128, 821);
  GET_AND_CHECK(t, 129, 921);

  fail_unless(!lrutable_del(t, 125));
  fail_unless(!lrutable_del(t, 126));
  fail_unless(!lrutable_del(t, 127));
  fail_unless(!lrutable_del(t, 128));
  fail_unless(!lrutable_del(t, 129));
  fail_unless(1 == lrutable_get(t, 125, &v));
  fail_unless(1 == lrutable_get(t, 126, &v));
  fail_unless(1 == lrutable_get(t, 127, &v));
  fail_unless(1 == lrutable_get(t, 128, &v));
  fail_unless(1 == lrutable_get(t, 129, &v));

  lrutable_free(&t);
  fail_unless(t == NULL);
}
END_TEST

START_TEST(test_free) {
  lrutable_t *t = lrutable_new(123);
  fail_unless(t != NULL);
  lrutable_free(&t);
  fail_unless(t == NULL);
}
END_TEST


/*********************************
 *  LRU table tests              *
 *  (stuff not in htable_test.c) *
 ********************************/

START_TEST(test_get_lru) {
  uint64_t key;
  void *val;
  lrutable_t *t = lrutable_new(5);

  /* empty table has no LRU */
  fail_unless(-1 == lrutable_get_lru(t, &key, &val));

  /* single element must be LRU */
  fail_unless(!lrutable_set(t, 0, (void *)100));
  fail_unless(!lrutable_get_lru(t, &key, &val));
  fail_unless(key == 0 && val == (void *)100);

  /* with many elements, the last set is the LRU */
  fail_unless(!lrutable_set(t, 1, (void *)101));
  fail_unless(!lrutable_set(t, 2, (void *)102));
  fail_unless(!lrutable_set(t, 3, (void *)103));
  fail_unless(!lrutable_get_lru(t, &key, &val));
  fail_unless(key == 0 && val == (void *)100);

  /* the get_lru operation does not turn LRU into MRU */
  fail_unless(!lrutable_get_lru(t, &key, &val));
  fail_unless(key == 0 && val == (void *)100);
  fail_unless(!lrutable_get_lru(t, &key, &val));
  fail_unless(key == 0 && val == (void *)100);

  /* filling the table doesn't muck this up */
  fail_unless(!lrutable_set(t, 4, (void *)104));
  fail_unless(-1 == lrutable_set(t, 5, (void *)105));

  fail_unless(!lrutable_get_lru(t, &key, &val));
  fail_unless(key == 0 && val == (void *)100);
  fail_unless(!lrutable_get_lru(t, &key, &val));
  fail_unless(key == 0 && val == (void *)100);

  lrutable_free(&t);
  fail_unless(t == NULL);
}
END_TEST

START_TEST(test_pop_lru) {
  uint64_t key;
  void *val;
  lrutable_t *t = lrutable_new(5);

  /* empty table has no LRU */
  fail_unless(-1 == lrutable_pop_lru(t, &key, &val));

  /* single element can be popped */
  fail_unless(!lrutable_set(t, 0, (void *)100));
  fail_unless(!lrutable_pop_lru(t, &key, &val));
  fail_unless(key == 0 && val == (void *)100);

  /* that should have removed the element */
  fail_unless(1 == lrutable_get(t, 0, &val));

  /* fill it up */
  fail_unless(!lrutable_set(t, 0, (void *)100));
  fail_unless(!lrutable_set(t, 1, (void *)101));
  fail_unless(!lrutable_set(t, 2, (void *)102));
  fail_unless(!lrutable_set(t, 3, (void *)103));
  fail_unless(!lrutable_set(t, 4, (void *)104));

  /* pop them all */
  fail_unless(!lrutable_pop_lru(t, &key, &val));
  fail_unless(key == 0 && val == (void *)100);
  fail_unless(!lrutable_pop_lru(t, &key, &val));
  fail_unless(key == 1 && val == (void *)101);
  fail_unless(!lrutable_pop_lru(t, &key, &val));
  fail_unless(key == 2 && val == (void *)102);
  fail_unless(!lrutable_pop_lru(t, &key, &val));
  fail_unless(key == 3 && val == (void *)103);
  fail_unless(!lrutable_pop_lru(t, &key, &val));
  fail_unless(key == 4 && val == (void *)104);

  /* table is now empty */
  fail_unless(-1 == lrutable_pop_lru(t, &key, &val));

  lrutable_free(&t);
  fail_unless(t == NULL);
}
END_TEST

START_TEST(test_del_lru) {
  uint64_t key;
  void *val;
  lrutable_t *t = lrutable_new(5);

  /* empty table has no LRU */
  fail_unless(-1 == lrutable_del_lru(t));

  /* table of size 1 has 1 LRU */
  fail_unless(!lrutable_set(t, 0, (void *)100));
  fail_unless(!lrutable_del_lru(t));
  fail_unless(-1 == lrutable_del_lru(t));

  /* with several elements, only the LRU disappears */
  fail_unless(!lrutable_set(t, 0, (void *)100));
  fail_unless(!lrutable_set(t, 1, (void *)101));
  fail_unless(!lrutable_set(t, 2, (void *)102));
  fail_unless(!lrutable_set(t, 3, (void *)103));
  fail_unless(!lrutable_del_lru(t));
  GET_AND_CHECK(t, 3, 103);
  GET_AND_CHECK(t, 2, 102);
  GET_AND_CHECK(t, 1, 101);
  fail_unless(1 == lrutable_get(t, 0, &val));

  /* fill it up and del_lru() until it's empty */
  fail_unless(!lrutable_set(t, 4, (void *)104));
  fail_unless(!lrutable_set(t, 5, (void *)105));
  /* MRU = 5, 4, 1, 2, 3 = LRU */
  fail_unless(!lrutable_del_lru(t));
  GET_AND_CHECK(t, 1, 101);
  GET_AND_CHECK(t, 2, 102);
  GET_AND_CHECK(t, 5, 105);
  GET_AND_CHECK(t, 4, 104);
  fail_unless(1 == lrutable_get(t, 3, &val));
  /* MRU = 4, 5, 2, 1 = LRU */
  fail_unless(!lrutable_del_lru(t));
  GET_AND_CHECK(t, 4, 104);
  GET_AND_CHECK(t, 5, 105);
  GET_AND_CHECK(t, 2, 102);
  fail_unless(1 == lrutable_get(t, 1, &val));
  /* MRU = 2, 5, 4 = LRU */
  fail_unless(!lrutable_del_lru(t));
  GET_AND_CHECK(t, 2, 102);
  GET_AND_CHECK(t, 5, 105);
  fail_unless(1 == lrutable_get(t, 4, &val));
  /* MRU = 5, 2 = LRU */
  fail_unless(!lrutable_del_lru(t));
  GET_AND_CHECK(t, 5, 105);
  fail_unless(1 == lrutable_get(t, 2, &val));
  /* MRU = 5 = LRU */
  fail_unless(!lrutable_del_lru(t));
  fail_unless(1 == lrutable_get(t, 5, &val));
  fail_unless(-1 == lrutable_del_lru(t));

  lrutable_free(&t);
  fail_unless(t == NULL);
}
END_TEST

START_TEST(test_lruness_of_htable_operations) {
  uint64_t key;
  void *val;
  lrutable_t *t = lrutable_new(5);

  /* fill it up */
  fail_unless(!lrutable_set(t, 0, (void *)100));
  fail_unless(!lrutable_set(t, 1, (void *)101));
  fail_unless(!lrutable_set(t, 2, (void *)102));
  fail_unless(!lrutable_set(t, 3, (void *)103));
  fail_unless(!lrutable_set(t, 4, (void *)104));

  /* least recently set is LRU */
  fail_unless(!lrutable_get_lru(t, &key, &val));
  fail_unless(key == 0 && val == (void *)100);

  /* lrutable_get() makes entry MRU */
  fail_unless(!lrutable_get(t, 0, &val));
  fail_unless(!lrutable_get_lru(t, &key, &val));
  fail_unless(key == 1 && val == (void *)101);
  fail_unless(!lrutable_get(t, 1, &val));
  fail_unless(!lrutable_get(t, 2, &val));
  fail_unless(!lrutable_get(t, 3, &val));
  fail_unless(!lrutable_get_lru(t, &key, &val));
  fail_unless(key == 4 && val == (void *)104);

  /* lrutable_del() doesn't break the ordering
   * current order: MRU = 3, 2, 1, 0, 4 = LRU */
  fail_unless(!lrutable_del(t, 1));
  fail_unless(!lrutable_get_lru(t, &key, &val));
  fail_unless(key == 4 && val == (void *)104);
  fail_unless(!lrutable_del(t, 4));
  fail_unless(!lrutable_get_lru(t, &key, &val));
  fail_unless(key == 0 && val == (void *)100);

  /* nor does lrutable_pop()
   * current order: MRU = 3, 2, 0 = LRU */
  fail_unless(!lrutable_pop(t, 2, &val));
  fail_unless(!lrutable_get_lru(t, &key, &val));
  fail_unless(key == 0 && val == (void *)100);
  fail_unless(!lrutable_pop(t, 0, &val));
  fail_unless(!lrutable_get_lru(t, &key, &val));
  fail_unless(key == 3 && val == (void *)103);

  lrutable_free(&t);
  fail_unless(t == NULL);
}
END_TEST


Suite *lrutable_suite() {
  TCase *tc;
  Suite *s;

  s = suite_create ("lrutable");

  tc = tcase_create ("hash");
  tcase_add_test (tc, test_set_get);
  tcase_add_test (tc, test_get_missing);
  tcase_add_test (tc, test_deletion);
  tcase_add_test (tc, test_free);
  tcase_add_test (tc, test_stacking);
  tcase_add_test (tc, test_set_full);
  suite_add_tcase (s, tc);

  tc = tcase_create ("lru");
  tcase_add_test (tc, test_get_lru);
  tcase_add_test (tc, test_pop_lru);
  tcase_add_test (tc, test_del_lru);
  tcase_add_test (tc, test_lruness_of_htable_operations);
  suite_add_tcase (s, tc);
  return s;
}

int main(void) {
  int number_failed;
  Suite *s = lrutable_suite();
  SRunner *sr = srunner_create(s);
  srunner_run_all (sr, CK_NORMAL);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return (number_failed == 0) ? 0 : 1;
}
