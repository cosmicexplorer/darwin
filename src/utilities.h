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
#include <string.h> // for strdup

// PORTABILITY MACROS
// IFFY: this could cause issues with processing files not made on linux systems
#define NEWLINE '\n'

// DEBUGGING MACROS

#define PRINT_ERROR(str) fprintf(stderr, "%s\n", (str))

#define PRINT_ERROR_NO_NEWLINE(str) fprintf(stderr, "%s", (str))

#define PRINT_ERROR_NEWLINE() fprintf(stderr, "\n")

#define PRINT_ERROR_SIZE_T_NO_NEWLINE(num) fprintf(stderr, "%zu", (num))

#define PRINT_ERROR_UNSIGNED_LONG_LONG_NO_NEWLINE(num) \
  fprintf(stderr, "%llu", (num))

#define PRINT_ERROR_MPZ_T_NO_NEWLINE(bignum) mpz_out_str(stderr, 10, (bignum))

#define PRINT_ERROR_AND_RETURN(str) \
  PRINT_ERROR(str);                 \
  return;

#define PRINT_ERROR_AND_RETURN_IF_NULL(ptr, str) \
  if (NULL == (ptr)) {                           \
    PRINT_ERROR((str));                          \
    return;                                      \
  }

#define PRINT_ERROR_DIVIDER_LINE() fprintf(stderr, "%s\n", "------------");

#define PRINT_ERROR_STRING_FIXED_LENGTH_NO_NEWLINE(str, len) \
  fprintf(stderr, "%.*s", (int) len, str)

// CONVENIENCE MACROS
// x is vertical (downwards), y is horizontal (rightwards)
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
#ifndef MEMPOOL
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
#endif

#ifdef MEMPOOL
// TODO: javadoc

#define STARTING_NUM_SWS_IN_POOL 20 // this can be tuned

#define INPUT_BLOCK_MEMPOOL_INDEX_VCSFMT 0
#define OUTPUT_BLOCK_MEMPOOL_INDEX_VCSFMT 1

#define INPUT_BLOCK_MEMPOOL_INDEX_DE_VCSFMT 0
#define OUTPUT_BLOCK_MEMPOOL_INDEX_DE_VCSFMT 1

// TODO: use this so that mem pools don't become overly large
#define MAX_MEMPOOL_LENGTH 2000

typedef struct {
  /* pointer to character string, NOT null-terminated! */
  char * string;
  /* current number of useful bytes this is storing */
  unsigned long long readable_bytes;
  /* full size of char * in bytes */
  unsigned long long size_in_memory;
  /* index of memory pool this belongs to */
  size_t mempool_index;
} string_with_size;

typedef struct {
  GQueue ** mempools;
  size_t num_sws_mempools;
  /* get the size_in_memory of the string_with_sizes in each pool */
  unsigned long long * mempool_size_in_mem;
} string_with_size_pools;

extern string_with_size_pools sws_mempools;
extern GMutex modify_sws_mempools_mutex;

void add_string_with_size_pool(unsigned long long size_in_mem,
                               size_t num_elems);

string_with_size *
 make_new_string_with_size_given_index(unsigned long long size_in_mem,
                                       size_t mempool_index);

string_with_size * make_new_string_with_size_from_pool(size_t mempool_index);

void free_string_with_size_to_pool(void * arg);

void free_string_with_size_mem_pools(void);

#endif

/* constructs new string_with_size of given size_in_memory
 * @size_in_memory size of string_with_size to create
 *
 * sets readable_bytes to 0
 *
 * @return constructed string_with_size
 */
string_with_size * make_new_string_with_size(unsigned long long size_in_memory);

#ifdef DEBUG
#ifndef MEMPOOL
/* helper function to make string_with_size from null-terminated str
 * @null_term_str null-terminated string to copy from
 *
 * note: does not copy over null character at end!
 *
 * @return constructed string_with_size
 */
string_with_size * make_new_string_with_size_given_string(char * null_term_str);
#endif
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
