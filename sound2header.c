/*
 * (C) Copyright 2009
 * Diego Barrios Romero, eldruin@eldruin.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <sndfile.h>

#define BUFFER_SIZE 512

void print_usage();

int main (int argc, char *argv[])
{
  sf_count_t count, read;
  int i, *buffer, linebreak, len, left_limit, right_limit;
  unsigned int number;
  char *outfilename, *number_str, *datatype;
  SNDFILE *input = NULL;
  FILE *output = NULL;
  SF_INFO sfinfo;

  if (argc != 5){
    print_usage();
    return 0;
  }

  left_limit = atoi(argv[2]);
  right_limit = atoi(argv[3])+1;

  if (left_limit >= right_limit-1){
    fprintf(stderr, "Error: the range is not valid.\n\n");
    print_usage();
    return 1;
  }

  datatype = argv[4];
  if (!strcmp(datatype, "char") && !strcmp(datatype, "short") &&
      !strcmp(datatype, "int")){
    fprintf(stdout, "Error: the data type is not valid.\n\n");
    print_usage();
    return 1;
  }

  if ( !(input = sf_open (argv[1], SFM_READ, &sfinfo)) ){
    fprintf(stderr, "Error opening the input file %s.\n", argv[1]);
    fprintf(stderr, "%s", sf_strerror (NULL));
    return 1;
  }

  /* Filename conversion */
  len = strlen(argv[1]);
  if ( !(outfilename = malloc (len + 3 * sizeof(char))) ){
      fprintf(stderr, "Error: No memory available.\n");
      sf_close(input);
      return 1;
  }
  strcpy(outfilename, argv[1]);
  strcpy(outfilename+len, ".h");

  if ( !(output = fopen(outfilename, "w")) ){
    fprintf(stderr, "Error opening the output file %s.\n", outfilename);
    sf_close(input);
    return 1;
  }

  for (i = 0; i <= len; i++){
    if (outfilename[i] == '.')
      outfilename[i] = '_';
  }
  outfilename[++i] = '\0';


  /* Header of the file */
  fprintf(output, "/* Header file automatically generated with 'Sound2"
	  "Header' program\n"
	  " * by Diego Barrios Romero."
	  " <http://blog.eldruin.com/sound2header/>\n"
	  " * Contains an array with the sound samples in the range [%i, %i],\n"
	  " * %i channels and %i samples.\n"
	  " */\n\n", left_limit, right_limit-1, (int)sfinfo.channels,
	  (int)sfinfo.frames);

  outfilename[i-2] = '_';
  fprintf(output, "const unsigned %s %s[%i] = {", datatype, outfilename,
	  (int)(sfinfo.frames * sfinfo.channels));

  free(outfilename);

  if ( !(buffer = (int*) malloc (sfinfo.channels * BUFFER_SIZE * sizeof(int))) ){
      fprintf(stderr, "Error: No memory available.\n");
      sf_close(input);
      fclose(output);
      return 1;
  }

  if ( !(number_str = (char*) malloc (16)) ){
      fprintf(stderr, "Error: No memory available.\n");
      free(buffer);
      sf_close(input);
      fclose(output);
      return 1;
  }

  linebreak = 0;
  count = BUFFER_SIZE * sfinfo.channels;
  while ( (read = sf_read_int (input, buffer, count)) > 0){
    for (i = 0; i < read; i++, linebreak++){
      if (!(linebreak%20)) /* Breaks the lines sometimes */
	fprintf(output, "\n");

      number = buffer[i] / (INT_MAX/(right_limit-left_limit)*2) +
	(right_limit - left_limit)/2 + left_limit;
      sprintf(number_str, "%i", number);

      if (linebreak) /* No comma the first time*/
	fprintf(output, ",%s", number_str);
      else
	fprintf(output, "%s", number_str);
    }
  }
  fprintf (output, "\n};\n");

  free(buffer);
  free(number_str);
  sf_close(input);
  fclose(output);

  return 0;
}

void print_usage()
{
  printf("Sound2Header program by Diego Barrios Romero.\n"
	 "http://blog.eldruin.com/sound2header\n\n"
	 "This program will generate a .h file called equal to "
	 "the input file\n"
	 "which can be included in any C source file.\n"
	 "That generated file will contain an array\n"
	 "with a number inside the given range [left, right] for "
	 "each sample.\n"
	 "These values must be positive.\n\n"
	 "Usage: ./sound2header <soundfile> <left range limit> <right "
	 "range limit> <char|short|int>\n\n");
}
