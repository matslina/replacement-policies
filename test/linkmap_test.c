#include <check.h>
#include "linkmap.h"

#include <stdio.h>


START_TEST(test_new) {
  linkmap_t *lm;

  /* linkmaps of different capacity can be created */
  lm = linkmap_new(0);
  fail_unless(lm != NULL);
  lm = linkmap_new(1);
  fail_unless(lm != NULL);
  lm = linkmap_new(4);
  fail_unless(lm != NULL);
}
END_TEST

START_TEST(test_free) {
  linkmap_t *lm;

  /* linkmaps of different capacity can be free'd */
  lm = linkmap_new(0);
  linkmap_free(&lm);
  fail_unless(lm == NULL);
  lm = linkmap_new(1);
  linkmap_free(&lm);
  fail_unless(lm == NULL);
  lm = linkmap_new(4);
  linkmap_free(&lm);
  fail_unless(lm == NULL);
}
END_TEST

START_TEST(test_set) {
  linkmap_t *lm;

  /* we can set until the capacity is reached */
  lm = linkmap_new(3);
  fail_unless(!linkmap_set(lm, 12, (void *)112));
  fail_unless(!linkmap_set(lm, 13, (void *)113));
  fail_unless(!linkmap_set(lm, 14, (void *)114));
  fail_unless(1 == linkmap_set(lm, 15, (void *)115));
  linkmap_free(&lm);
  fail_unless(lm == NULL);

  /* capacity < 1 => capacity == 1 */
  lm = linkmap_new(1);
  fail_unless(!linkmap_set(lm, 12, (void *)112));
  fail_unless(1 == linkmap_set(lm, 13, (void *)113));
  linkmap_free(&lm);
  fail_unless(lm == NULL);
}
END_TEST

START_TEST(test_get_head_tail) {
  linkmap_t *lm;
  uint64_t key;
  void *ptr;

  /* the empty linkmap has neither head nor tail */
  lm = linkmap_new(3);
  fail_unless(1 == linkmap_get_head(lm, &key, &ptr));
  fail_unless(1 == linkmap_get_tail(lm, &key, &ptr));

  /* the single entry is both head and tail */
  linkmap_set(lm, 12, (void *)112);
  fail_unless(0 == linkmap_get_head(lm, &key, &ptr));
  fail_unless(key == 12);
  fail_unless(ptr == (void *)112);
  fail_unless(0 == linkmap_get_tail(lm, &key, &ptr));
  fail_unless(key == 12);
  fail_unless(ptr == (void *)112);

  /* with many entries, the most recently set is head and the least
   * recently set is tail */
  linkmap_set(lm, 13, (void *)113);
  fail_unless(0 == linkmap_get_head(lm, &key, &ptr));
  fail_unless(key == 13 && ptr == (void *)113);
  fail_unless(0 == linkmap_get_tail(lm, &key, &ptr));
  fail_unless(key == 12 && ptr == (void *)112);
  linkmap_set(lm, 14, (void *)114);
  fail_unless(0 == linkmap_get_head(lm, &key, &ptr));
  fail_unless(key == 14 && ptr == (void *)114);
  fail_unless(0 == linkmap_get_tail(lm, &key, &ptr));
  fail_unless(key == 12 && ptr == (void *)112);

  linkmap_free(&lm);
  fail_unless(lm == NULL);
}
END_TEST

START_TEST(test_get) {
  linkmap_t *lm;
  void *ptr;

  /* we can't get anything from the empty list */
  lm = linkmap_new(5);
  fail_unless(1 == linkmap_get(lm, 112, &ptr));
  fail_unless(1 == linkmap_get(lm, 113, &ptr));

  /* if an element is set, then we can get it by key */
  linkmap_set(lm, 12, (void *)112);
  fail_unless(!linkmap_get(lm, 12, &ptr));
  fail_unless(ptr == (void *)112);

  /* with many set, all can be retrieved by key */
  linkmap_set(lm, 13, (void *)113);
  linkmap_set(lm, 14, (void *)114);
  linkmap_set(lm, 15, (void *)115);
  linkmap_set(lm, 16, (void *)116);
  fail_unless(!linkmap_get(lm, 13, &ptr));
  fail_unless(ptr == (void *)113);
  fail_unless(!linkmap_get(lm, 14, &ptr));
  fail_unless(ptr == (void *)114);
  fail_unless(!linkmap_get(lm, 15, &ptr));
  fail_unless(ptr == (void *)115);
  fail_unless(!linkmap_get(lm, 16, &ptr));
  fail_unless(ptr == (void *)116);
  fail_unless(!linkmap_get(lm, 12, &ptr));
  fail_unless(ptr == (void *)112);

  /* but we still can't retrieve things that doesn't exist */
  fail_unless(1 == linkmap_get(lm, 17, &ptr));
  fail_unless(1 == linkmap_get(lm, 18, &ptr));
  fail_unless(1 == linkmap_get(lm, 19, &ptr));
  fail_unless(1 == linkmap_get(lm, 20, &ptr));
  fail_unless(1 == linkmap_get(lm, 21, &ptr));

  linkmap_free(&lm);
  fail_unless(lm == NULL);
}
END_TEST

START_TEST(test_pop) {
  linkmap_t *lm;
  uint64_t key;
  void *ptr;

  lm = linkmap_new(5);

  /* nothing can be popped from the empty linkmap */
  fail_unless(1 == linkmap_pop(lm, 12, &ptr));
  fail_unless(1 == linkmap_pop(lm, 13, &ptr));

  /* we can pop entries if they exist */
  linkmap_set(lm, 12, (void *)112);
  linkmap_set(lm, 13, (void *)113);
  linkmap_set(lm, 14, (void *)114);
  linkmap_set(lm, 15, (void *)115);
  linkmap_set(lm, 16, (void *)116);
  fail_unless(!linkmap_pop(lm, 12, &ptr));
  fail_unless(ptr == (void *)112);
  fail_unless(!linkmap_pop(lm, 16, &ptr));
  fail_unless(ptr == (void *)116);
  fail_unless(!linkmap_pop(lm, 14, &ptr));
  fail_unless(ptr == (void *)114);

  /* popped entries are removed */
  fail_unless(1 == linkmap_get(lm, 12, &ptr));
  fail_unless(1 == linkmap_get(lm, 16, &ptr));
  fail_unless(1 == linkmap_get(lm, 14, &ptr));
  fail_unless(1 == linkmap_pop(lm, 12, &ptr));
  fail_unless(1 == linkmap_pop(lm, 16, &ptr));
  fail_unless(1 == linkmap_pop(lm, 14, &ptr));

  /* non-popped entries remain and their order is preserved */
  fail_unless(!linkmap_get(lm, 13, &ptr));
  fail_unless(ptr == (void *)113);
  fail_unless(!linkmap_get_head(lm, &key, &ptr));
  fail_unless(key == 15 && ptr == (void *)115);
  fail_unless(!linkmap_get_tail(lm, &key, &ptr));
  fail_unless(key == 13 && ptr == (void *)113);

  linkmap_free(&lm);
  fail_unless(lm == NULL);
}
END_TEST

START_TEST(test_pop_head_tail) {
  linkmap_t *lm;
  uint64_t key;
  void *ptr;

  lm = linkmap_new(5);

  /* empty list has neither head nor tail */
  fail_unless(1 == linkmap_pop_head(lm, &key, &ptr));
  fail_unless(1 == linkmap_pop_tail(lm, &key, &ptr));

  /* the single element is both head and tail */
  linkmap_set(lm, 12, (void *)112);
  fail_unless(!linkmap_pop_head(lm, &key, &ptr));
  fail_unless(key == 12 && ptr == (void *)112);
  fail_unless(1 == linkmap_pop_head(lm, &key, &ptr));
  linkmap_set(lm, 13, (void *)113);
  fail_unless(!linkmap_pop_tail(lm, &key, &ptr));
  fail_unless(key == 13 && ptr == (void *)113);
  fail_unless(1 == linkmap_pop_head(lm, &key, &ptr));

  /* with many elements, we can pop both head and tail */
  linkmap_set(lm, 11, (void *)111);
  linkmap_set(lm, 12, (void *)112);
  linkmap_set(lm, 13, (void *)113);
  linkmap_set(lm, 14, (void *)114);
  linkmap_set(lm, 15, (void *)115);
  fail_unless(!linkmap_pop_head(lm, &key, &ptr));
  fail_unless(key == 15 && ptr == (void *)115);
  fail_unless(!linkmap_pop_tail(lm, &key, &ptr));
  fail_unless(key == 11 && ptr == (void *)111);

  /* popped elements are no longer retrievable */
  fail_unless(1 == linkmap_get(lm, 11, &ptr));
  fail_unless(1 == linkmap_get(lm, 15, &ptr));

  /* ordering of remaining elements is not disturbed */
  fail_unless(!linkmap_get_head(lm, &key, &ptr));
  fail_unless(key == 14 && ptr == (void *)114);
  fail_unless(!linkmap_get_tail(lm, &key, &ptr));
  fail_unless(key == 12 && ptr == (void *)112);

  /* and the linkmap can be emptied by head and tail popping */
  fail_unless(!linkmap_pop_tail(lm, &key, &ptr));
  fail_unless(key == 12 && ptr == (void *)112);
  fail_unless(!linkmap_pop_head(lm, &key, &ptr));
  fail_unless(key == 14 && ptr == (void *)114);
  fail_unless(!linkmap_pop_head(lm, &key, &ptr));
  fail_unless(key == 13 && ptr == (void *)113);

  linkmap_free(&lm);
  fail_unless(lm == NULL);
}
END_TEST

START_TEST(test_del_head_tail) {
  linkmap_t *lm;
  uint64_t key;
  void *ptr;

  /* empty linkmap has neither head nor tail */
  lm = linkmap_new(5);
  fail_unless(1 == linkmap_del_head(lm));
  fail_unless(1 == linkmap_del_tail(lm));

  /* the single element is both head and tail */
  linkmap_set(lm, 12, (void *)112);
  fail_unless(!linkmap_del_head(lm));
  fail_unless(1 == linkmap_del_tail(lm));
  linkmap_set(lm, 12, (void *)112);
  fail_unless(!linkmap_del_tail(lm));
  fail_unless(1 == linkmap_del_head(lm));

  /* the order of the remaining elements is not disturbed */
  linkmap_set(lm, 12, (void *)112);
  linkmap_set(lm, 13, (void *)113);
  linkmap_set(lm, 14, (void *)114);
  linkmap_set(lm, 15, (void *)115);
  linkmap_set(lm, 16, (void *)116);
  fail_unless(!linkmap_del_head(lm));
  fail_unless(!linkmap_get_head(lm, &key, &ptr));
  fail_unless(key == 15 && ptr ==(void *)115);
  fail_unless(!linkmap_del_tail(lm));
  fail_unless(!linkmap_get_tail(lm, &key, &ptr));
  fail_unless(key == 13 && ptr ==(void *)113);
  fail_unless(!linkmap_del_head(lm));
  fail_unless(!linkmap_get_head(lm, &key, &ptr));
  fail_unless(key == 14 && ptr ==(void *)114);
  fail_unless(!linkmap_del_tail(lm));
  fail_unless(!linkmap_get_tail(lm, &key, &ptr));
  fail_unless(key == 14 && ptr ==(void *)114);

  /* deleted heads and tails can't be retrieved by key */
  fail_unless(!linkmap_get(lm, 14, &ptr));
  fail_unless(ptr == (void *)114);
  fail_unless(1 == linkmap_get(lm, 12, &ptr));
  fail_unless(1 == linkmap_get(lm, 13, &ptr));
  fail_unless(1 == linkmap_get(lm, 15, &ptr));
  fail_unless(1 == linkmap_get(lm, 16, &ptr));

  linkmap_free(&lm);
  fail_unless(lm == NULL);
}
END_TEST

START_TEST(test_del) {
  linkmap_t *lm;
  uint64_t key;
  void *ptr;

  /* can't delete anything from the empty linkmap */
  lm = linkmap_new(7);
  fail_unless(1 == linkmap_del(lm, 12));

  /* but we can delete entries that exist */
  linkmap_set(lm, 12, (void *)112);
  linkmap_set(lm, 13, (void *)113);
  linkmap_set(lm, 14, (void *)114);
  linkmap_set(lm, 15, (void *)115);
  linkmap_set(lm, 16, (void *)116);
  linkmap_set(lm, 17, (void *)117);
  linkmap_set(lm, 18, (void *)118);
  fail_unless(!linkmap_del(lm, 12));
  fail_unless(!linkmap_del(lm, 14));
  fail_unless(!linkmap_del(lm, 16));
  fail_unless(!linkmap_del(lm, 18));

  /* deleted entries truly disappear */
  fail_unless(1 == linkmap_get(lm, 12, &ptr));
  fail_unless(1 == linkmap_get(lm, 14, &ptr));
  fail_unless(1 == linkmap_get(lm, 16, &ptr));
  fail_unless(1 == linkmap_get(lm, 18, &ptr));

  /* the remaining entries remain */
  fail_unless(!linkmap_get(lm, 13, &ptr));
  fail_unless(ptr == (void *)113);
  fail_unless(!linkmap_get(lm, 15, &ptr));
  fail_unless(ptr == (void *)115);
  fail_unless(!linkmap_get(lm, 17, &ptr));
  fail_unless(ptr == (void *)117);

  /* deletion doesn't appear to break ordering of remaining entries */
  fail_unless(!linkmap_get_head(lm, &key, &ptr));
  fail_unless(key == 17 && ptr == (void *)117);
  fail_unless(!linkmap_get_tail(lm, &key, &ptr));
  fail_unless(key == 13 && ptr == (void *)113);

  /* deletion should clear space for new entries */
  fail_unless(!linkmap_set(lm, 12, (void *)112));
  fail_unless(!linkmap_get_head(lm, &key, &ptr));
  fail_unless(key == 12 && ptr == (void *)112);

  /* we can delete all entries */
  fail_unless(!linkmap_del(lm, 12));
  fail_unless(!linkmap_del(lm, 13));
  fail_unless(!linkmap_del(lm, 15));
  fail_unless(!linkmap_del(lm, 17));
  fail_unless(1 == linkmap_get_head(lm, &key, &ptr));
  fail_unless(1 == linkmap_get_tail(lm, &key, &ptr));

  linkmap_free(&lm);
  fail_unless(lm == NULL);
}
END_TEST

START_TEST(test_size) {
  linkmap_t *lm;
  uint64_t key;
  void *ptr;

  /* empty linkmap is empty */
  lm = linkmap_new(9);
  fail_unless(0 == linkmap_size(lm));

  /* set increases the size */
  linkmap_set(lm, 12, (void *)112);
  fail_unless(1 == linkmap_size(lm));
  linkmap_set(lm, 13, (void *)113);
  fail_unless(2 == linkmap_size(lm));
  linkmap_set(lm, 14, (void *)114);
  linkmap_set(lm, 15, (void *)115);
  linkmap_set(lm, 16, (void *)116);
  linkmap_set(lm, 17, (void *)117);
  linkmap_set(lm, 18, (void *)118);
  linkmap_set(lm, 19, (void *)119);
  linkmap_set(lm, 20, (void *)120);
  fail_unless(9 == linkmap_size(lm));

  /* retrieval doesn't affect size */
  linkmap_get(lm, 12, &ptr);
  fail_unless(9 == linkmap_size(lm));
  linkmap_get_head(lm, &key, &ptr);
  fail_unless(9 == linkmap_size(lm));
  linkmap_get_tail(lm, &key, &ptr);
  fail_unless(9 == linkmap_size(lm));

  /* popping does however affect the size */
  linkmap_pop_head(lm, &key, &ptr);
  fail_unless(8 == linkmap_size(lm));
  linkmap_pop_tail(lm, &key, &ptr);
  fail_unless(7 == linkmap_size(lm));
  linkmap_pop(lm, 13, &ptr);
  fail_unless(6 == linkmap_size(lm));

  /* deletion also affects the size */
  linkmap_del(lm, 14);
  fail_unless(5 == linkmap_size(lm));
  linkmap_del_head(lm);
  fail_unless(4 == linkmap_size(lm));
  linkmap_del_tail(lm);
  fail_unless(3 == linkmap_size(lm));

  /* and let's double check that the correct 3 entries remain */
  fail_unless(!linkmap_pop_head(lm, &key, &ptr));
  fail_unless(key == 18 && ptr == (void *)118);
  fail_unless(!linkmap_pop_tail(lm, &key, &ptr));
  fail_unless(key == 16 && ptr == (void *)116);
  fail_unless(!linkmap_pop(lm, 17, &ptr));
  fail_unless(ptr == (void *)117);

  /* since we popped the last three, the map should now be empty */
  fail_unless(0 == linkmap_size(lm));

  linkmap_free(&lm);
  fail_unless(lm == NULL);
}
END_TEST

START_TEST(test_set_overwrite) {
  linkmap_t *lm;
  uint64_t key;
  void *ptr;

  /* setting an already existing key overwrites that entry's ptr */
  lm = linkmap_new(5);
  linkmap_set(lm, 12, (void *)112);
  linkmap_set(lm, 13, (void *)113);
  linkmap_set(lm, 14, (void *)114);
  linkmap_set(lm, 15, (void *)115);
  linkmap_set(lm, 16, (void *)116);
  fail_unless(!linkmap_set(lm, 13, (void *)223));
  fail_unless(!linkmap_set(lm, 15, (void *)225));
  fail_unless(!linkmap_get(lm, 13, &ptr));
  fail_unless(ptr == 223);
  fail_unless(!linkmap_get(lm, 15, &ptr));
  fail_unless(ptr == 225);

  /* but the entry order is not affected */
  linkmap_pop_head(lm, &key, &ptr);
  fail_unless(key == 16 && ptr == (void *)116);
  linkmap_pop_head(lm, &key, &ptr);
  fail_unless(key == 15 && ptr == (void *)225);
  linkmap_pop_tail(lm, &key, &ptr);
  fail_unless(key == 12 && ptr == (void *)112);
  linkmap_pop_tail(lm, &key, &ptr);
  fail_unless(key == 13 && ptr == (void *)223);
  linkmap_pop_tail(lm, &key, &ptr);
  fail_unless(key == 14 && ptr == (void *)114);
  fail_unless(0 == linkmap_size(lm));

  linkmap_free(&lm);
  fail_unless(lm == NULL);
}
END_TEST

Suite *linkmap_suite() {
  TCase *tc;
  Suite *s;

  s = suite_create ("linkmap");

  tc = tcase_create ("linkmap");
  tcase_add_test (tc, test_new);
  tcase_add_test (tc, test_free);
  tcase_add_test (tc, test_set);
  tcase_add_test (tc, test_get_head_tail);
  tcase_add_test (tc, test_get);
  tcase_add_test (tc, test_pop_head_tail);
  tcase_add_test (tc, test_pop);
  tcase_add_test (tc, test_del_head_tail);
  tcase_add_test (tc, test_del);
  tcase_add_test (tc, test_size);
  tcase_add_test (tc, test_set_overwrite);
  suite_add_tcase (s, tc);

  return s;
}

int main(void) {
  int number_failed;
  Suite *s = linkmap_suite();
  SRunner *sr = srunner_create(s);
  srunner_run_all (sr, CK_NORMAL);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return (number_failed == 0) ? 0 : 1;
}
