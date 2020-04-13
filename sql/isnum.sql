-- -----------------------------------------------------------------------
-- $Id: isnum.sql 35 2011-03-10 04:00:01Z mjeffe $
--
-- Test if string can be converted to a number.  Could vary to return the
-- converted number, or return a boolean, or return Y or null, etc.
-- -----------------------------------------------------------------------

set echo on

create or replace
function isnum(instr varchar2) return varchar2 parallel_enable is
   n number;
begin
   if instr is null then
   return 'N';
   end if;
   n := to_number(instr);
   return 'Y';
exception
   --when invalid_number then
   when others then
      return 'N';
end;
/

select isnum('50') from dual;

select isnum('asdf') from dual;

select isnum('') from dual;

select isnum(null) from dual;


