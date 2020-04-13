-- =======================================================================
-- File: to_dob.sql
--
-- Description:
--  Returns the input date if it is a valid date and it is in the past.
--  This function operates just like the Oracle to_date function.  It requires
--  the date string and format model as parameters, and it will return a date
--  under the following conditions:
--    1) the date string and format model passed in are valid
--    2) the date is in the past.  ( dob < sysdate )
--  Otherwise it will return null.
--
-- Parameters:
--  dob           date of birth to be validated
--  date_format   format for the date being passed in
--
-- Revisions:
--  Date         userid    revision made
--  -----------  ------    -------------------------------------------
--  30-JUN-2000  mjeffe    created
--
-- =======================================================================

create or replace
function to_dob(dob varchar2, date_format varchar2) return date is
   tdate date := NULL;
begin
   -- test for a valid date - if this fails then the exception handler 
   -- should catch it and return null 
   tdate := to_date(dob,date_format);

   -- test for dob in the past
   if tdate >= sysdate then
      tdate := NULL;
   end if;

   return tdate;

exception
   when VALUE_ERROR then
      return null;

   -- specifically do not handle any other errors.  Let Oracle abort the 
   -- function and return the error message.  This helps for things like 
   -- "invalid date format", "invalid month", etc.

end to_dob;
/
