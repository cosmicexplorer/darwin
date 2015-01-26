#include "vcsfmt.h"           // required
#include "block_processing.h" // for vcsfmt functions

// VCSFMT
void vcsfmt(char * filename, char * output_file_path) {
  FILE * input_file = open_file_read(filename);
  PRINT_ERROR_AND_RETURN_IF_NULL(input_file, "Error in creating input file.");

  // create output filename
  char * output_file_name =
   malloc((strlen(filename) + strlen(OUTPUT_SUFFIX_VCSFMT) + 1) * sizeof(char));
  strcpy(output_file_name, filename);
  strcat(output_file_name, OUTPUT_SUFFIX_VCSFMT);

  char * full_out_file_path;
  if (strcmp(output_file_path, "") != 0) {
    full_out_file_path = output_file_path;
  } else {
    full_out_file_path = output_file_name;
  }
  FILE * output_file = create_file_binary_write(full_out_file_path);
  PRINT_ERROR_AND_RETURN_IF_NULL(output_file, "Error in creating output file.");

#ifndef CONCURRENT
  string_with_size * input_block_with_size =
   make_new_string_with_size(BLOCK_SIZE);

  string_with_size * output_block_with_size =
   make_new_string_with_size(BIN_BLOCK_SIZE);
#endif

  bool * is_within_orf = malloc(sizeof(bool));
  *is_within_orf = false; // file begins outside of orf
  unsigned long long * cur_orf_pos = malloc(sizeof(unsigned long long));
  *cur_orf_pos = 0;
  char * current_codon_frame = malloc(sizeof(char) * CODON_LENGTH);
  for (size_t base_index = 0; base_index < CODON_LENGTH; ++base_index) {
    current_codon_frame[ base_index ] = '\0';
  }

#ifdef CONCURRENT
#ifdef MEMPOOL
  // create string_with_size memory pools
  g_mutex_init(&modify_sws_mempools_mutex);
  add_string_with_size_pool(BLOCK_SIZE, STARTING_NUM_SWS_IN_POOL);
  add_string_with_size_pool(BIN_BLOCK_SIZE, STARTING_NUM_SWS_IN_POOL);
#endif
  GAsyncQueue * read_queue = g_async_queue_new();
  GAsyncQueue * write_queue = g_async_queue_new();
  volatile bool * is_reading_complete = malloc(sizeof(bool));
  *is_reading_complete = false;
  volatile bool * is_processing_complete = malloc(sizeof(bool));
  *is_processing_complete = false;
  GMutex * read_complete_mutex = malloc(sizeof(GMutex));
  g_mutex_init(read_complete_mutex);
  GMutex * process_complete_mutex = malloc(sizeof(GMutex));
  g_mutex_init(process_complete_mutex);

  concurrent_read_block_args_vcsfmt * args_to_read_block =
   malloc(sizeof(concurrent_read_block_args_vcsfmt));
  args_to_read_block->input_file = input_file;
  args_to_read_block->input_block_with_size = NULL;
  args_to_read_block->read_queue = read_queue;
  args_to_read_block->is_reading_complete = is_reading_complete;
  args_to_read_block->read_complete_mutex = read_complete_mutex;

  concurrent_process_block_args_vcsfmt * args_to_block_processing =
   malloc(sizeof(concurrent_process_block_args_vcsfmt));
  args_to_block_processing->is_within_orf = is_within_orf;
  args_to_block_processing->cur_orf_pos = cur_orf_pos;
  args_to_block_processing->current_codon_frame = current_codon_frame;
  args_to_block_processing->input_block_with_size = NULL;
  args_to_block_processing->output_block_with_size = NULL;
  args_to_block_processing->read_queue = read_queue;
  args_to_block_processing->write_queue = write_queue;
  args_to_block_processing->is_reading_complete = is_reading_complete;
  args_to_block_processing->is_processing_complete = is_processing_complete;
  args_to_block_processing->is_processing_complete = is_processing_complete;
  args_to_block_processing->read_complete_mutex = read_complete_mutex;
  args_to_block_processing->process_complete_mutex = process_complete_mutex;

  concurrent_write_block_args_vcsfmt * args_to_write_block =
   malloc(sizeof(concurrent_write_block_args_vcsfmt));
  args_to_write_block->output_file = output_file;
  args_to_write_block->output_block_with_size = NULL;
  args_to_write_block->write_queue = write_queue;
  args_to_write_block->is_processing_complete = is_processing_complete;
  args_to_write_block->process_complete_mutex = process_complete_mutex;

  // PIPELINE!!!
  // read_block_thread -> process_block_thread -> write_block_thread

  GThread * read_block_thread =
   g_thread_new("read_block_thread", (GThreadFunc) concurrent_read_block_vcsfmt,
                args_to_read_block);

  GThread * process_block_thread = g_thread_new(
   "process_block_thread", (GThreadFunc) concurrent_process_block_vcsfmt,
   args_to_block_processing);

  GThread * write_block_thread = g_thread_new(
   "write_block_thread", (GThreadFunc) concurrent_write_block_vcsfmt,
   args_to_write_block);

  // we know the threads will complete in this order
  // because of the queue pipelines

  PRINT_ERROR("HERE1");
  g_thread_join(read_block_thread);
  PRINT_ERROR("HERE2");
  g_thread_join(process_block_thread);
  g_thread_join(write_block_thread);

#else

  while (!feof(input_file) && !ferror(input_file) && !ferror(output_file)) {
    // Read a block
    read_block(input_file, input_block_with_size);

    // Process the block and write it to output.
    write_block(output_file,
                process_block_vcsfmt(
                 input_block_with_size, output_block_with_size, is_within_orf,
                 cur_orf_pos, current_codon_frame, feof(input_file)));
  }
#endif

  fputc(NEWLINE, output_file);

  // error handling
  if (ferror(input_file) && !feof(input_file)) {
    PRINT_ERROR("Error in reading from input file.");
  } else if (ferror(output_file) && !feof(input_file)) {
    PRINT_ERROR("Error in writing to output file.");
  } else if (feof(input_file)) {
    PRINT_ERROR("vcsfmt completed successfully.");
  } else {
    PRINT_ERROR("Unknown error in vcsfmt.");
  }

// cleanup allocated memory and open handles
#ifdef CONCURRENT
  free(args_to_read_block);
  free(args_to_block_processing);
  free(args_to_write_block);
  // TODO: fix mutex and thread memory leaks (if they actually exist???)
  g_async_queue_unref(read_queue);
  g_async_queue_unref(write_queue);
// TODO: don't leak mem
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-W#warnings"
#warning THE CAST HERE IS IFFY AND SHOULD BE FIXED; HOW TO FREE VOLATILE PTR?
  free((void *) is_reading_complete);
  free((void *) is_processing_complete);
#pragma GCC diagnostic pop
#else
#warning NON-GCC/CLANG COMPILERS NOT YET SUPPORTED
#endif
  g_mutex_clear(read_complete_mutex);
  g_mutex_clear(process_complete_mutex);
  free(read_complete_mutex);
  free(process_complete_mutex);
#ifdef MEMPOOL
  g_mutex_clear(&modify_sws_mempools_mutex);
  free_string_with_size_mem_pools();
#endif
#else
  free_string_with_size(input_block_with_size);
  free_string_with_size(output_block_with_size);
#endif
  free(output_file_name);
  free(is_within_orf);
  free(cur_orf_pos);
  free(current_codon_frame);
  // close open handles
  fclose(input_file);
  fclose(output_file);
}

// DE_VCSFMT
void de_vcsfmt(char * filename, char * output_file_path) {
  FILE * input_file = open_file_read(filename);
  PRINT_ERROR_AND_RETURN_IF_NULL(input_file, "Error in creating input file.");

  char * output_file_name = malloc(
   (strlen(filename) + strlen(OUTPUT_SUFFIX_DE_VCSFMT) + 1) * sizeof(char));
  strcpy(output_file_name, filename);
  strcat(output_file_name, OUTPUT_SUFFIX_DE_VCSFMT);

  char * full_out_file_path;
  if (strcmp(output_file_path, "") != 0) {
    full_out_file_path = output_file_path;
  } else {
    full_out_file_path = output_file_name;
  }
  FILE * output_file = create_file_binary_write(full_out_file_path);
  PRINT_ERROR_AND_RETURN_IF_NULL(output_file, "Error in creating output file.");

#ifndef CONCURRENT
  string_with_size * input_block_with_size =
   make_new_string_with_size(BIN_BLOCK_SIZE);
  string_with_size * output_block_with_size =
   make_new_string_with_size(2 * BIN_BLOCK_SIZE); // TODO: fix mem size here
#endif

  unsigned long long * cur_posn_in_line = malloc(sizeof(int));
  *cur_posn_in_line = 0;

#ifdef CONCURRENT
// #ifdef MEMPOOL
//   g_mutex_init(&modify_sws_mempools_mutex);
//   add_string_with_size_pool(BIN_BLOCK_SIZE, STARTING_NUM_SWS_IN_POOL);
//   add_string_with_size_pool(2 * BIN_BLOCK_SIZE, STARTING_NUM_SWS_IN_POOL);
// #endif
//   GAsyncQueue * read_queue = g_async_queue_new();
//   GAsyncQueue * write_queue = g_async_queue_new();
//   volatile bool * is_reading_complete = malloc(sizeof(bool));
//   *is_reading_complete = false;
//   volatile bool * is_processing_complete = malloc(sizeof(bool));
//   *is_processing_complete = false;
//   GMutex * read_complete_mutex = malloc(sizeof(GMutex));
//   g_mutex_init(read_complete_mutex);
//   GMutex * process_complete_mutex = malloc(sizeof(GMutex));
//   g_mutex_init(process_complete_mutex);
#else
  while (!((feof(input_file)) || ferror(input_file)) && !ferror(output_file)) {
    // Read a block of genetic data
    read_block(input_file, input_block_with_size);

    write_block(output_file, de_process_block_vcsfmt(input_block_with_size,
                                                     output_block_with_size,
                                                     cur_posn_in_line));
  }
#endif

  fputc(NEWLINE, output_file);

  if (ferror(input_file) && !feof(input_file)) {
    PRINT_ERROR("Error in reading from input file.");
  } else if (ferror(output_file) && !feof(input_file)) {
    PRINT_ERROR("Error in writing to output file.");
  } else if (feof(input_file)) {
    PRINT_ERROR("de_vcsfmt completed successfully");
  } else {
    PRINT_ERROR("Unknown error in de_vcsfmt");
  }
  // close open handles
  fclose(input_file);
  fclose(output_file);
  // cleanup mem
  free(output_file_name);
#ifdef CONCURRENT
#else
  free_string_with_size(input_block_with_size);
  free_string_with_size(output_block_with_size);
#endif
}
