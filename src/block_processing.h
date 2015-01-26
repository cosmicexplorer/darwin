#ifndef ___BLOCK_PROCESSING_H___
#define ___BLOCK_PROCESSING_H___

/*
    functions to process strings of dna data in sequential blocks
*/

#include "string_processing.h" // provide codon sequence data to files

// chosen so that maximum BIN_BLOCK_SIZE is 8192, a power of 2 (heuristic for
// fast file I/O)
// TODO: rename this to something more indicative of usage
#define BLOCK_SIZE 7928

// only look for orfs above 60 bases long (heuristic)
#define MIN_ORF_LENGTH 60
// TODO: convert this to program option at some point, not compile-time
// definition

// output block maximum size
// maximum possible size of output block, assuming every possible 60-char
// sequence is an orf (which won't happen)
// two newlines per orf in the worst possible case
#define BIN_BLOCK_SIZE \
  (unsigned long long)(BLOCK_SIZE * (1 + 2 / (double) MIN_ORF_LENGTH))

string_with_size * read_block(FILE * input_file,
                              string_with_size * input_string_with_size);

string_with_size * read_block_with_offset(
 FILE * input_file, string_with_size * input_string_with_size,
 unsigned long long size_to_read, unsigned long long offset);

string_with_size * write_block(FILE * output_file,
                               string_with_size * output_block_with_size);

mpz_t * increment_mpz_t(mpz_t * in);

bool less_than_mpz_t(mpz_t * lhs, mpz_t * rhs);

bool less_than_or_equal_to_mpz_t(mpz_t * lhs, mpz_t * rhs);

bool equal_to_mpz_t(mpz_t * lhs, mpz_t * rhs);

// CLOBBERS CUR_LINE BY SETTING EQUAL TO FINAL_LINE
// starts again from beginning of file if final_line < cur_line
FILE * advance_file_to_line(FILE * file, mpz_t * cur_line, mpz_t * final_line,
                            unsigned long long block_size);

void write_current_line_of_file(FILE * source_file, FILE * dest_file);

// CLOBBERS FROM_LINE_NUMBER (increments)
// SETS FILE POINTER TO FIRST CHARACTER OF *NEXT* LINE
// INCLUDES NEWLINE
void write_current_line_of_file_incf_index(mpz_t * from_line_number,
                                           FILE * source_file,
                                           FILE * dest_file);

// TODO: have function which advances to line and writes to output for speed

// CLOBBERS FROM_LINE_NUMBER
// i.e. sets it equal to to_line_number + 1
void write_line_number_from_file_to_file(mpz_t * from_line_number,
                                         mpz_t * to_line_number,
                                         FILE * source_file, FILE * dest_file);

// assumes file is at beginning of required line
// puts file pointer back where it started
string_with_size * get_current_line_of_file(FILE * source_file);

/**
 *  Given a continuous stream of DNA characters, this function will insert
 *  newline characters in between genes and junk DNA. In other words, each line
 *  in the output will be either a gene or junk DNA.
 *
 *  This function expects the input to only contain DNA characters (no new lines
 *  or anything else).
 *
 *  This function is also written to be able to process data in multiple
 *  chunks. If multiple calls are made to this function for different chunks of
 *  the same data, the same parameters should be passed in each call. This will
 *  let the function remember key information about the last chunk it processed.
 */
string_with_size *
 process_block_vcsfmt(string_with_size * input_block_with_size,
                      string_with_size * output_block_with_size,
                      bool * is_within_orf, unsigned long long * cur_orf_pos,
                      char * current_codon_frame, bool is_final_block);

// left non-const so it can be modified at runtime with argv
extern unsigned long long FASTA_LINE_LENGTH;
string_with_size *
 de_process_block_vcsfmt(string_with_size * input_block_with_size,
                         string_with_size * output_block_with_size,
                         unsigned long long * cur_posn_in_line);

#ifdef CONCURRENT
// send arguments to queue
// used as variadic arguments to function
// concurrent_read_and_process_block_vcsfmt for GThread
// TODO: obv javadoc
typedef struct {
  FILE * input_file;
  string_with_size * input_block_with_size;
  GAsyncQueue * read_queue;
  volatile bool * is_reading_complete;
  GMutex * read_complete_mutex;
} concurrent_read_block_args_vcsfmt;
typedef struct {
  bool * is_within_orf;
  unsigned long long * cur_orf_pos;
  char * current_codon_frame;
  string_with_size * input_block_with_size;
  string_with_size * output_block_with_size;
  GAsyncQueue * read_queue;
  GAsyncQueue * write_queue;
  volatile bool * is_reading_complete;
  volatile bool * is_processing_complete;
  GMutex * read_complete_mutex;
  GMutex * process_complete_mutex;
} concurrent_process_block_args_vcsfmt;
// used as variadic arguments to function
// concurrent_read_and_process_block_vcsfmt for GThread
// TODO: obv javadoc
typedef struct {
  FILE * output_file;
  string_with_size * output_block_with_size;
  GAsyncQueue * write_queue;
  volatile bool * is_processing_complete;
  GMutex * process_complete_mutex;
} concurrent_write_block_args_vcsfmt;

// TODO: javadoc
void concurrent_read_block_vcsfmt(concurrent_read_block_args_vcsfmt * args);

// TODO: javadoc
void concurrent_process_block_vcsfmt(
 concurrent_process_block_args_vcsfmt * args);

// TODO: javadoc
bool is_last_read_block_vcsfmt_concurrent(
 concurrent_process_block_args_vcsfmt * args);

// TODO: javadoc
void concurrent_write_block_vcsfmt(concurrent_write_block_args_vcsfmt * args);

// TODO: javadoc
// cool mutex stuff
bool is_processing_complete_vcsfmt_concurrent(
 concurrent_write_block_args_vcsfmt * args);

typedef struct {
  unsigned long long * cur_posn_in_line;
  string_with_size * input_block_with_size;
  string_with_size * output_block_with_size;
  GAsyncQueue * read_queue;
  GAsyncQueue * write_queue;
  volatile bool * is_reading_complete;
  volatile bool * is_processing_complete;
  GMutex * read_complete_mutex;
  GMutex * process_complete_mutex;
} concurrent_process_block_args_de_vcsfmt;

// TODO: javadoc
void concurrent_process_block_de_vcsfmt(
 concurrent_process_block_args_de_vcsfmt * args);

bool is_reading_complete_de_vcsfmt_concurrent(
 concurrent_process_block_args_de_vcsfmt * args);

#endif
#endif /*___BLOCK_PROCESSING_H___*/
