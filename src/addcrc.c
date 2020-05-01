/*****************************************************************************
 *
 * $Id: addcrc.c 19 2011-03-09 22:47:23Z mjeffe $
 *
 * DESCRITION
 *   appends a checksum of the record to the end of the record.
 *
 * NOTES
 *
 * TO DO
 *
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "crc.h"
/* #include "bintohex.h" */

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#define MAXFIELDS 1000

#define CHARACTER 1
#define NUMBER 2


typedef struct field {
   char *name;
   int start;
   int end;
   int len;
   int datatype;
} field_type;   

typedef char field_name_type[80];

/*
 * globals
 */
field_type layout[MAXFIELDS];
int layout_numfields;

int numcrcs;
int crc_numfields[MAXFIELDS];
char *crc_fields[MAXFIELDS][MAXFIELDS];

int numdiffs;
field_name_type *diff_fields;

int debug = FALSE;
int calcdiff = FALSE;
char *me;                          /* name of this program */
char *cvsid = "$Id: addcrc.c 19 2011-03-09 22:47:23Z mjeffe $";

/*
 * function prototypes
 */
void print_usage();
static int chomp(char *s);
void bintohex(char *s, size_t length, unsigned long *binary_value);
void read_control_card(char *control_card);
int read_layout(char *layout_file);
void process_input(int lrecl);


/*************************************************************************
 * MAIN
 ************************************************************************/
int main(int argc, char *argv[]) {
   int flag;
   int lrecl = 2048;                  /* record length default */
   int i=0, j=0;
   char *control_card;    /* cc file name */
   char *layout_file;     /* layout file name */

   
   me = argv[0];  /* set the global variable with this programs name */

   /* optarg = NULL; */
   while ((flag = getopt(argc, argv, "c:l:n:hdv")) != -1) {
      switch(flag) {

         /* record length */
         case 'n':
            lrecl = atoi(optarg); 
            if ( lrecl == (long int)NULL ) {
               fprintf(stderr, "%s: Invalid record length...\n", me);
               exit(1);
            }
            lrecl += 5; /* add a few bytes for \r, \n, \0, etc. */ 
            break;

         /* print help message */
         case 'h':
            print_usage();
            break;

         /* output debug info */
         case 'v':
            debug = TRUE;
            break;

         /* use [ DIFF ] section in the control card to calculate the difference between CRC values */
         case 'd':
            calcdiff = TRUE;
            break;

         /* control card file name */
         case 'c':
            control_card = optarg; 
            break;

         /* layout card file name */
         case 'l':
            layout_file = optarg; 
            break;

         default:
            fprintf(stderr,"%s: Unknown or invalid parameters\n",argv[0]);
            print_usage();
      }
   }


   read_control_card(control_card);
   layout_numfields = read_layout(layout_file);
   process_input(lrecl);
   /* write_layout(layout_file); */

   return(0);
}



/*************************************************************************
 * calculate and append checksums
 ************************************************************************/
void process_input(int lrecl) {

   typedef struct position {
      int start;
      int len;
   } position_type;   

   position_type positions[numcrcs][layout_numfields];
   //position_type *positions;
   position_type *diff_positions;

   int len;
   char *inbuf, *crcbuf, *b;
   char *outbuf, *p;
   int outbuf_len;
   FILE *outfile = stdout;
   FILE *infile = stdin;
   static char crchex[9];
   unsigned long crc;
   int i, j, k;
   int found_field = FALSE;


   /*
   ** first build a local array of the field positions for the DIFF fields, that need to be pulled from the input line 
   */

   if ( calcdiff ) {
      diff_positions = (position_type *) malloc (numdiffs * sizeof(position_type));
      if ( diff_positions == NULL ) {
	 fprintf(stderr, "%s: Unable to allocate memory for diff_positions in process_input()...\n", me);
	 exit(1);
      }

      for ( i=0; i<numdiffs; i++ ) {  /* for each defined diff section */

         /* cycle through the layout to find this field - this sure would be easier with a hash! */
         for ( k=0; k<layout_numfields; k++ ) {
            if ( strcasecmp(layout[k].name, diff_fields[i]) == 0 ) {
               diff_positions[i].start = layout[k].start;
               diff_positions[i].len = layout[k].len;
               found_field = TRUE;
               break;
            }
         }
         if ( ! found_field ) {
            fprintf(stderr,"%s: ERROR ==> The field %s in the control card does not exist in the layout!\n",me,diff_fields[i]);
            exit(1);
         }
      }
   }

   if ( debug ) {
      for ( i=0; i<numdiffs; i++ )
	 fprintf(stderr, "Debug info: diff %d: positions: %d %d\n",i+1, diff_positions[i].start, diff_positions[i].len);
   }


   /*
   ** first build a local array of the field positions for the CRC fields, that need to be pulled from the input line 
   */

/*
   positions = (position_type *) malloc(numcrcs * layout_numfields * sizeof(position_type));
   if ( positions == NULL ) {
      fprintf(stderr, "%s: Unable to allocate memory in process_input()...\n", me);
      exit(1);
   } 
*/

   found_field = FALSE;
   for ( i=0; i<numcrcs; i++ ) {  /* for each defined crc section */
      for ( j=0; j<crc_numfields[i]; j++ ) {  /* for each field defined in the section */

         /* cycle through the layout to find this field - this sure would be easier with a hash! */
         for ( k=0; k<layout_numfields; k++ ) {
	    if ( strcasecmp(layout[k].name, crc_fields[i][j]) == 0 ) {
	       positions[i][j].start = layout[k].start;
	       positions[i][j].len = layout[k].len;
               found_field = TRUE;
	       break;
	    }
         }
         if ( ! found_field ) {
            fprintf(stderr,"%s: ERROR ==> The field %s in the control card does not exist in the layout!\n",me,crc_fields[i][j]);
            exit(1);
         }
      } 
   }

   if ( debug ) {
      for ( i=0; i<numcrcs; i++ ) {
	 printf("Debug info: crc %d: positions: ",i+1);
	 for ( k=0; k<crc_numfields[i]; k++ ) {
	    printf("%d %d, ", positions[i][k].start, positions[i][k].len);
	 }

	 printf("\n");
      }
   }

   /* allocate memory for buffers */ 
   inbuf = (char *) malloc(lrecl + 20);
   crcbuf = (char *) malloc(lrecl + 20);
   if ( inbuf == NULL || crcbuf == NULL ) {
      fprintf(stderr, "%s: Unable to allocate memory for inbuf or crcbuf in process_input()...\n", me);
      exit(1);
   } 

   /* output buffer needs to account for crc fields, and diff flags */
   outbuf_len = lrecl;
   for (i=0; i<numcrcs; i++ ) {
      outbuf_len += 8;
      if ( calcdiff )
         outbuf_len += 1;
   }
   outbuf_len += 20;  /* add a few bytes to avoid buffer overflow */

   outbuf = (char *) malloc(outbuf_len);
   if ( outbuf == NULL ) {
      fprintf(stderr, "%s: Unable to allocate memory for outbuf in process_input()...\n", me);
      exit(1);
   } 

   /*
   ** now start reading input, calculate the crc's based on the positions in the input line that 
   ** correspond to the positions array that we just built.  If we are doing a diff, then pull
   ** those fields as well based on the diff_positions array we built above.
   */

   memset(outbuf, 0, outbuf_len);
   while( fgets(inbuf, lrecl, infile) ) {
      p = outbuf;

      /* write out the input record, minus the line terminator */
      len = chomp(inbuf);
      memcpy(p, inbuf, len);
      p += len;

      /* for each defined crc to be calculated... */
      for ( i=0; i<numcrcs; i++ ) {

         /* build the crcbuf from the input record  */
         memset(crcbuf, 0, lrecl);
         b = crcbuf;
         len = 0;   /* reused len variable */
         for ( j=0; j<crc_numfields[i]; j++ ) {  /* for each field defined for this crc */
            memcpy(b, inbuf + positions[i][j].start - 1, positions[i][j].len);
            b +=  positions[i][j].len;
            len += positions[i][j].len;
         }
         memcpy(b, "\0", 1);
         len += 1;

         /* calculate the crc and append it to the end of the output record */
         crc = crc32(crcbuf, len);
         /* sprintf(crchex, "%016X", crc); */
         bintohex(crchex, sizeof(unsigned long), &crc);   /* faster substitute for sprintf */
         memcpy(p, crchex, 8);
         p += 8;

         /* set the flag if there has been a change in data  */
         if ( calcdiff ) {
            if ( memcmp(inbuf + diff_positions[i].start - 1, crchex, 8) != 0 )
               memcpy(p, "c", 1);
            else
               memcpy(p, " ", 1);

            p++;
         }

         if ( debug ) {
	    printf("Debug info: crc %d: crcbuf:\n%s\n", i+1, crcbuf);
         }
      }

      memcpy(p, "\n\0", 2);  /* terminate the output record */
      p += 2;

      if  ( ! debug )
	 fputs(outbuf, outfile);
         //fwrite(outbuf, sizeof(char), p - outbuf + 1, outfile);
   }
}



/*************************************************************************
#  Blank lines and Perl style comments are allowed.  The control card is
#  devided into sections, each section defines a set of fields that will
#  be used to calculate a crc.  A section begins with the keyword CRC
#  enclosed with [ ].  Every non blank, non comment line until the next
#  section will be interpreted as a field name corresponding to a field
#  in the input layout.  The fields defined in each section will be
#  substringed out of the input, concatenated together into a single
#  buffer, the crc will be calculated on that buffer and output at the
#  end of the input record.  Each successive section will have it's crc
#  output in the order defined in the control card.
#
 ************************************************************************/
void read_control_card(char *control_card) {

#define COMMENT_CHAR '#'
#define SECTION_OPEN_CHAR '['
#define SECTION_CLOSE_CHAR ']'
#define CRC_SECTION 1
#define DIFF_SECTION 2

   FILE *infile;
   char buffer[2048];
   char *p;
   char *words[10];   /* each control card line gets parsed into words */
   int numwords;    
   int numfields;   
   int section_start = FALSE;
   int section_type = 0;
   int i=0, j=0;


   /* open control card file */
   infile = fopen(control_card, "r");
   if (infile == NULL) {
      fprintf(stderr, "%s: unable to open control card file %s\n", me, control_card);
      print_usage();
   }

   numfields = 0;
   numcrcs = -1;
   numdiffs = -1;

   while( fgets(buffer, sizeof(buffer), infile) ) {

      chomp(buffer); /* remove line terminator */
      p = buffer;
      
      numwords = 0;
      memset(words, 0, sizeof(words));
         
      /* parse each line into words */
      while ( *p != 0 && numwords < (sizeof (words) / sizeof(char *)) ) {

         /* skip white space */         
         while ( *p == ' ' || *p == '\t' || *p == ',' ) 
            p++;

         /* remaining line a comment, or end of line */
         if ( *p == COMMENT_CHAR || *p == 0 ) 
            break;

         /* beggining of a section */
         if ( *p == SECTION_OPEN_CHAR || *p == 0 ) {   
            section_start = TRUE;
            p++;   /* advance past "[" character */
	    while ( *p == ' ' || *p == '\t' || *p == ',' ) 
	       p++;   /* skip white space */         
         }

         /* if it is a quoted word, deal with it */
         if ( *p == '\'' || *p == '\"' ) {
            p++;      /* skip the quote character */

            words[numwords++] = p;  /* start word */

            /* advance to next quote character */
            while ( *p != '\'' && *p != '\"')
               p++;

         } else {
            words[numwords++] = p;

            /* advance to next white space or other special character */
            while ( *p != 0 && *p != ' ' && *p != '\t' && *p != ',' && *p != '\'' && *p != '\"' && *p != SECTION_CLOSE_CHAR )
            p++;
         }

	 /* stick string terminator after each word */
	 if ( *p != 0 ) {
	    *p = 0;
	    p++;
	 }
      }

      if ( numwords == 0 )
         continue;   /* blank or comment line */

      /* line is parsed into words, determine what to do based on what is in the line */ 

      if ( section_start ) {
         if ( strcasecmp(words[0], "crc") == 0 && words[0] != NULL ) {
            section_type = CRC_SECTION;
            section_start = FALSE;
            numcrcs++; 
            numfields = 0;
            /* need to pick up the crc name in word[1] */
            continue;
         }
         else if ( strcasecmp(words[0], "diff") == 0 && words[0] != NULL ) {
            section_type = DIFF_SECTION;
            section_start = FALSE;
            numdiffs++;
            continue;
         }
      }

      if ( section_type == CRC_SECTION ) {
         /* fill the field array */
         crc_fields[numcrcs][numfields] = (char *) malloc (strlen(words[0])+1);
         if (crc_fields == NULL) {
            fprintf(stderr, "%s: Can't allocate memory for crc_fields in read_control_card\n", me);
            exit(1);
         }

         strcpy(crc_fields[numcrcs][numfields], words[0]);
         crc_numfields[numcrcs] = ++numfields;
      }
      else if ( section_type == DIFF_SECTION ) {
         diff_fields = (field_name_type *) realloc (diff_fields, (numdiffs+1) * sizeof(field_name_type));
         if (diff_fields == NULL) {
            fprintf(stderr, "%s: Can't allocate memory for diff_fields in read_control_card\n", me);
            exit(1);
         }
         strcpy(diff_fields[numdiffs], words[0]);
         if ( debug )
            printf("diff %d fieldname = %s\n", numdiffs, diff_fields[numdiffs]);
      }
   }  /* end of while that reads control card lines */

   fclose(infile);

   /* localy  we used numcrcs as an array index, so increment it now to reflect */
   /* the actuall number of crc sections */
   numcrcs++;   
   numdiffs++;   

   if ( debug ) {
      printf("Debug info: control card...\n");
      for ( i=0; i<numcrcs; i++ ) {
	 printf("crc %d:\n", i+1);
	 for ( j=0; j<crc_numfields[i]; j++ ) {
	    printf("   field: %s\n", crc_fields[i][j]);
	 }
      }
   }
}



/*************************************************************************
#  Blank lines and Perl style comments are allowed.  The layout should be
#  a csv file with the following structure:
#     field_name, start, end, length
#  Any thing following the length column will be ignored, so it's ok to
# have a control card with datatype, description, etc
*************************************************************************/
int read_layout(char *layout_file) {

#define COMMENT_CHAR '#'

   FILE *infile;
   char buffer[2048];
   char *p;
   char *words[10];
   int numwords;    /* each control card line gets parsed into words */
   int numfields;   /* temporary field num counter */
   int i=0, j=0;
   int malformed_layout = 0;


   /* open layout file */
   infile = fopen(layout_file, "r");
   if (infile == NULL) {
      fprintf(stderr, "%s: unable to open control card file %s\n", me, layout_file);
      print_usage();
   }

   numfields = 0;

   while( fgets(buffer, sizeof(buffer), infile) ) {

      chomp(buffer); /* remove line terminator */
      p = buffer;
      
      numwords = 0;
      memset(words, 0, sizeof(words));
         
      /* parse each line into words */
      while ( *p != 0 && numwords < (sizeof (words) / sizeof(char *)) ) {

         /* skip white space */         
         while ( *p == ' ' || *p == '\t' || *p == ',' ) 
            p++;

         if ( *p == COMMENT_CHAR || *p == 0 )  /* remaining line a comment, or end of line */
            break;

         words[numwords++] = p;

         /* advance to next white space or end of line */
	 while ( *p != 0 && *p != ' ' && *p != '\t' && *p != ',' && *p != '\'' && *p != '\"')
	    p++;

	 /* stick string terminator after the word */
	 if ( *p != 0 ) {
	    *p = 0;
	    p++;
	 }
      }

      if ( numwords == 0 )
         continue;   /* blank or comment line */

      /* line is parsed into words, now fill the field structure */
      layout[numfields].name = (char *) malloc (strlen(words[0]) + 1);
      if (layout[numfields].name == NULL) {
	 fprintf(stderr, "%s: Can't allocate memory in read_layout\n", me);
	 exit(1);
      }
      strcpy(layout[numfields].name, words[0]);

      if ( atoi(words[1]) != (long int)NULL )
         layout[numfields].start = atoi(words[1]);
      else
         malformed_layout++;

      if ( atoi(words[2]) != (long int)NULL )
         layout[numfields].end = atoi(words[2]);
      else
         malformed_layout++;

      if ( atoi(words[3]) != (long int)NULL )
         layout[numfields].len = atoi(words[3]);
      else
         malformed_layout++;

      layout[numfields].datatype = CHARACTER; 

      if ( malformed_layout ) {
	 fprintf(stderr, "%s: Malformed layout\n", me);
	 exit(1);
      }

      numfields++;
   }

   fclose(infile);

   if ( debug ) {
      printf("Debug info: layout...\n");
      for ( i=0; i<numfields; i++ ) {
	 printf("field # %d:\t", i+1);
	 printf("name=%s\t", layout[i].name);
	 printf("start=%d\t", layout[i].start);
	 printf("end=%d\t", layout[i].end);
	 printf("len=%d\n", layout[i].len);
      }
   }

   return numfields;
}



/*************************************************************************
 * remove newline and/or carriage return from end of a string 
 * return length of new modified string 
 ************************************************************************/
static int chomp(char *s) {
   char *p;

   p = s + strlen(s) - 1; 

   while (p >= s && (*p == '\n' || *p == '\r'))
      *(p--) = 0;

   return (p - s + 1);
}


/*************************************************************************
 * print usage statement
 ************************************************************************/
void print_usage() {
   fprintf(stderr, "%s\n", cvsid);
   fprintf(stderr, "\n");
   fprintf(stderr, "Usage: %s -c control_card -l layout_file [-d] [-n record_length] [-d] [-h] < infile > outfile\n", me);
   fprintf(stderr, "\n");
   fprintf(stderr, "Where:\n");
   fprintf(stderr, "   -c control_card        file name for the control card\n");
   fprintf(stderr, "   -l layout_file         file name for the input layout\n");
   fprintf(stderr, "   -d                     indicates that a diff should be calculated between the current\n");
   fprintf(stderr, "                          CRCs being calculated and the CRC fields indicated in the\n");
   fprintf(stderr, "                          [ DIFF ] section of the control card.  Each [ DIFF ] section\n");
   fprintf(stderr, "                          should correspond with one of the CRCs being calculated, in the\n");
   fprintf(stderr, "                          order they appear in the control card.\n");
   fprintf(stderr, "   -n record_length       provide a record length.  The default is 2048 bytes.\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "Description:\n");
   fprintf(stderr, "   %s appends one hex representation (8 bytes) of a 32-bit Cyclic Redundancy\n",me);
   fprintf(stderr, "   Check (CRC-32) to the end of the record, for each [ CRC ] section defined in the\n");
   fprintf(stderr, "   control card.\n");
   fprintf(stderr, "   %s is a filter program, in other words it reads from stdin and writes to stdout.\n", me);
   fprintf(stderr, "\n");
   exit(1);
}


