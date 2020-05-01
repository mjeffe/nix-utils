set pagesize 999
set linesize 120

select 
   e.tablespace_name, 
   rpad(e.table_name,32) as table_name, 
   e.size_mb || ' MB' as size_mb, 
   to_char(o.created, '  DD-MON-YYYY HH24:MI:SS') as created_date
from (
	select
	   segment_name as table_name, 
      tablespace_name,
	   to_char(sum(bytes)/1048576,'999,999,999') as SIZE_MB
--	   , to_char(sum(bytes),'999,999,999,999,999') as SIZE_BYTES
	from user_extents
	where segment_type = 'TABLE'
	group by segment_name,tablespace_name
	--order by 2,3 desc
) e,
	user_objects o
where e.table_name = o.object_name
  and o.object_type = 'TABLE'
order by 1,3 desc, 2
/
