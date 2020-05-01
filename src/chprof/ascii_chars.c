/*
 * display all ascii chars
 */

#include <stdio.h>
#include <locale.h>
#include <ctype.h>

/* define non printable characters - missing ascii 127:delete - check for it below */
char *clist[256]= 
   { "NUL", "SOH", "STX", "ETX", "EOT", "ENQ", "ACK", "BEL", "BS",  "HT",  
     "LF",  "VT",  "FF",  "OD",  "SO",  "SI",  "SLE", "CS1", "DC2", "DC3", 
     "DC4", "NAK", "SYN", "ETB", "CAN", "EM",  "SIB", "ESC", "FS",  "GS", 
     "RS",  "US",  "Sp" }; 

int main(int argc, char *argv[]) {
   int i = 0;

   /*
      if you want to use the local settings rather than POSIX (7-bit ASCII) you have
      to call setlocal with an empty string to "wake it up"?
   */
   setlocale(LC_ALL, "");

   /* print report */
   printf("characters:\n\n");
   printf("Char\tDec\tHex\n");
   printf("----\t----\t----\n");
   for (i = 0; i < 256; i++) {
      if ( isprint(i) ) {
         printf("%c\t%d\t%x\n",i,i,i);
      } else {
         if (clist[i] != 0) {
            printf("%s\t%d\t%x\n", clist[i],i,i);
         } else if ( i == 127 ) {
            printf("DEL\t%d\t%x\n", i,i);
         } else {
            printf(" \t%d\t%x\n",i,i);
         }
      }
   }
   printf("\n");
}
