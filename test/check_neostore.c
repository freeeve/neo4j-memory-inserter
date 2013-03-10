#include <check.h>
#include <stdbool.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include "config.h"
#include "../src/neostore.h"

START_TEST(nodestore_ops)
{
  unsigned char *node = malloc(NODE_SIZE);
  set_node_first_rel(node, NIL);
  fail_unless(get_node_first_rel(node) == NIL, "set/get_node_first_rel acting strange");
  set_node_first_prop(node, NIL);
  fail_unless(get_node_first_prop(node) == NIL, "set/get_node_first_prop acting strange");
  set_node_first_prop(node, 0);
  fail_unless(get_node_first_prop(node) == 0, "set/get_node_first_prop acting strange");
  fail_unless(get_node_first_rel(node) == NIL, "set_node_first_prop seems to corrupt first_rel");
  set_node_first_rel(node, 1);
  fail_unless(get_node_first_rel(node) == 1, "set/get_node_first_rel acting strange");
  fail_unless(get_node_first_prop(node) == 0, "set_node_first_rel seems to corrupt first_prop");
  set_node_first_rel(node, NIL * 4 - 1);
  fail_unless(get_node_first_rel(node) == NIL * 4 - 1, "set/get_node_first_rel breaks at high rel ids");
  fail_unless(get_node_first_prop(node) == 0, "set_node_first_rel seems to corrupt first_prop with high rel ids");
  set_node_first_prop(node, NIL * 8 - 1);
  fail_unless(get_node_first_prop(node) == NIL * 8 - 1, "set/get_node_first_prop acting strange at high prop ids");
  fail_unless(get_node_first_rel(node) == NIL * 4 - 1, "set_node_first_prop seems to corrupt first_rel with high prop ids");
  set_node_inuse(node, 1);
  fail_unless(get_node_inuse(node) == 1, "set/get_node_inuse acting strange");
  fail_unless(get_node_first_prop(node) == NIL * 8 - 1, "set_node_inuse seems to break first_prop");
  fail_unless(get_node_first_rel(node) == NIL * 4 - 1, "set_node_inuse seems to break first_rel");
  free(node);
}
END_TEST

Suite *
import_c_suite(void)
{
  Suite *s = suite_create("neostore");
  TCase *tc = tcase_create("core");
  tcase_add_test(tc, nodestore_ops);
  suite_add_tcase(s, tc);
  return s;
}

int
main()
{
  int nf;
  Suite *s = import_c_suite();
  SRunner *sr = srunner_create(s);
  srunner_set_log(sr, "check_neostore.log");
  srunner_run_all(sr, CK_NORMAL);
  nf = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (nf == 0) ? 0 : 1;
}

