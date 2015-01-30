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

/* DEFINITIONS */

// required from argp.h
const char * argp_program_version = "darwin 0.1";
const char * argp_program_bug_address = "<danieldmcclanahan@gmail.com>";

struct argp_option options[] = {
 {"unzip", 'u', 0, 0, "Recover original file from vcsfmt or vcscmp file", 0},
 {"zip", 'z', 0, 0, "Produce vcsfmt file from original file.", 1},
 {"compare", 'c', 0, 0, "Produce vcscmp file from vcsfmt files.", 2},
 {"output", 'o', "DIR", 0, "Location to produce output files.", 3},
 {"width", 'w', "INTEGER", 0, "Width of FASTA file, as applicable.", 4},
 // TODO: make this option produce more information
 {"verbose", 'v', 0, 0, "Produce verbose output", 5},
 {0, 0, 0, 0, 0, 0}};

struct dwndiff_arguments
 initialize_dwndiff_arguments(struct dwndiff_arguments args) {
  args.files = NULL;
  args.is_zip = false;
  args.is_compare = false;
  args.is_unzip = false;
  args.width = 0;
  args.output_file_path = NULL;
  args.is_verbose = false;
  args.has_no_args = false;
  return args;
}

error_t parse_opt(int key, char * arg, struct argp_state * state) {
  struct dwndiff_arguments * args = (struct dwndiff_arguments *) state->input;
  switch (key) {
  case 'z':
    args->is_zip = true;
    break;
  case 'c':
    args->is_compare = true;
    break;
  case 'u':
    args->is_unzip = true;
    break;
  case 'o':
    args->output_file_path = arg;
    break;
  case 'v':
    args->is_verbose = true;
    break;
  case 'w':
    args->width = (unsigned long long) strtol(arg, NULL, 10);
    break;
  case ARGP_KEY_ARG: // file argument
    args->files = g_slist_append(args->files, arg);
    break;
  case ARGP_KEY_NO_ARGS:
    args->has_no_args = true;
  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

char args_doc[] = "FILE...";

char doc[] = "dwndiff is a command-line tool to format DNA sequences \
for use in version control systems. It is intended to be used as a backend \
for version control tools which act upon biological data. It defaults to \
\'format\' mode, where it formats a given DNA sequence to a special format, \
appending the \'.vcsfmt\' suffix. This file can be used efficiently in version \
control software.";

struct argp argp = {options, parse_opt, args_doc, doc, 0, 0, 0};


#endif /*___ARGPARSE_SETUP_H___*/
