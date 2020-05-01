set pagesize 999
set linesize 140

select
   tablespace_name,
   rpad(owner,20) as owner,
   segment_type,
   rpad(segment_name,32) as object_name,
   to_char(sum(bytes)/1048576,'999,999,999') || ' MB' as size_mb
--   to_char(sum(bytes),'999,999,999,999,999') as ' B' as size_bytes
from dba_extents
where tablespace_name not in ('SYSTEM','UNDOTBS2')
group by tablespace_name, owner, segment_name, segment_type
order by 1, 5 desc, 2, 4
/


