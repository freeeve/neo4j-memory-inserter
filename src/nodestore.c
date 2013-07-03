#include <stdlib.h>
#include <stdint.h>
#include "neostore.h"
#include "nodestore.h"

unsigned char *
get_node_rec(unsigned char **nodes, uint64_t id) {
  return nodes[id / LIMB_SIZE] + ((id % LIMB_SIZE) * NODE_SIZE);
}

uint64_t
get_node_first_rel(unsigned char *ptr) {
  uint64_t id = ((uint64_t)(ptr[0] & 0xE) << 31) | \
                ((uint64_t)ptr[1] << 24) | \
                ((uint64_t)ptr[2] << 16) | \
                ((uint64_t)ptr[3] << 8) | \
                (uint64_t)(ptr[4]);
  return id;
}

void
set_node_first_rel(unsigned char *ptr, uint64_t id) {
  ptr[0] = (ptr[0] & 0xF1) | ((id >> 31) & 0xE);
  ptr[1] = (id >> 24) & 0xFF;
  ptr[2] = (id >> 16) & 0xFF;
  ptr[3] = (id >> 8)  & 0xFF;
  ptr[4] = (id)       & 0xFF;
}

uint64_t
get_node_first_prop(unsigned char *ptr) {
  uint64_t id = ((uint64_t)(ptr[0] & 0xF0) << 28) | \
                ((uint64_t)ptr[5] << 24) | \
                ((uint64_t)ptr[6] << 16) | \
                ((uint64_t)ptr[7] << 8) | \
                ((uint64_t)ptr[8]);
  return id;
}

void
set_node_first_prop(unsigned char *ptr, uint64_t id) {
  ptr[0] = (ptr[0] & 0xF) | ((id >> 28) & 0xF0);
  ptr[5] = (id >> 24) & 0xFF;
  ptr[6] = (id >> 16) & 0xFF;
  ptr[7] = (id >> 8)  & 0xFF;
  ptr[8] = (id)       & 0xFF;
}

unsigned char
get_node_inuse(unsigned char *ptr) {
  return ptr[0] & 0x1;
}

void
set_node_inuse(unsigned char *ptr, unsigned char inuse) {
  ptr[0] = ptr[0] | (inuse & 0x1);
}
