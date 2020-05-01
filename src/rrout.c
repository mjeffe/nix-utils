/*****************************************************************************
 * Copyright (c) 2020, Matt Jeffery
 * All rights reserved.
 *
 * Descrition:
 *    reads records from a file, or stdin and writes them to multiple output
 *    files in round robin order.
 *
 * This file is part of mjeffe's round-robin-files utilities. This file is free
 * software and is distributed under the terms of the BSD 3-Clause License.
 * You should have received a copy of the BSD 3-Clause License along with this
 * program.  If not, see <https://opensource.org/licenses/BSD-3-Clause>
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define VERSION "0.3"
#define BUFSZ 65536   /* max record length */

int main(int argc, char **argv) { 
    FILE *infile = NULL; 
    FILE *outfiles[128];
    char *buffer;

    int n = 0; 
    int i = 0; 

    buffer = (char *) malloc(BUFSZ); 

    if (argc < 4) { 
        fprintf(stderr, "%s: insuficient arguments...\n", argv[0]); 
        fprintf(stderr, "\n"); 
        fprintf(stderr, "usage: %s infile [or - for stdin] outfile1 outfile2 ...\n", argv[0]); 
        fprintf(stderr, "\n"); 
        fprintf(stderr, "description: %s distributes records from the input file (or stdin)\n", argv[0]); 
        fprintf(stderr, "equally among the output files in round robin order.\n"); 
        fprintf(stderr, "\n"); 
        fprintf(stderr, "%s version: %s\n",argv[0], VERSION);
        fprintf(stderr, "Report any comments or bugs to Matt - matt@mattjeffery.dev\n");
        fprintf(stderr, "\n");
        exit(1); 
    }

    if (!strcmp(argv[1], "-")) {
        infile = stdin; 
    } else {
        infile = fopen(argv[1], "r"); 
    }

    if (!infile) { 
        fprintf(stderr, "%s: Can't open input file %s\n", argv[0], argv[1]); 
        exit(1); 
    }
    
    for (n = 0; n < (argc - 2); n++) { 
        if (NULL == (outfiles[n] = fopen(argv[n+2], "w"))) { 
            fprintf(stderr, "%s: Can't open output file %s\n", argv[0], argv[n+2]); 
            exit(1); 
        }
    } 
    
    /* round robin the input records to the output files */
    while (fgets(buffer, BUFSZ, infile)) { 
        fputs(buffer, outfiles[i++]); 
        if (i == n) {
            i = 0; 
        }
    }

    if (infile != stdin) {
        fclose(infile); 
    }

    for (i = 0; i < n; i++) {
        fclose(outfiles[i]); 
    }
    
    exit(0); 
}
    

