-- =======================================================================
-- File: valid_dob.sql
--
-- Description:
--  Returns the input date if it is in the past.
--
-- Parameters:
--  dob           date of birth to be validated
--
-- Revisions:
--  Date         userid    revision made
--  -----------  ------    -------------------------------------------
--  3-JUL-2000  mjeffe    created
--
-- =======================================================================

create or replace
function valid_dob(dob date) return date is
begin
   -- test for dob in the past
   if dob >= sysdate then
      return NULL;
   else
      return dob;
   end if;
end valid_dob;
/
