#ifndef NEO_INSERTER_RELTYPESTORE_H
#define NEO_INSERTER_RELTYPESTORE_H

unsigned char *
get_reltype_rec(unsigned char **ptr, uint64_t id);

uint64_t
get_reltype_idx(unsigned char *ptr);

void
set_reltype_idx(unsigned char *ptr, uint64_t id);

#endif
