#include "vcscmp.h"

// OPTIMIZATION: inline everything
line_id * line_id_set_is_orf(line_id * sid, bool set_is_orf) {
  sid->is_orf = set_is_orf;
  return sid;
}
line_id * line_id_set_str_hash(line_id * sid, unsigned long int str_hash) {
  sid->str_hash = str_hash;
  return sid;
}
line_id * line_id_set_str_length(line_id * sid, unsigned long int str_length) {
  sid->str_length = str_length;
  return sid;
}
line_id * line_id_set_line_number(line_id * sid, mpz_t * number_to_set_to) {
  mpz_set(sid->line_number, *number_to_set_to);
  return sid;
}
line_id * line_id_initialize_line_number(line_id * sid) {
  mpz_init(sid->line_number); // sets to 0
  return sid;
}
line_id * line_id_set_first_k_chars(line_id * sid,
                                    string_with_size * str_k_chars) {
  sid->first_k_chars = str_k_chars;
  return sid;
}
line_id * make_line_id(unsigned long int str_hash,
                       unsigned long int str_length) {
  return line_id_set_first_k_chars(
   line_id_set_str_length(
    line_id_set_str_hash(
     line_id_initialize_line_number(malloc(sizeof(line_id))), str_hash),
    str_length),
   set_string_with_size_readable_bytes(
    make_new_string_with_size(LEVENSHTEIN_CHECK_CHARS),
    LEVENSHTEIN_CHECK_CHARS));
}
line_id * make_line_id_given_string_with_size(unsigned long int str_hash,
                                              unsigned long int str_length,
                                              string_with_size * str_k_chars) {
  return line_id_set_first_k_chars(
   line_id_set_str_length(
    line_id_set_str_hash(
     line_id_initialize_line_number(malloc(sizeof(line_id))), str_hash),
    str_length),
   str_k_chars);
}
line_id * clone_line_id_with_string_null(line_id * base) {
  line_id * ret = malloc(sizeof(line_id));
  mpz_init_set(ret->line_number, base->line_number);
  ret->str_hash = base->str_hash;
  ret->first_k_chars = NULL;
  ret->is_orf = base->is_orf;
  return ret;
}
void free_line_id(void * arg) {
  if (NULL != arg) {
    line_id * lid = (line_id *) arg;
    free_string_with_size(lid->first_k_chars);
    mpz_clear(lid->line_number);
    free(lid);
  }
}
bool line_id_equal(line_id * a, line_id * b) {
  return a->str_hash == b->str_hash && a->str_length == b->str_length;
}

void set_bool_if_line_id_match(line_id * prev_line_id,
                               boolean_and_data * bool_data_bundle) {
  if (line_id_equal(prev_line_id, bool_data_bundle->data)) {
    bool_data_bundle->boolean = true;
  }
}
// basically macros
bool is_cur_line_in_prev_queue(GQueue * prev_file_queue,
                               GQueue * cur_file_queue) {
  boolean_and_data bool_data_bundle;
  bool_data_bundle.data = g_queue_peek_head(cur_file_queue);
  bool_data_bundle.boolean = false;
  g_queue_foreach(prev_file_queue, (GFunc) set_bool_if_line_id_match,
                  &bool_data_bundle);
  return bool_data_bundle.boolean;
}
#ifdef DEBUG
void print_line_id_first_k_chars(line_id * sid) {
  fprintf(stderr, "%.*s",
          (int) sid->first_k_chars->readable_bytes, // int cast required
          sid->first_k_chars->string);
}
#endif

line_id_pair * make_line_id_pair(line_id * prev, line_id * cur) {
  line_id_pair * ret = malloc(sizeof(line_id_pair));
  ret->prev_id = prev;
  ret->cur_id = cur;
  return ret;
}
void free_line_id_pair(void * arg) {
  if (NULL != arg) {
    line_id_pair * lip = (line_id_pair *) arg;
    free_line_id(lip->prev_id);
    free_line_id(lip->cur_id);
    free(lip);
  }
}

void if_close_levenshtein_dist_add_to_list(
 line_id * prev_line_id, line_id_with_edit_match_info * cur_data) {
  if (prev_line_id->is_orf && cur_data->id->is_orf &&
      !cur_data->is_leven_found) { // don't do it if close match already found
// use && instead of == so short non-orfs don't match
#ifdef DEBUG
    PRINT_ERROR_NO_NEWLINE("prev_line (");
    PRINT_ERROR_MPZ_T_NO_NEWLINE(prev_line_id->line_number);
    PRINT_ERROR_NO_NEWLINE("): ");
    PRINT_ERROR_STRING_FIXED_LENGTH_NO_NEWLINE(
     prev_line_id->first_k_chars->string,
     prev_line_id->first_k_chars->readable_bytes);
    PRINT_ERROR_NO_NEWLINE("\ncur_line (");
    PRINT_ERROR_MPZ_T_NO_NEWLINE(cur_data->id->line_number);
    PRINT_ERROR_NO_NEWLINE("): ");
    PRINT_ERROR_STRING_FIXED_LENGTH_NO_NEWLINE(
     cur_data->id->first_k_chars->string,
     cur_data->id->first_k_chars->readable_bytes);
    PRINT_ERROR_NEWLINE();
#endif
    unsigned long long leven_dist = get_levenshtein_distance(
     prev_line_id->first_k_chars, cur_data->id->first_k_chars);
    // TODO: instead of absolute levenshtein distance, use proportional
    if (leven_dist < LEVENSHTEIN_CHECK_THRESHOLD) {
#ifdef DEBUG
      PRINT_ERROR("CLOSE STRING FOUND BY LEVENSHTEIN EDITS");
      PRINT_ERROR_NO_NEWLINE("LEVEN_DIST: ");
      PRINT_ERROR_UNSIGNED_LONG_LONG_NO_NEWLINE(leven_dist);
      PRINT_ERROR_NEWLINE();
      PRINT_ERROR_NO_NEWLINE("PREV_STRING (LINE ");
      PRINT_ERROR_MPZ_T_NO_NEWLINE(prev_line_id->line_number);
      PRINT_ERROR_NO_NEWLINE(") (CHARS ");
      PRINT_ERROR_UNSIGNED_LONG_LONG_NO_NEWLINE(
       prev_line_id->first_k_chars->readable_bytes);
      PRINT_ERROR_NO_NEWLINE(") (ORF ");
      if (prev_line_id->is_orf) {
        PRINT_ERROR_NO_NEWLINE("YES): ");
      } else {
        PRINT_ERROR_NO_NEWLINE("NO):  ");
      }
      print_line_id_first_k_chars(prev_line_id);
      PRINT_ERROR_NO_NEWLINE("\nCUR_STRING  (LINE ");
      PRINT_ERROR_MPZ_T_NO_NEWLINE(cur_data->id->line_number);
      PRINT_ERROR_NO_NEWLINE(") (CHARS ");
      PRINT_ERROR_UNSIGNED_LONG_LONG_NO_NEWLINE(
       cur_data->id->first_k_chars->readable_bytes);
      PRINT_ERROR_NO_NEWLINE(") (ORF ");
      if (cur_data->id->is_orf) {
        PRINT_ERROR_NO_NEWLINE("YES): ");
      } else {
        PRINT_ERROR_NO_NEWLINE("NO):  ");
      }
      print_line_id_first_k_chars(cur_data->id);
      PRINT_ERROR_NEWLINE();
      PRINT_ERROR("-----------");
#endif
      *cur_data->edit_matches = g_slist_prepend( // adds in reverse order!!!
       *cur_data->edit_matches,
       make_line_id_pair(clone_line_id_with_string_null(prev_line_id),
                         clone_line_id_with_string_null(cur_data->id)));
      cur_data->is_leven_found = true;
      cur_data->id = prev_line_id;
    }
  }
}

boolean_and_data get_if_edit_line_and_if_so_add_to_list(
 GQueue * prev_file_queue, GQueue * cur_file_queue, GSList ** edit_matches) {
  line_id_with_edit_match_info liam;
  liam.id = g_queue_peek_head(cur_file_queue);
  liam.is_leven_found = false;
  liam.edit_matches = edit_matches;
  g_queue_foreach(prev_file_queue,
                  (GFunc) if_close_levenshtein_dist_add_to_list, &liam);
  boolean_and_data return_val;
  return_val.boolean = liam.is_leven_found;
  return_val.data = liam.id;
  return return_val;
}

#ifdef DEBUG
void print_line_id_pair(line_id_pair * arg) {
  PRINT_ERROR("EDITS:");
  PRINT_ERROR_NO_NEWLINE("prev=");
  PRINT_ERROR_MPZ_T_NO_NEWLINE(arg->prev_id->line_number);
  PRINT_ERROR_NO_NEWLINE(",cur=");
  PRINT_ERROR_MPZ_T_NO_NEWLINE(arg->cur_id->line_number);
  PRINT_ERROR_NEWLINE();
}
#endif

// CLOBBERS LINES_PROCESSED ARGUMENTS
// i.e. sets them to their new appropriate values
void write_line_and_if_new_add_to_list(
 GQueue * prev_file_line_ids_queue, GQueue * cur_file_line_ids_queue,
 unsigned long long * current_streak_of_newly_added_lines,
 mpz_t * input_file_lines_processed_for_edits, mpz_t * cur_file_lines_processed,
 mpz_t * output_file_lines_processed, bool * break_out_of_vcscmp,
 GSList ** edit_matches, FILE * prev_file_used_for_edits, FILE * cur_file,
 FILE * out_file) {
#ifdef DEBUG
  static unsigned long long count = 0;
  PRINT_ERROR_NO_NEWLINE("WRITE_LINE_COUNT: ");
  PRINT_ERROR_UNSIGNED_LONG_LONG_NO_NEWLINE(count);
  PRINT_ERROR_NEWLINE();
  ++count;
  if (*break_out_of_vcscmp) {
    PRINT_ERROR("F");
  } else {
    PRINT_ERROR("G");
  }
#endif
  // NOTE: break_out_of_vcscmp is passed by pointer, so if you do not
  // dereference it, it will return TRUE, since obviously a pointer value != 0
  // this screwed me over a bit so be careful
  if (!is_cur_line_in_prev_queue(prev_file_line_ids_queue,
                                 cur_file_line_ids_queue) &&
      !*break_out_of_vcscmp) {
#ifdef DEBUG
    PRINT_ERROR_NO_NEWLINE("NEWLY ADDED LINE AT LINE ");
    PRINT_ERROR_MPZ_T_NO_NEWLINE(*output_file_lines_processed);
    PRINT_ERROR_NO_NEWLINE(" (CUR: ");
    if (!((line_id *) g_queue_peek_head(cur_file_line_ids_queue))->is_orf) {
      PRINT_ERROR_NO_NEWLINE("NO ");
    }
    PRINT_ERROR_NO_NEWLINE("ORF)");
    PRINT_ERROR_NEWLINE();
    PRINT_ERROR_NO_NEWLINE("PREV_QUEUE SIZE: ");
    PRINT_ERROR_UNSIGNED_LONG_LONG_NO_NEWLINE(
     (unsigned long long) g_queue_get_length(prev_file_line_ids_queue));
    PRINT_ERROR_NEWLINE();
    PRINT_ERROR_NO_NEWLINE("CUR_QUEUE SIZE: ");
    PRINT_ERROR_UNSIGNED_LONG_LONG_NO_NEWLINE(
     (unsigned long long) g_queue_get_length(cur_file_line_ids_queue));
    PRINT_ERROR_NEWLINE();
#endif
    // adds to edit_matches list
    boolean_and_data is_edit_and_line_id_if_so =
     get_if_edit_line_and_if_so_add_to_list(
      prev_file_line_ids_queue, cur_file_line_ids_queue, edit_matches);
    if (is_edit_and_line_id_if_so.boolean) { // if current line is edit line
#ifdef DEBUG
      PRINT_ERROR("SNOOP DOGG");
#endif
      write_line_number_from_file_to_file(
       input_file_lines_processed_for_edits,
       &((line_id *) is_edit_and_line_id_if_so.data)->line_number,
       prev_file_used_for_edits, out_file);
    } else { // if just new line
#ifdef DEBUG
      PRINT_ERROR("JOHN TRAVOLTA");
#endif
      write_current_line_of_file_incf_index(cur_file_lines_processed, cur_file,
                                            out_file);
      ++*current_streak_of_newly_added_lines;
      if (*current_streak_of_newly_added_lines > QUEUE_HASH_CRITICAL_SIZE) {
        *break_out_of_vcscmp = true;
      }
    }
  } else { // just write from cur file to out file
// OPTIMIZATION: if break_out_of_vcscmp, just do straight block I/O
// instead of line-by-line like this
#ifdef DEBUG
    PRINT_ERROR("ACE VENTURA");
#endif
    write_line_number_from_file_to_file(
     cur_file_lines_processed,
     &((line_id *) g_queue_peek_head(cur_file_line_ids_queue))->line_number,
     cur_file, out_file);
  }
  increment_mpz_t(output_file_lines_processed);
}

void initialize_line_id(unsigned long int * ptr_hash,
                        unsigned long int * ptr_length, string_with_size ** sws,
                        bool * ptr_past_k_chars) {
  *ptr_hash = DJB2_HASH_BEGIN;
  *ptr_length = 0;
  *sws = make_new_string_with_size(LEVENSHTEIN_CHECK_CHARS);
  *ptr_past_k_chars = false;
}

void if_within_first_section_write_to_string(unsigned long int ptr_line_length,
                                             string_with_size * sws_first_chars,
                                             string_with_size * sws_block,
                                             unsigned long long ptr_index) {
  if (ptr_line_length < LEVENSHTEIN_CHECK_CHARS) {
    sws_first_chars->string[ ptr_line_length ] = sws_block->string[ ptr_index ];
  }
}

void write_string_and_update_hash_and_line_length(
 unsigned long int * ptr_line_length, string_with_size * sws_first_chars,
 string_with_size * sws_block, unsigned long long ptr_index,
 unsigned long int * instantaneous_hash, char * hash_str,
 bool * ptr_past_k_chars) {
  if_within_first_section_write_to_string(*ptr_line_length, sws_first_chars,
                                          sws_block, ptr_index);
  *instantaneous_hash =
   djb2_hash_on_string_index(*instantaneous_hash, hash_str, ptr_index);
  ++*ptr_line_length;
  if (*ptr_line_length >= LEVENSHTEIN_CHECK_CHARS && !*ptr_past_k_chars) {
    *ptr_past_k_chars = true;
  }
}

// OPTIMIZATION: string probably ok to allocate in loop, but perhaps not
void check_if_past_k_chars_push_tail_and_initialize_line_id(
 bool * ptr_past_k_chars, unsigned long int * ptr_line_length,
 GQueue * ids_queue, unsigned long int * ptr_line_hash,
 string_with_size ** first_few_chars, mpz_t * lines_processed,
 bool is_line_orf) {
  if (*ptr_past_k_chars) {
    *ptr_line_length = LEVENSHTEIN_CHECK_CHARS;
  }
  g_queue_push_tail(
   ids_queue, line_id_set_is_orf(
               line_id_set_line_number(make_line_id_given_string_with_size(
                                        *ptr_line_hash, *ptr_line_length,
                                        set_string_with_size_readable_bytes(
                                         *first_few_chars, *ptr_line_length)),
                                       lines_processed),
               is_line_orf));
  initialize_line_id(ptr_line_hash, ptr_line_length, first_few_chars,
                     ptr_past_k_chars);
}

// TODO: "ironed out" as below
// requires that string be >= CODON_LENGTH chars, which needs to be ironed out
// requires that file be formatted correctly so that lines of orf and non orf
// alternate, but vcsfmt already enforces that
bool is_first_line_orf(string_with_size * first_few_chars) {
  return is_start_codon(first_few_chars->string);
}

void react_to_next_character_of_block(
 string_with_size * input_block, unsigned long long block_index,
 mpz_t * lines_processed, bool * is_line_orf,
 string_with_size ** first_few_chars, bool * ptr_past_k_chars,
 unsigned long int * ptr_line_length, GQueue * ids_queue,
 unsigned long int * ptr_line_hash) {
  if (NEWLINE == input_block->string[ block_index ]) {
    if (mpz_cmp_ui(*lines_processed, 1)) { // if first line
      *is_line_orf = is_first_line_orf(*first_few_chars);
    }
    check_if_past_k_chars_push_tail_and_initialize_line_id(
     ptr_past_k_chars, ptr_line_length, ids_queue, ptr_line_hash,
     first_few_chars, lines_processed, *is_line_orf);
    increment_mpz_t(lines_processed);
    *is_line_orf = !*is_line_orf; // flip it
  } else {
    write_string_and_update_hash_and_line_length(
     ptr_line_length, *first_few_chars, input_block, block_index, ptr_line_hash,
     input_block->string, ptr_past_k_chars);
  }
}

void add_blocks_to_queue(FILE * active_file, GQueue * ids_queue,
                         string_with_size * input_block,
                         bool * ptr_past_k_chars,
                         unsigned long int * ptr_line_length,
                         unsigned long int * ptr_line_hash,
                         string_with_size ** first_few_chars,
                         mpz_t * lines_processed, bool * is_line_orf) {
  if (!(feof(active_file) || ferror(active_file)) &&
      g_queue_get_length(ids_queue) < QUEUE_HASH_CRITICAL_SIZE) {
    read_block(active_file, input_block);
    for (unsigned long long block_index = 0;
         block_index < input_block->readable_bytes; ++block_index) {
      react_to_next_character_of_block(
       input_block, block_index, lines_processed, is_line_orf, first_few_chars,
       ptr_past_k_chars, ptr_line_length, ids_queue, ptr_line_hash);
    }
    if (feof(active_file) || ferror(active_file)) { // if at end of file
      check_if_past_k_chars_push_tail_and_initialize_line_id(
       ptr_past_k_chars, ptr_line_length, ids_queue, ptr_line_hash,
       first_few_chars, lines_processed, *is_line_orf);
    }
  }
}

string_with_size * optimal_levenshtein_string_between_lines(FILE * prev_file,
                                                            FILE * cur_file) {
  return format_and_free_levenshtein_list_to_string_with_size(
   get_levenshtein_edits_and_free(get_current_line_of_file(prev_file),
                                  get_current_line_of_file(cur_file)));
}

void print_levenshtein_smallest_edits_to_out_file(
 line_id_pair * arg, diff_file_trio_with_indices * files_and_indices) {
  advance_file_to_line(files_and_indices->prev_file,
                       files_and_indices->prev_file_index,
                       &arg->prev_id->line_number, BIN_BLOCK_SIZE);
  advance_file_to_line(files_and_indices->cur_file,
                       files_and_indices->cur_file_index,
                       &arg->cur_id->line_number, BIN_BLOCK_SIZE);
  write_block(files_and_indices->out_file,
              optimal_levenshtein_string_between_lines(
               files_and_indices->prev_file, files_and_indices->cur_file));
  fputc('\n', files_and_indices->out_file); // delimit by newline
#ifdef DEBUG
  write_block(stderr,
              optimal_levenshtein_string_between_lines(
               files_and_indices->prev_file, files_and_indices->cur_file));
  fputc('\n', stderr);
#endif
}

void vcscmp(const char * prev_filename, const char * cur_filename,
            const char * out_filename) {
  FILE * prev_file = open_file_read(prev_filename);
  PRINT_ERROR_AND_RETURN_IF_NULL(prev_file, "Error in reading prev file.");
  FILE * cur_file = open_file_read(cur_filename);
  PRINT_ERROR_AND_RETURN_IF_NULL(cur_file, "Error in reading cur file.");
  FILE * out_file = create_file_binary_write(out_filename);
  PRINT_ERROR_AND_RETURN_IF_NULL(out_file, "Error in creating output file.");
  FILE * prev_file_used_for_edits = open_file_read(prev_filename);
  PRINT_ERROR_AND_RETURN_IF_NULL(prev_file_used_for_edits,
                                 "Error in reading prev file.");
  FILE * cur_file_used_for_edits = open_file_read(cur_filename);
  PRINT_ERROR_AND_RETURN_IF_NULL(cur_file_used_for_edits,
                                 "Error in reading cur file.");
#ifdef CONCURRENT
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-W#warnings"
#warning FUNCTIONALITY NOT IMPLEMENTED YET
#pragma GCC diagnostic pop
#else
#error NON-GCC/CLANG COMPILERS NOT YET SUPPORTED
#endif
#else
  // OPTIMIZATION:
  // implement fixed-size array-based queue for speed
  GQueue * prev_file_line_ids_queue = g_queue_new();
  GQueue * cur_file_line_ids_queue = g_queue_new();

  string_with_size * prev_block = make_new_string_with_size(BIN_BLOCK_SIZE);
  string_with_size * cur_block = make_new_string_with_size(BIN_BLOCK_SIZE);

  string_with_size * prev_first_few_chars =
   make_new_string_with_size(LEVENSHTEIN_CHECK_CHARS);
  string_with_size * cur_first_few_chars =
   make_new_string_with_size(LEVENSHTEIN_CHECK_CHARS);

  unsigned long int prev_file_line_hash = DJB2_HASH_BEGIN;
  unsigned long int prev_file_line_length = 0;
  unsigned long int cur_file_line_hash = DJB2_HASH_BEGIN;
  unsigned long int cur_file_line_length = 0;
  bool prev_length_past_k_chars = false;
  bool cur_length_past_k_chars = false;

  mpz_t prev_lines_processed;
  mpz_init_set_ui(prev_lines_processed, 1);
  mpz_t cur_lines_processed;
  mpz_init_set_ui(cur_lines_processed, 1);
  mpz_t output_lines_processed;
  mpz_init_set_ui(output_lines_processed, 1);
  mpz_t prev_file_lines_processed_for_edits;
  mpz_init_set_ui(prev_file_lines_processed_for_edits, 1);
  mpz_t cur_file_lines_processed_for_edits;
  mpz_init_set_ui(cur_file_lines_processed_for_edits, 1);

  bool prev_is_line_orf; // switches every line
  bool cur_is_line_orf;

  unsigned long long current_streak_of_newly_added_lines = 0;
  bool break_out_of_vcscmp = false;

  GSList * edit_matches = NULL; // list of lines counted as edits from a
                                // line in the previous file

  // TODO: count number of new lines added, and number of edited lines

  while ((!(feof(prev_file) || ferror(prev_file)) || // until both files EOF
          !(feof(cur_file) || ferror(cur_file))) &&
         !break_out_of_vcscmp) {
    add_blocks_to_queue(prev_file, prev_file_line_ids_queue, prev_block,
                        &prev_length_past_k_chars, &prev_file_line_length,
                        &prev_file_line_hash, &prev_first_few_chars,
                        &prev_lines_processed, &prev_is_line_orf);
    add_blocks_to_queue(cur_file, cur_file_line_ids_queue, cur_block,
                        &cur_length_past_k_chars, &cur_file_line_length,
                        &cur_file_line_hash, &cur_first_few_chars,
                        &cur_lines_processed, &cur_is_line_orf);
    if (g_queue_get_length(prev_file_line_ids_queue) >=
         QUEUE_HASH_CRITICAL_SIZE &&
        g_queue_get_length(cur_file_line_ids_queue) >=
         QUEUE_HASH_CRITICAL_SIZE) {
      if (mpz_cmp_ui(output_lines_processed, LINES_ABOVE_BELOW_TO_SEARCH) < 0) {
        while (mpz_cmp_ui(output_lines_processed, LINES_ABOVE_BELOW_TO_SEARCH) <
                0 &&
               !break_out_of_vcscmp) {
          write_line_and_if_new_add_to_list(
           prev_file_line_ids_queue, cur_file_line_ids_queue,
           &current_streak_of_newly_added_lines,
           &prev_file_lines_processed_for_edits,
           &cur_file_lines_processed_for_edits, &output_lines_processed,
           &break_out_of_vcscmp, &edit_matches, prev_file_used_for_edits,
           cur_file_used_for_edits, out_file);
          free_line_id(g_queue_pop_head(cur_file_line_ids_queue));
        }
      } else {
        write_line_and_if_new_add_to_list(
         prev_file_line_ids_queue, cur_file_line_ids_queue,
         &current_streak_of_newly_added_lines,
         &prev_file_lines_processed_for_edits,
         &cur_file_lines_processed_for_edits, &output_lines_processed,
         &break_out_of_vcscmp, &edit_matches, prev_file_used_for_edits,
         cur_file_used_for_edits, out_file);
        free_line_id(g_queue_pop_head(prev_file_line_ids_queue));
        free_line_id(g_queue_pop_head(cur_file_line_ids_queue));
      }
    }
  }
  // finish off remainder
  while (!g_queue_is_empty(cur_file_line_ids_queue) && !break_out_of_vcscmp) {
    write_line_and_if_new_add_to_list(
     prev_file_line_ids_queue, cur_file_line_ids_queue,
     &current_streak_of_newly_added_lines, &prev_file_lines_processed_for_edits,
     &cur_file_lines_processed_for_edits, &output_lines_processed,
     &break_out_of_vcscmp, &edit_matches, prev_file_used_for_edits,
     cur_file_used_for_edits, out_file);
    free_line_id(g_queue_pop_head(cur_file_line_ids_queue));
  }
  edit_matches = g_slist_reverse(edit_matches); // prepend-reverse idiom used
#ifdef DEBUG
  g_slist_foreach(edit_matches, (GFunc) print_line_id_pair, NULL);
#endif
  diff_file_trio_with_indices dftwi;
  dftwi.prev_file = prev_file;
  dftwi.prev_file_index = &prev_lines_processed;
  dftwi.cur_file = cur_file;
  dftwi.cur_file_index = &cur_lines_processed;
  dftwi.out_file = out_file;
  fputc('\n', out_file); // delimit by newline
  g_slist_foreach(edit_matches,
                  (GFunc) print_levenshtein_smallest_edits_to_out_file, &dftwi);
#ifdef DEBUG
  if (NULL == edit_matches) {
    PRINT_ERROR("LIST OF CLOSE MATCHES EMPTY");
  }
#endif
  // free memory and close open handles
  // TODO: fix weird failure to free gmps and string_with_sizes
  free_string_with_size(prev_block);
  free_string_with_size(cur_block);
  g_slist_free_full(edit_matches, free_line_id_pair);
  g_queue_free_full(prev_file_line_ids_queue, free_line_id);
  g_queue_free_full(cur_file_line_ids_queue, free_line_id);
#endif
}
