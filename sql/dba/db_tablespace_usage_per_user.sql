-- -----------------------------------------------------------------------
-- $Id: db_tablespace_usage_per_user.sql 34 2011-03-10 03:58:25Z mjeffe $
-- -----------------------------------------------------------------------
set pagesize 999

select 
   owner, tablespace_name, 
   to_char(sum(sum_bytes)/1048576,'999,999,999') || ' MB' as size_mb
from (
   select 
      e.tablespace_name, 
      rpad(e.owner,20) as owner,
      sum_bytes
   from 
      (
         select tablespace_name, owner, segment_name, sum(bytes) as sum_bytes
         from dba_extents
         where segment_type = 'TABLE'
           and tablespace_name != 'SYSTEM'
         group by  tablespace_name, owner, segment_name
      ) e,
           dba_objects o
   where e.segment_name = o.object_name
     and e.owner = o.owner
)
group by owner, tablespace_name
order by 1,2,3
;


