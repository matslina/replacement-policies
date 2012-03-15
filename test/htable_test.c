#include <check.h>
#include "htable.h"

#define GET_AND_CHECK(t, k, v)                          \
  do {                                                  \
    void *val;                                          \
    fail_unless(!htable_get(t, k, (void **)&val));       \
    fail_unless(val == (void *)v,                       \
                "expected %x got %x", v, val);         \
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
  tcase_add_test (tc, test_deletion);
  tcase_add_test (tc, test_free);
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
