#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include "import.h"

// pthread shared data
static unsigned char **buffer;
static unsigned int *buffer_length;
static unsigned int *buffer_status;
static int cur_read_idx;
static int cur_build_idx;
static int cur_write_idx;
static uint16_t process_status;
static pthread_mutex_t buffer_mutex;

static unsigned char **nodes;
static unsigned char **rels;
static unsigned char **reltypes;

static FILE *in_nodes;
static FILE *in_rels;

static FILE *nodestore;
static FILE *relstore;
// end pthread shared data

int 
main(int argc, char **argv) {
  if(argc < 4) printf("usage: import <num nodes> <num rels> nodes.csv rels.csv\n");
  pthread_t node_reader_thread, node_builder_thread, node_writer_thread; 
  pthread_t rel_reader_thread, rel_builder_thread, rel_writer_thread; 
  int i=BUFFERS;
  buffer = (unsigned char **)malloc(BUFFERS * sizeof(char *));
  buffer_length = (unsigned int*)malloc(BUFFERS * sizeof(unsigned int));
  buffer_status = (unsigned int*)malloc(BUFFERS * sizeof(unsigned int));
  for(;i<BUFFERS;i++) {
    buffer[i] = malloc(BUFFER_SIZE);
    buffer_length[i] = 0;
    buffer_status[i] = 0;
  }
  cur_read_idx = 0;
  cur_build_idx = 0;
  process_status = 0;
  in_nodes = fopen(argv[1], "r");
  in_rels = fopen(argv[2], "r");
  pthread_mutex_init(&buffer_mutex, NULL);

  pthread_create(&node_reader_thread, NULL, &node_reader, NULL);
  pthread_create(&node_builder_thread, NULL, &node_builder, NULL);

  pthread_join(node_reader_thread, &exit_status);
  pthread_join(node_builder_thread, &exit_status);

  pthread_create(&rel_reader_thread, NULL, &rel_reader, NULL);
  pthread_create(&rel_builder_thread, NULL, &rel_builder, NULL);

  pthread_join(rel_reader_thread, &exit_status);
  pthread_join(rel_builder_thread, &exit_status);

  cur_write_idx = 0;
  pthread_create(&node_writer_thread, NULL, &node_writer, NULL);
  pthread_create(&rel_writer_thread, NULL, &rel_writer, NULL);

  pthread_join(node_writer_thread, &exit_status);
  pthread_join(rel_writer_thread, &exit_status);

  pthread_mutex_destroy(&buffer_mutex);
}

unsigned char *
get_node_rec(uint64_t id) {
  return nodes[id / LIMB_SIZE] + ((id % LIMB_SIZE) * NODE_SIZE);
}

uint64_t
get_node_first_rel(unsigned char *ptr) {
  uint64_t id = ((ptr[0] & 0xE) << 31) | (ptr[1] << 24) | (ptr[2] << 16) | (ptr[3] << 8) | (ptr[4]);
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
  uint64_t id = ((ptr[0] & 0xF0) << 28) | (ptr[5] << 24) | (ptr[6] << 16) | (ptr[7] << 8) | (ptr[8]);
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
  uint64_t id = ((ptr[0] & 0x70) << 31) | (ptr[1] << 24) | (ptr[2] << 16) | (ptr[3] << 8) | (ptr[4]);
  return id;
}

void
set_rel_first_node(unsigned char *ptr, uint64_t id) {
  ptr[0] = ptr[0] | (0x700000000 & id) >> 31;
  ptr[1] = (id >> 24) & 0xFF;
  ptr[2] = (id >> 16) & 0xFF;
  ptr[3] = (id >> 8) & 0xFF;
  ptr[4] = (id) & 0xFF;
}

uint64_t 
get_rel_second_node(unsigned char *ptr) {
  
}

unsigned char *
get_reltype_rec(uint_64 id) {
  unsigned int limb = id / LIMB_SIZE;
  return reltype[id / LIMB_SIZE] + ((id % LIMB_SIZE) * RELTYPE_SIZE);
}

void *
node_reader(void *arg) {
  printf("reader thread started...\n");
  // loop until entire file read
  while(!feof(in_nodes)) {
    printf("attempting to claim buffer...\n");
    int buffer_num = -1;
    pthread_mutex_lock(&buffer_mutex);
    buffer_num = cur_read_idx; 
    buffer_length = buffer_length[buffer_num];
    pthread_mutex_unlock(&buffer_mutex);
    printf("claimed buffer: %d...\n", buffer_num);
    // check to see if this buffer is in use before overwriting it
    // (if buffer is length isn't 0 it hasn't been consumed)
    while(buffer_length > 0) {
      printf("buffer length not 0, sleeping...");
      sleep(1);
      pthread_mutex_lock(&buffer_mutex);
      buffer_length = buffer_length[buffer_num];
      pthread_mutex_unlock(&buffer_mutex);
    }
    size_t to_read = BUFFER_SIZE;
    size_t read = 0;
    unsigned char *buff = buffer[buffer_num];
    while(read != to_read && !feof(in_nodes)) {
      read += fread(buff, to_read, 1, in_nodes);
    }
    printf("read %d bytes from node input into buffer: %d\n", read, buffer_num);
    pthread_mutex_lock(&buffer_mutex);
    buffer_length[buffer_num] = read;
    buffer_status[buffer_num] = DONE_READING_NODES;
    if(buffer_num == 9) cur_read_idx = 0;
    else cur_read_idx++;
    printf("updated buffer idx: \n", cur_read_idx);
    pthread_mutex_unlock(&buffer_mutex);
  }
  pthread_mutex_lock(&buffer_mutex);
  pthread_mutex_unlock(&buffer_mutex);
  fclose(in_nodes);
  status = DONE_READING_NODES;
  printf("done reading node file, ending node_reader\n");
  return NULL;
}

void *
node_builder(void *arg) {
  pthread_mutex_lock(&buffer_mutex);
  int buffer_num = cur_build_idx; 
  int buffer_status = buffer_status[buffer_num];
  int buffer_length = buffer_length[buffer_num];
  int status = process_status;
  pthread_mutex_unlock(&buffer_mutex);
  printf("claimed buffer: %d...\n", buffer_num);
  
  while(buffer_length > 0) {
    while(buffer_status != DONE_READING_NODES) {
      buffer_status = 
    }

    pthread_mutex_lock(&buffer_mutex); 
    buffer_num = cur_build_idx;
    buffer_length = buffer_length[buffer_num];
    pthread_mutex_unlock(&buffer_mutex); 
  }
}
