#include <stdlib.h>							// for malloc
#include <string.h>							// for strlen/strcpy/strcat
#include <ctype.h>							// for toupper

#include "vcsfmt.h"							// required
#include "utilities.h"					// for compiler macros
#include "sequence_heuristics.h" // for codon sequence data

#define BLOCK_SIZE 8190 				// used in file I/O

int vcsfmt(char * filename){	
	FILE * input_file = open_file(filename);
	PRINT_ERROR_AND_RETURN_NEG_ONE_IF_NULL(input_file,"Error in creating input file.\n");

	// create filename long enough to concatenate filename and suffix
	char * output_file_name = (char*) malloc((strlen(filename) +
																						strlen(OUTPUT_SUFFIX) +
																						1) * sizeof(char));
	// create output filename
	strcpy(output_file_name,filename);
	strcat(output_file_name,OUTPUT_SUFFIX);
	FILE * output_file = create_outfile(output_file_name);
	PRINT_ERROR_AND_RETURN_NEG_ONE_IF_NULL(output_file,"Error in creating output file.\n");

	// allocate mem for input block
	string_with_size * input_stream_block = (string_with_size *) malloc(sizeof(string_with_size));
	input_stream_block->string = malloc((BLOCK_SIZE) * sizeof(char));
	input_stream_block->full_size = BLOCK_SIZE * sizeof(char);
	input_stream_block->cur_size = 0;		// no data written yet
	PRINT_ERROR_AND_RETURN_NEG_ONE_IF_NULL(input_stream_block->string,"Stream memory block not successfully allocated.\n");
	// same for output
	string_with_size * output_stream_block = (string_with_size *) malloc(sizeof(string_with_size));
	output_stream_block->string = malloc((BLOCK_SIZE) * sizeof(char));
	output_stream_block->full_size = BLOCK_SIZE * sizeof(char);
	output_stream_block->cur_size = 0;		// no data written yet
	PRINT_ERROR_AND_RETURN_NEG_ONE_IF_NULL(output_stream_block->string,"Stream memory block not successfully allocated.\n");


	size_t cur_bytes_read = 0;
	size_t total_bytes_read = 0;
	size_t cur_bytes_written = 0;
	size_t total_bytes_written = 0;

	bool * is_within_orf = (bool*) malloc(sizeof(bool));
	*is_within_orf = false;				// file begins outside of orf

	// perform block processing
	while (!feof(input_file) && !ferror(input_file) && !ferror(output_file)){
		// read in block
		read_block(input_file, input_stream_block);
		cur_bytes_read = input_stream_block->cur_size;
		total_bytes_read += cur_bytes_read;
		// process block
		process_block(input_stream_block,
									output_stream_block,
									is_within_orf);
		// write block
		write_block(output_file, output_stream_block);
		cur_bytes_written = output_stream_block->cur_size;
		total_bytes_written += cur_bytes_written;
	}
	// cur_bytes_read and cur_bytes_written are assigned differently,
	// one as the return value of a function and one as a value within the string_with_size struct,
	// because while reading a block is always <= BLOCK_SIZE,
	// writing can be and usually is > BLOCK_SIZE due to the newlines and other info inserted

	// cleanup mem
	free(output_file_name);
	free(input_stream_block->string);
	free(input_stream_block);
	free(is_within_orf);

	bool error_occurred = false;
	
	// error handling
	if (ferror(input_file) && !feof(input_file)){
		PRINT_ERROR("Error in reading from input file.\n");
		error_occurred = true;
	}
	else if (ferror(output_file) && !feof(input_file)){
		PRINT_ERROR("Error in writing to output file.\n");
	}
	else if (feof(input_file)){
		// diagnostics
		printf("%zu",total_bytes_read);
		printf(" bytes read.\n");
		printf("%zu",total_bytes_written);
		printf(" bytes written.\n");
		error_occurred = false;			// redundant, but here for clarity
	}
	else{
		PRINT_ERROR("Unknown error in vcsfmt.c\n");
		error_occurred = true;
	}

	// close open handles
	fclose(input_file);
	fclose(output_file);

	if (error_occurred){
		return -1;
	}
	else{
		return total_bytes_written;
	}
}

dna_reading_indices format_file(char * filename __attribute__ ((unused))){
	// does nothing right now
	dna_reading_indices active_dna_reading_indices;
	active_dna_reading_indices.begin_index = 0;
	active_dna_reading_indices.end_index = 0;
	return active_dna_reading_indices;
}

FILE * open_file(char * filename){
	FILE * input_file = fopen(filename,"r");
	// reject if incorrect filetype
	return input_file;
}

void read_block(FILE * input_file, string_with_size * input_str_with_size){
	input_str_with_size->cur_size = fread(input_str_with_size->string,sizeof(char),input_str_with_size->full_size,input_file);
}

void process_block(string_with_size * input_block_with_size, string_with_size * output_block_with_size, bool * is_within_orf){

	// OPTIMIZATION: make this static
	char current_codon[CODON_LENGTH];
  char output_byte;							// output of hash
	size_t bytes_written = 0;
	bool matched_key_codon = false; // used in orf delimiting

	for (size_t codon_index = 0; codon_index < input_block_with_size->cur_size; codon_index += CODON_LENGTH){
		// copy over bases
		for (size_t base_index = 0; base_index < CODON_LENGTH; ++base_index){
			current_codon[base_index] = toupper(input_block_with_size->string[codon_index + base_index]); // toupper to get rid of the lowercase dna which sometimes appears
		}

		// hash input codon to make byte
		output_byte = get_byte_from_codon(current_codon); // '\0' if not a real codon
// #if defined DEBUG
// 		fwrite(current_codon,sizeof(char),CODON_LENGTH,stdout);
// 		printf(" : ");
// 		printf("%c\n",output_byte);
// #endif		
		// check if well-formed
		if (output_byte != '\0'){		// branch predictions should be good on this one
			if (*is_within_orf){			// branching is iffier here though
				// OPTIMIZATION: could hash start/stop codons as well
				for (size_t stop_codon_index = 0; stop_codon_index < NUMBER_OF_STOP_CODONS; ++stop_codon_index){
					if (strncmp(current_codon,stop_codons[stop_codon_index],CODON_LENGTH) == 0){
						matched_key_codon = true;
						output_block_with_size->string[bytes_written] = output_byte;
						++bytes_written;
						output_block_with_size->string[bytes_written] = '\n'; // newline delimiting
						*is_within_orf = false;																// NOTE THE DIFFERENCE HERE FROM BELOW
					}
				}
				if (!matched_key_codon){
					output_block_with_size->string[bytes_written] = output_byte;
				}
			}
			else{
				for (size_t start_codon_index = 0; start_codon_index < NUMBER_OF_START_CODONS; ++ start_codon_index){
					if (strncmp(current_codon,start_codons[start_codon_index],CODON_LENGTH) == 0){
						matched_key_codon = true;
						output_block_with_size->string[bytes_written] = output_byte;
						++bytes_written;
						output_block_with_size->string[bytes_written] = '\n'; // newline delimiting
						*is_within_orf = true;																// NOTE THE DIFFERENCE HERE FROM ABOVE
					}
				}
				if (!matched_key_codon){
					output_block_with_size->string[bytes_written] = output_byte;
				}
			}
			matched_key_codon = false;
			++bytes_written;
		}
		else{												// if non-codon found, assume end of dna encoding and exit
			output_block_with_size->cur_size = bytes_written;
			printf("%s\n","end of dna found. exiting.");
		}
	
	}

	output_block_with_size->cur_size = bytes_written;

#if defined DEBUG
	fprintf(stderr,"block cur_size: ");
	fprintf(stderr,"%zu",output_block_with_size->cur_size);
	fprintf(stderr,"\nblock full_size: ");
	fprintf(stderr,"%zu",output_block_with_size->full_size);
	fprintf(stderr,"\n---------------\n");
#endif
}

void write_block(FILE * output_file, string_with_size * output_block_with_size){
	output_block_with_size->cur_size = fwrite(output_block_with_size, sizeof(char), output_block_with_size->cur_size,output_file);
}

FILE * create_outfile(char* filename){
	FILE * output_file = fopen(filename,"w");
	return output_file;
}

#define BINBLOCK_SIZE 2730			// 1/3 of BLOCK_SIZE, so that we don't overflow when expanding, counting newlines
int de_vcsfmt(char * filename){
	FILE * input_file = open_file(filename);
	PRINT_ERROR_AND_RETURN_NEG_ONE_IF_NULL(input_file,"Error in creating input file.\n");

	// create filename long enough to concatenate filename and suffix
	char * output_file_name = (char*) malloc((strlen(filename) +
																						strlen(OUTPUT_SUFFIX) +
																						1) * sizeof(char));
	// create output filename
	strcpy(output_file_name,filename);
	strcat(output_file_name,OUTPUT_SUFFIX);
	FILE * output_file = create_outfile(output_file_name);
	PRINT_ERROR_AND_RETURN_NEG_ONE_IF_NULL(output_file,"Error in creating output file.\n");

	// allocate mem for input block
	string_with_size * input_stream_block = (string_with_size *) malloc(sizeof(string_with_size));
	input_stream_block->string = malloc((BINBLOCK_SIZE) * sizeof(char));
	input_stream_block->full_size = BINBLOCK_SIZE * sizeof(char);
	input_stream_block->cur_size = 0;		// no data written yet
	PRINT_ERROR_AND_RETURN_NEG_ONE_IF_NULL(input_stream_block->string,"Stream memory block not successfully allocated.\n");
	// same for output
	string_with_size * output_stream_block = (string_with_size *) malloc(sizeof(string_with_size));
	output_stream_block->string = malloc((BLOCK_SIZE) * sizeof(char));
	output_stream_block->full_size = BLOCK_SIZE * sizeof(char);
	output_stream_block->cur_size = 0;		// no data written yet
	PRINT_ERROR_AND_RETURN_NEG_ONE_IF_NULL(output_stream_block->string,"Stream memory block not successfully allocated.\n");


	size_t cur_bytes_read = 0;
	size_t total_bytes_read = 0;
	size_t cur_bytes_written = 0;
	size_t total_bytes_written = 0;
	
	// perform block processing
	while (!feof(input_file) && !ferror(input_file) && !ferror(output_file)){
		// read in block
		read_block(input_file, input_stream_block);
		cur_bytes_read = input_stream_block->cur_size;
		total_bytes_read += cur_bytes_read;
		// process block
		de_process_block(input_stream_block,
										 output_stream_block);
		// write block
		write_block(output_file, output_stream_block);
		cur_bytes_written = output_stream_block->cur_size;
		total_bytes_written += cur_bytes_written;
	}
	// cur_bytes_read and cur_bytes_written are assigned differently,
	// one as the return value of a function and one as a value within the string_with_size struct,
	// because while reading a block is always <= BLOCK_SIZE,
	// writing can be and usually is > BLOCK_SIZE due to the newlines and other info inserted

	// cleanup mem
	free(output_file_name);
	free(input_stream_block->string);
	free(input_stream_block);

	bool error_occurred = false;
	
	// error handling
	if (ferror(input_file) && !feof(input_file)){
		PRINT_ERROR("Error in reading from input file.\n");
		error_occurred = true;
	}
	else if (ferror(output_file) && !feof(input_file)){
		PRINT_ERROR("Error in writing to output file.\n");
	}
	else if (feof(input_file)){
		// diagnostics
		printf("%zu",total_bytes_read);
		printf(" bytes read.\n");
		printf("%zu",total_bytes_written);
		printf(" bytes written.\n");
		error_occurred = false;			// redundant, but here for clarity
	}
	else{
		PRINT_ERROR("Unknown error in vcsfmt.c\n");
		error_occurred = true;
	}

	// close open handles
	fclose(input_file);
	fclose(output_file);

	if (error_occurred){
		return -1;
	}
	else{
		return total_bytes_written;
	}
}

void de_process_block(string_with_size * input_block_with_size, string_with_size * output_block_with_size){

	// OPTIMIZATION: make this static
  char current_byte;
	const char * output_codon;
	size_t bytes_read = 0;
	size_t bytes_written = 0;

	while (bytes_read < input_block_with_size->cur_size){
		current_byte = input_block_with_size->string[bytes_read];
		if (current_byte != '\n'){
			// de-hash
			output_codon = get_codon_from_byte(current_byte);
#if defined DEBUG
			fwrite(output_codon,sizeof(char),CODON_LENGTH,stdout);
			printf(" : ");
			printf("%c\n",current_byte);
#endif
			for (size_t base_index = 0; base_index < CODON_LENGTH; ++base_index){
				output_block_with_size->string[bytes_written + base_index] = output_codon[base_index];
				++bytes_written;
			}
		}
		++bytes_read;
	}
	output_block_with_size->cur_size = bytes_written;

#if defined DEBUG
	fprintf(stderr,"block cur_size: ");
	fprintf(stderr,"%zu",output_block_with_size->cur_size);
	fprintf(stderr,"\nblock full_size: ");
	fprintf(stderr,"%zu",output_block_with_size->full_size);
	fprintf(stderr,"\n---------------\n");
#endif

}
