/*************************************************************************
 * $Id: mdtof.h 293 2016-10-24 15:46:21Z u35616872 $
 *
 * multithreaded version of dtof
 ************************************************************************/

#ifndef DTOF_H
#define DTOF_H


/* THESE SHOULD BE IN mdtof.c !!! */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>     /* includes strcasecmp() */
#include <pthread.h>
#include <errno.h>       /* used by get_int() */
#include <limits.h>      /* used by get_int() */
#include "parsecsv.h"
#include "zfopen.h"


/* defines */

#define BLOCKS_PER_THREAD 5

#define BLOCK_STATUS_FREE 0
#define BLOCK_STATUS_READY 1
#define BLOCK_STATUS_COMPLETE 2



/* struct definitions */

typedef struct {
   int thread_id;
   char delimiter;
   char wrapper;
   char output_delim;
} worker_thread_parms_type;

typedef struct {
   FILE *input_file;
} input_thread_parms_type;

typedef struct {
   FILE *output_file;
} output_thread_parms_type;

typedef struct {
   int block_number;
   int status;
   int recs_in_block;
   char **in_block_ptr; 
   char **out_block_ptr; 
} buffer_block_type;




/******** threadsafe debugging macros - borrowed from bgille's silver app ****************/
#define D1(d,x) {if (debug_level >= d) { fprintf(debugfile, x); fflush(debugfile); } }
#define D2(d,x,y) {if (debug_level >= d) { fprintf(debugfile, x,y); fflush(debugfile); } }
#define D3(d,x,y,z) {if (debug_level >= d) { fprintf(debugfile, x,y,z); fflush(debugfile); } }
#define D4(d,w,x,y,z) {if (debug_level >= d) { fprintf(debugfile, w,x,y,z); fflush(debugfile); } }
#define D5(d,v,w,x,y,z) {if (debug_level >= d) { fprintf(debugfile, v,w,x,y,z); fflush(debugfile); } }
#define D6(d,u,v,w,x,y,z) {if (debug_level >= d) { fprintf(debugfile, u,v,w,x,y,z); fflush(debugfile); } }
#define D7(d,t,u,v,w,x,y,z) {if (debug_level >= d) { fprintf(debugfile, t,u,v,w,x,y,z); fflush(debugfile); } }

#endif  /* DTOF_H */

