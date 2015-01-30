#include "block_processing.h"

// OPTIMIZATION: use readahead to cache file contents over multiple freads!!!!

string_with_size * read_block(FILE * input_file,
                              string_with_size * input_string_with_size) {
  input_string_with_size->readable_bytes =
   fread(input_string_with_size->string, sizeof(char),
         input_string_with_size->size_in_memory, input_file);
  return input_string_with_size;
}

string_with_size * read_block_with_offset(
 FILE * input_file, string_with_size * input_string_with_size,
 unsigned long long size_to_read, unsigned long long offset) {
  input_string_with_size->readable_bytes +=
   fread(input_string_with_size->string + offset, sizeof(char), size_to_read,
         input_file);
  return input_string_with_size;
}

string_with_size * write_block(FILE * output_file,
                               string_with_size * output_block) {
  output_block->readable_bytes =
   fwrite(output_block->string, sizeof(char), output_block->readable_bytes,
          output_file);
  return output_block;
}

mpz_t * increment_mpz_t(mpz_t * in) {
  mpz_add_ui(*in, *in, 1);
  return in;
}

bool less_than_mpz_t(mpz_t * lhs, mpz_t * rhs) {
  return mpz_cmp(*lhs, *rhs) < 0;
}

bool less_than_or_equal_to_mpz_t(mpz_t * lhs, mpz_t * rhs) {
  return mpz_cmp(*lhs, *rhs) <= 0;
}

bool equal_to_mpz_t(mpz_t * lhs, mpz_t * rhs) {
  return mpz_cmp(*lhs, *rhs) == 0;
}

// CLOBBERS CUR_LINE BY SETTING EQUAL TO FINAL_LINE
// starts again from beginning of file if final_line < cur_line
FILE * advance_file_to_line(FILE * file, mpz_t * cur_line, mpz_t * final_line,
                            unsigned long long block_size) {
  if (!less_than_mpz_t(cur_line, final_line)) {
    // if final_line < cur_line
    if (!equal_to_mpz_t(cur_line, final_line)) {
      rewind(file); // IFFY: this will DESTROY concurrent access
      mpz_set_ui(*cur_line, 1);
      return advance_file_to_line(file, cur_line, final_line, block_size);
    }
    return file;
  } else {
    string_with_size * in_block = make_new_string_with_size(block_size);
    bool succeeded = false;
    while (!succeeded && !(feof(file) || ferror(file))) {
      read_block(file, in_block);
      for (unsigned long long block_index = 0;
           block_index < in_block->readable_bytes; ++block_index) {
        if (NEWLINE == in_block->string[ block_index ]) {
          increment_mpz_t(cur_line);
          if (!less_than_mpz_t(cur_line, final_line)) { // if ==
            // go back to beginning of line
            // IFFY: the off-by-one errors here are killer
            fseek(file,
                  ((long) block_index) - ((long) in_block->readable_bytes - 1),
                  SEEK_CUR);
            succeeded = true;
            break;
          }
        }
      }
    }
    free_string_with_size(in_block);
    return file;
  }
}

void write_current_line_of_file(FILE * source_file, FILE * dest_file) {
  string_with_size * io_block = make_new_string_with_size(BIN_BLOCK_SIZE);
  bool succeeded = false;
  while (!succeeded && !(feof(source_file) || ferror(source_file))) {
    read_block(source_file, io_block);
    for (unsigned long long block_index = 0;
         block_index < io_block->readable_bytes; ++block_index) {
      if (NEWLINE == io_block->string[ block_index ]) {
        // go back to beginning of line
        // IFFY: the off-by-one errors here are killer
        fseek(source_file,
              ((long) block_index) - ((long) io_block->readable_bytes - 1),
              SEEK_CUR);
        // +1 is to include the newline at the end
        set_string_with_size_readable_bytes(io_block, block_index + 1);
        succeeded = true;
        break;
      }
    }
    write_block(dest_file, io_block);
  }
  free_string_with_size(io_block);
}

// CLOBBERS FROM_LINE_NUMBER (increments)
// SETS FILE POINTER TO FIRST CHARACTER OF *NEXT* LINE
// INCLUDES NEWLINE
void write_current_line_of_file_incf_index(mpz_t * from_line_number,
                                           FILE * source_file,
                                           FILE * dest_file) {
  write_current_line_of_file(source_file, dest_file);
  increment_mpz_t(from_line_number);
}

// CLOBBERS FROM_LINE_NUMBER
// i.e. sets it equal to to_line_number + 1
void write_line_number_from_file_to_file(mpz_t * from_line_number,
                                         mpz_t * to_line_number,
                                         FILE * source_file, FILE * dest_file) {
  advance_file_to_line(source_file, from_line_number, to_line_number,
                       BIN_BLOCK_SIZE);
  write_current_line_of_file_incf_index(from_line_number, source_file,
                                        dest_file);
}

// assumes file is at beginning of required line
// puts file pointer back where it started
// assumes line is short enough to be placed in single contiguous memory region
string_with_size * get_current_line_of_file(FILE * source_file) {
  // get length of string_with_size
  unsigned long long current_offset = 0;
  string_with_size * io_block = make_new_string_with_size(BIN_BLOCK_SIZE);
  bool succeeded = false;
  while (!succeeded && !(feof(source_file) || ferror(source_file))) {
    read_block_with_offset(source_file, io_block, BIN_BLOCK_SIZE,
                           current_offset);
    for (unsigned long long current_slot_in_sws = current_offset;
         current_slot_in_sws < BIN_BLOCK_SIZE; ++current_slot_in_sws) {
      if (NEWLINE == io_block->string[ current_slot_in_sws ]) {
        fseek(source_file, -((long) io_block->readable_bytes), SEEK_CUR);
        // +1 to include newline
        io_block->readable_bytes = current_slot_in_sws + 1;
        succeeded = true;
        break;
      }
    }
    current_offset = io_block->readable_bytes;
    grow_string_with_size(&io_block, current_offset + BIN_BLOCK_SIZE);
  }
  return io_block;
}

string_with_size * process_block_vcsfmt(string_with_size * input_block,
                                        string_with_size * output_block,
                                        bool * is_within_orf,
                                        unsigned long long * cur_orf_pos,
                                        char * current_codon_frame,
                                        bool is_final_block) {
  set_string_with_size_readable_bytes(output_block, 0);
  for (unsigned long long codon_index = 0;
       codon_index < input_block->readable_bytes; ++codon_index) {
    // TODO: figure out why valgrind is saying there's an uninitialized value
    // cause this actually makes no sense whatsoever
    while (NEWLINE == input_block->string[ codon_index ] &&
           codon_index < input_block->readable_bytes) {
      ++codon_index;
    }
    // MUST be true iff newline is the last character in input_block
    if (codon_index >= input_block->readable_bytes) {
      break;
    } else {
      current_codon_frame[ CODON_LENGTH - 1 ] =
       input_block->string[ codon_index ];
    }
    // first base is only null at start/end of ORF or at beginning
    if ('\0' != current_codon_frame[ 0 ]) {
      if (NEWLINE == current_codon_frame[ 0 ]) {
#ifdef DEBUG
        PRINT_ERROR("SHOULD NEVER BE HERE; NEWLINES SHOULD BE GONE");
#endif
      } else if (*is_within_orf) {
        if (*cur_orf_pos >= MIN_ORF_LENGTH - CODON_LENGTH &&
            is_stop_codon(current_codon_frame)) {
          for (unsigned long long base_index = 0; base_index < CODON_LENGTH;
               ++base_index) {
            output_block->string[ output_block->readable_bytes + base_index ] =
             current_codon_frame[ base_index ];
            current_codon_frame[ base_index ] = '\0';
          }
          output_block->string[ output_block->readable_bytes + CODON_LENGTH ] =
           NEWLINE;
          output_block->readable_bytes += CODON_LENGTH + 1;
          *is_within_orf = false;
          *cur_orf_pos = 0;
        } else {
          for (unsigned long long base_index = 0; base_index < CODON_LENGTH;
               ++base_index) {
            output_block->string[ output_block->readable_bytes + base_index ] =
             current_codon_frame[ base_index ];
            current_codon_frame[ base_index ] = '\0';
          }
          *cur_orf_pos += CODON_LENGTH;
          output_block->readable_bytes += CODON_LENGTH;
        }
      } else {
        if (is_start_codon(current_codon_frame)) {
          output_block->string[ output_block->readable_bytes ] = NEWLINE;
          for (unsigned long long base_index = 0; base_index < CODON_LENGTH;
               ++base_index) {
            output_block
             ->string[ output_block->readable_bytes + base_index + 1 ] =
             current_codon_frame[ base_index ];
            current_codon_frame[ base_index ] = '\0';
          }
          output_block->readable_bytes += CODON_LENGTH + 1;
          output_block->string[ output_block->readable_bytes + CODON_LENGTH ] =
           NEWLINE;
          *is_within_orf = true;
          *cur_orf_pos = CODON_LENGTH;
        } else {
          output_block->string[ output_block->readable_bytes ] =
           current_codon_frame[ 0 ];
          ++output_block->readable_bytes;
        }
      }
    }

    // shuffle bases over
    for (unsigned long long base_index = 0; base_index < CODON_LENGTH - 1;
         ++base_index) {
      current_codon_frame[ base_index ] = current_codon_frame[ base_index + 1 ];
    }
    current_codon_frame[ CODON_LENGTH - 1 ] = '\0';
    // leaves first two codons in current_codon_frame pointer for next block
  }

  // if this is the last block, eject the last two bases
  if (is_final_block) {
    for (unsigned long long base_index = 0; base_index < CODON_LENGTH - 1;
         ++base_index) {
      if (current_codon_frame[ base_index ] != '\0') {
        output_block->string[ output_block->readable_bytes ] =
         current_codon_frame[ base_index ];
        ++output_block->readable_bytes;
      }
    }
  }
  return output_block;
}

string_with_size *
 de_process_block_vcsfmt(string_with_size * input_block,
                         string_with_size * output_block,
                         unsigned long long * cur_posn_in_line) {
  output_block->readable_bytes = 0;
  for (unsigned long long bytes_read = 0;
       bytes_read < input_block->readable_bytes; ++bytes_read) {
    if (*cur_posn_in_line >= FASTA_LINE_LENGTH &&
        // IFFY: this cast scares me
        NEWLINE != input_block->string[ bytes_read ]) {
      output_block->string[ output_block->readable_bytes ] = NEWLINE;
      ++output_block->readable_bytes;
      output_block->string[ output_block->readable_bytes ] =
       input_block->string[ bytes_read ];
      ++output_block->readable_bytes;
      *cur_posn_in_line = 1;
    } else if (NEWLINE != input_block->string[ bytes_read ]) {
      // if <= FASTA_LINE_LENGTH
      output_block->string[ output_block->readable_bytes ] =
       input_block->string[ bytes_read ];
      ++output_block->readable_bytes;
      ++*cur_posn_in_line;
    }
  }
  return output_block;
}

#ifdef CONCURRENT
void concurrent_read_block_vcsfmt(concurrent_read_block_args_vcsfmt * args) {
  while (!feof(args->input_file) && !ferror(args->input_file)) {
#ifdef MEMPOOL
    args->input_block_with_size =
     make_new_string_with_size_from_pool(INPUT_BLOCK_MEMPOOL_INDEX_VCSFMT);
#else
    args->input_block_with_size = make_new_string_with_size(BLOCK_SIZE);
#endif
    read_block(args->input_file, args->input_block_with_size);
    if (feof(args->input_file)) {
      g_mutex_lock(args->read_complete_mutex);
      // TODO: figure out GAsyncQueue is reporting a size of -1 here
      // sometimes! THIS IS WHY THE DEADLOCK IS OCCURRING
      fprintf(stderr, "in queue size: %d\n",
              g_async_queue_length(args->read_queue));
    }
    g_async_queue_push(args->read_queue, args->input_block_with_size);
  }
  *args->is_reading_complete = true;
  g_mutex_unlock(args->read_complete_mutex);
}

void concurrent_process_block_vcsfmt(
 concurrent_process_block_args_vcsfmt * args) {
  while (!is_last_read_block_vcsfmt_concurrent(args)) {
    args->input_block_with_size = g_async_queue_pop(args->read_queue);
#ifdef MEMPOOL
    args->output_block_with_size =
     make_new_string_with_size_from_pool(OUTPUT_BLOCK_MEMPOOL_INDEX_VCSFMT);
#else
    args->output_block_with_size = make_new_string_with_size(BIN_BLOCK_SIZE);
#endif
    process_block_vcsfmt(args->input_block_with_size,
                         args->output_block_with_size, args->is_within_orf,
                         args->cur_orf_pos, args->current_codon_frame, false);
    g_async_queue_push(args->write_queue, args->output_block_with_size);
#ifdef MEMPOOL
    free_string_with_size_to_pool(args->input_block_with_size);
#else
    free_string_with_size(args->input_block_with_size); // let's not leak memory
#endif
  }
#ifdef DEBUG
  if (g_async_queue_length(args->read_queue) != 1) {
    PRINT_ERROR("THIS SHOULD NEVER HAPPEN");
  }
#endif
  args->input_block_with_size = g_async_queue_pop(args->read_queue);
#ifdef MEMPOOL
  args->output_block_with_size =
   make_new_string_with_size_from_pool(OUTPUT_BLOCK_MEMPOOL_INDEX_VCSFMT);
#else
  args->output_block_with_size = make_new_string_with_size(BIN_BLOCK_SIZE);
#endif
  process_block_vcsfmt(args->input_block_with_size,
                       args->output_block_with_size, args->is_within_orf,
                       args->cur_orf_pos, args->current_codon_frame, true);
  g_async_queue_push(args->write_queue, args->output_block_with_size);
#ifdef MEMPOOL
  free_string_with_size_to_pool(args->input_block_with_size);
#else
  free_string_with_size(args->input_block_with_size);
#endif
  g_mutex_lock(args->process_complete_mutex);
  *args->is_processing_complete = true;
  g_mutex_unlock(args->process_complete_mutex);
}

bool is_last_read_block_vcsfmt_concurrent(
 concurrent_process_block_args_vcsfmt * args) {
#ifdef DEBUG
  if (g_async_queue_length(args->read_queue) == 0) {
    PRINT_ERROR("THIS IS WHAT CAUSED THE THING THAT SHOULD NEVER HAPPEN");
  }
#endif
  if (g_async_queue_length(args->read_queue) > 1) {
    return false;
  } else {
    bool result;
    g_mutex_lock(args->read_complete_mutex);
    result = *args->is_reading_complete;
    g_mutex_unlock(args->read_complete_mutex);
    return result;
  }
}

void concurrent_write_block_vcsfmt(concurrent_write_block_args_vcsfmt * args) {
  while (!is_processing_complete_vcsfmt_concurrent(args)) {
    args->output_block_with_size =
     (string_with_size *) g_async_queue_pop(args->write_queue);
    write_block(args->output_file, args->output_block_with_size);
#ifdef MEMPOOL
    free_string_with_size_to_pool(args->output_block_with_size);
#else
    free_string_with_size(args->output_block_with_size);
#endif
  }
  if (g_async_queue_length(args->write_queue) > 0) {
    PRINT_ERROR("SHOULD NEVER BE HERE");
    exit(-1);
  }
}

bool is_processing_complete_vcsfmt_concurrent(
 concurrent_write_block_args_vcsfmt * args) {
  if (g_async_queue_length(args->write_queue) > 0) {
    return false;
  } else {
    bool result;
    g_mutex_lock(args->process_complete_mutex);
    result = *args->is_processing_complete;
    g_mutex_unlock(args->process_complete_mutex);
    return result;
  }
}

// TODO: put this and concurrent_process_block_vcsfmt into one function so that
// code isn't duplicated
void concurrent_process_block_de_vcsfmt(
 concurrent_process_block_args_de_vcsfmt * args) {
  while (!is_reading_complete_de_vcsfmt_concurrent(args)) {
    args->input_block_with_size = g_async_queue_pop(args->read_queue);
#ifdef MEMPOOL
    args->output_block_with_size =
     make_new_string_with_size_from_pool(OUTPUT_BLOCK_MEMPOOL_INDEX_VCSFMT);
#else
    args->output_block_with_size = make_new_string_with_size(BIN_BLOCK_SIZE);
#endif
    de_process_block_vcsfmt(args->input_block_with_size,
                            args->output_block_with_size,
                            args->cur_posn_in_line);
    g_async_queue_push(args->write_queue, args->output_block_with_size);
#ifdef MEMPOOL
    free_string_with_size_to_pool(args->input_block_with_size);
#else
    free_string_with_size(args->input_block_with_size); // let's not leak memory
#endif
  }
  g_mutex_lock(args->process_complete_mutex);
  *args->is_processing_complete = true;
  g_mutex_unlock(args->process_complete_mutex);
}

bool is_reading_complete_de_vcsfmt_concurrent(
 concurrent_process_block_args_de_vcsfmt * args) {
  if (g_async_queue_length(args->read_queue) > 0) {
    return false;
  } else {
    bool result;
    g_mutex_lock(args->read_complete_mutex);
    result = *args->is_reading_complete;
    g_mutex_unlock(args->read_complete_mutex);
    return result;
  }
}

#endif
