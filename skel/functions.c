/* **************************************************************************
 * $Id: C.c 17 2011-03-09 21:06:25Z mjeffe $
 *
 * Usefull C code snippets
 * *************************************************************************/

/*************************************************************************
 * trims leading white space from a string
 *
 * #include <ctype.h>
 ************************************************************************/
int ltrim(char *s) {
    while (isspace((unsigned char)*s))
        s++;

    return s;
}

/*************************************************************************
 * trims trailing white space from a string
 *
 * #include <ctype.h>
 ************************************************************************/
int rtrim(char *s) {
    char *p;

    p = s + strlen(s) - 1; 
    while (p > s && isspace((unsigned char)*p))
        p--;

    p[1] = '\0'; 
    return(s); 
}

/*************************************************************************
 * trims trailing white space from a string
 *
 * No lib dependency
 ************************************************************************/
int rtrim(char *s) {
    int  i; 
    i = strlen(s) - 1; 
    while ((s[i] == ' ' || s[i] == '\t' || s[i] == '\n') && i >= 0) 
        i--; 
    s[i+1] = '\0'; 
    return(i+1); 
}

/*************************************************************************
 * returns true if the string is a comment line or blank line
 * #include <ctype.h>
 ************************************************************************/
int is_ignorable_line(char *s) {
   char *p = s;

   /* skip leading white space */
   //while ( *p == ' ' || *p == '\t' )
   while (isspace((unsigned char)*p))
      p++;

   if ( *p == COMMENT_CHAR || *p == '\n' || *p == '\r' )
      return TRUE;

   return FALSE;
}

/*************************************************************************
 * remove newline and/or carriage return from end of a string
 * return length of new modified string
 ************************************************************************/
static int chomp(char *s) {
   char *p;

   p = s + strlen(s) - 1;

   while (p >= s && (*p == '\n' || *p == '\r'))
      *(p--) = 0;

   return (p - s + 1);
}


