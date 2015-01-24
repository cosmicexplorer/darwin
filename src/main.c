#include <argp.h> // for argparse

#include "argparse_setup.h" // for argparse processing
#include "vcsfmt.h"         // for file processing and I/O
#include "vcscmp.h"         // for producing diff-compatible output

dwndiff_arguments args;

void format_file_arg(char * filename) {
  if (args.is_zip) {
    vcsfmt(filename, args.output_file_path);
  } else if (args.is_unzip) {
    de_vcsfmt(filename, args.output_file_path);
  } else {
    PRINT_ERROR_AND_RETURN("Error: Neither zip nor unzip specified for file.");
  }
}

int main(int argc, char ** argv) {

  args = initialize_dwndiff_arguments(args);
  argp_parse(&argp, argc, argv, 0, 0, &args);

  if (args.has_no_args) {
    argp_help(&argp, stderr, ARGP_HELP_USAGE, "standard");
  } else {
    if (0 != args.width) {
      FASTA_LINE_LENGTH = args.width;
    }
    if (!args.output_file_path) {
      args.output_file_path = "";
    }
    if (NULL != args.files) {
      if (args.is_zip || args.is_unzip) {
        if (g_slist_length(args.files) != 1) {
          PRINT_ERROR(
           "Error: zip/unzip is only allowed on one file at a time.");
          return -1;
        } else {
          format_file_arg(g_slist_nth_data(args.files, 0));
        }
      } else if (args.is_compare) {
        if (g_slist_length(args.files) != 2) {
          PRINT_ERROR("Error: comparison only works on two vcsfmt files.");
          return -1;
        } else {
          vcscmp(g_slist_nth_data(args.files, 0),
                 g_slist_nth_data(args.files, 1), args.output_file_path);
        }
      }
    } else {
      PRINT_ERROR("Error: No input files given.");
      return -1;
    }
  }
}
