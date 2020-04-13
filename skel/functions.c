/* **************************************************************************
 * $Id: C.c 17 2011-03-09 21:06:25Z mjeffe $
 *
 * Usefull C code snippets
 * *************************************************************************/

/*  Trims all trailing white space from strings */
int RightTrim( char *s) 
( 
    int  i; 
    i = strlen(s)-1; 
    while ((s[i] == ' ' || s[i] == '\t' || s[i] == '\n') && i >= 0) 
        i--; 
    s[i+1] = '\0'; 
    return(i+1); 
) 


void vTrimRight (char *szTrimMe) 
( 
     int i = -1; 

     i = strlen (szTrimMe); 

     i--; 
     while (szTrimMe[i] == ' ' && i >= 0) 
     ( 
          i--; 
     ) 

     szTrimMe[i+1] = NULL; 
) 

void vTrimLeft (char *szTrimMe) 
( 
     char szHold[1024]; 
     sprintf (szHold, "%s", szTrimMe); 
     int i = 0; 
     int iLength; 

     iLength = strlen (szHold); 

     while (szHold[i] == ' ' && i < iLength) 
     ( 
          i++; 
     ) 

     sprintf (szTrimMe, "%s", &szHold[i]); 
)

/*
 * Here is some code that will trim the right or left or both that I use quite often: 
 */

#define ALLSPACES  0     /* For StrTrimLeft/Right()*/ 

void StrShrink(char *String_) 
( 
  StrTrimLeft(String_,ALLSPACES); 
  StrTrimRight(String_,ALLSPACES); 
) 

void StrTrimLeft(char *String_, int Limit_) 
( 
  int i, j; 

  i = 0; 
  while(String_[i] == ' ') 
   i++; 
  if((Limit_ > 0) && (i > Limit_)) 
   i = Limit_; 
  for(j = 0; i < strlen(String_); i++) 
   ( 
    String_[j] = String_[i]; 
    j++; 
   ) 
  String_[j] = '0'; 
) 

void StrTrimRight(char *String_, int Limit_) 
( 
  int  i; 

  i = strlen(String_) - 1; 
  if(i == 0) 
   return; 
  while((String_[i] == ' ') || (String_[i] == 'r') || (String_[i] == 'n')) 
   i--; 
  if((Limit_ > 0) && (i < Limit_)) 
   i = Limit_ - 1; 
  String_[i + 1] = '0'; 
)




