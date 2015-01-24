#include "block_processing.h"

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
                                        bool * is_within_comment,
                                        unsigned long long * cur_orf_pos,
                                        char * current_codon_frame,
                                        bool is_final_block) {
  output_block->readable_bytes = 0;
  for (unsigned long long codon_index = 0;
       codon_index < input_block->readable_bytes; ++codon_index) {
    // first base is only null at start/end of ORF or at beginning
    if ('\0' != current_codon_frame[ 0 ]) {
      if (current_codon_frame[ 0 ] != 'A' && current_codon_frame[ 0 ] != 'G' &&
          current_codon_frame[ 0 ] != 'C' && current_codon_frame[ 0 ] != 'T' &&
          current_codon_frame[ 0 ] != '\n') {
        fprintf(stderr, "%c\n", current_codon_frame[ 0 ]);
      }
      if (*is_within_comment) {
        // terminate comments at end of line
        if (NEWLINE == current_codon_frame[ 0 ]) {
          *is_within_comment = false;
        }
        output_block->string[ output_block->readable_bytes ] =
         current_codon_frame[ 0 ];
        ++output_block->readable_bytes;
      } else if ('>' == current_codon_frame[ 0 ]) {
        PRINT_ERROR("yo lol");
        *is_within_comment = true;
        output_block->string[ output_block->readable_bytes ] = NEWLINE;
        ++output_block->readable_bytes;
        output_block->string[ output_block->readable_bytes ] = NEWLINE;
        ++output_block->readable_bytes;
        output_block->string[ output_block->readable_bytes ] =
         current_codon_frame[ 0 ];
        ++output_block->readable_bytes;
      } else if (NEWLINE == current_codon_frame[ 0 ]) {
        // do nothing
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
    current_codon_frame[ CODON_LENGTH - 1 ] = '\0'; // nullify final
    while (codon_index + 1 < input_block->readable_bytes) {
      if (NEWLINE != input_block->string[ codon_index + 1 ]) {
        current_codon_frame[ CODON_LENGTH - 1 ] =
         input_block->string[ codon_index + 1 ];
        break;
      } else {
        ++codon_index;
      }
    }
    // leaves first two codons in current_codon_frame pointer for next block
  }

  // if this is the last block, eject the last two bases
  if (is_final_block) {
    for (unsigned long long base_index = 0; base_index < CODON_LENGTH - 1;
         ++base_index) {
      if (output_block->string[ output_block->readable_bytes + base_index ] !=
          '\0') {
        output_block->string[ output_block->readable_bytes + base_index ] =
         current_codon_frame[ base_index ];
      }
    }
    output_block->readable_bytes += CODON_LENGTH - 1;
  }
  return output_block;
}

unsigned long long FASTA_LINE_LENGTH = 70;
string_with_size * de_process_block_vcsfmt(string_with_size * input_block,
                                           string_with_size * output_block,
                                           bool * is_in_comment,
                                           int * cur_posn_in_line) {
  output_block->readable_bytes = 0;
  for (unsigned long long bytes_read = 0;
       bytes_read < input_block->readable_bytes; ++bytes_read) {
    if (!*is_in_comment) {
      if ('>' == input_block->string[ bytes_read ]) {
        *is_in_comment = true;
        output_block->string[ output_block->readable_bytes ] = NEWLINE;
        ++output_block->readable_bytes;
        output_block->string[ output_block->readable_bytes ] =
         input_block->string[ bytes_read ];
        ++output_block->readable_bytes;
        *cur_posn_in_line = 0;
      } else if (*cur_posn_in_line > ((int) FASTA_LINE_LENGTH) - 2 &&
                 // IFFY: this cast scares me
                 NEWLINE != input_block->string[ bytes_read ]) {
        output_block->string[ output_block->readable_bytes ] = NEWLINE;
        ++output_block->readable_bytes;
        output_block->string[ output_block->readable_bytes ] =
         input_block->string[ bytes_read ];
        ++output_block->readable_bytes;
        *cur_posn_in_line = 0;
      } else if (NEWLINE != input_block->string[ bytes_read ]) {
        // if <= FASTA_LINE_LENGTH
        output_block->string[ output_block->readable_bytes ] =
         input_block->string[ bytes_read ];
        ++output_block->readable_bytes;
        ++*cur_posn_in_line;
      }
    } else { // in comment
      if (NEWLINE == input_block->string[ bytes_read ]) {
        *is_in_comment = false;
        *cur_posn_in_line = -1;
      } else {
        ++*cur_posn_in_line;
      }
      output_block->string[ output_block->readable_bytes ] =
       input_block->string[ bytes_read ];
      ++output_block->readable_bytes;
    }
  }
  return output_block;
}

#ifdef CONCURRENT
void concurrent_read_and_process_block_vcsfmt(
 concurrent_read_and_process_block_args_vcsfmt * args) {
  while (!feof(args->input_file) && !ferror(args->input_file)) {
    add_to_bytes_processed(
     args->total_bytes_read,
     read_block(args->input_file, args->input_block)->readable_bytes);
    // OPTIMIZATION: allocate from (possibly self-growing) pool of memory
    args->output_block = make_new_string_with_size(BIN_BLOCK_SIZE);
    process_block_vcsfmt(args->input_block, args->output_block,
                         args->is_within_orf, args->cur_orf_pos,
                         args->current_codon_frame, feof(args->input_file));
    if (feof(args->input_file)) { // if last loop
      g_mutex_lock(args->process_complete_mutex);
    }
    g_async_queue_push(args->active_queue, args->output_block);
  }
  *args->is_processing_complete = true;
  g_mutex_unlock(args->process_complete_mutex);
}

void concurrent_write_block_vcsfmt(
 concurrent_read_write_block_args_vcsfmt * args) {
  while (!is_processing_complete_vcsfmt_concurrent(args)) {
    g_mutex_unlock(args->process_complete_mutex);
    args->active_block =
     (string_with_size *) g_async_queue_pop(args->active_queue);
    add_to_bytes_processed(
     args->total_bytes_written,
     write_block(args->active_file, args->active_block)->readable_bytes);
    free_string_with_size(args->active_block);
  }
  // get the rest left over (if any)
  for (unsigned long long queue_size = g_async_queue_length(args->active_queue);
       queue_size != 0; --queue_size) {
    args->active_block =
     (string_with_size *) g_async_queue_pop(args->active_queue);
    add_to_bytes_processed(
     args->total_bytes_written,
     write_block(args->active_file, args->active_block)->readable_bytes);
    free_string_with_size(args->active_block);
  }
}

bool is_processing_complete_vcsfmt_concurrent(
 concurrent_read_write_block_args_vcsfmt * args) {
  if (g_async_queue_length(args->active_queue) != 0) {
    return false;
  } else {
    // OPTIMIZATION: make this variable static somehow
    bool result;
    g_mutex_lock(args->process_complete_mutex);
    result = *args->is_processing_complete;
    g_mutex_unlock(args->process_complete_mutex);
    return result;
  }
}

#endif
