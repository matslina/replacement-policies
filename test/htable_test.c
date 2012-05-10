#include <check.h>
#include "htable.h"

#define GET_AND_CHECK(t, k, v)                          \
  do {                                                  \
    void *val;                                          \
    fail_unless(!htable_get(t, k, (void **)&val));      \
    fail_unless(val == (void *)v,                       \
                "expected %x got %x", v, val);          \
  } while (0)


START_TEST(test_set_get) {
  htable_t *t = htable_new(10);

  fail_unless(!htable_set(t, 42, (void *)1337));
  GET_AND_CHECK(t, 42, 1337);

  fail_unless(!htable_set(t, 1, (void *)101));
  fail_unless(!htable_set(t, 2, (void *)102));
  GET_AND_CHECK(t, 42, 1337);
  GET_AND_CHECK(t, 1, 101);
  GET_AND_CHECK(t, 2, 102);

  fail_unless(!htable_set(t, 0xdeadbeefdeadbeefLL, (void *)0xcafebabe));
  GET_AND_CHECK(t, 42, 1337);
  GET_AND_CHECK(t, 1, 101);
  GET_AND_CHECK(t, 2, 102);
  GET_AND_CHECK(t, 0xdeadbeefdeadbeefLL, 0xcafebabe);

  fail_unless(!htable_set(t, 4711, NULL));
  GET_AND_CHECK(t, 42, 1337);
  GET_AND_CHECK(t, 1, 101);
  GET_AND_CHECK(t, 2, 102);
  GET_AND_CHECK(t, 0xdeadbeefdeadbeefLL, 0xcafebabe);
  GET_AND_CHECK(t, 4711, NULL);

  htable_free(&t);
  fail_unless(t == NULL);
}
END_TEST

START_TEST(test_set_full) {
  htable_t *t = htable_new(5);

  fail_unless(!htable_set(t, 1, (void *)101));
  fail_unless(!htable_set(t, 2, (void *)102));
  fail_unless(!htable_set(t, 3, (void *)103));
  fail_unless(!htable_set(t, 4, (void *)104));
  fail_unless(!htable_set(t, 5, (void *)105));
  GET_AND_CHECK(t, 1, 101);
  GET_AND_CHECK(t, 2, 102);
  GET_AND_CHECK(t, 3, 103);
  GET_AND_CHECK(t, 4, 104);
  GET_AND_CHECK(t, 5, 105);

  fail_unless(-1 == htable_set(t, 1, (void *)1231));
  fail_unless(-1 == htable_set(t, 6, (void *)1231));
}
END_TEST

START_TEST(test_stacking) {
  htable_t *t = htable_new(10);

  /* push */
  fail_unless(!htable_set(t, 42, (void *)1337));
  GET_AND_CHECK(t, 42, 1337);

  /* push */
  fail_unless(!htable_set(t, 42, (void *)12));
  GET_AND_CHECK(t, 42, 12);

  /* push */
  fail_unless(!htable_set(t, 42, (void *)NULL));
  GET_AND_CHECK(t, 42, NULL);

  /* pop */
  fail_unless(!htable_del(t, 42));
  GET_AND_CHECK(t, 42, 12);

  /* pop */
  fail_unless(!htable_del(t, 42));
  GET_AND_CHECK(t, 42, 1337);

  /* push */
  fail_unless(!htable_set(t, 42, (void *)44444));
  GET_AND_CHECK(t, 42, 44444);

  /* pop */
  fail_unless(!htable_del(t, 42));
  GET_AND_CHECK(t, 42, 1337);

  /* pop */
  fail_unless(!htable_del(t, 42));
  fail_unless(1 == htable_get(t, 42, NULL));

  htable_free(&t);
  fail_unless(t == NULL);
}
END_TEST

START_TEST(test_get_missing) {
  void *v;
  htable_t *t = htable_new(10);

  fail_unless(1 == htable_get(t, 42, &v));
  fail_unless(1 == htable_get(t, 4711, &v));

  fail_unless(!htable_set(t, 42, (void *)1337));
  GET_AND_CHECK(t, 42, 1337);
  fail_unless(1 == htable_get(t, 4711, &v));
  fail_unless(1 == htable_get(t, 1, &v));

  fail_unless(!htable_set(t, 4711, (void *)4711));
  GET_AND_CHECK(t, 4711, 4711);
  GET_AND_CHECK(t, 42, 1337);

  fail_unless(1 == htable_get(t, 1, &v));

  htable_free(&t);
  fail_unless(t == NULL);
}
END_TEST

START_TEST(test_pop) {
  void *val;
  htable_t *t = htable_new(5);

  /* fill the table */
  fail_unless(!htable_set(t, 0, (void *)100));
  fail_unless(!htable_set(t, 1, (void *)101));
  fail_unless(!htable_set(t, 2, (void *)102));
  fail_unless(!htable_set(t, 3, (void *)103));
  fail_unless(!htable_set(t, 4, (void *)104));
  fail_unless(-1 == htable_set(t, 5, (void *)105));

  /* can't pop entries not set */
  fail_unless(1 == htable_pop(t, 5, &val));
  fail_unless(1 == htable_pop(t, 6, &val));
  fail_unless(1 == htable_pop(t, 7, &val));
  fail_unless(1 == htable_pop(t, 8, &val));
  fail_unless(1 == htable_pop(t, 9, &val));
  fail_unless(1 == htable_pop(t, 10, &val));
  fail_unless(1 == htable_pop(t, 11, &val));

  /* set entries can be popped */
  fail_unless(!htable_pop(t, 1, &val));
  fail_unless(val == (void *)101);
  fail_unless(!htable_pop(t, 4, &val));
  fail_unless(val == (void *)104);

  /* popped entries can't be retrieved or popped */
  fail_unless(1 == htable_get(t, 1, &val));
  fail_unless(1 == htable_pop(t, 1, &val));
  fail_unless(1 == htable_get(t, 4, &val));
  fail_unless(1 == htable_pop(t, 4, &val));

  /* popping made room for new entries */
  fail_unless(!htable_set(t, 0, (void *)105));
  fail_unless(!htable_set(t, 1, (void *)106));

  /* multiple entries with same keys are popped as if stacked */
  fail_unless(!htable_pop(t, 0, &val));
  fail_unless(val == (void *)105);
  fail_unless(!htable_pop(t, 0, &val));
  fail_unless(val == (void *)100);
  fail_unless(1 == htable_pop(t, 0, &val));

  htable_free(&t);
  fail_unless(t == NULL);
}
END_TEST

START_TEST(test_deletion) {
  void *v;
  htable_t *t = htable_new(10);

  fail_unless(1 == htable_get(t, 125, &v));

  fail_unless(!htable_set(t, 125, (void *)521));
  GET_AND_CHECK(t, 125, 521);

  fail_unless(!htable_del(t, 125));
  fail_unless(1 == htable_get(t, 125, &v));

  fail_unless(!htable_set(t, 125, (void *)521));
  fail_unless(!htable_set(t, 126, (void *)621));
  fail_unless(!htable_set(t, 127, (void *)721));
  fail_unless(!htable_set(t, 128, (void *)821));
  fail_unless(!htable_set(t, 129, (void *)921));
  GET_AND_CHECK(t, 125, 521);
  GET_AND_CHECK(t, 126, 621);
  GET_AND_CHECK(t, 127, 721);
  GET_AND_CHECK(t, 128, 821);
  GET_AND_CHECK(t, 129, 921);

  fail_unless(!htable_del(t, 125));
  fail_unless(!htable_del(t, 127));
  fail_unless(!htable_del(t, 129));

  fail_unless(1 == htable_get(t, 125, &v));
  GET_AND_CHECK(t, 126, 621);
  fail_unless(1 == htable_get(t, 127, &v));
  GET_AND_CHECK(t, 128, 821);
  fail_unless(1 == htable_get(t, 129, &v));

  fail_unless(!htable_set(t, 125, (void *)521));
  fail_unless(!htable_set(t, 127, (void *)721));
  fail_unless(!htable_set(t, 129, (void *)921));
  GET_AND_CHECK(t, 125, 521);
  GET_AND_CHECK(t, 126, 621);
  GET_AND_CHECK(t, 127, 721);
  GET_AND_CHECK(t, 128, 821);
  GET_AND_CHECK(t, 129, 921);

  fail_unless(!htable_del(t, 125));
  fail_unless(!htable_del(t, 126));
  fail_unless(!htable_del(t, 127));
  fail_unless(!htable_del(t, 128));
  fail_unless(!htable_del(t, 129));
  fail_unless(1 == htable_get(t, 125, &v));
  fail_unless(1 == htable_get(t, 126, &v));
  fail_unless(1 == htable_get(t, 127, &v));
  fail_unless(1 == htable_get(t, 128, &v));
  fail_unless(1 == htable_get(t, 129, &v));

  htable_free(&t);
  fail_unless(t == NULL);
}
END_TEST

START_TEST(test_free) {
  htable_t *t = htable_new(123);
  fail_unless(t != NULL);
  htable_free(&t);
  fail_unless(t == NULL);
}
END_TEST

Suite *htable_suite() {
  TCase *tc;
  Suite *s;

  s = suite_create ("htable");
  tc = tcase_create ("foobar");
  tcase_add_test (tc, test_set_get);
  tcase_add_test (tc, test_get_missing);
  tcase_add_test (tc, test_pop);
  tcase_add_test (tc, test_deletion);
  tcase_add_test (tc, test_free);
  tcase_add_test (tc, test_stacking);
  tcase_add_test (tc, test_set_full);
  suite_add_tcase (s, tc);

  return s;
}

int main(void) {
  int number_failed;
  Suite *s = htable_suite();
  SRunner *sr = srunner_create(s);
  srunner_run_all (sr, CK_NORMAL);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return (number_failed == 0) ? 0 : 1;
}
