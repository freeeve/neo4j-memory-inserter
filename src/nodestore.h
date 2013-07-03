#ifndef NEO_INSERTER_NODESTORE_H
#define NEO_INSERTER_NODESTORE_H

unsigned char *
get_node_rec(unsigned char **nodes, uint64_t id);

uint64_t
get_node_first_rel(unsigned char *ptr);

uint64_t
get_node_first_prop(unsigned char *ptr);

unsigned char
get_node_inuse(unsigned char *ptr);

void
set_node_first_rel(unsigned char *ptr, uint64_t id);

void
set_node_first_prop(unsigned char *ptr, uint64_t id);

void
set_node_inuse(unsigned char *ptr, unsigned char inuse);

#endif
