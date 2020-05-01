/*****************************************************************************
 *
 * $Id: fixwav.c 18 2011-03-09 21:47:56Z mjeffe $
 *
 * DESCRITION
 *    This fixes wav files created by audiofile, which I use to digitize
 *    my records.  audiofile doesn't know how big the file will be when
 *    it starts to sample, so it just slams some big number in the "file
 *    size" positions of the wav file header.  This program slams the
 *    appropriate modified file size into the required positions of the
 *    wav file header.
 *
 *
 * NOTES
 *    It will only work for 16 bit sample rate, 44kHz wav files.
 *
 ****************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>



/*
 * globals
 */
char *me;
struct stat buf;


/*
 * function prototypes
 */
int process_file(char *file_name);



/*************************************************************************
 * MAIN
 ************************************************************************/
int main(int argc, char *argv[]) {
   int i;
   char *filename;

   me = argv[0];  /* set the global variable with this programs name */

   /* process each input file on the command line */

   if ( argc < 2 ) {
      fprintf(stderr, "%s: missing file to process\n", me);
      fprintf(stderr, "usage: %s file1 [file2 ...]\n", me);
      exit(1);
   } else {
      for ( i=1; i<argc; i++ ) {
         process_file(argv[i]);
      }
   } 

   return(0);

}

/*************************************************************************
 * fix wav file
 ************************************************************************/
int process_file(char *file_name) {
   FILE *infile;
   int i;
   unsigned int size1 = 0;
   unsigned int size2 = 0;
   

   /* open intput file */
   infile = fopen(file_name, "r+b");
   if (infile == NULL) {
      fprintf(stderr, "%s: unable to open %s\n", me, file_name);
      exit(1);
   }

   /* find out how big the file is */
   fstat(fileno(infile), &buf); 
   size1 = buf.st_size - 8;
   size2 = buf.st_size - 44;

   /*
    * first file size is at offset 0x04 (bytes 5 - 8)
    * This is the (file size) - 8
    */
   fseek(infile, 4, SEEK_SET);
   fwrite(&size1, 4, 1, infile);

   /*
    * second file size is at bytes 41 - 44
    * This is the (file size) - 44
    */
   fseek(infile, 40, SEEK_SET);
   fwrite(&size2, 4, 1, infile);

   /* close the input file */
   fclose(infile);

  return(0);
}




