/* ====================================================================
**
** $Id: chprof.c 338 2019-10-02 16:00:08Z higuito $
**
** Description:
**
** This program is designed to read the input file, then report on
** the characters found within the positions indicated in the control 
** file.
**
** TODO:
**   0. Rewrite to accept command line args like the GNU cut (coreutils)
**      command and do away with (or make optional?) the control card.
**      Also need to eliminate copying data into temp buffer, but rather
**      code more like GNU cut where we decide which characters will be
**      profiled.
**   1. it would be nice to build in the ability to deal with a control
**      card that had: fieldname length
**   2. Should replace any reference to \n with a #define CR \n, so it
**      would be more portable.
**   4. ??? need to fix the way num_parsed_fields is being calculated.
** ====================================================================
**/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "parsecsv.h"
#include "zfopen.h"

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#define BUFSZ 65536        /* arbitrary - used for max line length */

typedef struct field {
   char name[64]; 
   int start;
   int end;
   int field_number;
   long counters[256];
} field_type;


/*
 * Globals
 */

char *me;               /* name of this program */
int verbose = FALSE;    /* default to compact report */

int layout_as_first_row = FALSE;
char delimiter;
char field_wrapper = '"';
int num_fields;             /* number of fields defined in the control card */
unsigned long hex_search;   /* hex character to search for */
unsigned long hex_replace;  /* hex character to replace */
//unsigned char dec_search;   /* decimal character to search for */
//unsigned char dec_replace;  /* decimal character to replace */
//unsigned long search_char;
int max_position;           /* for fixed width files, the last possition of the last field */




/* define non printable characters */
char *clist[256]=
   { "NUL", "SOH", "STX", "ETX", "EOT", "ENQ", "ACK", "BEL", "BS",  "HT",
     "LF",  "VT",  "FF",  "OD",  "SO",  "SI",  "SLE", "CS1", "DC2", "DC3",
     "DC4", "NAK", "SYN", "ETB", "CAN", "EM",  "SIB", "ESC", "FS",  "GS",
     "RS",  "US",  "Sp" };



/*
 * Function Prototypes
 */
void print_usage();
void print_help();
int read_control_card(char *control_card, field_type *fields[], char file_format);
int is_ignorable_line(char *line); 
void process_file(char *data_file, field_type *fields[], char file_format);
//char **get_fixed_data_by_field(field_type *fields[], char *line, char *field_data[]);
int get_fixed_data_by_field(field_type *fields[], char *line, char *field_data[]);
int get_delim_data_by_field(field_type *fields[], char *line, char **field_data);
int get_row_data_as_field(field_type *fields[], char *line, char *field_data[]);
int do_profile(field_type *fields[], char **field_data); 
void print_profile_report(field_type *fields[], char file_format);
void print_summary(int recs_processed, char *data_file, long bad_recs);
static int chomp(char *s);
#ifdef DEBUG
void print_verbose_profile_report(field_type *fields[], char *data_file, char file_format);
#else
void print_verbose_profile_report(field_type *fields[], char file_format);
#endif



/*************************************************************************
 * MAIN
 ************************************************************************/
int main(int argc, char *argv[]) {
   int profile = TRUE;         /* indicates a profile is to be performed, default behavior */
   int flag;                   /* used for command line argument parsing */
   field_type *fields[MAXNUMBEROFFIELDS];   /* an array of pointers to field_type elements */
   unsigned int hold_search;   /* temp variables, to properly parse hex command line opts */
   unsigned int hold_replace;
   int i;
   char file_format = 'F';     /* D = delimited, F = fixed or R = row (entire row as a single field) */
   char *control_card = NULL; 
   char *data_file = NULL; 


   /* set the global variable with the name of this program */
   me = argv[0];  

   /* get command line args */
   optarg = NULL;
   while ( (flag = getopt(argc, argv, "vc:s:r:hd:")) != -1 ) {
      switch (flag) {

         /* delimiter */
         case 'd':
            if (optarg[0] == '\\') {
               if (strlen(optarg) != 2) {
                  fprintf(stderr,"%s: escape sequence for delimiter is malformed\n", me);
                  print_usage();
               }

               switch(optarg[1]) {
                  case 't':
                     delimiter = '\t';
                     file_format = 'D';
                     break;

                  default:
                     fprintf(stderr,"%s: invalid escape sequence for delimiter\n", me);
                     print_usage();
               }
            } 
            else {
               if (strlen(optarg) != 1) {
                  fprintf(stderr,"%s: delimiter must be a single character\n", me);
                  print_usage();
               }

               delimiter = optarg[0];
               file_format = 'D';
            }
            break;

         /* control card file name */
         case 'c':
            control_card = optarg;
            break;

         /* hex search character */
         case 's':
            /*
            sscanf(optarg,"%x",&hold_search);
            hex_search = (char)hold_search;
            */
            /* 
             * see man page for strtoul.  In short, you can give -s values
             * in decimal, or hex if precedded with 0x, or octal if precedded
             * by a leading 0.  For example, all of the following would find
             * the "c" characters:
             *    "-s 99"       # decimal
             *    "-s 0x63"     # hex
             *    "-s 0143"     # octal
             */
            //hex_search = (unsigned char)strtoul(optarg,NULL,16);
            hex_search = strtoul(optarg,NULL,0);
            //fprintf(stderr, "search char = '%d'\n", hex_search);
            break;

         /* hex replace character */
         case 'r':
            /*
            sscanf(optarg,"%x",&hold_replace);
            hex_replace = (unsigned char)hold_replace;
            */
            //hex_replace = (unsigned char)strtoul(optarg,NULL,16);
            hex_replace = strtoul(optarg,NULL,0);
            break;

         case 'v':
            verbose = TRUE;
            break;

         /* layout header is present as first row of input */
         case 'l':
            layout_as_first_row = TRUE;
            break;

         case 'h':
            print_help();

         default:
            fprintf(stderr,"%s: Unknown or invalid parameters\n",argv[0]);
            print_usage();
      }
   }

/*
printf("mf hex_search: %c\n", hex_search);
printf("mf hex_replace: %c\n", hex_replace);
*/


   /* make sure we have a control card, if using -d */
   if ( file_format == 'D' && control_card == NULL ) {
      fprintf(stderr, "%s: must supply control card when profiling a delimited file\n", me);
      exit(1);
   }

   /* If no file name, then use stdin */
   if ( argc < optind + 1 )
      data_file = "stdin";
   else
      data_file = argv[optind];

   /* no control card, profile the entire row as a single field */
   if ( control_card == NULL )
      file_format = 'R';

   num_fields = read_control_card(control_card, fields, file_format);

   /* run the profile on the data */
   process_file(data_file, fields, file_format);

   return 0; 

}




/*************************************************************************
 * Print the usage statement
 ************************************************************************/
void print_usage() {
   printf("\n");
   printf("For help, type: %s -h\n",me);
   printf("\n");
   exit(1);
}




/*************************************************************************
 * Print the usage statement
 ************************************************************************/
void print_help() {

   printf("\n");
   printf("Usage:\n");
   printf("  %s [-vh] [-d delimiter] [-c control_card] [infile] \n", me);
/* not implemented 
   printf("  %s -s 0xhh -r 0xhh -[c control_card] data_file \n", me); 
*/
   printf("\n");
   printf("Switches:\n");
/*
   printf("  -r              Replace searced for hex value with hex value 0xhh\n");
*/
   printf("  -v              Verbose report.  This output is really verbose!  It generates\n");
   printf("                  a count for all of the 255 ASCII values for each field.\n");
   printf("  -h              Print this screen\n");
   printf("  -c control_card Control card file name.  See Control Card Description below.\n");
   printf("  -d delimter     Character used to delimit fields in the input file, \"\\t\" is\n"); 
   printf("                  implemented for a tab character.  Note that without this\n");
   printf("                  switch %s assumes the file is fixed width.\n", me);
   printf("  -s char         Search for character char and output the line number on stdout.\n");
   printf("                  It's kind of experimental so there is not much support around it\n");
   printf("                  nor any guarantee that it works as reported.  The char parm\n");
   printf("                  can be given with values in decimal, hex if preceded by 0x,\n");
   printf("                  or octal if preceeded by a leading 0.  For example, all of the\n");
   printf("                  following would find the \"c\" character:\n");
   printf("                     \"-s 99\"       # decimal\n");
   printf("                     \"-s 0x63\"     # hex\n");
   printf("                     \"-s 0143\"     # octal\n");
   printf("\n");
   printf("Description:\n");
   printf("  %s generates a frequency report on the (single byte) characters in a\n", me);
   printf("  file, or on stdin.  If the -c switch is used, then only the fields defined in\n");
   printf("  the control card will be profiled, and will be reported on by field name.\n");
   printf("  If the -c option is not used, the entire file will be profiled.  Note that\n");
   printf("  %s counts the ASCII values of the characters (0-255), therefore it\n", me);;
   printf("  will not work on a multibyte character set file (or at best the behavior is\n");
   printf("  undefined).  Note that the Char column in the report attempts to display\n");
   printf("  the actual character.  However for characters above decimal 127 it is\n");
   printf("  unreliable because it depends on several things, such as your terminal's\n");
   printf("  font settings, locale settings on your system, etc.  Therefore it's better\n");
   printf("  to rely on the decimal or hex values.  For non printable control characters\n");
   printf("  (decimal 0 - 31) and the space character (decimal 32) mnemonic representations\n");
   printf("  are substituted in the Char column of the report.\n");
   printf("\n");
   printf("Control Card Description:\n");
   printf("  Comments are allowed on a line by themselves, and are anything following a\n");
   printf("  pound (\"#\") sign.  Comments are not allowed on a line with a field definition.\n");
   printf("  Totally blank lines are allowed.  Each non-blank, non-comment line represents\n");
   printf("  a field to be profiled.  Each line is composed of values separated by white\n");
   printf("  space (blanks or tabs).  The control card for a fixed width file has following\n");
   printf("  format:\n");
   printf("     field_name start_position end_position\n");
   printf("\n");
   printf("  For delimited files it must have the following format:\n");
   printf("     field_name field_number\n");
   printf("\n");
   printf("Examples:\n");
   printf("  Fixed width file control card example:\n");
   printf("\n");
   printf("  # this control card will profile 3 fields from the input, the first name\n");
   printf("  # field which starts at position 1 and goes to position 20, last name which\n");
   printf("  # starts at position 22, etc.\n");
   printf("  first_name 1 20\n");
   printf("  last_name 22 41\n");
   printf("  phone 150 156\n");
   printf("\n");
   printf("  Delimited file control card example\n");
   printf("\n");
   printf("  # this control card will profile 4 fields from the input, the first field\n");
   printf("  # (first_name), third field (last_name), tenth field...\n");
   printf("  first_name 1\n");
   printf("  last_name  3\n");
   printf("  phone      10\n");
   printf("  account_id 23\n");
   printf("\n");
   printf("cvs version: $Id: chprof.c 338 2019-10-02 16:00:08Z higuito $ \n");
   printf("Report any comments or bugs to Matt Jeffery - mjeffe@acxiom.com\n");
   printf("\n");

   exit(1);

}




/*************************************************************************
 * read the control card
 ************************************************************************/
int read_control_card(char *control_card, field_type *fields[], char file_format) {

   FILE *controlfile;        /* control card */
   char buffer[BUFSZ];
   char fieldname [256];
   int start, end;
   int num_fields = 0; 
   int field_number = 0;     /* for delimited files, field order number */
   int mp = 0;               /* local max position parm */
   int i;



   if ( file_format == 'R' ) {

      fields[0] = (field_type *) malloc(sizeof(field_type));
      if ( fields[0] == NULL ) {
         fprintf(stderr, "%s: unable to allocate memory in read_control_card\n", me);
         exit(1);
      }

      /* initialize one field struct */
      memset(fields[0], 0, sizeof(field_type));
      strcpy(fields[0]->name, "record");

      /* stop here */
      return 1;  
   }


   /*
    * we are doing the profile by field, read the control card 
    */

   controlfile = fopen(control_card, "r");
   if (controlfile == NULL) {
      fprintf(stderr, "unable to open control file %s for input\n", control_card);
      print_usage();
   }


   while ( fgets(buffer, sizeof(buffer), controlfile) != NULL ) {

      chomp(buffer);

      /* ignore comment lines and blank lines */
      if ( is_ignorable_line(buffer) ) {
         continue;
      }

      /* allocate memory and initialize a field structure */
      if ( NULL == (fields[num_fields] = (field_type *) malloc(sizeof(field_type))) ) {
         fprintf(stderr, "%s: unable to allocate memory in read_control_card\n", me);
         exit(1);
      }
      memset(fields[num_fields], 0, sizeof(fields[num_fields]));
           
      /* 
       * delimited file format
       */
      if ( file_format == 'D' ) {
         if ( sscanf(buffer, "%s %d", fieldname, &field_number) == 2 ) {
            /* the field number is used as an index into an array, so subtract */
            /* 1 from it, so that it works the same as fixed                   */
            fields[num_fields]->field_number = field_number - 1;
            strcpy(fields[num_fields]->name, fieldname);
            num_fields++;
         }
         else {
            fprintf(stderr, "%s: malformed control card entry\n", me);
            fprintf(stderr, "%s\n", buffer);
            print_usage();
         }
      }
      
      /* 
       * Fixed width file format 
       */
      else if ( file_format == 'F' ) {
          /* if there are 3 parms on the first line of the control card, then  */
          /* assume layout is: fieldname start end                             */
         if ( sscanf(buffer, "%s %d %d", fieldname, &start, &end) == 3 ) {
   
            /* keep track of the highest index */
            mp = ( mp < end ) ? end : mp; 
   
            strcpy(fields[num_fields]->name, fieldname);
            fields[num_fields]->start = start;
            fields[num_fields]->end = end;
            fields[num_fields]->field_number = num_fields;
            num_fields++;
         } 
         else {
            fprintf(stderr, "%s: malformed control card entry\n", me);
            fprintf(stderr, "%s\n", buffer);
            print_usage();
         }
      }
      else {
         fprintf(stderr, "%s: unknown file format type in read_control_card\n", me);
         exit(1);
      }
   }

   fclose(controlfile);

   max_position = mp;
   return num_fields;
}





/*************************************************************************
 * returns true if a line has nothing but white space (spaces and tabs)
 * OR
 * returns true if a line has a comment '#' indicator which is preceeded
 * only by white space.
 ************************************************************************/
int is_ignorable_line(char *line) {

   int i;


   for ( i = 0; i < strlen(line); i++ ) {

      /* is it a non white space character or end of the line? */
      if ( line[i] != ' ' && line[i] != '\t' && line[i] != '\n') {

         /* if so, is it the comment character? */
         if ( line[i] == '#' ) {
            return TRUE;
         }

         /* nope, must be a real control card line */
         else {
            return FALSE;  
         }
      }
   } 

   /* must be a blank line */
   return TRUE;
}





/*************************************************************************
 * opens the data file, breaks each row of data into the fields defined
 * in the control card, then runs the profile.
 ************************************************************************/
void process_file(char *data_file, field_type *fields[], char file_format) {

   FILE *infile;
   char *buffer;
   long recs_processed = 0;   /* total number of records read in from data file */
   long bad_recs = 0;         /* records with parsing problems */
   int num_parsed_fields = 0;
   int i;
   int hex_search_count = 0;

   /* the data will be parsed into it's individual fields, this is an array of */
   /* pointers to those individual field strings.                              */
   static char *field_data[MAXNUMBEROFFIELDS];

   buffer = (char *) malloc(BUFSZ + 4);

   /* allocate the memeory for the strings that will be copied out of the buffer */
   /* into the parsed field_data array */
   if ( file_format == 'F' || file_format == 'R' ) {
      for ( i = 0; i < num_fields; i++) {
         if ( NULL == (field_data[i] = (char *) malloc(fields[i]->end - fields[i]->start + 1)) ) {
            fprintf(stderr, "%s: unable to allocate memory in process_file\n", me);
            exit(1);
         }
      }
   }

   /* open the input data file for reading */
   if ( strcmp(data_file, "stdin") == 0 ) {
      infile = stdin;
   } else {
      infile = zfopen(data_file, "r");
   }
   if (infile == NULL) {
      fprintf(stderr, "%s: unable to open data file %s for input\n", me, data_file);
      print_usage();
   }

   memset(buffer, 0, BUFSZ + 4);

   while (fgets(buffer, BUFSZ, infile) != NULL) {

      /* break each row into fields */
      if ( file_format == 'F' ) {
         num_parsed_fields = get_fixed_data_by_field(fields, buffer, field_data); 
      } 
      else if ( file_format == 'D' ) {
         num_parsed_fields = get_delim_data_by_field(fields, buffer, field_data);
      } 
      else if ( file_format == 'R' ) {
         num_parsed_fields = get_row_data_as_field(fields, buffer, field_data);
      } 
      else {
         fprintf(stderr, "%s: unknown file_format %c in process_file\n", me, file_format);
      }

      /* profile this record */
      if ( num_parsed_fields < num_fields ) {
         bad_recs++;
      }
      else {
         hex_search_count = do_profile(fields, field_data);
      }

      recs_processed++;
      if ( hex_search_count > 0 )
         fprintf(stderr, "%ld,%d\n", recs_processed, hex_search_count);

      hex_search_count = 0;
   }

   if ( strcmp(data_file, "stdin") != 0 ) {
      ozfclose(infile, data_file);
   }

   /* print the report */
   if ( verbose ) {
#ifdef DEBUG
      print_verbose_profile_report(fields, data_file, file_format);
#else
      print_verbose_profile_report(fields, file_format);
#endif
   } else {
      print_profile_report(fields, file_format);
   }
   
   print_summary(recs_processed, data_file, bad_recs);
}





/*************************************************************************
 * break the fixed width data record into it's independent field elements
 ************************************************************************/
//char **get_fixed_data_by_field(field_type *fields[], char *line, char *field_data[]) {
int get_fixed_data_by_field(field_type *fields[], char *line, char *field_data[]) {

   int start, end;            /* column positions */
   int i, j;                  /* general loop counters */


   /* check for lines that end before the last field position */
   /* this will probably slow processing down, so I may want to experiment with it, to see if it is worth it */
   if ( strlen(line) < max_position + 1 ) {
      return(0);
   }

   /* cut out the string between start and end indexes and put it in the field_data array */
   /* position corresponding with the field number                                        */
   j=0;
   for ( i = 0; i < num_fields; i++) {
      j = fields[i]->field_number;
      memcpy(field_data[j], line + fields[i]->start - 1, fields[i]->end - fields[i]->start + 1);
      * (field_data[j] + (fields[i]->end - fields[i]->start + 1)) = 0;
      
#ifdef DEBUG
      fprintf(stderr, "%d:%s:%d-%d:'%.1s'+'%.1s' = -->|%s|<--\n",
         fields[i]->field_number, fields[i]->name, fields[i]->start, fields[i]->end, 
         line + fields[i]->start - 1, line + fields[i]->end - 1, field_data[j] ); 
#endif

   }

// TODO: This looks not good to me, need to come back and look at this.
   return(num_fields);
}






/*************************************************************************
 * parse the delimited data record into it's independent field elements
 ************************************************************************/
int get_delim_data_by_field(field_type *fields[], char *line, char **field_data) {

   int i, j;
   int field_count;

   chomp(line);    /* strip the line terminator */
   //field_data = parsecsv(line, delimiter, field_wrapper, &field_count);
   parsecsvTS(line, delimiter, field_wrapper, &field_count, field_data); 
   return(field_count);
}





/*************************************************************************
 * return the entire row as field 1
 ************************************************************************/
int get_row_data_as_field(field_type *fields[], char *line, char *field_data[]) {

   field_data[0] = line;
   return(1);
}





/*************************************************************************
 * profile characters in the input file
 *
 * num_fields number of fields structures have been created to contain the
 * layout data for the data file.  Each fields structure has a member called
 * field_number.  I use this number to reference the right element in the field_data
 * array, which is the data row parsed into it's independent field elements.
 ************************************************************************/
int do_profile(field_type *fields[], char **field_data) {

   int i, j;                  /* general loop counters */
   char *field = NULL;        /* temp holding variable for the field string */
   int found_hex_search = 0;


   for ( i = 0; i < num_fields; i++ ) {
      field = field_data[fields[i]->field_number];
      j = 0;

      while ( field[j] != '\0' ) {
         fields[i]->counters[(unsigned char)field[j]]++;
         //if ( (unsigned long)field[j] == hex_search )
         if ( field[j] == hex_search )
            found_hex_search++;
         j++;
      } 

      field = NULL;
   }
   return(found_hex_search);
}





/*************************************************************************
 * single column output, only report on characters that that were 
 * actually encountered.  Give the report by field.
 ************************************************************************/
void print_profile_report(field_type *fields[], char file_format) {

   int i, j;


   for ( i = 0; i < num_fields; i++) {

     if ( i > 0 )
        printf("\n\n");

     /* the next line does not make sense if there is no layout, so don't print it    */
     if ( file_format != 'R' ) {
         printf("Field:     %s\n", fields[i]->name);
         if ( file_format == 'F' ) {
            printf("Positions: %d - %d\n", fields[i]->start,fields[i]->end);
         } 
         else if ( file_format == 'D' ) {
            printf("Position:  %d\n", fields[i]->field_number + 1);
         }
     }

      printf("\n");

      /* print the column header */
      printf("Char\tDec\tHex\tCount\n");
      printf("----\t---\t---\t-----\n");

      for ( j = 0; j < 256; j++ ) {
         
         /* only report on chars that were found, in otherwords, don't output char X with a count of 0 */
         if ( fields[i]->counters[j] > 0 ) {

            /* if it is a non printable character, then use the clist string */
            if ( clist[j] != 0 ) {
               printf("%s\t%d\t%x\t%ld",clist[j],j,j,fields[i]->counters[j]);
            }
            else {
               printf("%c\t%d\t%x\t%ld",j,j,j,fields[i]->counters[j]);
               /*
               if ( isprint(j) )
                  printf("%c\t%d\t%x\t%d",j,j,j,fields[i]->counters[j]);
               else
                  printf("%c\t%d\t%x\t%d",' ',j,j,fields[i]->counters[j]);
               */
            }
            printf("\n");
         }
      }
   }
}






/*************************************************************************
 * print the profile summary
 ************************************************************************/
void print_summary(int recs_processed, char *data_file, long bad_recs) {

   /* make stdin look special */
    char *f = "<stdin>";

   if ( strcmp(data_file, "stdin") != 0 ) {
      f = data_file;
   }      
  
   printf("\n\n\n");
   printf("File processed:     %s\n", f);

   if ( bad_recs == 0 ) {
      printf("Records processed:  %d\n", recs_processed);
      printf("\n\n");
   } 
   else {
      printf("\n\n");
      printf("****************************************************\n");
      printf("                 !!!! WARNING !!!!                  \n");
      printf("                                                    \n");
      printf("Some records were encountered which were shorter    \n");
      printf("than the positions indicated in the control file!  \n");
      printf("                                                    \n");
      printf("Total Records Read: %d\n", recs_processed);
      printf("Records Profiled:   %ld\n", recs_processed - bad_recs);
      printf("                                                    \n");
      printf("****************************************************\n");
      printf("\n\n");
   }
}



/*
 * this function is rather ugly, and not very useful.  I will probably
 * eliminate it eventually, or maybe improve it to have a dynamicaly 
 * calculated column offset
 */

/*************************************************************************
 * print a two column verbose profile report
 ************************************************************************/
#ifdef DEBUG
void print_verbose_profile_report(field_type *fields[], char *data_file, char file_format) {
#else
void print_verbose_profile_report(field_type *fields[], char file_format) {
#endif

   FILE *datafile;          /* input file */
   int offset;              /* column offset for report display */
   int i, j, k;             /* used in the report display section */

#ifdef DEBUG
   char buffer[BUFSZ];      /* stores each line read from data file */

   /*
    * reopen the file to show an example of each field, this really helps 
    * tune the control file.  However, don't print this under two conditions: 
    *   1. if input is from stdin then we can't reopen the file 
    *   2. if the profile type is entire file, rather than by field, then
    *      there is no need to print out the entire row.
    */

   if ( file_format != 'R' && strcmp(data_file, "stdin") != 0 ) {
   
      datafile = zfopen(data_file, "r");
      if (datafile == NULL) {
         fprintf(stderr, "Unable to open data file %s for input\n", data_file);
         print_usage();
      }
   
      /* display first 3 rows */
      printf("\n\nExample fields to be profiled:\n\n");
      for ( k = 0; k < 3; k++ ) {
         if (fgets(buffer, sizeof(buffer), datafile) != NULL) {
            printf("RECORD %d\n", k+1);
            for ( i = 0; i < num_fields; i++) {
               printf("Field: %s\t|",fields[i]->name);
               for ( j = fields[i]->start - 1; j < fields[i]->end; j++ ) {
                  /* print the characters */
          	       printf("%c",buffer[j]);
               }
               printf("|\n");
            }
            printf("\n\n");
         }
      }
   
      zfclose(datafile);
   }
#endif

   /*
    * two column, report on every character, regardless of count 
    */

   offset = 128; /* offset for 2 columns */
   for ( i = 0; i < num_fields; i++) {

      /* the next line does not make sense if there is no layout, to don't print it    */
      if ( file_format!= 'R' ) {
         printf("\n\n");
         printf("Field:     %s\n", fields[i]->name);
         printf("Positions: %d - %d\n", fields[i]->start,fields[i]->end);
      }
      printf("\n");

      printf("Char\tDec\tHex\tCount\t\tChar\tDec\tHex\tCount\n");
      printf("----\t---\t---\t-----\t\t----\t---\t---\t-----\n");

      for ( j = 0; j < offset; j++ ) {
         /* column 1 */
         if (clist[j] != 0) {
            printf("%s\t%d\t%x\t%ld",clist[j],j,j,fields[i]->counters[j]);
         }
         else {
            printf("%c\t%d\t%x\t%ld",j,j,j,fields[i]->counters[j]);
            /*
            if ( isprint(j) )
               printf("%c\t%d\t%x\t%d",j,j,j,fields[i]->counters[j]);
            else
               printf("%c\t%d\t%x\t%d",' ',j,j,fields[i]->counters[j]);
            */
         }

         /* column 2 */
         if (clist[j+offset] != 0) {
            printf("\t\t%s\t%d\t%x\t%ld",clist[j+offset],j+offset,j+offset,fields[i]->counters[j+offset]);
         }
         else {
            printf("\t\t%c\t%d\t%x\t%ld",j+offset,j+offset,j+offset,fields[i]->counters[j+offset]);
            /*
            if ( isprint(j+offset) )
               printf("\t\t%c\t%d\t%x\t%d",j+offset,j+offset,j+offset,fields[i]->counters[j+offset]);
            else
               printf("\t\t%c\t%d\t%x\t%d",' ',j+offset,j+offset,fields[i]->counters[j+offset]);
            */
         }
         printf("\n"); 
      }
   }
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



   
