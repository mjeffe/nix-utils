-- ------------------------------------------------------------------------
-- $Id: sqlplus.sql 17 2011-03-09 21:06:25Z mjeffe $
-- 
-- Usefull sqlplus code snippets
-- ------------------------------------------------------------------------

-- ------------------------------------------------------------------------
-- bind variable
-- ------------------------------------------------------------------------
VARIABLE job_no NUMBER

begin
	select s.nextval into :job_no from dual;
end;
/

select :job_no from dual; 






exit



