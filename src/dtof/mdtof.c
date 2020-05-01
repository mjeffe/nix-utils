/*************************************************************************
 * $Id: mdtof.c 23 2011-03-09 23:10:33Z mjeffe $
 *
 * multithreaded version of dtof
 *
 * KNOWN ISSUES
 *
 * - This uses parsecsv which strips warpper quotes, so the output can be
 * different than the input since we currently don't rebuild it properly.
 ************************************************************************/

#include "mdtof.h"

/* globals */
char *me;                       /* name of this program */
int lrecl = 2048;
FILE *debugfile;
int debug_level = 0;
int num_threads = 1;
int buffer_size = 1;            /* this will be relative to the number of threads */
int block_size = 1000;          /* number of records per block */
int last_block = 0, eof = 0;    /* so output writer knows when to stop */
int sofar = 100;
int usleep_time = 10;
buffer_block_type *io_buffer;

pthread_t *worker_threads;
worker_thread_parms_type **worker_thread_parms;

pthread_t input_thread;
input_thread_parms_type *input_thread_parms;

pthread_t output_thread;
output_thread_parms_type *output_thread_parms;



/* function prototypes */
int init();
buffer_block_type *init_io_buffer(int buff_size);
int read_file();         /* controller thread */
int process_records();   /* worker threads */
int output();            /* output thread */
void print_usage();
static int chomp(char *s);
void process_file(FILE *fileptr, char delim, char wrapper, char output_delim);
void *input_reader_thread(void *);
void *output_writer_thread(void *);
void *dtof_worker_thread(void *);
void insleep(int t);
void outsleep(int t);
void workersleep(int t);
int parm_to_int(char *str, char *parm);




/*************************************************************************
 * usage and die
 ************************************************************************/
void print_usage() {
   fprintf(stderr, "\n\nusage: %s <input_file>\n", me);
   exit(1);
}


/*************************************************************************
 * main
 ************************************************************************/
int main(int argc, char *argv[]) {
   int i, j, ret;
   int flag;
   char delimiter = '|';       /* default */
   char field_wrapper = '"';   /* not an option */
   char output_delimiter = ',';
   char *filename;
   char *control_card;
   FILE *infile;
   

   me = argv[0];
   debugfile = stderr;


   optarg = NULL;
   while ((flag = getopt(argc, argv, "d:r:c:t:b:g:")) != -1) {
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

         /* replacement delimiter */
         case 'r':
            if (optarg[0] == '\\') {
               if (strlen(optarg) != 2) {
                  fprintf(stderr,"%s: Escape sequence for delimiter is malformed\n",me);
                  print_usage();
               }

               switch(optarg[1]) {
                  case 't':
                     output_delimiter = '\t';
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

               output_delimiter = optarg[0];
            }
            break;

         /* control card file name */
         case 'c':
            control_card = optarg; 
            break;

         /* number of threads */
         case 't':
            num_threads = parm_to_int(optarg, "-t"); 
            break;

         /* records per block */
         case 'b':
            block_size = parm_to_int(optarg, "-b");
            break;

         /* debug level */
         case 'g':
            debug_level = parm_to_int(optarg, "-g"); 
            break;

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


   infile = zfopen(filename, "r");

   if (infile == NULL) {
      fprintf(stderr, "%s: unable to open %s\n", me, filename);
      perror("Error oppening input file");
      print_usage();
   }

   ret = init();
   if ( ret ) {
      fprintf(debugfile, "%s: Error during init: %d\n", me, ret);
      exit(ret);
   } else {
      D2(2, "%s: finished init successfully\n", me);
   }

   process_file(infile, delimiter, field_wrapper, output_delimiter);

   /* zfclose(infile); */   /* bug in zfclose function, use ozfclose for linux */
   ozfclose(infile, filename);
   return(0);

}


/*************************************************************************
 * main control function
 ************************************************************************/
void process_file(FILE *fileptr, char delim, char wrapper, char output_delim) {
   int t, tret;


   /* start the input thread */
   input_thread_parms = (input_thread_parms_type *) malloc(sizeof(input_thread_parms_type));
   if ( input_thread_parms == NULL ) {
      fprintf(stderr, "%s: ERROR allocating memory for input thread parms in process_file()\n", me);
      exit(1);
   }
   input_thread_parms->input_file = fileptr;

   D2(2, "%s: creating input thread\n", me);
   tret = pthread_create(&input_thread, NULL, input_reader_thread, (void *) input_thread_parms);
   if (tret) {
      fprintf(stderr, "%s: Error starting input thread\n", me);
      perror("Error starting input thead\n");
      exit(1);
   }

   /* start the output thread */
   output_thread_parms = (output_thread_parms_type *) malloc(sizeof(output_thread_parms_type));
   if ( output_thread_parms == NULL ) {
      fprintf(stderr, "%s: ERROR allocating memory for output thread parms in process_file()\n", me);
      exit(1);
   }
   output_thread_parms->output_file = stdout;

   D2(2, "%s: creating output thread\n", me);
   tret = pthread_create(&output_thread, NULL, output_writer_thread, (void *) output_thread_parms);
   if (tret) {
      fprintf(stderr, "%s: Error starting output thread\n", me);
      perror("Error starting output thead\n");
      exit(1);
   }


   /* start the worker threads */
   worker_threads = (pthread_t *) malloc( num_threads * sizeof(pthread_t));
   if ( worker_threads == NULL ) {
      fprintf(stderr, "%s: ERROR allocating memory for worker threads array process_file()\n", me);
      exit(1);
   }

   worker_thread_parms = (worker_thread_parms_type **) malloc( num_threads * sizeof(worker_thread_parms_type *));
   if ( worker_thread_parms == NULL ) {
      fprintf(stderr, "%s: ERROR allocating memory for worker thread parms array in process_file()\n", me);
      exit(1);
   }

   for (t=0; t< num_threads; t++) {
      worker_thread_parms[t] = malloc(sizeof(worker_thread_parms_type));
      if ( worker_thread_parms[t] == NULL ) {
         fprintf(stderr, "%s: ERROR allocating memory for worker thread parms in process_file()\n", me);
         exit(1);
      }
      worker_thread_parms[t]->thread_id = t;
      worker_thread_parms[t]->delimiter = delim;
      worker_thread_parms[t]->wrapper = wrapper;
      worker_thread_parms[t]->output_delim = output_delim;

      D3(2, "%s: creating worker thread %d\n", me, t);
      tret = pthread_create(&worker_threads[t], NULL, dtof_worker_thread, (void *) worker_thread_parms[t]);
      if (tret) {
         fprintf(stderr, "%s: Error starting thread %d\n", me, t);
         perror("Error starting thead\n");
         exit(1);
      }

   }

   D3(2, "%s: joining worker threads\n", me, t);
   for (t=0; t< num_threads; t++) {
      pthread_join(worker_threads[t], NULL);
   }

   /* wait for the output thread to finish */
   D3(2, "%s: joining output thread\n", me, t);
   pthread_join(output_thread, NULL);
}



/*************************************************************************
 * buffer input file
 ************************************************************************/
void *input_reader_thread(void *tparms) {
   input_thread_parms_type *parms;
   int block_id = 0;
   int block_row = 0;
   int rec = 0, blk = 0;


   parms = (input_thread_parms_type *) tparms;
   D2(2, "%s: input_reader_thread starting input file read\n", me);

   /* for each block */
   while ( !eof ) {

      /* wait for the block to be free */
      while ( io_buffer[block_id].status != BLOCK_STATUS_FREE ) {
         insleep(usleep_time);
      }
      D4(4, "%s: input_reader_thread reading block %d, using block_id %d\n", me, blk, block_id);

      /* read a block of records */
      io_buffer[block_id].block_number = blk;
      io_buffer[block_id].recs_in_block = 0;
      for (block_row=0; block_row < block_size; block_row++) {

         if( fgets(io_buffer[block_id].in_block_ptr[block_row], lrecl, parms->input_file) ) {
            io_buffer[block_id].recs_in_block = block_row + 1;
            rec++;
         }
         else if ( feof(parms->input_file) ) {
            D2(2, "%s: finished reading file\n", me);
            eof = 1;
            break;
         }
         else if ( ferror(parms->input_file) ) {
            fprintf(stderr, "%s: Error reading file\n", me);
            perror("Error reading file");
            exit(1);
         }
         else {
            fprintf(stderr, "%s: Unknown Error reading file\n", me);
            perror("Unknown Error reading file");
            exit(1);
         }
      }
      last_block = blk; /* used by the output writer to know when to stop */
      io_buffer[block_id].status = BLOCK_STATUS_READY;

      /* just keep cycling through the buffer until we're done */
      blk++;
      block_id = blk % buffer_size;
   }

   D4(2, "%s: input_reader_thread finished, records: %d, blocks: %d\n", me, rec, blk - 1);
   return NULL;

}


/*************************************************************************
 * buffer output file
 ************************************************************************/
void *output_writer_thread(void *tparms) {
   output_thread_parms_type *parms;
   int block_row = 0;
   int block_id = 0;
   int rec = 0, blk = 0;
   int block_number = 0;


   parms = (output_thread_parms_type *) tparms;
   D2(2, "%s: output_writer_thread starting output file write\n", me);

   /* for each block */
   while ( !eof || blk <= last_block ) {

      /* wait until the next buffer is available */
      while ( io_buffer[block_id].status != BLOCK_STATUS_COMPLETE ) {
         outsleep(usleep_time);
      }

      /* this should never happen */
      if ( io_buffer[block_id].block_number != blk ) {
         fprintf(stderr, "%s: INTERNAL ERROR - trying to output buffer blocks in wrong order\n", me);
         fprintf(stderr, "   io_buffer[%d].block_number = %d, blk = %d\n", 
               block_id, io_buffer[block_id].block_number, blk);
         exit(1);
      }

      /* write the block out */
      D4(4, "%s: output writer thread writing block %d, using block_id %d\n", me, blk, block_id);
      for (block_row=0; block_row< io_buffer[block_id].recs_in_block; block_row++) {
         D3(6, "  output block %d, rec %d\n", block_id, block_row);
         fprintf(parms->output_file, "%s", io_buffer[block_id].out_block_ptr[block_row]);
         rec++;
      }

      /* free the block */
      io_buffer[block_id].status = BLOCK_STATUS_FREE;
      blk++;
      block_id = blk % buffer_size;
   }
   D4(2, "%s: output_writer_thread finished, records: %d, blocks: %d\n", me, rec, blk - 1);

   return NULL;
}



/*************************************************************************
 * read input file and process it a batch of records at a time
 * tparms = tid, delim, wrapper, output_delim
 ************************************************************************/
void *dtof_worker_thread(void *tparms) {
   worker_thread_parms_type *parms;
   char *pin, *pout;
   int block_row = 0;
   int block_id = 0;
   int blk;
   int i;

   /* used by parsecsv */
   char *parsed_fields[MAXNUMBEROFFIELDS+1];  // hard limit!!!
   int field_count;

   
   parms = (worker_thread_parms_type *) tparms;
   block_id = parms->thread_id;
   blk = block_id;

   /* for each block */
   /* while ( !(eof && blk > last_block) )  */
   while ( !eof || blk <= last_block ) {

      /* wait until the block is ready */
      while ( io_buffer[block_id].status != BLOCK_STATUS_READY) {
         workersleep(usleep_time);

         /* A worker thread could get stuck in here because the input thread
          * was reading the last block of the file, so eof was not true, but
          * the worker thread has a block_id > last_block.  We have to check
          * for that condition and bail out if true.  */
         if ( eof && blk > last_block ) goto End_Thread;
      }

      /*
       * convert each input line to fixed width
       */
      D5(5, "%s: [t%d] processing block_id %d - blocks so far: %d\n", me, parms->thread_id, block_id, blk/num_threads);
      for (block_row=0; block_row < io_buffer[block_id].recs_in_block; block_row++) {
         /* set up shorthand pointers */
         pin = io_buffer[block_id].in_block_ptr[block_row];
         pout = io_buffer[block_id].out_block_ptr[block_row];

         //chomp(pin);
         field_count = MAXNUMBEROFFIELDS;
         parsecsvTS(pin, parms->delimiter, parms->wrapper, &field_count, parsed_fields);

         /* -------------------------------  start dummy dtof code --------------------------- */
         /*
         memset(p++, '<', 1);
         sprintf(pout, "%d", parms->thread_id);
         pout += strlen(pout);
         memset(pout++, '>', 1);
         */
         
         /* output each input record, replacing each delimiter */
         for (i = 0; i < field_count; i++) {
            strcpy(pout, parsed_fields[i]);
            pout += strlen(parsed_fields[i]);
            memset(pout++, parms->output_delim, 1);
         }

         //memcpy(pout - 1, "\n\0", 2);  /* overwrite the last delimiter */
         memcpy(pout - 1, "\0", 1);  /* overwrite the last delimiter */

         /* -------------------------------    end dummy dtof code --------------------------- */

      }
      io_buffer[block_id].status = BLOCK_STATUS_COMPLETE;

      blk += num_threads;
      block_id = blk % buffer_size;
   }

   End_Thread:
   D4(2, "%s: [t%d] finished.  Processed %d blocks\n", me, parms->thread_id, blk/num_threads);

   return NULL;
}


/*************************************************************************
 * remove newline and/or carriage return from end of a string 
 * return length of new modified string 
 ************************************************************************/
static int chomp(char *s) {
   char *p;

abort();
   p = s + strlen(s) - 1; 

   while (p >= s && (*p == '\n' || *p == '\r'))
      *(p--) = 0;

   return (p - s + 1);
}


/*************************************************************************
 * sleep t useconds - made these functions so I can distinguish them in gprof output
 ************************************************************************/
void insleep(int t) {
   usleep(t);
}
void outsleep(int t) {
   usleep(t);
}
void workersleep(int t) {
   usleep(t);
}



/*************************************************************************
 * startup stuff
 ************************************************************************/
int init() {

   /* figure out the correct relationship between number of threads and buffer size */
   buffer_size = num_threads * BLOCKS_PER_THREAD;

   io_buffer = init_io_buffer(buffer_size);
   if ( io_buffer == NULL ) {
      fprintf(stderr, "%s: Error during init for io_buffer\n", me);
      exit(1);
   }
   /*
   input_buffer = init_io_buffer(buffer_size);
   if ( input_buffer == NULL ) {
      fprintf(stderr, "%s: Error during init for input_buffer\n", me);
      exit(1);
   }
   output_buffer = init_io_buffer(buffer_size);
   if ( output_buffer == NULL ) {
      fprintf(stderr, "%s: Error during init for output_buffer\n", me);
      exit(1);
   }
   */

   D2(3, "%s: init():\n", me);
   D2(3, "   buffer_size: %d\n", buffer_size);
   D2(3, "   block_size : %d\n", block_size);
   D2(3, "   num_threads: %d\n", num_threads);
   D2(3, "   sofar      : %d\n", sofar);
   D2(3, "   usleep_time: %d\n", usleep_time);

   return(0);
}


/*************************************************************************
 * put error checking around atoi for input parameter parsing
 * based on code in the strtol man page
 ************************************************************************/
int parm_to_int(char *str, char *parm) {
   char *endptr;
   long val;

   errno = 0;    /* To distinguish success/failure after call */
   val = strtol(str, &endptr, 10);

   if (*endptr != '\0') {
      fprintf(stderr, "%s: Invalid characters in numeric input parameter: %s %s\n", me, parm, str);
      exit(EXIT_FAILURE);
   }

   if ( errno ) {
      fprintf(stderr, "%s: Error converting input parameter to a number: %s %s\n", me, parm, str);
      perror("strtol");
      exit(EXIT_FAILURE);
   }

   /* we got a number */

   if ( val < INT_MIN || val > INT_MAX ) {
      printf("%s: Input parameter is out of integer range: %s %ld\n", me, parm, val);
      exit(EXIT_FAILURE);
   }

   return (int)val;
}


/*************************************************************************
 * malloc and initialize an I/O buffer
 ************************************************************************/
buffer_block_type *init_io_buffer(int buff_size) {
   int i, j;
   buffer_block_type *buff;
   

   /* 
    * the io buffer is an array of buffer_block structures, with buff_size
    * number of elements 
    */
   buff = (buffer_block_type *) malloc(sizeof(buffer_block_type) * buff_size);
   if ( buff == NULL ) {
      fprintf(stderr, "%s: Unable to allocate memory for buff in init()\n",me);
      return(NULL);
   }

   /* 
    * each element in the io buffer array, is a buffer_block_type structure.
    * It has a few simple members as well as an input record block and an
    * output record block of block_size records each.
    */
   for (i=0; i<buff_size; i++) {
      buff[i].block_number = 0;
      buff[i].status = BLOCK_STATUS_FREE;
      buff[i].recs_in_block = 0;
           
      /* in block ptr (pointer to an array of strings) */
      buff[i].in_block_ptr = (char **) malloc(block_size * sizeof(char *));
      if ( buff[i].in_block_ptr == NULL ) {
         fprintf(stderr, "%s: Unable to allocate memory for input block ptrs in init()\n",me);
         return(NULL);
      }
      /* out block ptr (pointer to an array of strings) */
      buff[i].out_block_ptr = (char **) malloc(block_size * sizeof(char *));
      if ( buff[i].out_block_ptr == NULL ) {
         fprintf(stderr, "%s: Unable to allocate memory for input block ptrs in init()\n",me);
         return(NULL);
      }

      /* block rows (array of strings) */
      for (j=0; j<block_size; j++) {
         buff[i].in_block_ptr[j] = (char *) malloc(lrecl);
         buff[i].out_block_ptr[j] = (char *) malloc(lrecl);
         if ( buff[i].in_block_ptr[j] == NULL || buff[i].out_block_ptr[j] == NULL) {
            fprintf(stderr, "%s: Unable to allocate memory for io blocks in init()\n",me);
            return(NULL);
         }
         buff[i].in_block_ptr[j][0] = '\0';
         buff[i].out_block_ptr[j][0] = '\0';
      }
   }

   return(buff);
}

