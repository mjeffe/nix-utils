-- $Id: db_tablespace_size.sql 34 2011-03-10 03:58:25Z mjeffe $
--
-- returns total size and total free space of each tablespace

set pagesize 999

select 'total' flag1, a.tablespace_name, 
   --round(sum(a.bytes/1048576),2) as MB
   to_char(sum(bytes)/1048576,'999,999,999') as mb
from dba_data_files a
group by a.tablespace_name
union
select 'free' flag1, b.tablespace_name,
   --round(sum(b.bytes/1048576),2) as MB
   to_char(sum(bytes)/1048576,'999,999,999') as mb
from dba_temp_files b
group by b.tablespace_name
union
select 'free' flag1, c.tablespace_name,
   --round(sum(c.bytes/1048576),2) as MB
   to_char(sum(bytes)/1048576,'999,999,999') as mb
from dba_free_space c
group by c.tablespace_name
order by 2,1
/

prompt
prompt ! ! ! ! ! DO NOT USE THIS SCRIPT ! ! ! ! !
prompt
prompt USE db_tablespace_usage.sql instead - it gives more accurate results
prompt


