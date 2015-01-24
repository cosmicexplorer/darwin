#ifndef ___VCS_CMP_H___
#define ___VCS_CMP_H___

/*
    functions and utilities for implementing character-by-character
    comparison on .vcsfmt-formatted files
*/

#include "block_processing.h"

/* cldoc:begin-category(line_id) */

// TODO: allow modification of this, find reasoning for choice of value
#define LEVENSHTEIN_CHECK_CHARS 80
#define LEVENSHTEIN_CHECK_THRESHOLD 20

/* aggregate structure representing the identity of a line
 */
typedef struct {
    /* gmp bignum representing line number */
    mpz_t line_number;
    /* hash of line */
    unsigned long int str_hash;
    /* length of line */
    unsigned long int str_length;
    /* first k characters of string, where k is defined previously */
    string_with_size * first_k_chars;
    /* whether line is an orf */
    bool is_orf;
} line_id;

/* set is_orf to given value
 * @sid line_id to modify
 * @set_is_orf boolean to send
 *
 * @return modified line_id
 */
line_id * line_id_set_is_orf(line_id * sid, bool set_is_orf);
/* set str_hash to given value
 * @sid line_id to modify
 * @str_hash value to set to
 *
 * @return modified line_id
 */
line_id * line_id_set_str_hash(line_id * sid, unsigned long int str_hash);
/* set str_length to given value
 * @sid line_id to modify
 * @str_length value to set to
 *
 * @return modified line_id
 */
line_id * line_id_set_str_length(line_id * sid, unsigned long int str_length);
/* set line_number to given value
 * @sid line_id to modify
 * @number_to_set_to value to set to
 *
 * @return modified line_id
 */
line_id * line_id_set_line_number(line_id * sid, mpz_t * number_to_set_to);
/* initialize line_number data member
 * @sid line_id to initialize
 *
 * @return initialized line_id
 */
line_id * line_id_initialize_line_number(line_id * sid);
/* set first_k_chars to given value
 * @sid line_id to modify
 * @str_k_chars value to set to
 *
 * @return modified line_id
 */
line_id * line_id_set_first_k_chars(line_id * sid,
                                    string_with_size * str_k_chars);
/* allocate and initialize a line_id
 * @str_hash hash to begin with
 * @str_length length to begin with
 *
 * @return constructed line_id
 */
line_id * make_line_id(unsigned long int str_hash,
                       unsigned long int str_length);
/* allocate and initialize a line_id given a string_with_size argument
 * @str_hash hash to begin with
 * @str_length length to begin with
 * @str_k_chars characters to draw from
 *
 * @return constructed line_id
 */
line_id * make_line_id_given_string_with_size(unsigned long int str_hash,
                                              unsigned long int str_length,
                                              string_with_size * str_k_chars);
/* return a newly allocated clone of the given line_id
 * @base line_id to clone
 *
 * @return clone of given line_id
 */
line_id * clone_line_id_with_string_null(line_id * base);
/* free memory allocated by constructing a line_id
 * @sid line_id to free
 */
void free_line_id(void * arg);
/* test if two line_id objects are equal
 * @a pointer to one line_id
 * @b pointer to another line_id
 *
 * @return whether the two are equal
 */
bool line_id_equal(line_id * a, line_id * b);
/* cldoc:end-category() */

/* cldoc:begin-category(queue_processing) */

/* helper struct used in glib for_each loops */
typedef struct {
    /* arbitrary data to hold */
    void * data;
    /* boolean to represent something */
    bool boolean;
} boolean_and_data;

// modify LINES_ABOVE_BELOW_TO_SEARCH, not QUEUE_HASH_CRITICAL_SIZE
#define LINES_ABOVE_BELOW_TO_SEARCH 5
// size of queue wrt # of lines
#define QUEUE_HASH_CRITICAL_SIZE (2 * LINES_ABOVE_BELOW_TO_SEARCH + 1)

/* set boolean to true if line_ids in arguments match
 * @prev_line_id line_id of some line in prev file
 * @bool_data_bundle boolean_and_data containing some boolean and a line_id for
 * cur_file
 *
 * compiler will emit a non-inline version of this, since a pointer is taken to
 * it when g_queue_foreach is used
 */
void set_bool_if_line_id_match(line_id * prev_line_id,
                               boolean_and_data * bool_data_bundle);
/* return true if current line is in previous line queue
 * @prev_file_queue queue of line_ids of lines of prev file
 * @cur_file_queue queue of line_ids of lines of cur file
 *
 * @return whether the line_id at top of cur_file_queue matches any line_id in
 * prev_file_queue
 */
bool is_cur_line_in_prev_queue(GQueue * prev_file_queue,
                               GQueue * cur_file_queue);
#ifdef DEBUG
/* debugging function to print out first_k_chars of given line_id
 * @sid pointer to line_id to print out
 */
void print_line_id_first_k_chars(line_id * sid);
#endif

/* helper struct to carry a pair of line_ids around */
typedef struct {
    /* line_id from prev file */
    line_id * prev_id;
    /* line_id from cur file */
    line_id * cur_id;
} line_id_pair;
/* constructs a line_id_pair given two live line_ids
 * @prev line_id from prev file
 * @cur line_id from cur file
 *
 * @return constructed line_id_pair
 */
line_id_pair * make_line_id_pair(line_id * prev, line_id * cur);
/* frees a line_id_pair
 * @arg line_id_pair to deallocate memory for
 */
void free_line_id_pair(void * arg);

/* helper struct to carry line_id and edit matching info in glib for_each */
typedef struct {
    line_id * id;
    /* whether a string with a sufficiently close levenshtein distance exists */
    bool is_leven_found;
    /* list of all such strings */
    GSList ** edit_matches;
} line_id_with_edit_match_info;

/* create line_id_pair and add to list if line_ids are similar by levenshtein
 * @prev_line_id line_id from prev file
 * @cur_data line_id_with_edit_match_info containing line_id from cur file
 *
 * compiler will emit a non-inline version of this too, since a pointer is taken
 * to it when g_queue_foreach is used
 */
void if_close_levenshtein_dist_add_to_list(
  line_id * prev_line_id, line_id_with_edit_match_info * cur_data);
/* if line on top of cur_file_queue is close to a previous line, add it to list
 * @prev_file_queue queue of previous line_ids
 * @cur_file_queue queue of current line_ids (only top used)
 * @edit_matches list of line_id_pairs of lines which are similar to one another
 *
 * @return whether exists a close line, and if so which one
 */
boolean_and_data get_if_edit_line_and_if_so_add_to_list(
  GQueue * prev_file_queue, GQueue * cur_file_queue, GSList ** edit_matches);

/* takes in files and writes lines from one to another as appropriate
 * @prev_file_line_ids_queue queue of line_ids from prev file
 * @cur_file_line_ids_queue queue of line_ids from cur file
 * @current_streak_of_newly_added_lines current number of lines added; if too
 * many, stop the process altogether to avoid pathological inputs
 * @input_file_lines_processed_for_edits index into input file
 * @cur_file_lines_processed index into cur file
 * @output_file_lines_processed index into output file
 * @break_out_of_vcscmp whether too many new lines have been found in cur file:
 * if so, stop to avoid pathological inputs
 * @edit_matches list of similar line_id_pairs by levenshtein distance
 * @prev_file_used_for_edits FILE * of prev file
 * @cur_file_lines_processed FILE * of cur file
 * @out_file FILE * of output file
 *
 * CLOBBERS LINES_PROCESSED ARGUMENTS by setting them to their new appropriate
 * values
 */
void write_line_and_if_new_add_to_list(
  GQueue * prev_file_line_ids_queue,
  GQueue * cur_file_line_ids_queue,
  unsigned long long * current_streak_of_newly_added_lines,
  mpz_t * input_file_lines_processed_for_edits,
  mpz_t * cur_file_lines_processed,
  mpz_t * output_file_lines_processed,
  bool * break_out_of_vcscmp,
  GSList ** edit_matches,
  FILE * prev_file_used_for_edits,
  FILE * cur_file,
  FILE * out_file);

/* initializes line_id with given data
 * @ptr_hash pointer to str_hash value
 * @ptr_length pointer to str_length value
 * @sws pointer to pointer to string_with_size to initialize from
 * @ptr_past_k_chars whether the line in file is longer than a threshold
 */
void initialize_line_id(unsigned long int * ptr_hash,
                        unsigned long int * ptr_length,
                        string_with_size ** sws,
                        bool * ptr_past_k_chars);

/* if within treshold of line length, write to given string
 * @ptr_line_length current length of line
 * @sws_first_chars current string to be written to
 * @sws_block raw input string read from file
 * @ptr_index index into string
 */
void if_within_first_section_write_to_string(unsigned long int ptr_line_length,
                                             string_with_size * sws_first_chars,
                                             string_with_size * sws_block,
                                             unsigned long long ptr_index);

/* write character to string, and update a given hash and line length
 * @ptr_line_length pointer to current length of line
 * @sws_first_chars pointer to string_with_size containing first k chars of line
 * @sws_block raw input string read from file
 * @ptr_index current index into string
 * @instantaneous_hash hash at this point in string (using moving djb2 hash)
 * @hash_str string to hash with
 * @ptr_past_k_chars whether string is past a threshold length
 */
void write_string_and_update_hash_and_line_length(
  unsigned long int * ptr_line_length,
  string_with_size * sws_first_chars,
  string_with_size * sws_block,
  unsigned long long ptr_index,
  unsigned long int * instantaneous_hash,
  char * hash_str,
  bool * ptr_past_k_chars);

/* if past k chars, finish off initializing line_id
 * @ptr_past_k_chars whether past k chars
 * @ptr_line_length current length of line
 * @ids_queue queue of line_ids to push to
 * @ptr_line_hash current hash of line
 * @first_few_chars pointer to pointer to string_with_size with first k chars
 * @lines_processed current lines processed of file
 * @is_line_orf whether line is open reading frame
 *
 * OPTIMIZATION: string probably ok to allocate in loop, but perhaps not
 */
void check_if_past_k_chars_push_tail_and_initialize_line_id(
  bool * ptr_past_k_chars,
  unsigned long int * ptr_line_length,
  GQueue * ids_queue,
  unsigned long int * ptr_line_hash,
  string_with_size ** first_few_chars,
  mpz_t * lines_processed,
  bool is_line_orf);

/* helper function to ask whether line is an orf
 * @first_few_chars first chars of line
 *
 * requires that string be >= CODON_LENGTH chars, which needs to be TODO: fixed
 * requires that file be formatted correctly so that lines of orf and non orf
 * alternate, but vcsfmt already enforces that
 *
 * @return whether line begins with a start codon
 */
bool is_first_line_orf(string_with_size * first_few_chars);

/* update hash and string length, or push constructed line_id to queue
 * @input_block string_with_size to analyze
 * @block_index current index into block
 * @is_line_orf whether line is orf
 * @first_few_chars pointer to pointer to string_with_size containg first k
 * characters of line
 * @ptr_past_k_chars whether line is greater than k characters in length
 * @ptr_line_length current length of line
 * @ids_queue queue to push line_id to as required
 * @ptr_line_hash current hash of line
 */
void react_to_next_character_of_block(string_with_size * input_block,
                                      unsigned long long block_index,
                                      mpz_t * lines_processed,
                                      bool * is_line_orf,
                                      string_with_size ** first_few_chars,
                                      bool * ptr_past_k_chars,
                                      unsigned long int * ptr_line_length,
                                      GQueue * ids_queue,
                                      unsigned long int * ptr_line_hash);

/* read blocks from file and add as line_ids to a queue
 * @active_file file to read from
 * @ids_queue queue to push to
 * @input_block memory block to use for file I/O
 * @ptr_past_k_chars whether line is greater than k characters long
 * @ptr_line_length current length of line
 * @ptr_line_hash current hash of line
 * @first_few_chars pointer to pointer to string_with_size containing first k
 * characters of line
 * @lines_processed lines processed of active_file
 * @is_line_orf whether line is an orf
 */
void add_blocks_to_queue(FILE * active_file,
                         GQueue * ids_queue,
                         string_with_size * input_block,
                         bool * ptr_past_k_chars,
                         unsigned long int * ptr_line_length,
                         unsigned long int * ptr_line_hash,
                         string_with_size ** first_few_chars,
                         mpz_t * lines_processed,
                         bool * is_line_orf);

#ifdef DEBUG
/* helper function for debugging to print pairs of a line_id
 * @arg input line_id_pair
 */
void print_line_id_pair(line_id_pair * arg);
#endif

/* helper struct used for glib for_each*/
typedef struct {
    /* FILE * to prev file */
    FILE * prev_file;
    /* index into prev file */
    mpz_t * prev_file_index;
    /* FILE * to cur file */
    FILE * cur_file;
    /* index into cur file */
    mpz_t * cur_file_index;
    /* FILE * to output file */
    FILE * out_file;
} diff_file_trio_with_indices;

/* get optimal levenshtein distance string between current lines of two files
 * @prev_file FILE * to prev file at beginning of intended line
 * @cur_file_index FILE * to cur file at beginning of intended line
 *
 * @return string_with_size representing the two
 */
string_with_size * optimal_levenshtein_string_between_lines(FILE * prev_file,
                                                            FILE * cur_file);

/* send optimal levenshtein distance string_with_size to given file
 * @arg the line_id_pair to compare and get levenshtein distance string from
 * @files_and_indices metadata indicating where to get levenshtein distances
 *
 * since address being taken for use in for_each, compiler will also emit
 * non-inline version
 */
void print_levenshtein_smallest_edits_to_out_file(
  line_id_pair * arg, diff_file_trio_with_indices * files_and_indices);

/* compare two vcsfmt files and output to given file
 * @prev_filename name of prev file
 * @cur_filename name of cur file
 * @out_filename name of output file
 */
void vcscmp(const char * prev_filename,
            const char * cur_filename,
            const char * out_filename);

/* cldoc:end-category() */

#endif /*___VCS_CMP_H___*/
