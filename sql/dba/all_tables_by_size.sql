set pagesize 9999
set linesize 140

select 
   e.tablespace_name, 
   rpad(e.owner,20) as owner,
   rpad(e.segment_name,32) as table_name, 
   to_char(e.sum_bytes/1048576,'999,999,999') || ' MB' as size_mb, 
   to_char(o.created, '  DD-MON-YYYY HH24:MI:SS') as created_date
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
--  and o.object_type = 'TABLE'
order by 1, 4 desc, 3, 2
/
