-- $Id: tables_by_lastddl_in_tbs.sql 34 2011-03-10 03:58:25Z mjeffe $
--
-- This shows tables by last_ddl in a certain tbs
--

select c.last_ddl_time, c.owner, c.object_type, c.object_name
from all_tables b, all_objects c
where b.table_name = c.object_name and b.owner = c.owner
and b.tablespace_name = 'BGILLE_TBS'
order by 1 desc, 2

