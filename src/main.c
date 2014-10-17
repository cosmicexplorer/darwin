#include <argp.h> // for argparse

#include "argparse_setup.h" // for argparse processing
#include "vcsfmt.h"         // for file processing and I/O
#include "vcscmp.h"         // for producing diff-compatible output

// example function used for g_slist_foreach
void print_list_string(char *input_str) {
  printf("%s\n", input_str);
}

int main(int argc, char **argv) {

  dwndiff_arguments args;
  args = initialize_dwndiff_arguments(args);
  argp_parse(&argp, argc, argv, 0, 0, &args);

  if (args.has_no_args) {
    argp_help(&argp, stderr, ARGP_HELP_USAGE, "standard");
  } else {
    if (args.is_write) {
      printf("HAHA\n");
    }
    if (args.is_compare) {
      printf("YO THIS WORKS!\n");
    }
    if (args.is_verbose) {
      printf("YOOO THIS TOO!\n");
    }
    if (strcmp(args.preformat_loc_dir, "") != 0) {
      printf("%s\n", args.preformat_loc_dir);
    }
    if (args.files != NULL) {
      g_slist_foreach(args.files, (GFunc)print_list_string, NULL);
    }
  }

  vcsfmt("500_lines_of_dna_minus_lines_0.fasta");
  vcsfmt("500_lines_of_dna_minus_lines_1.fasta");

  // de_vcsfmt("500_lines_of_dna_minus_lines.fasta.vcsfmt");

  vcscmp("500_lines_of_dna_minus_lines_0.fasta.vcsfmt",
         "500_lines_of_dna_minus_lines_1.fasta.vcsfmt",
         "WEIRD_OUTPUT_FILE.vcscmp");
}
