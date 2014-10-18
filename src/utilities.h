#ifndef ___UTILITIES_H___
#define ___UTILITIES_H___

// description
/*
    preprocessor macros, system headers, and other generic additions to make
    extending this program easier by cataloging all nonstandard utilities in
    one spot
*/

// searchable comment syntax used throughout project
/*
    IFFY: could cause errors, check if weird things are happening
    TODO: do this now
    OPTIMIZATION: if speed needed in this function, check here first
*/

/* cldoc:begin-category(utilities) */
// HEADERS AND LIBRARIES

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h> // for malloc
#include <string.h> // for strlen/strcpy/strcat
#include <glib.h>   // for queues, lists, and threads
#include <gmp.h>    // for bignums

// PORTABILITY MACROS
// IFFY: this could cause issues with processing files not made on linux systems
#define NEWLINE '\n'

// MACROS
// extra parens used around all macro arguments for evaluation before calling

#define PRINT_ERROR_NO_NEWLINE(str) fprintf(stderr, "%s", (str))

#define PRINT_ERROR(str) fprintf(stderr, "%s\n", (str))

#define PRINT_ERROR_NEWLINE() fprintf(stderr, "\n")

#define PRINT_ERROR_SIZE_T_NO_NEWLINE(num) fprintf(stderr, "%zu", (num))

#define PRINT_ERROR_UNSIGNED_LONG_LONG_NO_NEWLINE(num) \
    fprintf(stderr, "%llu", (num))

#define PRINT_ERROR_MPZ_T_NO_NEWLINE(bignum) mpz_out_str(stderr, 10, (bignum))

#define PRINT_ERROR_AND_RETURN_IF_NULL(ptr, str) \
    if (NULL == (ptr)) {                         \
        PRINT_ERROR((str));                      \
        return;                                  \
    }

#define PRINT_ERROR_DIVIDER_LINE() fprintf(stderr, "%s\n", "------------");

#define PRINT_ERROR_STRING_FIXED_LENGTH_NO_NEWLINE(str, len) \
    fprintf(stderr, "%.*s", (int) len, str)

// x is vertical (downwards), y is horizontal (rightwards)
// parentheses required because of distributivity
#define TWO_D_ARRAY_INDEX(arr, x, y, max_y) (arr)[ (x) * (max_y) + (y) ]

/* cldoc:begin-category(utilities::file functions) */

/* creates a new FILE * by opening a file with the given name for reading
 * @filename name of file to read from
 *
 * @return FILE * to intended file
 */
FILE * open_file_read(const char * filename);
/* creates a new FILE * by opening a file with the given name for writing
 * @filename name of file to write to
 *
 * Note: truncates file if already exists
 *
 * @return FILE * to intended file
 */
FILE * create_file_binary_write(const char * filename);
/* cldoc:end-category() */

/* cldoc:begin-category(utilities::string_with_size) */
/* used to return a char string, along with size information
 * readable_bytes is used by functions like fread because while
 * they will typically fill the entire memory space sometimes they do less, upon
 * reaching EOF or some other ferror
 */
typedef struct {
    /* pointer to character string, NOT null-terminated! */
    char * string;
    /* current number of useful bytes this is storing */
    unsigned long long readable_bytes;
    /* full size of char * in bytes */
    unsigned long long size_in_memory;
} string_with_size;

/* constructs new string_with_size of given size_in_memory
 * @size_in_memory size of string_with_size to create
 *
 * sets readable_bytes to 0
 *
 * @return constructed string_with_size
 */
string_with_size * make_new_string_with_size(unsigned long long size_in_memory);
#ifdef DEBUG
/* helper function to make string_with_size from null-terminated str
 * @null_term_str null-terminated string to copy from
 *
 * note: does not copy over null character at end!
 *
 * @return constructed string_with_size
 */
string_with_size * make_new_string_with_size_given_string(char * null_term_str);
#endif
/* helper function to set readable_bytes of a string_with_size
 * @sws string_with_size to set
 * @readable_bytes logical (not memory) size to set to
 *
 * @return modified string_with_size
 */
string_with_size *
  set_string_with_size_readable_bytes(string_with_size * sws,
                                      unsigned long long readable_bytes);
/* copies data from a string_with_size into another
 * @from_sws string_with_size to copy from
 * @to_sws string_with_size to copy to
 *
 * note: to_sws->size_in_memory > from_sws->readable_bytes, or this will
 * segfault
 *
 * @return copied string_with_size
 */
string_with_size * copy_string_with_size(string_with_size * from_sws,
                                         string_with_size * to_sws);
/* increases size_in_memory of string_with_size if neccesary, copying over data
 * @sws pointer to string_with_size to grow
 * @final_size_in_mem final size to grow to
 *
 * note: does nothing if growth is not required
 *
 * @return possibly-grown string_with_size
 */
string_with_size * grow_string_with_size(string_with_size ** sws,
                                         unsigned long long final_size_in_mem);
/* deallocates memory used by a string_with_size
 * @arg string_with_size to free
 */
void free_string_with_size(void * arg);
/* cldoc:end-category() */
/* cldoc:end-category() */

#endif /*___UTILITIES_H___*/
