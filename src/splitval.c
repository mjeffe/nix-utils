/*****************************************************************************
 *
 * $Id: splitval.c 25 2011-03-09 23:15:52Z mjeffe $
 *
 * DESCRITION
 *   reads records from a file, or stdin and writes them to multiple output
 *   files, based on the value in a field.
 * 
 * NOTE
 *   - zfopen is part of James Lemley's jamestools.
 *   - Much of this code was taken from rrvalue program in the 
 *     sawmill/mjeffestools project.
 *
 * DEBUG VALUES
 *   - 1 - init messages
 *   - 2 - events that happen only occasionally, such as oppening a new file
 *   - 3 - unused
 *   - 4 - events that happen for each record
 *   - 5 - more verbose per record messages
 *
 * TODO:
 *   - have this create a new output file for every new value found in a field
 *   
 * BUGS:
 *
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "zfopen.h"
#include "parsecsv.h"

/* on AIX, getopt is part of unistd.h, but getopt_long is not */
/* availabe anyway, so this remains unported */
#include <getopt.h>

#define VERSION "0.2" /* version number of this release */
#define PROGRAM_NAME "splitval"

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

/* buffer sizes */
#define VALUE_BUFSZ 5120      /* buffer size for the key value string */
#define LINE_BUFSZ 65536      /* buffer size for input file record */
#define GROUP_BUFSZ 314572800 /* allocate group buffer in 300Meg chunks */

/* default number of digits to start with on output file names */
#define SUFFIX_DIGITS 2

/* The default prefix for output file names. */
char DEFAULT_PREFIX[] = "xx";

/* used with limit_type */
#define BYTES_LIMIT 1
#define LINES_LIMIT 2

typedef struct group_buffer {
   char *buff;
   char *end;
   int buff_size;
   long long count;
   long long largest_group;  /* for sofar output */
} group_buffer_type;

/* command line opts */
static struct option const longopts[] =
{
  {"suffix-digits", required_argument, NULL, 'n'},
  {"bytes", required_argument, NULL, 'b'},
  {"lines", required_argument, NULL, 'l'},
  {"delimiter", required_argument, NULL, 'd'},
  {"wrapper", required_argument, NULL, 'w'},
  {"field", required_argument, NULL, 'f'},
  {"position", required_argument, NULL, 'p'},
  {"prefix", required_argument, NULL, 'x'},
  {"debug", required_argument, NULL, 'g'},
  {"sofar", required_argument, NULL, 's'},
  {"no-sort-check", no_argument, NULL, 'c'},
  {"string-sort-check", no_argument, NULL, 'a'},
  {"gzip-output", no_argument, NULL, 'z'},
  {"help", no_argument, NULL, 'h'},
  {NULL, 0, NULL, 0}
};

char *me;                       /* name of this program */
int startp, endp, fieldp;       /* start possition, end possition and field possition of data value */
char delimiter = ',';
char wrapper = '"';             /* aka fielder */
int debug = 0;                  /* debug level */
long sofar = 0;
int suffix_digits;
long long max_limit;   /* used with -b or -l to limit output file size */
int limit_type;        /* bytes or lines */
char limit_type_str[20];
short sort_check = TRUE;
short string_sort_check = FALSE;
short gzip_output = FALSE;

/*
 * function prototypes
 */
void print_usage();
FILE * open_input(char *fname);
FILE * open_output_file(char *fname);
char * get_next_filename(char *prefix);
void close_file(char *fname, FILE *file);
void process_input();
static int chomp(char *s);
char * get_value(char *line, long recnum, char *value, int max_valuelen);
long long max_bytes_strtoll(char *str);
long long max_lines_strtoll(char *str);
void add_to_groupbuff(group_buffer_type *gb, char *line);
static char * get_time(void);




/*************************************************************************
 * MAIN
 ************************************************************************/
int main(int argc, char **argv) { 
   int flag, j, n;
   char *infname;
   char *prefix = NULL;

   /* me = argv[0]; */
   me = PROGRAM_NAME;

   if ( argc < 2 )
      print_usage();

   optarg = NULL;
   while ((flag = getopt_long(argc, argv, "n:b:l:d:w:f:p:x:g:s:hcaz", longopts, NULL)) != -1) {
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
               fprintf(stderr, "%s: Malformed argument for -p|--position option, start and end must be separated with a :\n", me);
               exit(1);
            }

            startp = atoi((char *)strtok(optarg, ":"));
            endp   = atoi((char *)strtok(NULL, ":"));
            if ( ! startp || ! endp ) {
               fprintf(stderr, "%s: Malformed argument for -p|--position option, start and end must be integers\n", me);
               exit(1);
            }
            break;

         case 'f':
            fieldp = atoi(optarg);
            if ( ! fieldp ) {
               fprintf(stderr, "%s: Malformed argument for -f|--field option, field possition must be an integer\n", me);
               exit(1);
            }
            break;

         case 'x':
            prefix = optarg;
            break;

         case 'b':
            max_limit = max_bytes_strtoll(optarg);
            limit_type = BYTES_LIMIT;
            strcpy(limit_type_str,"bytes");
            if ( ! max_limit ) {
               fprintf(stderr, "%s: Malformed argument for -b|--bytes option\n", me);
               exit(1);
            }
            break;

         case 'l':
            max_limit = max_lines_strtoll(optarg);
            limit_type = LINES_LIMIT;
            strcpy(limit_type_str,"lines");
            if ( ! max_limit ) {
               fprintf(stderr, "%s: Malformed argument for -l|--lines option\n", me);
               exit(1);
            }
            break;

         case 'n':
            suffix_digits = atoi(optarg);
            if ( ! suffix_digits ) {
               fprintf(stderr, "%s: Malformed argument for -n|--suffix-digits option\n", me);
               exit(1);
            }
            break;

         case 'g':
            debug = atoi(optarg);
            if ( ! debug ) {
               fprintf(stderr, "%s: Malformed argument for -g|--debug option\n", me);
               exit(1);
            }
            break;

         case 's':
            /* sofar = atol(optarg); */
            sofar = (long)max_lines_strtoll(optarg);
            if ( ! sofar ) {
               fprintf(stderr, "%s: Malformed argument for -s|--sofar option\n", me);
               exit(1);
            }
            break;

         case 'c':
            sort_check = FALSE;
            break;

         case 'a':
            string_sort_check = TRUE;
            break;

         case 'z':
            gzip_output = TRUE;
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

   /* get input file */
   for (n = 0; n < (argc - optind); n++) 
   { 
      infname = (char *)malloc(strlen(argv[optind + n]) + 2);
      if ( infname == NULL ) {
         fprintf(stderr, "%s: Unable to allocate memory in main...\n",me);
         exit(1);
      }
      memset(infname, 0, strlen(argv[optind + n]) + 2 );

      if ( argv[optind + n] ) {
         strcpy(infname, argv[optind + n]);
      } else {
         fprintf(stderr, "%s: no input file supplied.\n", me);
         exit(1);
      }
   } 


   /* 
    * validate command line opts 
    */

   /* don't mix delimited and fixed options */
   if ( (startp && endp) && fieldp ) {
      fprintf(stderr, "%s: conflicting options, use either -f|--field for delimited input\n",me);
      fprintf(stderr, "  or -p|--position for fixed width input, not both\n",me);
      exit(1);
   }

   if ( ! max_limit ) { 
      fprintf(stderr, "%s: missing option, you need to provide one of -b|--bytes or -l|--lines\n",me);
      exit(1);
   }

   /*
   if ( max_bytes && max_lines ) { 
      fprintf(stderr, "%s: you can only specify one of -b|--bytes or -l|--lines\n",me);
      exit(1);
   }
   */

   /*
    * set defaults
    */

   if ( ! suffix_digits ) { 
      suffix_digits = SUFFIX_DIGITS;
   }

   if ( ! prefix ) { 
      prefix = DEFAULT_PREFIX;
   }


   /*
    * print init info
    */

   if ( debug >= 1 ) {
      fprintf(stderr, "%s: init finished, command line args are:\n",me);

      if ( fieldp ) {
         fprintf(stderr, "  delimited input file: field possition = %d, delimiter = \'%c\' wrapper = \'%c\'\n",
               fieldp,delimiter,wrapper);
      } else {
         fprintf(stderr, "  fixed width input: start possition = %d, end possition = %d\n",startp, endp);
      }

      if ( strcmp(infname,"-") == 0 )
         fprintf(stderr, "  processing input from stdin\n");
      else
         fprintf(stderr, "  processing input file %s\n",infname);

      if ( gzip_output )
         fprintf(stderr, "  output files will be gziped\n");

      if ( sofar )
         fprintf(stderr, "  sofar = %d\n",sofar);

      if ( sort_check ) {
         if ( string_sort_check )
            fprintf(stderr, "  string sort check is turned on\n");
         else
            fprintf(stderr, "  numeric sort check is turned on (default)\n");
      }

      fprintf(stderr, "  limit per file is %lld %s\n", max_limit, limit_type_str );
      fprintf(stderr, "  output filename prefix is: %s\n", prefix);
      fprintf(stderr, "  output filename suffix digits is: %d\n", suffix_digits);

   }

   /* do the hokey-pokey */
   process_input(infname, prefix);
   exit(0); 
}
   


/*************************************************************************
 * print the usage statement
 ************************************************************************/
void print_usage()
{   
   int i;
   #define USAGE_LINE_SIZE 80

   char mesg[][USAGE_LINE_SIZE] = {
   "\n",
   "Description:\n",
   "  " PROGRAM_NAME " distributes records from a sorted input file (or stdin) in\n",
   "  fixed-size chunks of input to PREFIX01, PREFIX02, ...; default prefix\n",
   "  is `xx'. Groups of records with the same value key are kept together.\n",
   "  This means that output files may vary in size, but will never exceed\n",
   "  the limits defined by -b|--bytes or -l|lines.\n",
   "\n",
   "Usage:\n",
   "  " PROGRAM_NAME " OPTIONS... INPUT (or - for stdin)\n",
   "\n",
   "Switches:\n",
   "  -x, --prefix=PREFIX     PREFIX will be used in output file names.\n",
   "  -b, --bytes=SIZE        Put at most SIZE bytes per output file.  SIZE\n",
   "                          may have a multiplier suffix: appending `b'\n",
   "                          multiplies SIZE by 512, `k' by 1024, and `m' by\n", 
   "                          1048576, and `g' by 1073741824.\n",
   "  -l, --lines=NUMBER      Put at most NUMBER lines per output file. NUMBER\n",
   "                          may have a multiplier suffix: appending `k'\n",
   "                          multiplies LINES by 1000, `m' by 1000000\n",
   "  -d, --delimter=D        Character used to delimit fields in the input file,\n",
   "                          a \'\\t\' is implemented for a tab character, and\n",
   "                          a \',\' is the default.\n",
   "  -w, --wrapper=C         Character used to wrap delimited fields which\n",
   "                          contain an embedded delimiter, default is \'\"\'.\n",
   "                          To turn off this functionality, use the delimiter\n",
   "                          character.\n",
   "  -f, --field=N           Field position (for delimited file) which containes\n",
   "                          the key value to split on.\n",
   "  -p, --position=START:END\n",  
   "                          START and END positions (in a fixed width file)\n",
   "                          which contains the key value to split on.\n",
   "  -n, --suffix-digits=N   Use N digits in output file names.\n",
   "  -c, --no-sort-check     By default a numeric sort order check is done on\n",
   "                          the input, turns the sort check off.\n",
   "  -a, --string-sort-check Use strcmp() for sort order check rather than >.\n",
   "  -z, --gzip-output       Create gziped output files, named PREFIX01.gz, ...\n",
   "  -g, --debug=N           Higher N produces more output messages.\n",
   "  -s, --sofar=NUMBER      Prints message every NUMBER records processesed.\n",
   "                          NUMBER may have a multiplier suffix the same as\n",
   "                          the -l option.\n",
   "  -h, --help              Prints this message.\n",
   "\n",
   "  If you specify conflicting options, such as both -b and -l, the last one\n",
   "  encountered will override the previous options.\n",
   "\n",
   "\n",
   "Examples:\n",
   "\n",
   "       " PROGRAM_NAME " -l 100 -f 1 -x out. input.csv\n",
   "\n",
   "  Given the CSV input file input.csv with 310 records, this could\n",
   "  produce four output files:\n",
   "     out.01 - with <= than 100 records\n",
   "     out.02 - with <= 100 records\n",
   "     out.03 - with <= 100 records\n",
   "     out.04 - with >= 10 records\n",
   "\n",
   "       " PROGRAM_NAME " --bytes 1M --position 10:26 --prefix out. input.fixed\n",
   "\n",
   "  Given the 2.2 Meg, fixed width input file input.fixed, this could\n",
   "  produce three output files:\n",
   "     out.01 - with <= than 1 megabytes\n",
   "     out.02 - with <= than 1 megabytes\n",
   "     out.03 - with >= than 0.2 megabytes\n",
   "\n",
   PROGRAM_NAME " version: " VERSION "\n",
   "cvs $Id: splitval.c 25 2011-03-09 23:15:52Z mjeffe $ \n",
   "\n",
   "Report any comments or bugs to Matt Jeffery - mjeffe@acxiom.com\n",
   "\n"};

    for (i = 0; i < sizeof(mesg)/USAGE_LINE_SIZE; i++)
        fputs(mesg[i], stdout);

    exit(1);
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


/*************************************************************************
 * Open input files
 ************************************************************************/
FILE * open_input(char *fname) {

   FILE *infile;

   if ( debug >= 1 )
      fprintf(stderr, "%s: opening input file %s\n",me, fname);

   if ( strcmp(fname, "-") == 0 ) 
      infile = stdin; 
   else
      infile = zfopen(fname, "r"); 
   if ( ! infile ) { 
      fprintf(stderr, "%s: Can't open input file %s\n", me, fname); 
      exit(1); 
   }
   
   return infile;
}


/*************************************************************************
 * converts a max_bytes string to a number
 * Appending `b' multiplies BYTES by 512, `k' by 1024, and `m' by 
 * 1048576, and `g' by 1073741824
 *
 * input strings can look like this:
 *
 * "456"  - 456 bytes
 * "123b" - 123 * 512 bytes (62976 bytes)
 * "123k" - 123 * 1024 bytes (125952 bytes)
 * "26M"  - 26 megabytes or 26 million lines
 * "5G"   - 5 gigabytes or 5 billion lines
 ************************************************************************/
long long max_bytes_strtoll(char *str) {

   if ( index(str, 'b') || index(str, 'B') )
      return atoll(str) * 512;
   else if ( index(str, 'k') || index(str, 'K') )
      return atoll(str) * 1024;
   else if ( index(str, 'm') || index(str, 'M') )
      return atoll(str) * 1048576;
   else if ( index(str, 'g') || index(str, 'G') )
      return atoll(str) * 1073741824;
   else
      return atoll(str);
}


/*************************************************************************
 * converts a max_lines string to a number
 * Appending `k' multiplies LINES by 1000, `m' by 1000000
 *
 * input strings can look like this:
 *
 * "456"  - 456 lines
 * "123k" - 123 * 1000 line (123,000 lines)
 * "26m"  - 26 * 1000000 lines (26,000,000 lines)
 ************************************************************************/
long long max_lines_strtoll(char *str) {

   if ( index(str, 'k') || index(str, 'K') )
      return atol(str) * 1000;
   else if ( index(str, 'm') || index(str, 'M') )
      return atol(str) * 1000000;
   else
      return atol(str);
}

/*************************************************************************
 * get next output file name
 ************************************************************************/
char * get_next_filename(char *prefix) {
   static char *fname;
   static int num_outfiles = 1;   /* start with 01 */

   /* first time it's used */
   if ( fname == NULL ) {
      fname = (char *) malloc(260);
      if ( fname == NULL ) {
         fprintf(stderr, "%s: can't allocate static variable in open_output_file...\n", me);
         exit(1);
      }
   }

   strcpy(fname, prefix);
   sprintf(fname + strlen(prefix), "%0*u", suffix_digits, num_outfiles);
   num_outfiles++;

   if ( gzip_output )
      sprintf(fname + strlen(fname), ".gz");

   return fname;
}


/*************************************************************************
 * Open output file
 ************************************************************************/
FILE * open_output_file(char *fname) {
   FILE *outfile;

   if ( debug >= 2 )
      fprintf(stderr, "%s: opening output file %s\n",me, fname);

   if (NULL == (outfile = zfopen(fname, "w"))) { 
      fprintf(stderr, "%s: Can't open output file %s\n", me, fname); 
      exit(1); 
   }
   
   return outfile;
}


/*************************************************************************
 * close a file
 ************************************************************************/
void close_file(char *fname, FILE *file) {

   if ( debug >= 2 )
      fprintf(stderr, "%s: closing file %s\n",me, fname);

   if ( file != stdin ) 
      ozfclose(file,fname); 
}


/*************************************************************************
 * get time for sofar, error, log, etc messages
 ************************************************************************/
static char * get_time(void) {
   time_t ltime;
   static char *mytime;

   if ( mytime == NULL ) {
      mytime = (char *) malloc(1024);
      if ( mytime == NULL ) {
         fprintf(stderr, "%s: Unable to allocate memory in get_time for static parm mytime...\n",me);
         exit(1);
      }
      memset(mytime, 0, sizeof(mytime));
   }

   time(&ltime);
   strcpy(mytime, ctime(&ltime));
   chomp(mytime);
   return mytime;
}


/*************************************************************************
 * extract the key "value" string from the line of data
 ************************************************************************/
char * get_value(char *line, long recnum, char *value, int max_valuelen) {
   char **parsed_fields;   /* used by parsecsv */
   int field_count = 0;    /* used by parsecsv */
   int valuelen;
   static char *linecpy;   /* note to mjeffe - static means it hangs arround between function calls */

   /* delimited */
   if ( fieldp ) {

      /* allocate memory for static buffer, the first time it's needed */
      if ( linecpy == NULL ) {
         linecpy = (char *) malloc(LINE_BUFSZ); 
         if ( linecpy == NULL ) {
            fprintf(stderr, "%s: Unable to allocate memory in get_value for static buffer linecpy...\n",me);
            exit(1);
         }
      }
            
      strcpy(linecpy, line);
      chomp(linecpy);
      parsed_fields = parsecsv(linecpy, delimiter, wrapper, &field_count);

      /* check to make sure the field we want exists */
      if ( field_count < fieldp ) {
         fprintf(stderr, "%s: ERROR, key field %d does not exist on input record %ld\n", me, fieldp, recnum);
         fprintf(stderr, "  record %ld:\n", recnum);
         fprintf(stderr, "%s\n",line);
         exit(1);
      }

      if ( strlen(parsed_fields[fieldp - 1]) > max_valuelen ) {
         fprintf(stderr, "%s: ERROR, key value field is too big in record %ld!\n", me, recnum);
         exit(1);
      }
      strcpy(value, parsed_fields[fieldp - 1]);

   /* fixed */
   } 
   else {
      valuelen = endp - startp + 1;

      if ( valuelen > max_valuelen ) {
         fprintf(stderr, "%s: ERROR, key value field length is too big!\n", me);
         exit(1);
      }

      /* check to make sure the field we want exists */
      if ( strlen(line) < endp ) {
         fprintf(stderr, "%s: ERROR, input record %ld is shorter than end possition %d\n", me, recnum, endp);
         fprintf(stderr, "  record %ld:\n", recnum);
         fprintf(stderr, "%s\n",line);
         exit(1);
      }

      strncpy(value, line + startp - 1, valuelen);
   }

   if ( debug >= 5 )
      fprintf(stderr, "%s: input record: %ld, value found = %s\n",me, recnum, value);

   return value;
}


/*************************************************************************
 * send input records to the output file, changing to a new file when the
 * limit has been reached, and keeping the entire group of records with
 * the same "value" key in the same file.
 ************************************************************************/
void process_input(char *infname, char *prefix) {
   int i = 0, len = 0;
   char *linebuff;
   char *value;
   char *last_value;
   long recnum = 0;
   int sofarcnt = 0;
   long long writtencnt = 0;
   FILE *infile;
   FILE *outfile;
   char *outfilename;

   group_buffer_type gb;
   group_buffer_type *gbuff = &gb;


   if ( debug >= 1 )
      fprintf(stderr, "%s: processing input...\n",me);

   linebuff = (char *) malloc(LINE_BUFSZ); 
   value = (char *) malloc(VALUE_BUFSZ); 
   last_value = (char *) malloc(VALUE_BUFSZ); 
   gbuff->buff = (char *) malloc(GROUP_BUFSZ);  /* initial chunk, we can add more later */
   gbuff->buff_size = GROUP_BUFSZ;
   gbuff->end = gbuff->buff;
   if ( linebuff == NULL || value == NULL || last_value == NULL || gbuff->buff == NULL ) {
      fprintf(stderr, "%s: Unable to allocate memory in process_input...\n",me);
      exit(1);
   }

   memset(linebuff, 0, sizeof(linebuff));
   memset(value, 0, sizeof(value));
   memset(last_value, 0, sizeof(last_value));
   memset(gbuff->buff, 0, sizeof(gbuff->buff));
   gbuff->count = 0;
   gbuff->largest_group = 0;

   /* open input file and the first output file */
   infile = open_input(infname);
   outfilename = get_next_filename(prefix);
   outfile = open_output_file(outfilename);
   
   /* 
    * If this record is part of the same group, buffer it.  If this record is
    * part of a new group, write out the current group buffer (checking to make
    * sure we don't exceed one of our filesize limits), then buffer the new record.
    */
   while (fgets(linebuff, LINE_BUFSZ, infile)) { 
      recnum++;
      sofarcnt++;
      len = strlen(linebuff);
      if ( debug >= 4 ) fprintf(stderr, "%s: input record %ld\n",me, recnum);
      if ( debug >= 5 ) fprintf(stderr, "   %s",linebuff);

      value = get_value(linebuff, recnum, value, VALUE_BUFSZ - 1);
      if ( recnum == 1 )
         strcpy(last_value, value);

      /*
       * same value key, add it to the buffer
       */
      if ( strcmp(value, last_value) == 0 ) {
         if ( debug >= 4 ) fprintf(stderr, "%s: same value %s, saving record in group buffer...\n",me, value);
         add_to_groupbuff(gbuff, linebuff);
      }
      /*
       * new value key, write out current buffer and start a fresh one
       */
      else {

         /* sort order check */
         if ( sort_check ) {
            if ( (string_sort_check && strcmp(last_value, value) > 0) ||
                 (atoll(last_value) > atoll(value))
               )
            {
                  fprintf(stderr, "%s: ERROR - the file appears to be out of (%s) sort order:\n",
                        me, string_sort_check ? "string" : "numeric" );
                  fprintf(stderr, "   recnum=<%ld>, current value key=<%s>, last value key=<%s>\n",
                        recnum, value, last_value);
                  exit(1);
            }
         }

         strcpy(last_value, value);

         /* ready for a new file? */
         if ( writtencnt + gbuff->count > max_limit ) {
            if ( debug >= 2 ) {
               fprintf(stderr, "%s: oppening a new file... status of group buffer:\n", me);
               fprintf(stderr, "   end-buff=%d, buff_size=%d, %s count=%lld, largest group=%lld\n", 
                     gbuff->end - gbuff->buff, gbuff->buff_size, limit_type_str, gbuff->count, gbuff->largest_group );
            }
            if ( debug >= 5 )
               fprintf(stderr, "   buffer contents:\n%s\n----------------------\n",gbuff->buff);

            close_file(outfilename, outfile);
            writtencnt = 0;
            outfilename = get_next_filename(prefix);
            outfile = open_output_file(outfilename);
         }

         /* make sure the buffer does no exceed the file size limit */
         if ( gbuff->count > max_limit ) {
            fprintf(stderr, "%s: ERROR - current group has %lld %s, which exceeds your %lld %s per file limit\n",
                  me, gbuff->count, limit_type_str, max_limit, limit_type_str);
            fprintf(stderr, "   The value key of the current group is: %s\n", value);
            exit(1);
         }

         /* write the buffer and reset */
         fputs(gbuff->buff, outfile); 
         writtencnt += gbuff->count;
         gbuff->largest_group = (gbuff->count > gbuff->largest_group) ? gbuff->count : gbuff->largest_group;
         gbuff->end = gbuff->buff;
         gbuff->count = 0;

         /* add current record to the fresh buffer */
         add_to_groupbuff(gbuff, linebuff);
      }

      if ( sofar && sofarcnt >= sofar ) {
         fprintf(stderr, "%s: <%s> records read: %ld, largest group: %lld, current output file: %s\n",
               me, get_time(), recnum, gbuff->largest_group, outfilename);
         fflush(stderr);
         sofarcnt = 0;
      }
   }

   /*
    * write out the last buffer
    */

   /* ready for a new file? */
   if ( writtencnt + gbuff->count > max_limit ) {
      if ( debug >= 2 ) {
         fprintf(stderr, "%s: LAST BUFFER, oppening a new file... status of group buffer:\n", me);
         fprintf(stderr, "   end-buff=%d, buff_size=%d, %s count=%lld, largest group=%lld\n", 
               gbuff->end - gbuff->buff, gbuff->buff_size, limit_type_str, gbuff->count, gbuff->largest_group );
      }
      if ( debug >= 5 ) 
         fprintf(stderr, "   buffer contents:\n%s\n----------------------\n",gbuff->buff);

      close_file(outfilename, outfile);
      outfilename = get_next_filename(prefix);
      outfile = open_output_file(outfilename);
   }

   /* make sure the buffer does no exceed the file size limit */
   if ( gbuff->count > max_limit ) {
      fprintf(stderr, "%s: ERROR - current group has %lld %s, which exceeds your %lld %s per file limit\n",
            me, gbuff->count, limit_type_str, max_limit, limit_type_str);
      fprintf(stderr, "   The value key of the current group is: %s\n", value);
      exit(1);
   }

   fputs(gbuff->buff, outfile); 
}


/*************************************************************************
 * add input line to the group buffer, growing buffer if needed, and
 * keeping up with count and end of the buffer.
 ************************************************************************/
void add_to_groupbuff(group_buffer_type *gb, char *line) {
   int temp_end = 0;
   int len;

   len = strlen(line);

   /* do I need to grow the buffer? */
   if ( (gb->end - gb->buff + len) >= gb->buff_size ) {

      do
         gb->buff_size += GROUP_BUFSZ;
      while ( (gb->end - gb->buff + len) >= gb->buff_size );

      temp_end = gb->end - gb->buff;  /* save the index of where we are in the buffer */
      gb->buff = realloc(gb->buff, gb->buff_size);
      if ( gb->buff == NULL ) {
         fprintf(stderr, "%s: ERROR - unable to realloc group buffer in process_input()\n", me);
         exit(1);
      }
      gb->end = gb->buff + temp_end;

      if ( debug >= 2 ) fprintf(stderr, "%s: had to grow group buffer - it's now %d bytes\n", me, gb->buff_size);
   }

   /* add line to the buffer */
   memcpy(gb->end, line, len + 1);  /* pick up the \0 from the line string */
   gb->end += len;

   switch ( limit_type ) {
      case BYTES_LIMIT:
         gb->count += len;
         break;
      case LINES_LIMIT:
         gb->count += 1;
         break;
   }

   if ( debug >= 4 ) {
      fprintf(stderr, "%s: added record to group buffer. status of group buffer:\n", me);
      fprintf(stderr, "   end-buff=%d, buff_size=%d, %s count=%lld, largest group=%lld\n", 
            gb->end - gb->buff, gb->buff_size, limit_type_str, gb->count, gb->largest_group );
   }
   if ( debug >= 5 ) {
      fprintf(stderr, "   buffer contents:\n%s\n==================\n", gb->buff);
   }
}



