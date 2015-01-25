#ifndef ___ARGPARSE_SETUP_H___
#define ___ARGPARSE_SETUP_H___

/*
    hosts all required variables and functions to implement argparse
*/

#include "utilities.h" // for GSList, bool
#include <argp.h>      // for argparse

// documentation

extern struct argp_option options[];

// used to communicate with parse_opt
struct dwndiff_arguments {
  GSList * files;
  bool is_zip;
  bool is_compare;
  int is_unzip;
  char * output_file_path;
  unsigned long long width;
  bool is_verbose;
  bool has_no_args;
};

// CTOR
struct dwndiff_arguments
 initialize_dwndiff_arguments(struct dwndiff_arguments args);

extern error_t parse_opt(int key, char * arg, struct argp_state * state);

extern char args_doc[];

extern char doc[];

extern struct argp argp;

#endif /*___ARGPARSE_SETUP_H___*/
