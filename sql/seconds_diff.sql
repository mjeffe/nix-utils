CREATE OR REPLACE FUNCTION seconds_diff (
   DATE_1 IN DATE, DATE_2 IN DATE) RETURN NUMBER IS

NDATE_1 NUMBER;
NDATE_2 NUMBER;
NSECOND_1 NUMBER(5,0);
NSECOND_2 NUMBER(5,0);

BEGIN
  -- Get Julian date number from first date (DATE_1)
  NDATE_1 := TO_NUMBER(TO_CHAR(DATE_1, 'J'));

  -- Get Julian date number from second date (DATE_2)
  NDATE_2 := TO_NUMBER(TO_CHAR(DATE_2, 'J'));

  -- Get seconds since midnight from first date (DATE_1)
  NSECOND_1 := TO_NUMBER(TO_CHAR(DATE_1, 'SSSSS'));

  -- Get seconds since midnight from second date (DATE_2)
  NSECOND_2 := TO_NUMBER(TO_CHAR(DATE_2, 'SSSSS'));

  RETURN (((NDATE_2 - NDATE_1) * 86400)+(NSECOND_2 - NSECOND_1));
END seconds_diff;
/
