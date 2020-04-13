/*****************************************************************************
 * $Id: bf.c 21 2011-03-09 22:49:11Z mjeffe $
 *
 * Another bug-ugly utility from Matt's code archive.
 *
 * DESCRITION
 *   Scans a file (binary or text) and breaks it into records using the
 *   specified character as the record terminator.
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


/*
 * globals
 */
char *me;
#define RT 0x80       /* Record Terminator - hex 80, decimal 128 */
#define LRECL 400




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

   /* get input file name (can't use stdin very easily since we */
   /* have to make two passes on the file                       */
   if ( argc < 2 ) {
      filename = "-";
   } else if ( argc == 2 ) {
      filename = argv[1];
   } else {
      fprintf(stderr, "%s: too many files on command line...\n", me);
      exit(1);
   }

   process_file(filename);
   return(0);

}

/*************************************************************************
 * Processes input records.
 ************************************************************************/
int process_file(char *file_name) {
   FILE *infile;
   unsigned char c;
   int i, j;
   int maxlen = 0;
   int recs = 0;
   

   /* open intput file */
   if (strcmp(file_name, "-") == 0) {
      infile = stdin;
   }
   else {
      infile = fopen(file_name, "r");
      if (infile == NULL) {
         fprintf(stderr, "%s: unable to open %s\n", me, file_name);
         exit(1);
      }
   }

   
   /* read a byte at a time, and write a byte at a time, you can't get any simpler!! */
   /* create fixed length records:           */
   /*   - search for the record terminator   */
   /*   - pad out to LRECL when we find it   */
   i = 0;
   while ( (c = getc(infile)) != EOF ) { 
      if ( feof(infile) ) break;
       
      if ( c == RT ) {
         if ( i > maxlen ) {   /* keep track of the max possition we encounter RT */
            maxlen = i;
         }
         recs++;

         /* pad the record out to LRECL bytes */
         for ( j = i; j < LRECL; j++ ) {
            putc(0x40, stdout);
         }
         i=0;
      } else {
         putc(c, stdout);
         i++;
      }
   }

   /* pad the last record */
   for ( j = i; j < LRECL; j++ ) {
      putc(0x40, stdout);
   }
   recs++;

   /* close the input file */
   fclose(infile);

   /* print the report */
   fprintf(stderr, "Max reclen: %d,  Recs counted: %d\n", maxlen, recs); 
   if ( maxlen > LRECL ) {
      fprintf(stderr, "!!! ERROR - a record was encountered that is longer than the \n");
      fprintf(stderr, "            hard coded record length of %d\n", LRECL);
      fprintf(stderr, "THIS FILE IS BAD!!! \n");
   }

  return(0);
}




