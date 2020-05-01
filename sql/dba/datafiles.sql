-- $Id: datafiles.sql 34 2011-03-10 03:58:25Z mjeffe $
--
-- returns total size and total free space of each tablespace

--set linesize 120 
set pagesize 999
--set long 2000000000
column tablespace_name format A15
column file_name format A50
--column mb format A10


select tablespace_name, file_name, to_char(bytes/1048576,'999,999,999') as mb
from dba_data_files
union
select tablespace_name, file_name, to_char(bytes/1048576,'999,999,999') as mb
from dba_temp_files
order by 1,2
/


