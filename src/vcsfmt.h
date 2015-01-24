#ifndef ___VCS_FMT_H___
#define ___VCS_FMT_H___

/*
    functions for breaking down DNA sequences into specially formatted files
    which work well with traditional line-based version control software
*/

/* cldoc:begin-category(vcsfmt) */

#define OUTPUT_SUFFIX ".vcsfmt"

/* Produces formatted file with extension ".vcsfmt".
 * @filename the file to produce a vcsfmt file from.
 *
 * vcsfmt file produced in same directory as given file.
 * This method simply forwards to the appropriate strategy depending
 * on the original format of the file (FASTA, Genebank, etc.)
 */
// TODO: redo cldoc so that it has all args
void vcsfmt(char * filename, char * output_file_path);
/* Produces unformatted file from vcsfmt file.
 * @filename the file to produce an unformatted version of.
 *
 * Unformatted file produced in same directory as vcsfmt file.
 * This method simply forwards to the appropriate strategy depending
 * on the original format of the file (FASTA, Genebank, etc.)
 */
void de_vcsfmt(char * filename, char * output_file_path);

/* cldoc:end-category() */

#endif /*___VCS_FMT_H___*/
