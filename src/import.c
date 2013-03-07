#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "import.h"

// pthread shared data
static unsigned char **buffers;
static unsigned int *buffer_lengths;
static unsigned int *buffer_statuses;
static int cur_read_idx;
static int cur_build_idx;
static uint16_t process_status;
static pthread_mutex_t buffer_mutex;

static unsigned char **nodes;
static unsigned char **rels;
//static unsigned char **reltypes;

static FILE *in_nodes;
static FILE *in_rels;

static FILE *out_nodestore;
static FILE *out_relstore;

static uint64_t node_id;
static uint64_t rel_id;
static uint64_t prop_id;

uint64_t num_nodes;
uint64_t num_rels;
// end pthread shared data

int 
main(int argc, char **argv) {
  if(argc < 4) printf("usage: import <num nodes> <num rels> nodes.csv rels.csv <path to db>\n");
  pthread_t node_reader_thread, node_builder_thread, node_writer_thread; 
  pthread_t rel_reader_thread, rel_builder_thread, rel_writer_thread; 
  void *exit_status;
  uint32_t i = 0;
  num_nodes = atoll(argv[1]);
  num_rels = atoll(argv[2]);
  int32_t node_limbs = (num_nodes / LIMB_SIZE);
  uint32_t rel_limbs = (num_rels / LIMB_SIZE);
  if(num_nodes % LIMB_SIZE != 0) node_limbs++; 
  if(num_rels % LIMB_SIZE != 0) rel_limbs++; 
  node_id = 0;
  rel_id = 0;
  prop_id = 0;

  struct stat st = {0};
  if(stat(argv[5], &st) == -1) {
    mkdir(argv[5], 0700);
  }

  unsigned char *nodestore_filename = malloc(200);
  strncpy(nodestore_filename, argv[5], 200);
  printf("debug %s\n", nodestore_filename);
  strncat(nodestore_filename, "/neostore.nodestore.db", 200);
  out_nodestore = fopen(nodestore_filename, "wb");
  free(nodestore_filename);

  buffers = (unsigned char **)malloc(BUFFERS * sizeof(unsigned char *));
  assert(buffers != NULL);
  buffer_lengths = (unsigned int*)malloc(BUFFERS * sizeof(unsigned int));
  assert(buffer_lengths != NULL);
  buffer_statuses = (unsigned int*)malloc(BUFFERS * sizeof(unsigned int));
  assert(buffer_statuses != NULL);
  for(i = 0; i < BUFFERS;i++) {
    buffers[i] = malloc(BUFFER_SIZE);
    assert(buffers[i] != NULL);
    buffer_lengths[i] = 0;
    buffer_statuses[i] = 0;
  }
  nodes = (unsigned char **)malloc(node_limbs * sizeof(unsigned char *));
  assert(nodes != NULL);
  for(i = 0;i < node_limbs; i++) {
    nodes[i] = (unsigned char *)malloc(LIMB_SIZE * NODE_SIZE);
    assert(nodes[i] != NULL);
  }
  rels = (unsigned char **)malloc(rel_limbs * sizeof(unsigned char *));
  assert(rels != NULL);
  for(i = 0;i < rel_limbs; i++) {
    rels[i] = (unsigned char *)malloc(LIMB_SIZE * REL_SIZE);
    assert(rels[i] != NULL);
  }

  cur_read_idx = 0;
  cur_build_idx = 0;
  process_status = 0;
  in_nodes = fopen(argv[3], "r");
  in_rels = fopen(argv[4], "r");
  pthread_mutex_init(&buffer_mutex, NULL);

  pthread_create(&node_reader_thread, NULL, &node_reader, NULL);
  pthread_create(&node_builder_thread, NULL, &node_builder, NULL);

  pthread_join(node_reader_thread, &exit_status);
  pthread_join(node_builder_thread, &exit_status);

  pthread_create(&rel_reader_thread, NULL, &rel_reader, NULL);
  pthread_create(&rel_builder_thread, NULL, &rel_builder, NULL);

  pthread_join(rel_reader_thread, &exit_status);
  pthread_join(rel_builder_thread, &exit_status);

  pthread_create(&node_writer_thread, NULL, &node_writer, NULL);
  pthread_create(&rel_writer_thread, NULL, &rel_writer, NULL);
  //pthread_create(&reltype_writer_thread, NULL, &reltype_writer, NULL);

  pthread_join(node_writer_thread, &exit_status);
  pthread_join(rel_writer_thread, &exit_status);
  //pthread_join(reltype_writer_thread, &exit_status);

  pthread_mutex_destroy(&buffer_mutex);
}

unsigned char *
get_node_rec(uint64_t id) {
  return nodes[id / LIMB_SIZE] + ((id % LIMB_SIZE) * NODE_SIZE);
}

uint64_t
get_node_first_rel(unsigned char *ptr) {
  uint64_t id = ((uint64_t)(ptr[0] & 0xE) << 31) | (ptr[1] << 24) | (ptr[2] << 16) | (ptr[3] << 8) | (ptr[4]);
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
  uint64_t id = ((uint64_t)(ptr[0] & 0xF0) << 28) | (ptr[5] << 24) | (ptr[6] << 16) | (ptr[7] << 8) | (ptr[8]);
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
get_rel_rec(uint64_t id) {
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

void *
node_reader(void *arg) {
  printf("node_reader: thread started...\n");
  // loop until entire file read
  int done = 0;
  while(!done) {
    printf("node_reader: attempting to claim buffer...\n");
    pthread_mutex_lock(&buffer_mutex);
    int buffer_num = cur_read_idx; 
    int buffer_length = buffer_lengths[buffer_num];
    pthread_mutex_unlock(&buffer_mutex);
    printf("node_reader: claimed buffer: %d...\n", buffer_num);
    // check to see if this buffer is in use before overwriting it
    // (if buffer is length isn't 0 it hasn't been consumed)
    while(buffer_length > 0) {
      printf("node_reader: buffer length not 0--need to wait for it to be processed, sleeping...\n");
      sleep(1);
      pthread_mutex_lock(&buffer_mutex);
      buffer_length = buffer_lengths[buffer_num];
      pthread_mutex_unlock(&buffer_mutex);
    }
    uint32_t to_read = BUFFER_SIZE;
    uint32_t read = 0;
    unsigned char *buff = buffers[buffer_num];
    while(read != to_read) {
      uint32_t cur = fread(buff + read, 1, to_read - read, in_nodes);
      if(cur == 0) { 
        done = 1;
        printf("node_reader: breaking, hit end of file\n");
        break;
      }
      read += cur;
      printf("node_reader: read %d so far.. looping\n", read);
    }
    printf("node_reader: read %d bytes from node input into buffer: %d\n", read, buffer_num);
    pthread_mutex_lock(&buffer_mutex);
    buffer_lengths[buffer_num] = read;
    buffer_statuses[buffer_num] = DONE_READING_NODES;
    if(buffer_num == 9) cur_read_idx = 0;
    else cur_read_idx++;
    printf("node_reader: updated buffer idx: %d\n", cur_read_idx);
    pthread_mutex_unlock(&buffer_mutex);
  }
  pthread_mutex_lock(&buffer_mutex);
  pthread_mutex_unlock(&buffer_mutex);
  fclose(in_nodes);
  process_status = DONE_READING_NODES;
  printf("node_reader: thread ending...\n");
  return NULL;
}

void 
create_prop(uint64_t id, unsigned char *prop, unsigned char *value) {

}

void 
create_node_from_tsv(unsigned char *props, unsigned char *line) {
  //printf("creating node [%s] from tsv... [%s]\n", props, line);
  unsigned char *ptr = get_node_rec(node_id++);
  set_node_inuse(ptr, 0x1);
  set_node_first_rel(ptr, NIL);
  set_node_first_prop(ptr, NIL);
  unsigned char *buff = props;
  int first_prop = 1;
  uint64_t new_prop_id = NIL;
  uint64_t prev_prop_id = NIL;
  while(1) {
    //printf("create_node_from_tsv: property loop\n");
    unsigned char *propend = strchr(buff, '\t');
    unsigned char *valend = strchr(line, '\t');
    int32_t propsize = propend - buff;
    int32_t valsize = valend - line;
    unsigned char *prop, *val;
    if(propsize > 0) {
      prop = malloc(propsize + 1);
      assert(prop != NULL);
    } else {
      break;
    }
    if(valsize > 0) {
      val = malloc(valsize + 1); 
      assert(val != NULL);
    } else {
      break;
    }

    strncpy(prop, props, propsize);
    buff = propend + 1;
    strncpy(val, line, valsize);
    line = valend + 1;

    prev_prop_id = new_prop_id;
    new_prop_id = prop_id++;

    create_prop(new_prop_id, prop, val);

    if(first_prop) {
      first_prop = 0;
      set_node_first_prop(ptr, new_prop_id);
    } else {
      //unsigned char *prev = get_prop_rec(prev_prop_id);
      //set_prop_next_prop(prev, new_prop_id);
    }

    free(val);
    free(prop);
  }
  //printf("create_node_from_tsv: leaving...\n");
}

void *
node_builder(void *arg) {
  printf("node_builder: thread started...\n");
  pthread_mutex_lock(&buffer_mutex);
  int buffer_num = cur_build_idx++; 
  int buffer_status = buffer_statuses[buffer_num];
  int buffer_length = buffer_lengths[buffer_num];
  int status = process_status;
  pthread_mutex_unlock(&buffer_mutex);
  printf("node_builder: looking at buffer: %d...\n", buffer_num);
  int first_run = 1;
  while(1) {
    while(buffer_length == 0 || buffer_status != DONE_READING_NODES) {
      printf("node_builder: buffer status not ready to build nodes, sleeping...\n");
      sleep(1);
      pthread_mutex_lock(&buffer_mutex); 
      buffer_status = buffer_statuses[buffer_num];
      buffer_length = buffer_lengths[buffer_num];
      status = process_status;
      pthread_mutex_unlock(&buffer_mutex); 
      if(status == DONE_READING_NODES && buffer_status != DONE_READING_NODES) {
        pthread_mutex_lock(&buffer_mutex); 
        process_status = DONE_BUILDING_NODES;
        status = process_status;
        pthread_mutex_unlock(&buffer_mutex); 
        break;
      }
    }
    if(status == DONE_BUILDING_NODES) {
      break;
    }
    printf("node_builder: waking to build nodes from buffer: %d...\n", buffer_num);
    
    unsigned char *buffer = buffers[buffer_num];
    unsigned char *line = buffer, *props;
    //TODO need to handle carryover
    while(buffer < buffers[buffer_num] + BUFFER_SIZE) {
      unsigned char *endline = buffer;
      while(*endline != '\n' && endline < buffers[buffer_num] + BUFFER_SIZE) {
        endline++;
      }

      uint32_t diff = endline - buffer;
      if(diff > 0) line = malloc(diff + 1); 
      else if(buffer < buffers[buffer_num] + buffer_length) {
        create_node_from_tsv("", ""); 
        buffer++;
        continue;
      } else {
        break;
      }
      strncpy(line, buffer, diff);
      line[diff] = '\0';
      buffer = endline + 1;
      if(first_run) {
        first_run = 0;
        //printf("node_builder: first line: %s\n", line);
        props = malloc(diff + 1);
        assert(props != NULL);
        strncpy(props, line, diff);
        props[diff] = '\0';
        //free(line);
        continue;
      }
      create_node_from_tsv(props, line);
      free(line);
    }
    //free(props);

    pthread_mutex_lock(&buffer_mutex); 
    buffer_statuses[buffer_num] = DONE_BUILDING_NODES;
    buffer_lengths[buffer_num] = 0;
    if(cur_build_idx == BUFFERS) cur_build_idx = 0;
    buffer_num = cur_build_idx++;
    buffer_length = buffer_lengths[buffer_num];
    buffer_status = buffer_statuses[buffer_num];
    pthread_mutex_unlock(&buffer_mutex); 
  }
  printf("node_builder: thread ending...\n");
  return NULL;
}

void *
node_writer(void *arg) {
  printf("node_writer: thread starting...\n");
  uint32_t limbs = num_nodes / LIMB_SIZE + (num_nodes % LIMB_SIZE == 0 ? 0 : 1);
  uint32_t i = 0;
  for(;i < limbs; i++) {
    uint32_t to_write = LIMB_SIZE * NODE_SIZE;
    if(i == limbs - 1) {
      to_write = (num_nodes % LIMB_SIZE) * NODE_SIZE;
    }
    uint32_t written = 0;
    while(written < to_write) {
      written += fwrite(nodes[i], 1, to_write - written, out_nodestore); 
    }
  }
  unsigned char *label = "NodeStore v0.A.0";
  uint32_t to_write = 16;
  uint32_t written = 0;
  while(written < to_write) {
    written += fwrite(label, 1, to_write - written, out_nodestore);
  }
  fclose(out_nodestore);
  printf("node_writer: thread ending...\n");
  return NULL;
}

void *
rel_reader(void *arg) {
  printf("rel_reader: thread starting...\n");
  printf("rel_reader: thread ending...\n");
  return NULL;
}

void *
rel_builder(void *arg) {
  printf("rel_builder: thread starting...\n");
  printf("rel_builder: thread ending...\n");
  return NULL;
}

void *
rel_writer(void *arg) {
  printf("rel_writer: thread starting...\n");
  printf("rel_writer: thread ending...\n");
  return NULL;
}
