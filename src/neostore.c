#include <stdlib.h>
#include <stdint.h>
#include "neostore.h"

// keeping track of the last rel in the list is an optimization for the inserter only
unsigned char *
get_node_last_rel_rec(unsigned char **ptr, uint64_t id) {
  return ptr[id / LIMB_SIZE] + ((id % LIMB_SIZE) * 5);
}

uint64_t
get_node_last_rel(unsigned char *ptr) {
  uint64_t id = ((uint64_t)ptr[0] << 32) | \
                ((uint64_t)ptr[1] << 24) | \
                ((uint64_t)ptr[2] << 16) | \
                ((uint64_t)ptr[3] << 8) | \
                (uint64_t)ptr[4];
  return id;
}

void
set_node_last_rel(unsigned char *ptr, uint64_t id) {
  ptr[0] = (id >> 32) & 0xFF;
  ptr[1] = (id >> 24) & 0xFF;
  ptr[2] = (id >> 16) & 0xFF;
  ptr[3] = (id >> 8) & 0xFF;
  ptr[4] = (id) & 0xFF;
}

