/*****************************************************************************
 *
 * $Id: rrvalue.c 25 2011-03-09 23:15:52Z mjeffe $
 *
 * DESCRITION
 *   reads records from a file, or stdin and writes them to multiple output
 *   files, based on the value in a field.
 * 
 * NOTE
 *   zfopen, parsecsv, and crc are part of James Lemley's jamestools
 *
 * TODO:
 *   - have this create a new output file for every new value found in a field
 *
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "zfopen.h"
#include "parsecsv.h"
#include "crc.h"

#define VERSION "0.3"

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#define BUFSZ 65536
#define MAX_OUTFILES 128

/*
 * globals
 */
char *me;                       /* name of this program */
FILE *infile = NULL; 
FILE *outfiles[MAX_OUTFILES];
int num_outfiles = 0;
int startp, endp, fieldp;       /* start possition, end possition and field possition of data value */
char delimiter = ',';
char wrapper = '"';             /* aka fielder */
int debug = 0;                  /* debug level */
int sofar = 0;
int use_crc = 0;


/*
 * function prototypes
 */
void print_usage();
void open_files(char *infname, char **outfnames);
void close_files(char *infname, char **outfnames);
int process_input();
static int chomp(char *s);




/*************************************************************************
 * print the usage statement
 ************************************************************************/
void print_usage() {
   fprintf(stderr, "\n");
   fprintf(stderr, "Description:\n"); 
   fprintf(stderr, "  %s distributes records from a sorted input file (or stdin) to the\n",me);
   fprintf(stderr, "  output files, by using a round robin scheme, keeping all records with\n");
   fprintf(stderr, "  the same value in the defined field together.  Input must be sorted\n");
   fprintf(stderr, "  if not spliting on the CRC of the key value.\n"); 
   fprintf(stderr, "\n"); 
   fprintf(stderr, "Usage:\n"); 
   fprintf(stderr, "  %s -p start:end [-c] -i infile [or - for stdin] outfile1 outfile2 ...\n", me); 
   fprintf(stderr, "  %s -f field [-d delimiter] [-w field_wrapper ] [-c] -i infile [or - for stdin] outfile1 outfile2 ...\n", me); 
   fprintf(stderr, "\n"); 
   fprintf(stderr, "Switches:\n"); 
   fprintf(stderr, "  -d delimter       Character used to delimit fields in the input file, \'\\t\' is\n");
   fprintf(stderr, "                    implemented for a tab character, a \',\' is the default.\n");
   fprintf(stderr, "  -w wrapper        Character used to wrap delimited fields which contain an embedded delimiter, default is \'\"\'\n");
   fprintf(stderr, "  -f field          Field position (for delimited file) which containes the value to split on.\n");
   fprintf(stderr, "  -p start:end      Start and end possitions (in a fixed width file) which contains the value to split on.\n");
   fprintf(stderr, "  -i input          Sorted input data file or - for stdin.\n");
   //fprintf(stderr, "  -v value:file     Value is the string of data which, if found will cause the record to be written\n");
   //fprintf(stderr, "                    to the outfile.  \n");
   //fprintf(stderr, "  -o default_file   Any record which does not contain one of the predefined values will be written to\n");
   //fprintf(stderr, "                    this file.\n");
   fprintf(stderr, "  -c                Split records based on a CRC of the value key.  This does a mod with the\n");
   fprintf(stderr, "                    number of output files to keep all records with the same CRC together.\n");
   fprintf(stderr, "                    Note that the input file does not need to be sorted to use this feature.\n");
   fprintf(stderr, "  -g debug_level    The higher the number, the more verbose the output messages.\n");
   fprintf(stderr, "  -s n              Sofar value, prints message every n records processesed.\n");
   fprintf(stderr, "  -1                Disables the check condition when only one output file will be used.\n");
   fprintf(stderr, "                    Obviously this is a silly request, but it is helpful when building\n");
   fprintf(stderr, "                    the %s command line dynamically.\n", me);
   fprintf(stderr, "\n"); 
   fprintf(stderr, "%s version: %s\n",me, VERSION);
   fprintf(stderr, "cvs $Id: rrvalue.c 25 2011-03-09 23:15:52Z mjeffe $ \n");
   fprintf(stderr, "\n"); 
   fprintf(stderr, "Report any comments or bugs to Matt Jeffery - mjeffe@acxiom.com\n");
   fprintf(stderr, "\n"); 
   exit(1); 
}


/*************************************************************************
 * MAIN
 ************************************************************************/
int main(int argc, char **argv) { 
   int flag, j, n;
   int one_input = 0;
   char *infname;
   char *outfnames[MAX_OUTFILES];
   int ret;

   me = argv[0];

   if ( argc < 2 )
      print_usage();

   optarg = NULL;
   while ((flag = getopt(argc, argv, "d:w:f:p:i:g:s:1ch?")) != -1) {
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

         /* wrapper */
         case 'w':
            if (strlen(optarg) != 1) {
               fprintf(stderr,"%s: Wrapper must be a single character\n",me);
               print_usage();
            }

            wrapper = optarg[0];
            break;

         case 'p':
            if ( ! strchr(optarg,':') ) {
               fprintf(stderr, "%s: Malformed possition argument for -p option, start and end must be separated with a :\n", me);
               exit(1);
            }

            startp = atoi((char *)strtok(optarg, ":"));
            endp   = atoi((char *)strtok(NULL, ":"));
            if ( ! startp || ! endp ) {
               fprintf(stderr, "%s: Malformed possition argument for -p option, start and end must be integers\n", me);
               exit(1);
            }
            break;

         case 'f':
            fieldp = atoi(optarg);
            if ( ! fieldp ) {
               fprintf(stderr, "%s: Malformed argument for -f option, field possition must be an integer\n", me);
               exit(1);
            }
            break;

         case 'i':
            infname = optarg;
            break;

         case 'g':
            debug = atoi(optarg);
            break;

         case 's':
            sofar = atoi(optarg);
            break;

         case 'c':
            use_crc = 1;
            break;

         case '1':
            one_input = 1;
            break;

         case 'h':
            print_usage();
            break;
         
         case '?':
            print_usage();
            break;

         default:
            fprintf(stderr,"%s: Unknown or invalid parameters\n",me);
            exit(1);

      }
   }

   /* get output file names */
   for (n = 0; n < (argc - optind); n++) 
   { 
      outfnames[n] = (char *)malloc(strlen(argv[optind + n]) + 2);
      if ( outfnames[n] == NULL ) {
         fprintf(stderr, "%s: Unable to allocate memory in main...\n",me);
         exit(1);
      }
      strcpy(outfnames[n], argv[optind + n]);
      num_outfiles++;
   } 


   /* 
    * validate command line opts 
    */

   /* don't mix delimited and fixed options */
   if ( (startp && endp) && fieldp ) {
      fprintf(stderr, "%s: conflicting options, use either -f for delimited input or -p for fixed width input, not both\n",me);
      exit(1);
   }

   if ( ! infname ) {
      fprintf(stderr, "%s: missing required input file (-i) option\n", me);
      exit(1);
   }

   /* if one_input is set, then disable this check */
   if ( ! one_input ) {
      if ( num_outfiles < 2 ) {
         fprintf(stderr, "%s: you must specify more than one output file, or -1\n", me);
         exit(1);
      }
   }


   /* print init info */
   if ( debug >= 3 ) {
      fprintf(stderr, "%s: init finished, command line args are:\n",me);

      if ( fieldp )
         fprintf(stderr, "  delimited input file: field possition = %d, delimiter = \'%c\' wrapper = \'%c\'\n",fieldp,delimiter,wrapper);
      else
         fprintf(stderr, "  fixed width input: start possition = %d, end possition = %d\n",startp, endp);

      fprintf(stderr, "  output files are: ");
      for (j=0; j < num_outfiles; j++) {
         fprintf(stderr,"%s",outfnames[j]);
         if ( j != num_outfiles - 1 )
            fprintf(stderr, ", ");
      }
      fprintf(stderr, "\n");

      if ( strcmp(infname,"-") == 0 )
         fprintf(stderr, "  processing input from stdin\n");
      else
         fprintf(stderr, "  processing input file %s\n",infname);

      if ( use_crc )
         fprintf(stderr, "  using CRC hash value on key field to distribute records\n");

      if ( sofar )
         fprintf(stderr, "  sofar = %d\n",sofar);
   }


   /* do the hokey-pokey */
   open_files(infname, outfnames);
   ret = process_input();
   close_files(infname, outfnames);
   
   exit(ret); 
}
   

/*************************************************************************
 * Open all files
 ************************************************************************/
void open_files(char *infname, char **outfnames) {

   int i = 0;

   if ( debug >= 3 )
      fprintf(stderr, "%s: opening all files...\n",me);

   /* input file */
   if ( strcmp(infname, "-") == 0 ) 
      infile = stdin; 
   else
      infile = zfopen(infname, "r"); 
   if ( ! infile ) { 
      fprintf(stderr, "%s: Can't open input file %s\n", me, infname); 
      exit(1); 
   }
   
   /* output files */
   for (i = 0; i < num_outfiles; i++) { 
      if (NULL == (outfiles[i] = zfopen(outfnames[i], "w"))) { 
         fprintf(stderr, "%s: Can't open output file %s\n", me, outfnames[i]); 
         exit(1); 
      }
   } 
}


/*************************************************************************
 * close all files
 ************************************************************************/
void close_files(char *infname, char **outfnames) {

   int i = 0;

   if ( debug >= 3 )
      fprintf(stderr, "%s: closing all files...\n",me);

   if ( infile != stdin ) 
      ozfclose(infile,infname); 

   for (i = 0; i < num_outfiles; i++) {
      ozfclose(outfiles[i],outfnames[i]); 
   }
}


/*************************************************************************
 * round robin the input record to the appropriate output file, based on
 * the values found in the input.
 ************************************************************************/
int process_input() {
   int i = 0, recnum = 0;
   char *buffer;
   char *buffercpy;        /* parsecsv destroys the buffer, so I need to save a copy I can output */
   char value[2048];       /* accomodate value string up to 2k bytes */
   char last_value[2048];  /* accomodate value string up to 2k bytes */
   int valuelen;
   char **parsed_fields;   /* used by parsecsv */
   int field_count = 0;    /* used by parsecsv */
   int sofarcnt = 0;
   time_t ltime;
   char mytime[1024];
   unsigned long crc;
   //static char crchex[9];

   if ( debug >= 3 )
      fprintf(stderr, "%s: processing input...\n",me);

   buffer = (char *) malloc(BUFSZ); 
   buffercpy = (char *) malloc(BUFSZ); 
   if ( buffer == NULL || buffercpy == NULL ) {
      fprintf(stderr, "%s: Unable to allocate memory in process_input...\n",me);
      return(1);
   }

   /* calculate the length of the value field for fixed input */
   if ( ! fieldp ) {
      valuelen = endp - startp + 1;
      if ( debug >= 3 )
         fprintf(stderr, "%s: valuelen = %d\n",me, valuelen);
   }

   /* read input records and send to appropriate output file */
   while (fgets(buffer, BUFSZ, infile)) { 
      recnum++;
      sofarcnt++;

      /* 
       * get the value from the input data 
       */

      /* delimited */
      if ( fieldp ) {
         strcpy(buffercpy, buffer);
         chomp(buffercpy);
         parsed_fields = parsecsv(buffercpy, delimiter, wrapper, &field_count);

         /* check to make sure the field we want exists */
         if ( field_count < fieldp ) {
            fprintf(stderr, "%s: ERROR, field %d does not exist on input record %d\n", me, fieldp, recnum);
            fprintf(stderr, "  record %d:\n", recnum);
            fprintf(stderr, "%s\n",buffer);
            return(1);
         }

         strcpy(value, parsed_fields[fieldp - 1]);

      /* fixed */
      } 
      else {
         /* check to make sure the field we want exists */
         if ( strlen(buffer) < endp ) {
            fprintf(stderr, "%s: ERROR, input record %d is shorter than end possition %d\n", me, recnum, endp);
            fprintf(stderr, "  record %d:\n", recnum);
            fprintf(stderr, "%s\n",buffer);
            return(1);
         }

         strncpy(value, buffer + startp - 1, valuelen);
      }

      if ( debug >= 4 )
         fprintf(stderr, "%s: input record: %d, value found = %s\n",me, recnum, value);

      /* 
       * write this record to the appropriate output file
       */

      /* If using CRC:
       *
       * hash the key value field and use the mod of the number of output files to decide
       * which one to put this record in.
       */
      if ( use_crc ) {
         /* for delimited data, we have not calculated the string lenght */
         if ( fieldp ) valuelen = strlen(value);
         crc = crc32(value, valuelen);
         i = (unsigned int)crc % num_outfiles;
         if ( debug >= 4 )
            printf("value: %s, valuelen: %d, crc: %ld, i: %d\n", value, valuelen, crc, i);
      }

      /* If using round robin:
       *
       * input records need to be sorted, so we move to the next file if the value changes
       */
      else {
         if ( strcmp(value, last_value) != 0 ) {
            strcpy(last_value, value);
            i++;
            if (i == num_outfiles)    
               i = 0; 
         }
      }

      if ( i < 0 || i >= num_outfiles ) {
         fprintf(stderr, "%s: ERROR, mod of crc is out of bounds (%d)\n", me, i);
         return(1);
      }

      fputs(buffer, outfiles[i]); 

      if ( sofar && sofarcnt >= sofar ) {
         time(&ltime);
         strcpy(mytime, ctime(&ltime));
         chomp(mytime);
         //fprintf(stderr, "%d rows read at %s",recnum, ctime(&ltime));
         fprintf(stderr, "%s: <%s> records read so far: %d\n",me, mytime, recnum);
         fflush(stderr);
         sofarcnt = 0;
      }
   }

   return(0);
}

/*************************************************************************
 * remove newline and/or carriage return from end of a string 
 * return length of new modified string 
 ************************************************************************/
static int chomp(char *s) {
   char *p;

   p = s + strlen(s) - 1; 

   while (p >= s && (*p == '\n' || *p == '\r'))
      *(p--) = '\0';

   return (p - s + 1);
}

