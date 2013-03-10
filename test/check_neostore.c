#include <check.h>
#include <stdbool.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include "config.h"
#include "../src/neostore.h"

START_TEST(test_set_node_inuse)
{
  unsigned char *node = malloc(NODE_SIZE);
  memset(node, 0, NODE_SIZE);
  set_node_inuse(node, 1);
  fail_unless(node[0] == 0x1, "set_node_inuse failing to properly set the bit in its byte");
  unsigned char checksum = 0;
  for(int i = 1; i < NODE_SIZE; i++) {
    checksum |= node[i];
  }
  fail_unless(0 == checksum, "set_node_inuse corrupts other bits");
  free(node);
}
END_TEST

START_TEST(test_get_node_inuse)
{
  unsigned char *node = malloc(NODE_SIZE);
  memset(node, 0, NODE_SIZE);
  fail_unless(get_node_inuse(node) == 0, "get_node_inuse should be false when all bits are 0");
  set_node_inuse(node, 1);
  fail_unless(get_node_inuse(node) == 1, "get_node_inuse should be true after set");
  free(node);
}
END_TEST

START_TEST(test_set_node_first_rel)
{
  unsigned char *node = malloc(NODE_SIZE);
  memset(node, 0, NODE_SIZE);
  // max rel id
  set_node_first_rel(node, ((uint64_t)0xFFFFFFFF + 1) * 8 - 1);
  fail_unless(node[0] == 0x0E, "node[0] wrong - max id");
  fail_unless(node[1] == 0xFF, "node[1] wrong - max id");
  fail_unless(node[2] == 0xFF, "node[2] wrong - max id");
  fail_unless(node[3] == 0xFF, "node[3] wrong - max id");
  fail_unless(node[4] == 0xFF, "node[4] wrong - max id");
  fail_unless(node[5] == 0x0, "node[5] corrupted - max id");
  fail_unless(node[6] == 0x0, "node[6] corrupted - max id");
  fail_unless(node[7] == 0x0, "node[7] corrupted - max id");
  fail_unless(node[8] == 0x0, "node[8] corrupted - max id");
  memset(node, 0xFF, NODE_SIZE); // reversed bits
  // rel id 0
  set_node_first_rel(node, 0);
  fail_unless(node[0] == 0xF1, "node[0] wrong - id 0");
  fail_unless(node[1] == 0x00, "node[1] wrong - id 0");
  fail_unless(node[2] == 0x00, "node[2] wrong - id 0");
  fail_unless(node[3] == 0x00, "node[3] wrong - id 0");
  fail_unless(node[4] == 0x00, "node[4] wrong - id 0");
  fail_unless(node[5] == 0xFF, "node[5] corrupted - id 0");
  fail_unless(node[6] == 0xFF, "node[6] corrupted - id 0");
  fail_unless(node[7] == 0xFF, "node[7] corrupted - id 0");
  fail_unless(node[8] == 0xFF, "node[8] corrupted - id 0");
  free(node);
}
END_TEST

START_TEST(test_get_node_first_rel)
{
  unsigned char *node = malloc(NODE_SIZE);
  memset(node, 0, NODE_SIZE);
  set_node_first_rel(node, NIL);
  fail_unless(get_node_first_rel(node) == NIL, "get_node_first_rel returns NIL");
  set_node_first_rel(node, 277777);
  fail_unless(get_node_first_rel(node) == 277777, "get_node_first_rel returns 277777");
  set_node_first_rel(node, 0);
  fail_unless(get_node_first_rel(node) == 0, "get_node_first_rel returns 0");
  free(node);
}
END_TEST

START_TEST(test_set_node_first_prop)
{
  unsigned char *node = malloc(NODE_SIZE);
  memset(node, 0, NODE_SIZE);
  // max prop id
  set_node_first_prop(node, ((uint64_t)0xFFFFFFFF + 1) * 16 - 1);
  fail_unless(node[0] == 0xF0, "node[0] wrong - max id");
  fail_unless(node[1] == 0x00, "node[1] corrupted - max id");
  fail_unless(node[2] == 0x00, "node[2] corrupted - max id");
  fail_unless(node[3] == 0x00, "node[3] corrupted - max id");
  fail_unless(node[4] == 0x00, "node[4] corrupted - max id");
  fail_unless(node[5] == 0xFF, "node[5] wrong - max id");
  fail_unless(node[6] == 0xFF, "node[6] wrong - max id");
  fail_unless(node[7] == 0xFF, "node[7] wrong - max id");
  fail_unless(node[8] == 0xFF, "node[8] wrong - max id");
  memset(node, 0xFF, NODE_SIZE); // reversed bits
  // prop id 0
  set_node_first_prop(node, 0);
  fail_unless(node[0] == 0x0F, "node[0] wrong - id 0");
  fail_unless(node[1] == 0xFF, "node[1] corrupted - id 0");
  fail_unless(node[2] == 0xFF, "node[2] corrupted - id 0");
  fail_unless(node[3] == 0xFF, "node[3] corrupted - id 0");
  fail_unless(node[4] == 0xFF, "node[4] corrupted - id 0");
  fail_unless(node[5] == 0x00, "node[5] wrong - id 0");
  fail_unless(node[6] == 0x00, "node[6] wrong - id 0");
  fail_unless(node[7] == 0x00, "node[7] wrong - id 0");
  fail_unless(node[8] == 0x00, "node[8] wrong - id 0");
  free(node);
}
END_TEST

START_TEST(test_get_node_first_prop)
{
  unsigned char *node = malloc(NODE_SIZE);
  memset(node, 0, NODE_SIZE);
  set_node_first_prop(node, NIL);
  fail_unless(get_node_first_prop(node) == NIL, "get_node_first_prop returns NIL");
  set_node_first_prop(node, 277777);
  fail_unless(get_node_first_prop(node) == 277777, "get_node_first_prop returns 277777");
  set_node_first_prop(node, 0);
  fail_unless(get_node_first_prop(node) == 0, "get_node_first_prop returns 0");
  free(node);
}
END_TEST

Suite *
nodestore_suite(void)
{
  Suite *s = suite_create("nodestore");
  TCase *tc = tcase_create("core");
  tcase_add_test(tc, test_set_node_inuse);
  tcase_add_test(tc, test_get_node_inuse);
  tcase_add_test(tc, test_set_node_first_rel);
  tcase_add_test(tc, test_get_node_first_rel);
  tcase_add_test(tc, test_set_node_first_prop);
  tcase_add_test(tc, test_get_node_first_prop);
  suite_add_tcase(s, tc);
  return s;
}

START_TEST(test_set_rel_inuse)
{
  unsigned char *rel = malloc(REL_SIZE);
  memset(rel, 0, REL_SIZE);
  set_rel_inuse(rel, 1);
  fail_unless(rel[0] == 0x1, "set_rel_inuse failing to properly set the bit in its byte");
  unsigned char checksum = 0;
  for(int i = 1; i < REL_SIZE; i++) {
    checksum |= rel[i];
  }
  fail_unless(0 == checksum, "set_rel_inuse corrupts other bits");
  free(rel);
}
END_TEST

START_TEST(test_get_rel_inuse)
{
  unsigned char *rel = malloc(REL_SIZE);
  memset(rel, 0, REL_SIZE);
  fail_unless(get_rel_inuse(rel) == 0, "get_rel_inuse should be false when all bits are 0");
  set_rel_inuse(rel, 1);
  fail_unless(get_rel_inuse(rel) == 1, "get_rel_inuse should be true after set");
  free(rel);
}
END_TEST

START_TEST(test_set_rel_first_node)
{
  unsigned char *rel = malloc(REL_SIZE);
  memset(rel, 0, REL_SIZE);
  // max node id
  set_rel_first_node(rel, ((uint64_t)0xFFFFFFFF + 1) * 8 - 1);
  fail_unless(rel[0] == 0x0E, "[0] wrong - max id");
  fail_unless(rel[1] == 0xFF, "[1] wrong - max id");
  fail_unless(rel[2] == 0xFF, "[2] wrong - max id");
  fail_unless(rel[3] == 0xFF, "[3] wrong - max id");
  fail_unless(rel[4] == 0xFF, "[4] wrong - max id");
  fail_unless(rel[5] == 0x00, "[5] corrupted - max id");
  fail_unless(rel[6] == 0x00, "[6] corrupted - max id");
  fail_unless(rel[7] == 0x00, "[7] corrupted - max id");
  fail_unless(rel[8] == 0x00, "[8] corrupted - max id");
  fail_unless(rel[9] == 0x00, "[9] corrupted - max id");
  fail_unless(rel[10] == 0x00, "[10] corrupted - max id");
  fail_unless(rel[11] == 0x00, "[11] corrupted - max id");
  fail_unless(rel[12] == 0x00, "[12] corrupted - max id");
  fail_unless(rel[13] == 0x00, "[13] corrupted - max id");
  fail_unless(rel[14] == 0x00, "[14] corrupted - max id");
  fail_unless(rel[15] == 0x00, "[15] corrupted - max id");
  fail_unless(rel[16] == 0x00, "[16] corrupted - max id");
  fail_unless(rel[17] == 0x00, "[17] corrupted - max id");
  fail_unless(rel[18] == 0x00, "[18] corrupted - max id");
  fail_unless(rel[19] == 0x00, "[19] corrupted - max id");
  fail_unless(rel[20] == 0x00, "[20] corrupted - max id");
  fail_unless(rel[21] == 0x00, "[21] corrupted - max id");
  fail_unless(rel[22] == 0x00, "[22] corrupted - max id");
  fail_unless(rel[23] == 0x00, "[23] corrupted - max id");
  fail_unless(rel[24] == 0x00, "[24] corrupted - max id");
  fail_unless(rel[25] == 0x00, "[25] corrupted - max id");
  fail_unless(rel[26] == 0x00, "[26] corrupted - max id");
  fail_unless(rel[27] == 0x00, "[27] corrupted - max id");
  fail_unless(rel[28] == 0x00, "[28] corrupted - max id");
  fail_unless(rel[29] == 0x00, "[29] corrupted - max id");
  fail_unless(rel[30] == 0x00, "[30] corrupted - max id");
  fail_unless(rel[31] == 0x00, "[31] corrupted - max id");
  fail_unless(rel[32] == 0x00, "[32] corrupted - max id");
  memset(rel, 0xFF, REL_SIZE); // reversed bits
  // node id 0
  set_rel_first_node(rel, 0);
  fail_unless(rel[0] == 0xF1, "[0] wrong - id 0");
  fail_unless(rel[1] == 0x00, "[1] corrupted - id 0");
  fail_unless(rel[2] == 0x00, "[2] corrupted - id 0");
  fail_unless(rel[3] == 0x00, "[3] corrupted - id 0");
  fail_unless(rel[4] == 0x00, "[4] corrupted - id 0");
  fail_unless(rel[5] == 0xFF, "[5] wrong - id 0");
  fail_unless(rel[6] == 0xFF, "[6] wrong - id 0");
  fail_unless(rel[7] == 0xFF, "[7] wrong - id 0");
  fail_unless(rel[8] == 0xFF, "[8] wrong - id 0");
  fail_unless(rel[9] == 0xFF, "[9] wrong - id 0");
  fail_unless(rel[10] == 0xFF, "[10] wrong - id 0");
  fail_unless(rel[11] == 0xFF, "[11] wrong - id 0");
  fail_unless(rel[12] == 0xFF, "[12] wrong - id 0");
  fail_unless(rel[13] == 0xFF, "[13] wrong - id 0");
  fail_unless(rel[14] == 0xFF, "[14] wrong - id 0");
  fail_unless(rel[15] == 0xFF, "[15] wrong - id 0");
  fail_unless(rel[16] == 0xFF, "[16] wrong - id 0");
  fail_unless(rel[17] == 0xFF, "[17] wrong - id 0");
  fail_unless(rel[18] == 0xFF, "[18] wrong - id 0");
  fail_unless(rel[19] == 0xFF, "[19] wrong - id 0");
  fail_unless(rel[20] == 0xFF, "[20] wrong - id 0");
  fail_unless(rel[21] == 0xFF, "[21] wrong - id 0");
  fail_unless(rel[22] == 0xFF, "[22] wrong - id 0");
  fail_unless(rel[23] == 0xFF, "[23] wrong - id 0");
  fail_unless(rel[24] == 0xFF, "[24] wrong - id 0");
  fail_unless(rel[25] == 0xFF, "[25] wrong - id 0");
  fail_unless(rel[26] == 0xFF, "[26] wrong - id 0");
  fail_unless(rel[27] == 0xFF, "[27] wrong - id 0");
  fail_unless(rel[28] == 0xFF, "[28] wrong - id 0");
  fail_unless(rel[29] == 0xFF, "[29] wrong - id 0");
  fail_unless(rel[30] == 0xFF, "[30] wrong - id 0");
  fail_unless(rel[31] == 0xFF, "[31] wrong - id 0");
  fail_unless(rel[32] == 0xFF, "[32] wrong - id 0");
  free(rel);
}
END_TEST

START_TEST(test_get_rel_first_node)
{
  unsigned char *rel = malloc(REL_SIZE);
  memset(rel, 0, REL_SIZE);

  free(rel);
}
END_TEST

START_TEST(test_set_rel_second_node)
{
  unsigned char *rel = malloc(REL_SIZE);
  memset(rel, 0, REL_SIZE);

  free(rel);
}
END_TEST

START_TEST(test_get_rel_second_node)
{
  unsigned char *rel = malloc(REL_SIZE);
  memset(rel, 0, REL_SIZE);

  free(rel);
}
END_TEST

START_TEST(test_set_rel_type)
{
  unsigned char *rel = malloc(REL_SIZE);
  memset(rel, 0, REL_SIZE);

  free(rel);
}
END_TEST

START_TEST(test_get_rel_type)
{
  unsigned char *rel = malloc(REL_SIZE);
  memset(rel, 0, REL_SIZE);

  free(rel);
}
END_TEST

START_TEST(test_set_rel_first_prev_rel)
{
  unsigned char *rel = malloc(REL_SIZE);
  memset(rel, 0, REL_SIZE);

  free(rel);
}
END_TEST

START_TEST(test_get_rel_first_prev_rel)
{
  unsigned char *rel = malloc(REL_SIZE);
  memset(rel, 0, REL_SIZE);

  free(rel);
}
END_TEST

START_TEST(test_set_rel_first_next_rel)
{
  unsigned char *rel = malloc(REL_SIZE);
  memset(rel, 0, REL_SIZE);

  free(rel);
}
END_TEST

START_TEST(test_get_rel_first_next_rel)
{
  unsigned char *rel = malloc(REL_SIZE);
  memset(rel, 0, REL_SIZE);

  free(rel);
}
END_TEST

START_TEST(test_set_rel_second_prev_rel)
{
  unsigned char *rel = malloc(REL_SIZE);
  memset(rel, 0, REL_SIZE);

  free(rel);
}
END_TEST

START_TEST(test_get_rel_second_prev_rel)
{
  unsigned char *rel = malloc(REL_SIZE);
  memset(rel, 0, REL_SIZE);

  free(rel);
}
END_TEST

START_TEST(test_set_rel_second_next_rel)
{
  unsigned char *rel = malloc(REL_SIZE);
  memset(rel, 0, REL_SIZE);

  free(rel);
}
END_TEST

START_TEST(test_get_rel_second_next_rel)
{
  unsigned char *rel = malloc(REL_SIZE);
  memset(rel, 0, REL_SIZE);

  free(rel);
}
END_TEST

START_TEST(test_set_rel_first_prop)
{
  unsigned char *rel = malloc(REL_SIZE);
  memset(rel, 0, REL_SIZE);

  free(rel);
}
END_TEST

START_TEST(test_get_rel_first_prop)
{
  unsigned char *rel = malloc(REL_SIZE);
  memset(rel, 0, REL_SIZE);

  free(rel);
}
END_TEST

Suite *
relstore_suite(void)
{
  Suite *s = suite_create("relstore");
  TCase *tc = tcase_create("core");
  tcase_add_test(tc, test_set_rel_inuse);
  tcase_add_test(tc, test_get_rel_inuse);
  tcase_add_test(tc, test_set_rel_first_node);
  tcase_add_test(tc, test_get_rel_first_node);
  tcase_add_test(tc, test_set_rel_second_node);
  tcase_add_test(tc, test_get_rel_second_node);
  tcase_add_test(tc, test_set_rel_type);
  tcase_add_test(tc, test_get_rel_type);
  tcase_add_test(tc, test_set_rel_first_prev_rel);
  tcase_add_test(tc, test_get_rel_first_prev_rel);
  tcase_add_test(tc, test_set_rel_first_next_rel);
  tcase_add_test(tc, test_get_rel_first_next_rel);
  tcase_add_test(tc, test_set_rel_second_prev_rel);
  tcase_add_test(tc, test_get_rel_second_prev_rel);
  tcase_add_test(tc, test_set_rel_second_next_rel);
  tcase_add_test(tc, test_get_rel_second_next_rel);
  tcase_add_test(tc, test_set_rel_first_prop);
  tcase_add_test(tc, test_get_rel_first_prop);
  suite_add_tcase(s, tc);
  return s;
}

int
main()
{
  int nf;
  Suite *s = nodestore_suite();
  SRunner *sr = srunner_create(s);
  srunner_add_suite(sr, relstore_suite());
  srunner_set_log(sr, "check_neostore.log");
  srunner_run_all(sr, CK_NORMAL);
  nf = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (nf == 0) ? 0 : 1;
}

