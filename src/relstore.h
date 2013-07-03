#ifndef NEO_INSERTER_RELSTORE_H
#define NEO_INSERTER_RELSTORE_H

unsigned char *
get_rel_rec(unsigned char **rels, uint64_t id);

unsigned char
get_rel_inuse(unsigned char *ptr);

uint64_t
get_rel_first_node(unsigned char *ptr);

uint64_t
get_rel_second_node(unsigned char *ptr);

uint64_t
get_rel_type(unsigned char *ptr);

uint64_t
get_rel_first_prev_rel(unsigned char *ptr);

uint64_t
get_rel_first_next_rel(unsigned char *ptr);

uint64_t
get_rel_second_prev_rel(unsigned char *ptr);

uint64_t
get_rel_second_next_rel(unsigned char *ptr);

uint64_t
get_rel_first_prop(unsigned char *ptr);

void
set_rel_inuse(unsigned char *ptr, unsigned char inuse);

void
set_rel_first_node(unsigned char *ptr, uint64_t id);

void
set_rel_second_node(unsigned char *ptr, uint64_t id);

void
set_rel_type(unsigned char *ptr, uint64_t id);

void
set_rel_first_prev_rel(unsigned char *ptr, uint64_t id);

void
set_rel_first_next_rel(unsigned char *ptr, uint64_t id);

void
set_rel_second_prev_rel(unsigned char *ptr, uint64_t id);

void
set_rel_second_next_rel(unsigned char *ptr, uint64_t id);

void
set_rel_first_prop(unsigned char *ptr, uint64_t id);

#endif
