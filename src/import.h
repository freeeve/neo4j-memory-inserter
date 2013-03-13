static const uint32_t BUFFERS = 2;
static const uint32_t BUFFER_SIZE = 1024*1024*50;

static const uint16_t DONE_READING_NODES = 1;
static const uint16_t DONE_BUILDING_NODES = 2;
static const uint16_t DONE_WRITING_NODES = 3;
static const uint16_t DONE_READING_RELS = 4;
static const uint16_t DONE_BUILDING_RELS = 5;
static const uint16_t DONE_WRITING_RELS = 6;

void
create_node_from_tsv(unsigned char *props, unsigned char *line);

void
create_rel_from_tsv(unsigned char *props, unsigned char *line);

void
create_prop(uint64_t id, unsigned char *prop, unsigned char *value);

// thread runners
void *
node_reader(void *arg);

void *
node_builder(void *arg);

void *
node_writer(void *arg);

void *
rel_reader(void *arg);

void *
rel_builder(void *arg);

void *
rel_writer(void *arg);

