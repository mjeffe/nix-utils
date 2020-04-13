/* $Id: spin.c 18 2011-03-09 21:47:56Z mjeffe $
 *
 * Another bug-ugly utility from Matt's code archive.
 *
 * Demo for an ascii spinner - not really a utility, more of
 * a snippet to include when you need it.
 */

#include <stdio.h>
#include <stdlib.h>


#define UDELAY 500000
#define DELAY 2
#define VERSION "$Id: spin.c 18 2011-03-09 21:47:56Z mjeffe $"

char *this;
char spin[4] = { '|', '/', '-', '\\' };

int main(int argc, char *argv[]) {
   int i, udelay = 0, tm = DELAY;
   this = argv[0];
   if ( argc > 1 ) {
      if ( strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0 ) {
         printf("usage: %s [-h|--help] [-v|--version] [-u|--udelay [usleep_delay]] [sleep_delay]\n", this);
         exit(0);
      }
      else if ( strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0 ) {
         printf("%s: version %s\n", this, VERSION);
         exit(0);
      }
      else if ( strcmp(argv[1], "-u") == 0 || strcmp(argv[1], "--udelay") == 0 ) {
         udelay = 1;
         tm = UDELAY;
         if ( argv[2] ) { 
            tm = atoi(argv[2]); 
            if ( tm <= 0 ) { tm = UDELAY; }
         }
      }
      else {
         tm = atoi(argv[1]);
         if ( tm <= 0 ) { tm = DELAY; }
      }
   }

   //printf("delay is %d, udelay is %d\n", tm, udelay);

   /* reasigning "i" keeps it from incrementing indefinitely */
   for ( i = 0; ; i = ++i%4 ) {
      printf("%c\x08", spin[i]);  /* \x08 is a backspace */
      udelay ? usleep(tm) : sleep(tm);
      fflush(stdout);
   }
}


