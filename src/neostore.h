static const uint64_t NIL = 0xFFFFFFFF;
static const size_t LIMB_SIZE = 1024*1024;
static const size_t NODE_SIZE = 9;
static const size_t REL_SIZE = 33;
static const size_t RELTYPE_SIZE = 5;

static const uint64_t MAX_NODE_ID = ((uint64_t)0xFFFFFFFF << 3) | 0x7;
static const uint64_t MAX_REL_ID = ((uint64_t)0xFFFFFFFF << 3) | 0x7;
static const uint64_t MAX_RELTYPE_ID = (uint64_t)0xFFFF;
static const uint64_t MAX_PROP_ID = ((uint64_t)0xFFFFFFFF << 4) | 0xF;

// node functions
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


// rel functions
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

// node_last_rel functions
unsigned char *
get_node_last_rel_rec(unsigned char **ptr, uint64_t id);

uint64_t
get_node_last_rel(unsigned char *ptr);

void
set_node_last_rel(unsigned char *ptr, uint64_t id);

// reltype functions
unsigned char *
get_reltype_rec(unsigned char **ptr, uint64_t id);

uint64_t
get_reltype_idx(unsigned char *ptr);

void
set_reltype_idx(unsigned char *ptr, uint64_t id);

void
printlog(const char *fmt, ...);
