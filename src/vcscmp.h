#ifndef ___VCS_CMP_H___
#define ___VCS_CMP_H___

/*
    functions and utilities for implementing character-by-character
    comparison on .vcsfmt-formatted files
*/

#include "block_processing.h"

// TODO: allow modification of this, find reasoning for choice of value
#define LEVENSHTEIN_CHECK_CHARS 80
#define LEVENSHTEIN_CHECK_THRESHOLD 20

// STRING_ID
// TODO: javadoc
typedef struct {
    unsigned long str_hash;   // used because canonical djb2 uses unsigned long
    unsigned long str_length; // arbitrary choice of width
    string_with_size * first_k_chars;
} string_id;
static inline string_id * string_id_set_str_hash(string_id * sid,
                                                 unsigned long str_hash) {
    sid->str_hash = str_hash;
    return sid;
}
static inline string_id * string_id_set_str_length(string_id * sid,
                                                   unsigned long str_length) {
    sid->str_length = str_length;
    return sid;
}
static inline string_id *
  string_id_set_first_k_chars(string_id * sid, string_with_size * str_k_chars) {
    sid->first_k_chars = str_k_chars;
    return sid;
}
static inline string_id * make_string_id(unsigned long str_hash,
                                         unsigned long str_length) {
    return string_id_set_first_k_chars(
      string_id_set_str_length(
        string_id_set_str_hash(malloc(sizeof(string_id)), str_hash),
        str_length),
      set_string_with_size_readable_bytes(
        make_new_string_with_size(LEVENSHTEIN_CHECK_CHARS),
        LEVENSHTEIN_CHECK_CHARS));
}
static inline string_id *
  make_string_id_given_string_with_size(unsigned long str_hash,
                                        unsigned long str_length,
                                        string_with_size * str_k_chars) {
    return string_id_set_first_k_chars(
      string_id_set_str_length(
        string_id_set_str_hash(malloc(sizeof(string_id)), str_hash),
        str_length),
      str_k_chars);
}
static inline void free_string_id(string_id * sid) {
    free_string_with_size(sid->first_k_chars);
    free(sid);
}
static inline bool string_id_equal(string_id * a, string_id * b) {
    return a->str_hash == b->str_hash && a->str_length == b->str_length;
}

// QUEUE_PROCESSING
/**
 * @brief:
 *
 */
// TODO: rename struct so intended usage is clear, mention foreach loop
typedef struct {
    void * data;
    bool * boolean;
} boolean_and_data;

// TODO: javadoc all of this
// modify LINES_ABOVE_BELOW_TO_SEARCH, not QUEUE_HASH_CRITICAL_SIZE
#define LINES_ABOVE_BELOW_TO_SEARCH 5
// size of queue wrt # of lines
#define QUEUE_HASH_CRITICAL_SIZE 2 * LINES_ABOVE_BELOW_TO_SEARCH + 1
void set_bool_if_string_id_match(string_id * prev_string_id,
                                 boolean_and_data * bool_data_bundle);
// basically macros
static inline bool is_string_id_at_top_in_prev_queue(GQueue * prev_file_queue,
                                                     GQueue * cur_file_queue) {
    bool is_string_id_found = false;
    boolean_and_data bool_data_bundle;
    bool_data_bundle.data = g_queue_peek_head(cur_file_queue);
    bool_data_bundle.boolean = &is_string_id_found;
    g_queue_foreach(
      prev_file_queue, (GFunc) set_bool_if_string_id_match, &bool_data_bundle);
    return is_string_id_found;
}
void if_close_levenshtein_dist_add_to_list(string_id * prev_string_id,
                                           string_id * cur_string_id);
// basically  macros
static inline void
  if_similar_edit_levenshtein_dist_queue_add_to_list(GQueue * prev_file_queue,
                                                     GQueue * cur_file_queue) {
    g_queue_foreach(prev_file_queue,
                    (GFunc) if_close_levenshtein_dist_add_to_list,
                    g_queue_peek_head(cur_file_queue));
}
static inline void
  if_new_line_then_add_to_list(GQueue * prev_file_string_ids_queue,
                               GQueue * cur_file_string_ids_queue,
                               size_t * ptr_current_streak_of_newly_added_lines,
                               mpz_t * ptr_lines_processed,
                               bool * ptr_break_out_of_vcscmp) {
    if (!is_string_id_at_top_in_prev_queue(prev_file_string_ids_queue,
                                           cur_file_string_ids_queue)) {
        ++*ptr_current_streak_of_newly_added_lines;
#ifdef DEBUG
        fprintf(stderr, "NEWLY ADDED LINE AT LINE ");
        mpz_out_str(stderr, 10, *ptr_lines_processed);
        fprintf(stderr, "\n");
#else
#error FUNCTIONALITY NOT IMPLEMENTED YET
#endif
        if_similar_edit_levenshtein_dist_queue_add_to_list(
          prev_file_string_ids_queue, cur_file_string_ids_queue);
    }
    if (*ptr_current_streak_of_newly_added_lines > QUEUE_HASH_CRITICAL_SIZE) {
        *ptr_break_out_of_vcscmp = true;
    }
}

static inline void initialize_string_id(unsigned long * ptr_hash,
                                        unsigned long * ptr_length,
                                        string_with_size ** sws) {
    *ptr_hash = DJB2_HASH_BEGIN;
    *ptr_length = 0;
    *sws = make_new_string_with_size(LEVENSHTEIN_CHECK_CHARS);
}

static inline void if_within_first_section_write_to_string(
  unsigned long * ptr_instantaneous_length,
  string_with_size * sws_first_chars,
  string_with_size * sws_block,
  size_t * ptr_index) {
    if (*ptr_instantaneous_length < LEVENSHTEIN_CHECK_CHARS) {
        sws_first_chars->string[*ptr_instantaneous_length] =
          sws_block->string[*ptr_index];
    }
}

void vcscmp(char * prev_filename, char * cur_filename);

#endif /*___VCS_CMP_H___*/
