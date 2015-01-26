#include "utilities.h"

FILE * open_file_read(const char * filename) {
  FILE * input_file = fopen(filename, "r");
  return input_file;
}
FILE * create_file_binary_write(const char * filename) {
  FILE * output_file = fopen(filename, "wb");
  return output_file;
}

// note that sets readable_bytes to 0
string_with_size *
 make_new_string_with_size(unsigned long long size_in_memory) {
  string_with_size * sws_to_return = malloc(sizeof(string_with_size));
  sws_to_return->string = malloc(size_in_memory * (sizeof(char)));
  sws_to_return->readable_bytes = 0;
  sws_to_return->size_in_memory = size_in_memory;
  return sws_to_return;
}
#ifndef MEMPOOL
#ifdef DEBUG
// DOES NOT COPY OVER NULL CHAR TERMINATING
string_with_size *
 make_new_string_with_size_given_string(char * null_term_str) {
  string_with_size * sws_to_return = malloc(sizeof(string_with_size));
  sws_to_return->string = malloc(sizeof(char) * (strlen(null_term_str)));
  memcpy(sws_to_return->string, null_term_str, strlen(null_term_str));
  sws_to_return->readable_bytes = strlen(null_term_str);
  return sws_to_return;
}
#endif
#else
string_with_size_pools sws_mempools = {NULL, 0, NULL};

void add_string_with_size_pool(unsigned long long size_in_mem,
                               size_t num_elems) {
  size_t prev_num_pools = sws_mempools.num_sws_mempools;
  GQueue ** new_mempools = malloc((prev_num_pools + 1) * sizeof(GQueue));
  unsigned long long * new_mempool_sizes =
   malloc((prev_num_pools + 1) * sizeof(unsigned long long));
  // copy over locations of previous pools
  for (size_t prev_mempool_index = 0; prev_mempool_index < prev_num_pools;
       ++prev_mempool_index) {
    new_mempools[ prev_mempool_index ] =
     sws_mempools.mempools[ prev_mempool_index ];
  }
  // now add new
  new_mempools[ prev_num_pools ] = g_queue_new();
  // copy over sizes of previous pools
  for (size_t prev_mempool_index = 0; prev_mempool_index < prev_num_pools;
       ++prev_mempool_index) {
    new_mempool_sizes[ prev_mempool_index ] =
     sws_mempools.mempool_size_in_mem[ prev_mempool_index ];
  }
  // now add new
  new_mempool_sizes[ prev_num_pools ] = size_in_mem;

  // now actually add some string_with_size structs to the new queue
  for (size_t sws_index = 0; sws_index < num_elems; ++sws_index) {
    g_queue_push_tail(
     new_mempools[ prev_num_pools ],
     make_new_string_with_size_given_index(size_in_mem, prev_num_pools));
  }

  // now update the singleton and free old mem
  free(sws_mempools.mempools);
  sws_mempools.mempools = new_mempools;
  ++sws_mempools.num_sws_mempools;
  free(sws_mempools.mempool_size_in_mem);
  sws_mempools.mempool_size_in_mem = new_mempool_sizes;
}

string_with_size *
 make_new_string_with_size_given_index(unsigned long long size_in_mem,
                                       size_t mempool_index) {
  string_with_size * sws_to_return = malloc(sizeof(string_with_size));
  // get size in mem from sws_mempools, and allocate that much
  sws_to_return->string = malloc(size_in_mem * sizeof(char));
  sws_to_return->readable_bytes = 0;
  sws_to_return->size_in_memory = size_in_mem;
  sws_to_return->mempool_index = mempool_index;
  return sws_to_return;
}

string_with_size * make_new_string_with_size_from_pool(size_t mempool_index) {
  if (g_queue_get_length(sws_mempools.mempools[ mempool_index ]) > 0) {
    return g_queue_pop_head(sws_mempools.mempools[ mempool_index ]);
  } else {
    string_with_size * sws_to_return = malloc(sizeof(string_with_size));
    // get size in mem from sws_mempools, and allocate that much
    sws_to_return->string =
     malloc(sws_mempools.mempool_size_in_mem[ mempool_index ] * sizeof(char));
    sws_to_return->readable_bytes = 0;
    sws_to_return->size_in_memory =
     sws_mempools.mempool_size_in_mem[ mempool_index ];
    sws_to_return->mempool_index = mempool_index;
    return sws_to_return;
  }
}

void free_string_with_size_to_pool(void * arg) {
  if (NULL != arg) {
    string_with_size * sws = (string_with_size *) arg;
    sws->readable_bytes = 0; // make it clean
    g_queue_push_tail(sws_mempools.mempools[ sws->mempool_index ], sws);
  }
}
#endif
// TODO: javadoc
string_with_size *
 set_string_with_size_readable_bytes(string_with_size * sws,
                                     unsigned long long readable_bytes) {
  sws->readable_bytes = readable_bytes;
  return sws;
}
// assumes enough memory in to_sws exists to handle this operation
string_with_size * copy_string_with_size(string_with_size * from_sws,
                                         string_with_size * to_sws) {
  for (unsigned long long index = 0; index < from_sws->readable_bytes;
       ++index) {
    to_sws->string[ index ] = from_sws->string[ index ];
  }
  return to_sws;
}
string_with_size * grow_string_with_size(string_with_size ** sws,
                                         unsigned long long final_size_in_mem) {
  if (final_size_in_mem > (*sws)->size_in_memory) {
    // double size of sws for amortized constant time growth
    // +1 catches size == 0
    unsigned long long optimal_size = (*sws)->size_in_memory + 1;
    while (optimal_size < final_size_in_mem) {
      optimal_size *= 2;
    }
    string_with_size * new_sws = make_new_string_with_size(optimal_size);
    new_sws->readable_bytes = (*sws)->readable_bytes;
    copy_string_with_size(*sws, new_sws);
    free_string_with_size(*sws);
    *sws = new_sws; // guaranteed not to fail
  }
  return *sws;
}
// TODO: javadoc
void free_string_with_size(void * arg) {
  if (NULL != arg) {
    string_with_size * sws_to_free = (string_with_size *) arg;
    free(sws_to_free->string);
    free(sws_to_free);
  }
}
