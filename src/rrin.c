/*****************************************************************************
 * Copyright (c) 2020, Matt Jeffery
 * All rights reserved.
 *
 * Descrition:
 *     reads records from multiple files in round robin order and writes to stdout
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

#define VERSION "0.1"
#define BUFSZ 65536   /* max record length */

int main(int argc, char **argv) { 
    FILE *infiles[128];
    FILE *outfile = stdout; 
    char *buffer;
    int files_finished = 0, files_finished_mask = 0;
    int n = 0; 

    if (argc < 3) { 
        fprintf(stderr, "%s: insuficient arguments...\n", argv[0]); 
        fprintf(stderr, "\n"); 
        fprintf(stderr, "usage: %s infile1 infile2 ...\n", argv[0]); 
        fprintf(stderr, "\n"); 
        fprintf(stderr, "Description: %s reads one record at a time from each infile\n", argv[0]);
        fprintf(stderr, "in round robin order, and writes the combined output to stdout\n"); 
        fprintf(stderr, "\n"); 
        fprintf(stderr, "%s version: %s\n",argv[0], VERSION);
        fprintf(stderr, "Report any comments or bugs to Matt - matt@mattjeffery.dev\n");
        fprintf(stderr, "\n");
        exit(1); 
    }

    buffer = malloc(BUFSZ); 
    if (buffer == NULL) {
        fprintf(stderr, "%s: unable to allocate memory for buffer\n", argv[0]);
        exit(1);
    }

    /* open input files and set files_finished bitmask */
    for (n = 0; n < (argc - 1); n++) { 
        if (NULL == (infiles[n] = fopen(argv[n+1], "r"))) { 
            fprintf(stderr, "%s: Can't open input file %s\n", argv[0], argv[n+1]); 
            exit(1); 
        }
        files_finished_mask = (1 << n);
    } 

    /* 
     * Round robin one record from each input file to the output.
     * We want to keep reading and writing until all files are done,
     * keep track of which files are finished by setting the file number bit in the files_finished
     * once all n bits are set, it should == the mask
     */
    while ((files_finished & files_finished_mask) != files_finished_mask) {
        for (n = 0; n < (argc - 1); n++) {
            if (fgets(buffer, BUFSZ, infiles[n])) {
                fputs(buffer, outfile); 
            } else {
                files_finished = (1 << n);
            }
        }
    }

    for (n = 0; n < (argc - 1); n++) {
        fclose(infiles[n]); 
    }
    
    exit(0); 
}
    

