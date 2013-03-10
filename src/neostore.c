#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "neostore.h"

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
  ptr[0] = ptr[0] | ((id >> 31) & 0xE);
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
  ptr[0] = ptr[0] | ((id >> 28) & 0xF0);
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

unsigned char *
get_rel_rec(unsigned char **rels, uint64_t id) {
  return rels[id / LIMB_SIZE] + ((id % LIMB_SIZE) * REL_SIZE);
}

unsigned char
get_rel_inuse(unsigned char *ptr) {
  return ptr[0] & 0x1;
}

void
set_rel_inuse(unsigned char *ptr, unsigned char inuse) {
  ptr[0] = ptr[0] | 0x1;
}

uint64_t
get_rel_first_node(unsigned char *ptr) {
  uint64_t id = ((uint64_t)(ptr[0] & 0xE) << 31) | (ptr[1] << 24) | (ptr[2] << 16) | (ptr[3] << 8) | (ptr[4]);
  return id;
}

void
set_rel_first_node(unsigned char *ptr, uint64_t id) {
  ptr[0] = ptr[0] | ((id >> 31) & 0xE);
  ptr[1] = (id >> 24) & 0xFF;
  ptr[2] = (id >> 16) & 0xFF;
  ptr[3] = (id >> 8)  & 0xFF;
  ptr[4] = (id)       & 0xFF;
}

uint64_t 
get_rel_second_node(unsigned char *ptr) {
  uint64_t id = ((uint64_t)(ptr[9] & 0x70) << 28) | (ptr[5] << 24) | (ptr[6] << 16) | (ptr[7] << 8) | ptr[8];
  return id;
}

void
set_rel_second_node(unsigned char *ptr, uint64_t id) {
  ptr[9] = ptr[9] | ((id >> 28) & 0x7);
  ptr[5] = (id >> 24) & 0xFF;
  ptr[6] = (id >> 16) & 0xFF;
  ptr[7] = (id >> 8)  & 0xFF;
  ptr[8] = (id)       & 0xFF;
}

uint64_t
get_rel_type(unsigned char *ptr) {
  uint64_t id = (ptr[11] << 8) | ptr[12];
  return id;
}

void
set_rel_type(unsigned char *ptr, uint64_t id) {
  ptr[11] = (id >> 8) & 0xFF;
  ptr[12] = id & 0xFF;
}

uint64_t
get_rel_first_prev_rel(unsigned char *ptr) {
  uint64_t id = ((uint64_t)(ptr[9] & 0x0E) << 31) | (ptr[13] << 24) | (ptr[14] << 16) | (ptr[15] << 8) | ptr[16];
  return id;
}

void
set_rel_first_prev_rel(unsigned char *ptr, uint64_t id) {
  ptr[9] = ptr[9] | ((id >> 31) & 0xE);
  ptr[13] = (id >> 24) & 0xFF;
  ptr[14] = (id >> 16) & 0xFF;
  ptr[15] = (id >> 8)  & 0xFF;
  ptr[16] = (id)       & 0xFF;
}

uint64_t
get_rel_first_next_rel(unsigned char *ptr) {
  uint64_t id = ((uint64_t)(((ptr[9] & 0x1) << 2) | ((ptr[10] & 0xC0) >> 6)) << 32) | (ptr[17] << 24) | (ptr[18] << 16) | (ptr[19] << 8) | ptr[20];
  return id;
}

void
set_rel_first_next_rel(unsigned char *ptr, uint64_t id) {
  ptr[9] = ptr[9] | (id >> 34) & 0x1;
  ptr[10] = ptr[10] | (id >> 32) & 0xC0;
  ptr[17] = (id >> 24) & 0xFF;
  ptr[18] = (id >> 16) & 0xFF;
  ptr[19] = (id >> 8)  & 0xFF;
  ptr[20] = (id)       & 0xFF;
}

uint64_t
get_rel_second_prev_rel(unsigned char *ptr) {
  uint64_t id = ((uint64_t)(ptr[10] & 0x38) << 29) | (ptr[21] << 24) | (ptr[22] << 16) | (ptr[23] << 8) | ptr[24];
  return id;
}

void
set_rel_second_prev_rel(unsigned char *ptr, uint64_t id) {
  ptr[10] = ptr[10] | (id >> 29) & 0x38; 
  ptr[21] = (id >> 24) & 0xFF;
  ptr[22] = (id >> 16) & 0xFF;
  ptr[23] = (id >> 8)  & 0xFF;
  ptr[24] = (id)       & 0xFF;
}

uint64_t
get_rel_second_next_rel(unsigned char *ptr) {
  uint64_t id = ((uint64_t)(ptr[10] & 0x7) << 32) | (ptr[25] << 24) | (ptr[26] << 16) | (ptr[27] << 8) | ptr[28];
  return id;
}

void
set_rel_second_next_rel(unsigned char *ptr, uint64_t id) {
  ptr[10] = ptr[10] | ((id >> 32) & 0x7);
  ptr[25] = (id >> 24) & 0xFF;
  ptr[26] = (id >> 16) & 0xFF;
  ptr[27] = (id >> 8)  & 0xFF;
  ptr[28] = (id)       & 0xFF;
}

uint64_t
get_rel_first_prop(unsigned char *ptr) {
  uint64_t id = ((uint64_t)(ptr[0] & 0xF0) << 28) | (ptr[29] << 24) | (ptr[30] << 16) | (ptr[31] << 8) | ptr[32];
  return id;
}

void
set_rel_first_prop(unsigned char *ptr, uint64_t id) {
  ptr[0] = ptr[0] | ((id >> 28) & 0xF0);
  ptr[29] = (id >> 24) & 0xFF;
  ptr[30] = (id >> 16) & 0xFF;
  ptr[31] = (id >> 8)  & 0xFF;
  ptr[32] = (id)       & 0xFF;
}

//unsigned char *
//get_reltype_rec(uint64_t id) {
//  return reltypes[id / LIMB_SIZE] + ((id % LIMB_SIZE) * RELTYPE_SIZE);
//}

