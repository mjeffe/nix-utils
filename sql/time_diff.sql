CREATE OR REPLACE FUNCTION time_diff (
DATE_1 IN DATE, DATE_2 IN DATE) RETURN varchar2 is

NDATE_1 NUMBER;
NDATE_2 NUMBER;
NSECOND_1 NUMBER(5,0);
NSECOND_2 NUMBER(5,0);

seconds number;
hours number;
minutes number;
sec_remain number;

BEGIN
  -- Get Julian date number from first date (DATE_1)
  NDATE_1 := TO_NUMBER(TO_CHAR(DATE_1, 'J'));

  -- Get Julian date number from second date (DATE_2)
  NDATE_2 := TO_NUMBER(TO_CHAR(DATE_2, 'J'));

  -- Get seconds since midnight from first date (DATE_1)
  NSECOND_1 := TO_NUMBER(TO_CHAR(DATE_1, 'SSSSS'));

  -- Get seconds since midnight from second date (DATE_2)
  NSECOND_2 := TO_NUMBER(TO_CHAR(DATE_2, 'SSSSS'));

  /* seconds per minute 60            = 60
     seconds per hour   60 * 60       = 3600
     seconds per day    60 * 60 * 24  = 86400
   */

  /* diff in seconds */   
  seconds := (((NDATE_2 - NDATE_1) * 86400)+(NSECOND_2 - NSECOND_1));

  /* get hours, and keep track of minutes left over */
  hours := trunc(seconds / 3600);
  sec_remain := seconds - (hours * 3600);
  
  minutes := trunc(sec_remain / 60);
  sec_remain := sec_remain - (minutes * 60);
  
  return hours || ':' || minutes || ':' || sec_remain;
  
END time_diff;
/
