#include "vcsfmt.h"           // required
#include "block_processing.h" // for vcsfmt functions

// VCSFMT
void vcsfmt(char * filename, char * output_file_path) {
  FILE * input_file = open_file_read(filename);
  if (NULL == input_file) {
    printf("Error in creating input file.\n");
    return;
  }

  // create output filename
  char * output_file_name =
   malloc((strlen(filename) + strlen(OUTPUT_SUFFIX) + 1) * sizeof(char));
  strcpy(output_file_name, filename);
  strcat(output_file_name, OUTPUT_SUFFIX);

  char * full_out_file_path;
  if (strcmp(output_file_path, "") != 0) {
    full_out_file_path = output_file_path;
  } else {
    full_out_file_path = output_file_name;
  }
  FILE * output_file = create_file_binary_write(full_out_file_path);
  PRINT_ERROR_AND_RETURN_IF_NULL(output_file, "Error in creating output file.");

  string_with_size * input_block_with_size =
   make_new_string_with_size(BLOCK_SIZE);
#ifndef CONCURRENT
  string_with_size * output_block_with_size =
   make_new_string_with_size(BIN_BLOCK_SIZE);

#endif
  bool is_within_orf = false; // file begins outside of orf
  unsigned long long cur_orf_pos = 0;
  char * current_codon_frame = malloc(sizeof(char) * CODON_LENGTH);
  for (size_t base_index = 0; base_index < CODON_LENGTH; ++base_index) {
    current_codon_frame[ base_index ] = '\0';
  }

#ifdef CONCURRENT
  GAsyncQueue * active_queue = g_async_queue_new();
  bool * is_processing_complete = malloc(sizeof(bool));
  *is_processing_complete = false;
  GMutex * process_complete_mutex = malloc(sizeof(GMutex));
  g_mutex_init(process_complete_mutex);

  concurrent_read_and_process_block_args_vcsfmt args_to_block_processing;
  args_to_block_processing.input_file = input_file;
  args_to_block_processing.input_block_with_size = input_block_with_size;
  args_to_block_processing.is_within_orf = &is_within_orf;
  args_to_block_processing.cur_orf_pos = &cur_orf_pos;
  args_to_block_processing.current_codon_frame = current_codon_frame;
  args_to_block_processing.is_final_block = false;
  args_to_block_processing.active_queue = active_queue;
  // TODO: remove this?
  // args_to_block_processing.total_bytes_read = total_bytes_read;
  args_to_block_processing.is_processing_complete = is_processing_complete;
  args_to_block_processing.process_complete_mutex = process_complete_mutex;

  concurrent_read_write_block_args_vcsfmt args_to_write_block;
  args_to_write_block.active_file = output_file;
  args_to_write_block.active_queue = active_queue;
  // TODO: remove this?
  // args_to_write_block.total_bytes_written = total_bytes_written;
  args_to_write_block.is_processing_complete = is_processing_complete;
  args_to_write_block.process_complete_mutex = process_complete_mutex;

  GThread * read_and_process_block_thread =
   g_thread_new("read_and_process_block_thread",
                (GThreadFunc) concurrent_read_and_process_block_vcsfmt,
                &args_to_block_processing);
  GThread * write_block_thread = g_thread_new(
   "write_block_thread", (GThreadFunc) concurrent_write_block_vcsfmt,
   &args_to_write_block);
  g_thread_join(read_and_process_block_thread); // TODO: is this required?
  g_thread_join(write_block_thread);            // implicitly frees thread

#else

  while (!feof(input_file) && !ferror(input_file) && !ferror(output_file)) {
    // Read a block
    read_block(input_file, input_block_with_size);

    // Process the block and write it to output.
    write_block(output_file,
                process_block_vcsfmt(
                 input_block_with_size, output_block_with_size, &is_within_orf,
                 &cur_orf_pos, current_codon_frame, feof(input_file)));
  }

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

#endif
// cleanup allocated memory and open handles
#ifdef CONCURRENT
  // TODO: fix mutex and thread memory leaks (if they actually exist???)
  g_async_queue_unref(active_queue);
  free(is_processing_complete);
  g_mutex_clear(process_complete_mutex);
  g_thread_unref(read_and_process_block_thread); // write thread freed by join
#endif
  free(output_file_name);
  free_string_with_size(input_block_with_size);
#ifndef CONCURRENT
  free_string_with_size(output_block_with_size);
#endif
  // close open handles
  fclose(input_file);
  fclose(output_file);
}

// DE_VCSFMT
void de_vcsfmt(char * filename, char * output_file_path) {
  FILE * input_file = open_file_read(filename);
  PRINT_ERROR_AND_RETURN_IF_NULL(input_file, "Error in creating input file.");
  // create filename long enough to concatenate filename and suffix
  // assume filename ends in .vcsfmt (7 chars)
  char * output_file_name =
   malloc((strlen(filename) - strlen(OUTPUT_SUFFIX)) * sizeof(char));
  strncpy(output_file_name, filename, strlen(filename) - strlen(OUTPUT_SUFFIX));
  output_file_name[ strlen(filename) - strlen(OUTPUT_SUFFIX) ] = '\0';

  char * full_out_file_path;
  if (strcmp(output_file_path, "") != 0) {
    full_out_file_path = output_file_path;
  } else {
    full_out_file_path = output_file_name;
  }
  FILE * output_file = create_file_binary_write(full_out_file_path);
  PRINT_ERROR_AND_RETURN_IF_NULL(output_file, "Error in creating output file.");

  string_with_size * input_block_with_size =
   make_new_string_with_size(BIN_BLOCK_SIZE);
  string_with_size * output_block_with_size =
   make_new_string_with_size(2 * BIN_BLOCK_SIZE); // TODO: fix mem size here

  int * cur_posn_in_line = malloc(sizeof(int));
  *cur_posn_in_line = 0;
  while (!((feof(input_file)) || ferror(input_file)) && !ferror(output_file)) {
    // Read a block of genetic data
    read_block(input_file, input_block_with_size);

    write_block(output_file, de_process_block_vcsfmt(input_block_with_size,
                                                     output_block_with_size,
                                                     cur_posn_in_line));
  }
  fputc(NEWLINE, output_file);

  // cleanup mem
  free(output_file_name);
  free_string_with_size(input_block_with_size);
  free_string_with_size(output_block_with_size);
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
}
