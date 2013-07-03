#ifndef NEO_INSERTER_NEOSTORE_H
#define NEO_INSERTER_NEOSTORE_H

// neo uses a weird value for nil in 1.9
// need to update for 2.0
static const uint64_t NIL = 0xFFFFFFFF;
static const size_t LIMB_SIZE = 1024*1024;
static const size_t NODE_SIZE = 9;
static const size_t REL_SIZE = 33;
static const size_t RELTYPE_SIZE = 5;

static const uint64_t MAX_NODE_ID = ((uint64_t)0xFFFFFFFF << 3) | 0x7;
static const uint64_t MAX_REL_ID = ((uint64_t)0xFFFFFFFF << 3) | 0x7;
static const uint64_t MAX_RELTYPE_ID = (uint64_t)0xFFFF;
static const uint64_t MAX_PROP_ID = ((uint64_t)0xFFFFFFFF << 4) | 0xF;

// node_last_rel functions
unsigned char *
get_node_last_rel_rec(unsigned char **ptr, uint64_t id);

uint64_t
get_node_last_rel(unsigned char *ptr);

void
set_node_last_rel(unsigned char *ptr, uint64_t id);

void
printlog(const char *fmt, ...);

#endif
