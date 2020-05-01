/*****************************************************************************
 *
 * $Id: dtof.c 345 2020-01-11 16:31:25Z higuito $
 *
 * DESCRITION
 *   Displays a delimited file as fixed width based on max lengths of each
 *   field in the input stream.  See print_usage() function below.
 *
 * NOTES
 *   0. There is plenty of atrocious code here and I would love to rewrite
 *      this.  But it has been developed over a long time and there just has
 *      not been a compelling reason to rewrite it...
 *   1. be aware that the current version of Compaq's "more" does not
 *      display extended ascii characters properly (at least with typical 
 *      Acxiom locale settings).
 *   2. MAXNUMBEROFFIELDS is defined in parsecsv.h
 *
 * TO DO
 *  1) Eliminate usage of ACX libs
 *    - just remove zfopen, not really needed functionality
 *    - rewrite to use libcsv
 *  2) clean up some of the older, crappy code...
 *   - Eliminiate hard coded limits such as MAXNUMBEROFFIELDS and LINE_BUF_SIZE
 *     to dynamically resize if limits are detected.
 *   - Add all command line options to the control card.  Make sure command
 *     line options would override the control card.
 *   - add a sofar parm, like that in splitval
 *
 *
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "dmax.h"
#include "parsecsv.h"
#include "zfopen.h"
/* #include "ansicode.h" */


//#define MAXFIELDS /* max number of csv fields */

#define VERSION "0.55" /* version number of this release */

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

/* control card keywords */
#define CONSTANT     1
#define SEQUENCE     2
#define LEFTJUSTIFY  3
#define RIGHTJUSTIFY 4
#define ZEROPAD      5
#define CHARACTER    6

#define COMMENT_CHAR '#'

/* warnings and errors */
#define MAX_WARNING_TYPES 3
#define WARN_BLANK_LINES 0
#define WARN_REQUESTED_FIELD_NOT_FOUND 1
#define WARN_INCONSISTENT_NUM_FIELDS 2

#define LINE_BUF_SIZE 65536


typedef struct field {
   int input_pos;
   int output_len;
   int justify;
   int type;  /* CONSTANT, SEQUENCE or CHARACTER */
   char *constant; /* constant data if field is of type CONSTANT */
   int seq_start;
   int seq_incr;
   long seq;
   int zeropad;
   /* char *name; */
} field_type;   

/*
 * globals
 */

/* The total number of fields we expect to find in the input file.  This can be  */
/* set in one of two ways. If no control card is present, we use dmax which will */
/* give us the field count from the first non-blank record.  If a control card   */
/* is used, then we know how many fields there are */
int num_output_fields;                     

field_type fields[MAXNUMBEROFFIELDS];
char *control_card;                /* cc file name */

int layout_as_first_row = FALSE;   /* default is no */
char *out_layout_file;             /* output file for layout */
char *bad_file;                    /* output file for problem records */
int skip_inconsistent_recs=TRUE;   /* skip records with inconsistant number of fields */
int printhex = FALSE;              /* default is no */
char *me;                          /* name of this program */
int col_ruler= FALSE;              /* print the column numbers every col_ruler lines */
int lrecl = LINE_BUF_SIZE;         /* record length default */
int output_delimiter = FALSE;      /* output delimiter with the data */
int verbose = FALSE;               /* verbose warning messages */


/*
 * function prototypes
 */
void print_usage();
void print_help();
int process_file(char *file_name, char delimiter, char field_wrapper);
void print_fixed(char *file_name, char delimiter, char field_wrapper, char *ruler);
void write_layout_file();
int read_control_card(char *control_card);
void build_col_ruler(char *ruler);
int is_ignorable_line(char *s);
static int chomp(char *s);


/*************************************************************************
 * MAIN
 ************************************************************************/
int main(int argc, char *argv[]) {
   int flag;
   char delimiter = '|';       /* default */
   char field_wrapper = '"';   /* not an option */
   char *filename;

   me = argv[0];  /* set the global variable with this programs name */

   if ( argc == 1 ) {
      print_usage();
   }

   /* for backward compatability, capture the -s option, but ignore it. */
   optarg = NULL;
   while ((flag = getopt(argc, argv, "d:lo:b:Sxc:n:r:hs:iv")) != -1) {
      switch(flag) {

         /* delimiter */
         case 'd':
            if (optarg[0] == '\\') {
               if (strlen(optarg) != 2) {
                  fprintf(stderr,"%s: Escape sequence for delimiter is malformed\n",me);
                  print_usage();
               }

               switch(optarg[1]) {
                  case 't':
                     delimiter = '\t';
                     break;

                  default:
                     fprintf(stderr,"%s: Invalid escape sequence for delimiter\n",me);
                     print_usage();
               }
            } 
            else {
               if (strlen(optarg) != 1) {
                  fprintf(stderr,"%s: Delimiter must be a single character\n",me);
                  print_usage();
               }

               delimiter = optarg[0];
            }
            break;

         /* for backward compatability, capture the -s option, but ignore it. */
         case 's':
            fprintf(stderr, "%s: WARNING! You are attempting to use an obsolete option from an older", me); 
            fprintf(stderr, " version of %s!\n%s: Ignoring -s and proceeding...\n",me,me); 
            break;

         /* verbose warning messages */
         case 'v':
            verbose = TRUE;
            break;

         /* layout header is first row in the input file */
         case 'l':
            layout_as_first_row = TRUE;
            break;

         /* file name to put the fixed layout in */
         case 'o':
            out_layout_file = optarg; 
            break;

         /* filename for bad records */
         case 'b':
            bad_file = optarg;
            break;

         /* DO NOT skip records with inconsistant number of fields */
         case 'S':
            skip_inconsistent_recs = FALSE;
            break;

         /* pipe output through James Lemle's prhex */
         case 'x':
            printhex = TRUE; 
            break;

         /* character that separates fields in the output */
         case 'i':
            output_delimiter = TRUE;
            break;

         /* control card file name */
         case 'c':
            control_card = optarg; 
            break;

         /* record length */
         case 'n':
            lrecl = atoi(optarg); 
            if ( ! lrecl ) {
               fprintf(stderr, "%s: Invalid -n record length...\n", me);
               exit(1);
            }
            //lrecl += 5; /* add a few bytes for \r, \n, \0, etc. */ 
            break;

         /* print the column numbers every col_ruler lines */
         case 'r':
            col_ruler = atoi(optarg); 
            break;

         /* print help message */
         case 'h':
            print_help();

         default:
            fprintf(stderr,"%s: Unknown or invalid parameters\n",argv[0]);
            print_usage();
      }
   }

   /* get input file name */
   if ( argc < optind + 1 ) {
      fprintf(stderr, "%s: missing file name...\n", me);
      print_usage();
   } else if ( argc == optind + 1 ) {
      filename = argv[optind];
   } else {
      fprintf(stderr, "%s: too many files on command line...\n", me);
      fprintf(stderr, "%s: I can only process one file at a time\n", me);
      print_usage();
   }

   /* verify required parms */
   if ( strcmp(filename, "-") == 0 ) {
      if ( control_card == NULL ) {
         fprintf(stderr, "%s: input is from stdin, you must provide a control card...\n", me);
         print_usage();
      }
   }

   process_file(filename, delimiter, field_wrapper);
   return(0);

}


/*************************************************************************
 * print the usage statement
 ************************************************************************/
void print_usage() {
   fprintf(stderr, "\n");
   fprintf(stderr, "For help, type: %s -h\n",me);
   fprintf(stderr, "\n");
   exit(1);
}


/*************************************************************************
 * print full help
 *
 * NOTE: Edit the file help.txt, then process it with the mjeffestools util:
 *
 *   txt2printf -p dtof,%s,me < help.txt 
 *
 ************************************************************************/
void print_help() {

   printf("%s version: %s\n\n",me, VERSION);
   /* -------------------------------------------------------------------------------- */
   /* --------------          start output from txt2printf                ------------ */
   /* When invoking from within vim, use :r! txt2printf -p dtof,\%s,me < help.txt      */
   /* -------------------------------------------------------------------------------- */
   printf("\n");
   printf("Name:\n");
   printf("\n");
   printf("   %s - convert files from Delimited To Fixed format\n",me);
   printf("\n");
   printf("Usage:\n");
   printf("\n");
   printf("   %s [options] <file_name|->\n",me);
   printf("\n");
   printf("Options:\n");
   printf("\n");
   printf("   -d delimter    Character used to delimit fields in the input file, \"\\t\" is\n");
   printf("                  Implemented for a tab character, a '|' is the default\n");
   printf("   -i             Include delimiter in the output.\n");
   printf("   -c ctl_card    Control card file name.  See Control Card Description below\n");
   printf("   -o layout_file Put generated layout for the output file in layout_file.\n");
   printf("   -n lrecl       Max length of input record in bytes.  Default is 8192.\n");
   printf("   -r N           Print the column ruler every N lines\n");
   printf("   -x             View output as hex print. (uses James Lemley's prhex -l -w)\n");
   printf("   -l             Layout record is first row, skip it.\n");
   printf("   -v             Print verbose warning messages\n");
   printf("   -h             Print this message\n");
   printf("   input_file|-   Input file name or \"-\" if reading from stdin.  Note that if\n");
   printf("                  reading from stdin, a control card is required, since %s\n",me);
   printf("                  will not be able to make two passes on the input in order to\n");
   printf("                  calculate max widths.\n");
   printf("   -b bad_file    Write bad records to this file.  Normally bad records are\n");
   printf("                  just skipped, but with this option, they will also be\n");
   printf("                  written to this file.  Bad records are those that do not\n");
   printf("                  have a the requested output field (short records), and if\n");
   printf("                  the -I flag is used, will also be records with an\n");
   printf("                  inconsistent number of fields.\n");
   printf("   -S             Try to process records with inconsistent number of fields.\n");
   printf("                  The number of fields in the first non-blank record is used\n");
   printf("                  as a standard, against which all other records are compared.\n");
   printf("                  By default, if any record has a different number of fields,\n");
   printf("                  it will be skipped (if -b is used, written to the bad_file),\n");
   printf("                  and a warning issued at the end of the run.  However, if\n");
   printf("                  this flag is set, %s will check to see if all requested\n",me);
   printf("                  output fields exist on this record and if so, it will\n");
   printf("                  process the record and not skip it.  A warning will still be\n");
   printf("                  issued at the end of the run.\n");
   printf("\n");
   printf("Description:\n");
   printf("\n");
   printf("   %s is a tool that converts delimited files to a fixed width format.  It\n",me);
   printf("   reads from a file or stdin (but there are caveats when reading from stdin,\n");
   printf("   see below) and writes to stdout.  You can supply a control card that\n");
   printf("   defines the output layout. If no control card is supplied, %s will figure\n",me);
   printf("   out the max width of each field by making an initial pass of the file, and\n");
   printf("   optionally write the output layout to a file.  Obviously, this adds to the\n");
   printf("   run time and can be noticable on very large files.  If %s detects that\n",me);
   printf("   output is to a terminal, then the output is paged.  These abilities also\n");
   printf("   make %s a convenient interactive tool for visually inspecting delimited\n",me);
   printf("   files on screen (although paging will wrap long lines, which is ugly).\n");
   printf("\n");
   printf("   For example, to view a pipe delimited file on screen, simply type:\n");
   printf("\n");
   printf("      %s -i file_name\n",me);
   printf("\n");
   printf("   The optional -i includes the delimiter in the output, which helps to\n");
   printf("   visually define columns.\n");
   printf("\n");
   printf("   When using %s to convert files, simply redirect the output to a file.\n",me);
   printf("   For example, to convert a comma delimited file, and let %s figure out the\n",me);
   printf("   layout, use this:\n");
   printf("\n");
   printf("      %s -d , -o layout_file delimited_file > fixed_file\n",me);
   printf("\n");
   printf("   %s can read from stdin if you specify - as the file name, however, you\n",me);
   printf("   must provide a control card when reading from stdin, since %s will not be\n",me);
   printf("   able to make an initial pass of the data to calculate max widths.  For\n");
   printf("   example:\n");
   printf("\n");
   printf("      some_operation | %s -c control_card - > fixed_file\n",me);
   printf("\n");
   printf("   Be sure to read the Warnings and Errors section below for additional\n");
   printf("   information on how %s deals with problems in the input file.\n",me);
   printf("\n");
   printf("Control Card Description:\n");
   printf("\n");
   printf("   Comments are allowed and are anything following a pound (\"#\") sign.\n");
   printf("   Totally blank lines are allowed.  Each non-blank, non-comment line\n");
   printf("   represents a field in the output record.  Output fields can be a constant\n");
   printf("   value, a sequence number or a field from the input record.  Each line is\n");
   printf("   composed of two or three comma separated fields.  The first field contains\n");
   printf("   the input field position to be output or one of the special functions\n");
   printf("   CONSTANT or SEQUENCE described below.  The second field contains the length\n");
   printf("   of this field on the output record.  The optional third field contains one\n");
   printf("   of the formating flags LEFTJUSTIFY or RIGHTJUSTIFY, or the special\n");
   printf("   formating flag for sequences, ZEROPAD.  All fields default to LEFTJUSTIFY.\n");
   printf("   Case is unimportant in control cards except in the value of a CONSTANT.\n");
   printf("   Following is a description of each field type:\n");
   printf("\n");
   printf("   A Standard Input Field Definition:\n");
   printf("\n");
   printf("      input_field_position, length, justify_flag\n");
   printf("\n");
   printf("   Where input_field_possition is the field number on the input record.\n");
   printf("   justify_flag is optional.\n");
   printf("\n");
   printf("   A Constant Field Definition:\n");
   printf("\n");
   printf("      CONSTANT \"value\", length, justify_flag\n");
   printf("\n");
   printf("   Where value is the string to plug in the field.  Value must be enclosed in\n");
   printf("   quotes if the string contains spaces.  On constant fields, the length is\n");
   printf("   optional, if it is not supplied, then the length of the value string is\n");
   printf("   used.  justify_flag is optional.\n");
   printf("\n");
   printf("   A Sequence Field Definition:\n");
   printf("\n");
   printf("      SEQUENCE start increment, length, justify_flag ZEROPAD\n");
   printf("\n");
   printf("   Where start and increment are integers (negative values work as expected).\n");
   printf("   justify_flag and ZEROPAD are optional.\n");
   printf("\n");
   printf("Example Usage:\n");
   printf("\n");
   printf("   To convert the following file (named delim_file):\n");
   printf("\n");
   printf("      123|a|abc|defghi\n");
   printf("      456|a|aaa|bbb \n");
   printf("      789|b|bbbbbbb|c \n");
   printf("\n");
   printf("   Using the following control card (named ctl_card):\n");
   printf("\n");
   printf("      SEQUENCE 1 1, 5, zeropad     # generate a unique id\n");
   printf("      1, 5, righjustify\n");
   printf("      3, 10\n");
   printf("      2, 2    # 2nd field on input will be 4th on output, 2 bytes wide\n");
   printf("      4, 10\n");
   printf("\n");
   printf("   Use this command:\n");
   printf("\n");
   printf("      %s -c ctl_card delim_file > fixed_file\n",me);
   printf("\n");
   printf("   The file fixed_file will then conain:\n");
   printf("\n");
   printf("     00001  123abc       a defghi    \n");
   printf("     00002  456aaa       a bbb       \n");
   printf("     00003  789bbbbbbb   b c         \n");
   printf("\n");
   printf("Warnings and Errors:\n");
   printf("\n");
   printf("   The following describes the way %s handles certain input file problems\n",me);
   printf("   and which command line options affect this behavior.  For each record %s\n",me);
   printf("   reads, it does the following before processing the record:\n");
   printf("\n");
   printf("      - Is the record a blank line? If so skip it (keep count) and issue a\n");
   printf("        warning at the end of the run.  A blank line is one where the newline\n");
   printf("        is the first character in the record.\n");
   printf("      - Is the -l (layout is first row) option set?  If so, skip the first\n");
   printf("        non-blank record.\n");
   printf("      - For the first non-blank (and non-layout) record, save a count of the\n");
   printf("        number of fields.  This will be used to compare against all subsequent\n");
   printf("        records.\n");
   printf("      - Does the max output field requested (in the control card) exist on\n");
   printf("        this record?  If false, and this is the first record, then die with an\n");
   printf("        error message.  If this is not the first record, skip it (keep count)\n");
   printf("        and issue a warning at the end of the run.  If the -b option is used,\n");
   printf("        also write this record to the bad_file.\n");
   printf("      - Is the count of fields in this record the same as the first record?\n");
   printf("        If false, skip it (and write to bad_file if -b is used), keep count\n");
   printf("        and issue a warning at the end of the run about inconsistent number of\n");
   printf("        fields.  However, if the -S option is set, %s will check to see if\n",me);
   printf("        the max output field requested exists on this record, and if it does,\n");
   printf("        it will process the record rather than skipping it.  A warning will\n");
   printf("        stil be issued at the end of the run however.\n");
   printf("\n");
   printf("\n");
   /* ---------------- end output from txt2printf -p dtof,\%s,me < help.txt ----------- */
   printf("%s version: %s\n",me, VERSION);
   printf("svn $Id: dtof.c 345 2020-01-11 16:31:25Z higuito $ \n");
   printf("Report any comments or bugs to Matt Jeffery - mjeffe@yahoo.com\n");
   printf("\n");

   exit(1);
}



/*************************************************************************
 * Processes input records.
 ************************************************************************/
int process_file(char *file_name, char delimiter, char field_wrapper) {
   FILE *infile;
   int *lengths;
   char *ruler; 
   char *buffer;
   int i;
   

   ruler = (char *) malloc(lrecl + 10);
   buffer = (char *) malloc(lrecl + 10);
   if ( ruler == NULL || buffer == NULL ) {
      fprintf(stderr, "%s: Can not allocate %d bytes in process_file\n",me, lrecl+10);
      exit(1); 
   }

   ruler[0] = 0;

   /* calculate (or read from control card) the max lengths for each field */
   if ( control_card ) {
      /* NOTE:  if reading from stdin, a control card must be provided */
      read_control_card(control_card);
   } else {

      /* open input file */
      infile = zfopen(file_name, "r");
      if (infile == NULL) {
         fprintf(stderr, "%s: unable to open %s\n", me, file_name);
         print_usage();
      }

      /* discard the first row if it is the layout header, then */
      /* pass the rest to dmax to calculate max field lengths   */
      buffer[lrecl+1] = 0;
      if ( layout_as_first_row ) {
         fgets(buffer, lrecl + 5, infile);
         if (buffer[lrecl+1] != 0 ) {
            fprintf(stderr, "%s: ERROR - length of layout record is longer than %d bytes.\n", me, lrecl);
            fprintf(stderr, "Try running with a larger -n lrecl value\n");
            exit(1);
         }
      } 

      //lengths = dmax(infile, delimiter, field_wrapper, &num_output_fields, lrecl);
      lengths = dmax(infile, delimiter, field_wrapper, &num_output_fields);

      /* load the fields struct */
      for ( i = 0; i < num_output_fields; i++ ) {
         fields[i].type = CHARACTER;
         fields[i].input_pos = i + 1;
         fields[i].output_len = lengths[i];
         fields[i].justify = LEFTJUSTIFY;
      }

      /* close the input file, so we can reopen it to make the second pass */
      /* zfclose(infile); */   /* bug in zfclose function, use ozfclose for linux */
      ozfclose(infile, file_name);
   }
   
   /* write max lengths to the layout file */
   if ( out_layout_file ) {
      write_layout_file();
   }

   /* build the column ruler string */
   if ( col_ruler )
      build_col_ruler(ruler);
   
   /* print the fields as fixed width, using max lengths */
   print_fixed(file_name, delimiter, field_wrapper, ruler);

   free(ruler);
   free(buffer);

  return(0);
}




/*************************************************************************
 * returns true if the string is a comment line or blank line
 ************************************************************************/
int is_ignorable_line(char *s) {
   char *p = s;

   /* skip leading white space */
   while ( *p == ' ' || *p == '\t' ) 
      p++;

   if ( *p == COMMENT_CHAR || *p == '\n' || *p == '\r' ) 
      return TRUE;

   return FALSE;
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
 * print out the delimited input records as fixed width
 ************************************************************************/
void print_fixed(char *file_name, char delimiter, char field_wrapper, char *ruler) {

   char *inbuf;
   char *inbuf_copy;
   char *outbuf;

   FILE *infile;
   FILE *outfile = stdout;
   FILE *badfile = NULL;   /* good practice to define files as NULL */
   char **parsed_fields;   /* used by parsecsv */
   int field_count = 0;    /* used by parsecsv */
   int i, len = 0;
   char *p;
   char seqstr[22];
   char *pager;
   int max_field_requested = 0;
   int first_row_field_count = 0;
   unsigned long rows = 0;
   unsigned long warnings[MAX_WARNING_TYPES];

   
   inbuf = (char *) malloc(lrecl + 10);
   outbuf = (char *) malloc(lrecl + 10);
   inbuf_copy = (char *) malloc(lrecl + 10);  /* may not need this, but go ahead and malloc */
   if ( inbuf == NULL || outbuf == NULL || inbuf_copy == NULL ) {
      fprintf(stderr, "%s: Unable to allocate memory in print_fixed...\n",me);
      exit(1);
   } 

   for ( i = 0; i < MAX_WARNING_TYPES; i++ ) {
      warnings[i] = 0;
   }

   /* open a pipe to James Lemley's prhex to make the output look pretty */
   if ( printhex ) {
      outfile = popen("prhex -l -w", "w");

      if ( outfile == NULL ) {
         fprintf(stderr, "%s: unable to open pipe to prhex... using stdout instead\n", me);
         outfile = stdout;
      }
   }

#ifdef _UNIX_
   /* IF we are writing to a terminal, pipe output to more */
   else if (isatty(fileno(stdout))) {

      if ( NULL == (pager = getenv("PAGER")))
         pager = "more -d";

      /* removed -e option to more, so it would be compatible with GNU more */
      if ( NULL == (outfile = popen(pager, "w")))
         outfile = stdout;  /* oh well */
   }
#endif

   /* if no control card was provided, this will be the second time we open the file */
   if ( strcmp(file_name, "-") == 0 )
      infile = stdin;
   else
      infile = zfopen(file_name, "r");

   if (infile == NULL) {
      fprintf(stderr, "%s: unable to open %s\n", me, file_name);
      perror("Error oppening input file");
      print_usage();
   }

   if ( bad_file ) {
      badfile = fopen(bad_file, "w");
      if (badfile == NULL) {
         fprintf(stderr, "%s: unable to open %s for writing\n", me, bad_file);
         perror("Error oppening bad_file");
         print_usage();
      }
   }
   
   for ( i = 0; i < num_output_fields; i++ ) {
      if ( fields[i].input_pos > max_field_requested )
         max_field_requested = fields[i].input_pos;
   }

   /*
    * convert each input line to fixed width
    *
    * Here we set lrecl+1 to a null string terminator.  Then we read (up to)
    * lrecl+5. If lrecl+1 is not null after the read, we know that fgets did
    * not find a newline before lrecl bytes, and therefore the line is longer
    * than our buffer.  This technique is slightly faster than looking for a
    * newline in strlen - 1.
    */
   inbuf[lrecl+1] = 0;
   while( fgets(inbuf, lrecl + 5, infile) ) {
      rows++;
      if (inbuf[lrecl+1] != 0 ) {
         fprintf(stderr, "%s: ERROR - length of record %ld is longer than %d bytes.\n", me, rows, lrecl);
         fprintf(stderr, "Try running with a larger -n lrecl value.\n");
         exit(1);
      }

      /* just in case it's bad, and we need to write it out */
      if ( bad_file )
         memcpy(inbuf_copy, inbuf, lrecl);

      chomp(inbuf);

      /* skip blank lines */
      if ( inbuf[0] == '\0' ) {
         warnings[WARN_BLANK_LINES]++;
         continue;
      }

      parsed_fields = parsecsv(inbuf, delimiter, field_wrapper, &field_count);

      if ( field_count == MAXNUMBEROFFIELDS ) {
         fprintf(stderr, "%s: ERROR - reached hard coded limit of %d parsed fields\n", me, MAXNUMBEROFFIELDS);
         fprintf(stderr, "If your input file really has more than %d fields, increase\n", MAXNUMBEROFFIELDS);
         fprintf(stderr, "MAXNUMBEROFFIELDS in parsecsv.h and recompile %s.\n", me);
         exit(1);
      }

      /* the first non-blank record will be my standard */
      if ( first_row_field_count == 0 ) {
         first_row_field_count = field_count;
         if ( field_count < max_field_requested ) {
            fprintf(stderr, "%s: ERROR! A field requested for output is not present on the first record.\n",me);
            fprintf(stderr, "%s: Either the first record is bad or your control card has a problem.\n",me);
            exit(1);
         }
      }
 
      if ( field_count < max_field_requested ) {
         warnings[WARN_REQUESTED_FIELD_NOT_FOUND]++;
         if ( badfile != NULL )
            fputs(inbuf_copy, badfile); 
         continue;
      }

      if ( field_count != first_row_field_count ) {
         warnings[WARN_INCONSISTENT_NUM_FIELDS]++;
         if ( skip_inconsistent_recs ) {
            if ( badfile != NULL )
               fputs(inbuf_copy, badfile); 
            continue;
         }
      }

      /* print ruler every col_ruler lines */
      if ( col_ruler && (rows % col_ruler == 0) && ! printhex ) {
         fprintf(outfile,"%s",ruler);
      }


      /*
       * build the output buffer
       */
      p = outbuf;
      /* memset(outbuf, 0, lrecl); */
      for (i = 0; i < num_output_fields; i++) {

         /* calculate length of data */
         switch ( fields[i].type ) {
            case CHARACTER:
               len = strlen(parsed_fields[fields[i].input_pos - 1]);
               break;
            case CONSTANT:
               len = strlen(fields[i].constant);
               break;
            case SEQUENCE:
               if ( fields[i].zeropad ) {
                  sprintf(seqstr, "%0*ld", fields[i].output_len, fields[i].seq);
               }
               else {
                  sprintf(seqstr, "%ld", fields[i].seq);
                  len = strlen(seqstr);
               }
               break;
         }

         /* adjust len if data is longer that we want to output */
         if ( len > fields[i].output_len )
            len = fields[i].output_len;

         /* lpad with spaces if RIGHTJUSTIFIED */
         if ( fields[i].justify == RIGHTJUSTIFY ) {
            memset(p, ' ', fields[i].output_len - len);
            p += fields[i].output_len - len;
         }

         /* add the data to buffer */
         switch ( fields[i].type ) {
            case CHARACTER:
               memcpy(p, parsed_fields[fields[i].input_pos - 1], len);
               p += len;
               break;
            case CONSTANT:
               memcpy(p, fields[i].constant, len);
               p += len;
               break;
            case SEQUENCE:
               memcpy(p, seqstr, len);
               p += len;
               fields[i].seq += fields[i].seq_incr;
               break;
         }

         /* rpad if LEFTJUSTIFY */
         if ( fields[i].justify != RIGHTJUSTIFY ) {
            memset(p, ' ', fields[i].output_len - len);
            p += fields[i].output_len - len;
         }

         /* put delimiter between each field */
         if ( output_delimiter ) {
            if ( i < num_output_fields - 1 ) /* but not after the last field */
               memset(p++, delimiter, 1);
         }
      }
 
      memcpy(p, "\n\0", 2);
      fputs(outbuf, outfile); 
   }

   if (outfile != stdout) {
      pclose(outfile);
   }

   if ( badfile != NULL ) {
      fclose(badfile);
      badfile = NULL;
   }

   /* zfclose(infile); */   /* bug in zfclose function, use ozfclose for linux */
   ozfclose(infile, file_name);
   free(inbuf);
   free(outbuf);
   free(inbuf_copy);


   for (i = 0; i < MAX_WARNING_TYPES; i++) {
      if ( warnings[i] > 0 ) {
         switch ( i ) {
            case WARN_BLANK_LINES:
               fprintf(stderr, "%s: WARNING! %ld blank records were skipped\n", me, warnings[i]);
               break;
            case WARN_REQUESTED_FIELD_NOT_FOUND:
               fprintf(stderr, 
                     "%s: WARNING! %ld records were skipped where the requested output field could not be found\n", 
                     me, warnings[i]);
               break;
            case WARN_INCONSISTENT_NUM_FIELDS:
               fprintf(stderr, "%s: WARNING! %ld records were ", me, warnings[i]);
               if ( skip_inconsistent_recs )
                  fprintf(stderr, "skipped");
               else
                  fprintf(stderr, "encountered");
               fprintf(stderr, " which did not have the same number of fields as\n");
               fprintf(stderr, "the first non-blank record in the file.  ");
               if ( ! skip_inconsistent_recs ) {
                  fprintf(stderr, "They were not skipped, since -S was used and all\n"); 
                  fprintf(stderr, "requested output fields were present on the record.\n"); 
               }
               fprintf(stderr, "\n");
               break;
         }
      }
   }

}



/*************************************************************************
 * build the column number ruler string
 ************************************************************************/
void build_col_ruler(char *ruler) {

   char str[11];
   int cols = 0;    /* number of columns */
   int i;
   int j;

   
   /* add up the total number of columns */
   for (i = 0; i < num_output_fields - 1; i++) {
      cols += fields[i].output_len;

      if ( output_delimiter )
         cols++;  /* add the delimiter */
   }

   /* no sparator after the last field */
   cols += fields[i].output_len;

   /*
    * every 10 columns print the column number, with the appropriate number of dashes 
    */

   /* set the first ten since they are a little funky */
   strcat(ruler,"1---|---10");

   for (i = 5; i < cols - 5; i += 5) {
      if ( i <= 10 ) {
         continue;  /* they are already built */
      }

      /* put a number every 10 columns, and a | every 5 */
      if ( i % 10 == 0 ) {
      
      if ( i < 100 ) {
         sprintf(str,"---%d",i);
         strcat(ruler, str);
      }
      else if ( i < 1000 ) {
         sprintf(str,"--%d",i);
         strcat(ruler, str);
      }
      else if ( i < 10000 ) {
         sprintf(str,"-%d",i);
         strcat(ruler, str);
      }
      /* can't do it (don't mess with it!) if the column number is more than 4 digits */
      else if ( i > 9999 ) {
         strcpy(ruler,"");
         col_ruler = 0;
      }
      } else {
         strcat(ruler, "----|");
      }
   }

   /*
    * add the last number to the ruler 
    */
   
   /* set i = to the end of the ruler + 1 */
   /* i -= 9; */
   i = strlen(ruler) + 1;

   /* add the appropriate number or dashes to the end of the string */
   for ( j = i; j < cols + 1; j++ ) {
      if ( j % 5 == 0 ) {
         strcat(ruler,"|");
      } else {
         strcat(ruler,"-");
      }
   }

   /* back up the number of digits in cols */
   if ( cols < 100 ) {
      ruler[strlen(ruler)-2] = 0;
   }
   else if ( cols < 1000 ) {
      ruler[strlen(ruler)-3] = 0;
   }
   else if ( cols < 10000 ) {
      ruler[strlen(ruler)-4] = 0;
   }

   /* now put the cols number back in the string as the final number */
   sprintf(str,"%d\n",cols);
   strcat(ruler, str);
}




/*************************************************************************
 * write the layout to the layout file 
 * start, end, length
 ************************************************************************/
void write_layout_file() {

   FILE *layoutf;
   int i;
   int start, end, len;

   layoutf = fopen(out_layout_file, "w");
   if (layoutf == NULL) {
      fprintf(stderr, "%s: unable to open %s for writing\n", me, out_layout_file);
      print_usage();
   }
   
   /* start, end, length */    
   start = 1;
   end = 0;
   for ( i = 0; i < num_output_fields; i++ ) {
      len = fields[i].output_len;
      if ( output_delimiter )
         len++;
      end += len;
      fprintf(layoutf, "%d, %d, %d\n", start, end, len);
      start += len; 
   }

   fclose(layoutf);
}




/*************************************************************************
# each non-blank or non-comment line represents a field in the output
# the first column is the field number on the input, the second parameter
# is the length to be used for that field in the output.
# the first non-blank or non-comment line will determine what kind of
# control card is being used.  the alternative is a single column control
# card where the input record possitions directly correspond with the
# positions, only a length is provided (see read_layout_file above).  This 
# is for backwards compatability with previous versions.  Note that in the
# old style control card, CONSTANT and SEQUENCE are not supported!
# The third column could be options, but currently only the justify options                                                       
# are supported. Defaults to LEFTJUSTIFY.

# example

CONSTANT 100    # must be one word, no spaces, can optionally add a field length ( , 6 for example)
SEQUENCE 100000 1, 16, RIGHTJUSTIFY ZEROPAD   # start/increment, optional ZEROPAD
1, 10, RIGHTJUSTIFY
2, 2           # all remaining fields default to LEFTJUSTIFY
3, 15
5, 10          # note input field 5 being output here (out of order)
4, 10


# This would process the first five fields on the input file and output
# a file 66 byes wide (not including the \n) with seven fields.

 ************************************************************************/
int read_control_card(char *control_card) {

   FILE *infile;
   char buffer[1024];
   char *p;
   char *words[10];
   int numwords;
   int linenum = 0, i = 0;


   /* open control card file */
   infile = fopen(control_card, "r");
   if (infile == NULL) {
      fprintf(stderr, "%s: unable to open control card file %s\n", me, control_card);
      print_usage();
   }

   num_output_fields = 0;
   memset(fields, 0, sizeof(fields));

   while( fgets(buffer, sizeof(buffer), infile) ) {

      chomp(buffer); /* remove line terminator */
      p = buffer;
      linenum++;
      
      numwords = 0;
      memset(words, 0, sizeof(words));
         
      /* parse each line into words */
      while ( *p != 0 && numwords < (sizeof (words) / sizeof(char *)) ) {

         /* skip white space */         
         while ( *p == ' ' || *p == '\t' || *p == ',' ) 
            p++;

         if ( *p == COMMENT_CHAR || *p == 0 )
            break;   /* remaining line a comment, or end of line */

         /* if it is a quoted word, such as in a CONSTANT 'asfd' deal with it */
         if ( *p == '\'' || *p == '\"' ) {
            p++;      /* skip the quote character */
      
            words[numwords++] = p;  /* start word */

            /* advance to next quote character */
            while ( *p != '\'' && *p != '\"')
               p++;

         } else {
            words[numwords++] = p;

            /* advance to next white space or end of line */
            while ( *p != 0 && *p != ' ' && *p != '\t' && *p != ',' && *p != '\'' && *p != '\"')
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

      /* line is parsed into words, now fill the fields struct */

      /* check for too many fields */
      if ( num_output_fields > MAXNUMBEROFFIELDS - 1 ) {
         fprintf(stderr, "%s: control card has too many input fields. %s has a hard coded limit of %d fields\n",
               me, me, MAXNUMBEROFFIELDS);
         fprintf(stderr, "If your input file really has more than %d fields, increase\n", 
               MAXNUMBEROFFIELDS);
         fprintf(stderr, "MAXNUMBEROFFIELDS in parsecsv.h and recompile %s.\n", me);
         exit(1);
      }

      /* field possition definition */
      if ( words[0] != NULL && words[1] != NULL 
            && atoi(words[0]) && atoi(words[1]) ) {

         fields[num_output_fields].type = CHARACTER; 
         fields[num_output_fields].input_pos = atoi(words[0]);
         fields[num_output_fields].output_len = atoi(words[1]);

         /* too many fields could also be triggered by requesting a field position larger than MAXNUMBEROFFIELDS */
         if ( fields[num_output_fields].input_pos > MAXNUMBEROFFIELDS ) {
            fprintf(stderr, "%s: requested input field position %d is larger than my limit of %d fields\n",
                  me, fields[num_output_fields].input_pos, MAXNUMBEROFFIELDS);
            fprintf(stderr, "If your input file really has more than %d fields, increase\n", 
                  MAXNUMBEROFFIELDS);
            fprintf(stderr, "MAXNUMBEROFFIELDS in parsecsv.h and recompile %s.\n", me);
            exit(1);
         }

         if ( words[2] == NULL )
            fields[num_output_fields].justify = LEFTJUSTIFY;  /* default to left */
         else if ( strcasecmp(words[2], "leftjustify" ) == 0 )
            fields[num_output_fields].justify = LEFTJUSTIFY;
         else if ( strcasecmp(words[2], "rightjustify" ) == 0 )
            fields[num_output_fields].justify = RIGHTJUSTIFY;
      }
            
      /* CONSTANT */
      else if ( strcasecmp(words[0], "constant") == 0 && words[1] != NULL ) {
         fields[num_output_fields].type = CONSTANT; 

         fields[num_output_fields].constant = (char *) malloc (strlen(words[1]) + 1);
         if (fields[num_output_fields].constant == NULL) {
            fprintf(stderr, "%s: Can't allocate memory in read_control_card\n", me);
            exit(1);
         }
         strcpy(fields[num_output_fields].constant, words[1]);

         if ( words[2] == NULL )
            fields[num_output_fields].output_len = strlen(fields[num_output_fields].constant);  /* default to length of the constant */
         else
            if ( atoi(words[2]) )
               fields[num_output_fields].output_len = atoi(words[2]);
            else {
               fprintf(stderr, "%s: malformed control card line: %d\n",me, linenum);
               print_usage();
            }

         if ( words[3] == NULL )
            fields[num_output_fields].justify = LEFTJUSTIFY;  /* default to left */
         else if ( strcasecmp(words[3], "leftjustify" ) == 0 )
            fields[num_output_fields].justify = LEFTJUSTIFY;
         else if ( strcasecmp(words[3], "rightjustify" ) == 0 )
            fields[num_output_fields].justify = RIGHTJUSTIFY;
         else {
            fprintf(stderr, "%s: malformed control card line: %d\n",me, linenum);
            print_usage();
         }
      }

      /* SEQUENCE */
      else if ( strcasecmp(words[0], "sequence") == 0 
                && words[1] != NULL 
                && words[2] != NULL 
                && words[3] != NULL
                && atoi(words[1]) != (int)NULL 
                && atoi(words[2])
                && atoi(words[3]) != (int)NULL) {
         fields[num_output_fields].type = SEQUENCE; 
         fields[num_output_fields].seq_start = atoi(words[1]);
         fields[num_output_fields].seq_incr = atoi(words[2]);
         fields[num_output_fields].seq = fields[num_output_fields].seq_start;
         fields[num_output_fields].output_len = atoi(words[3]);

         /* set defaults */
         fields[num_output_fields].justify = LEFTJUSTIFY;
         fields[num_output_fields].zeropad = 0;

         /* check for any ramaining special instructions */
         i=4;
         while ( words[i] != NULL ) {
            if ( strcasecmp(words[i], "leftjustify" ) == 0 )
               fields[num_output_fields].justify = LEFTJUSTIFY;
            else if ( strcasecmp(words[i], "rightjustify" ) == 0 )
               fields[num_output_fields].justify = RIGHTJUSTIFY;
            else if ( strcasecmp(words[i], "zeropad" ) == 0 )
               fields[num_output_fields].zeropad = ZEROPAD;
            else {
               fprintf(stderr, "%s: malformed control card line: %d\n",me, linenum);
               print_usage();
            }

            i++;
         }
      }

      else {
         fprintf(stderr, "%s: malformed control card line: %d\n",me, linenum);
         print_usage();
      }

      num_output_fields++;
   }  /* end of while that reads control card lines */

   fclose(infile);

#ifdef DEBUG
   for ( i=0; i<num_output_fields; i++ ) {
      printf("Debug info...\n");
      printf("line %d:\n", i + 1);
      printf("     type: %d\n", fields[i].type);
      printf("  justify: %d\n", fields[i].justify);
      printf("   length: %d\n", fields[i].output_len);
      //printf("possition: %d\n", fields[i].input_pos);
   }
#endif

   return num_output_fields;
}


