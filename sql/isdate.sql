-- -----------------------------------------------------------------------
-- $Id: isdate.sql 34 2011-03-10 03:58:25Z mjeffe $
--
-- Test if string can be converted to a date.  Could vary to return the
-- converted date, or return a boolean, or return Y or null, etc.
-- -----------------------------------------------------------------------

set echo on


create or replace
function isdate(instr varchar2, datefmt varchar2) return varchar2  parallel_enable is
   d date;
begin
   if instr is null then
   return 'N';
   end if;

   d := to_date(instr, datefmt);
   return 'Y';
exception
   --when invalid_number then
   when others then
      return 'N';
end isdate;
/


select isdate('20080805', 'YYYYMMDD') from dual;

select isdate('July 15, 2008', 'Mon DD, YYYY') from dual;

select isdate('July 35, 2008', 'Mon DD, YYYY') from dual;

-- empty string
select isdate('', 'Mon DD, YYYY') from dual;

-- null
select isdate(null, 'Mon DD, YYYY') from dual;



