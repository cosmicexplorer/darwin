#ifndef ___ARGPARSE_SETUP_H___
#define ___ARGPARSE_SETUP_H___

/*
    hosts all required variables and functions to implement argparse
*/

#include "utilities.h" // for GSList, bool

// version
const char * argp_program_version = "darwin 0.1";
const char * argp_program_bug_address = "<danieldmcclanahan@gmail.com>";
// documentation

static struct argp_option options[] = {
    {"unzip", 'u', 0, 0, "Recover original file from vcsfmt or vcscmp file", 0},
    {"zip", 'z', 0, 0, "Produce vcsfmt file from original file.", 1},
    {"output", 'o', "DIR", 0, "Location to produce output files.", 2},
    {"input", 'i', "FILE", 0, "Location of input files.", 3},
    // TODO: make this option produce more information
    {"verbose", 'v', 0, 0, "Produce verbose output", 4},
    {0, 0, 0, 0, 0, 0}};

// used to communicate with parse_opt
typedef struct {
  GSList * files;
  bool unzip_or_zip; // true => unzip, false => zip
  char * output_file_location;
  char * input_file_location;
  bool is_verbose;
  bool has_no_args;
} dwndiff_arguments;
// CTOR
dwndiff_arguments initialize_dwndiff_arguments(dwndiff_arguments args) {
  args.files = NULL;
  args.unzip_or_zip = false; // default to zip
  args.output_file_location = NULL;
  args.input_file_location = NULL;
  args.is_verbose = false;
  args.has_no_args = false;
  return args;
}

static error_t parse_opt(int key, char * arg, struct argp_state * state) {
  dwndiff_arguments * args = state->input;
  switch (key) {
  case 'u':
    args->unzip_or_zip = true;
    break;
  case 'z':
    args->unzip_or_zip = false;
    break;
  case 'o':
    args->preformat_loc_dir = arg;
    break;
  case 'i':
    args->preformat_loc_dir = arg;
    break;
  case 'v':
    args->is_verbose = true;
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

static char args_doc[] = "FILE...";

static char doc[] = "dwndiff is a command-line tool to format DNA sequences \
for use in version control systems. It is intended to be used as a backend \
for version control tools which act upon biological data. It defaults to \
\'format\' mode, where it formats a given DNA sequence to a special format, \
appending the \'.vcsfmt\' suffix. This file can be used efficiently in version \
control software.";

static struct argp argp = {options, parse_opt, args_doc, doc, 0, 0, 0};

#endif /*___ARGPARSE_SETUP_H___*/
