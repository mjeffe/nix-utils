/*
 * Print out the sizes of each data type.
 * Useful if you can't find the doc for your architecture
 *
 * Another bug-ugly utility from Matt's code archive.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

void main(int argc, char **argv) {

   printf("datatype                 bytes\t\trange\n");
   printf("-------------------------------------------------------\n");
   printf("\n[Calculated sizes]\n");
   printf("int                      %d\t\t%d\n", 
         sizeof(int), (int)(pow(2, sizeof(int)*8) - 1));
   printf("unsigned int             %d\t\t%u\n", 
         sizeof(unsigned int), (unsigned int)(pow(2, sizeof(unsigned int)*8) - 1));
   printf("long int                 %d\t\t%ld\n", 
         sizeof(long int), (long int)(pow(2, sizeof(long int)*8) - 1));
   printf("unsigned long int        %d\t\t%lu\n", 
         sizeof(unsigned long int), (unsigned long int)(pow(2, sizeof(unsigned long int)*8) - 1));
   printf("long long int            %d\t\t%lld\n", 
         sizeof(long long int), (long long int)(pow(2, sizeof(long long int)*8) - 1));
   printf("unsigned long long int   %d\t\t%llu\n", 
         sizeof(unsigned long long int), (unsigned long long int)(pow(2, sizeof(unsigned long long int)*8) - 1));
   printf("float                    %d\t\t%f\n", 
         sizeof(float), (float)(pow(2, sizeof(float)*8) - 1));
   printf("double                   %d\t\t%f\n", 
         sizeof(double), (double)(pow(2, sizeof(double)*8) - 1));
   printf("long double              %d\t\t%Lf\n", 
         sizeof(long double), (long double)(pow(2, sizeof(long double)*8) - 1));
   printf("\n");
   /* ---------------------------------------------------------------- */
   printf("\n[limits.h sizes]\n");
   printf("int                      %d\t\t%d\n", 
         sizeof(int), (int)INT_MAX);
   printf("unsigned int             %d\t\t%u\n", 
         sizeof(unsigned int), (unsigned int)UINT_MAX);
   printf("long int                 %d\t\t%ld\n", 
         sizeof(long int), (long int)LONG_MAX);
   printf("unsigned long int        %d\t\t%lu\n", 
         sizeof(unsigned long int), (unsigned long int)ULONG_MAX);
   printf("long long int            %d\t\t%lld\n", 
         sizeof(long long int), (long long int)LLONG_MAX);
   printf("unsigned long long int   %d\t\t%llu\n", 
         sizeof(unsigned long long int), (unsigned long long int)ULLONG_MAX);
   /*
   printf("float                    %d\t\t%f\n", 
         sizeof(float), (float)FLOAT_MAX);
   printf("double                   %d\t\t%f\n", 
         sizeof(double), (double)DOUBLE_MAX);
   printf("long double              %d\t\t%Lf\n", 
         sizeof(long double), (long double)LDOUBLE_MAX);
   printf("\n");
   */
}

