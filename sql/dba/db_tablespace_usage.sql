set pagesize 999

--create or replace view tablespace_usage
--(tablespace_name, total_gb, used_gb, free_gb, percent_free) as 
select
   tablespace_name,
   to_char(total_gb,'999,999.99') as total_gb,
   to_char(used_gb,'999,999.99') as used_gb,
   to_char(free_gb,'999,999.99') as free_gb,
   to_char(percent_free,'999,999.99') || '%' as pct_free
from (
select 
   tablespace_name, 
   round(max_mb / 1024, 2) as total_gb, 
   round((allocated_mb - free_mb) / 1024, 2) as used_gb, 
   round((max_mb - (allocated_mb - free_mb)) / 1024, 2) as free_gb, 
   round(((max_mb - (allocated_mb - free_mb)) / max_mb) * 100, 2) as percent_free 
from 
   (select a.tablespace_name,
       sum (b.BYTES) / 1048576 free_mb, 
       sum (a.BYTES) / 1048576 allocated_mb, 
       sum (greatest(nvl(a.maxbytes,0), a.BYTES)) / 1048576 max_mb 
    from 
       dba_data_files a, 
       (select tablespace_name, file_id, SUM (BYTES) BYTES 
        from dba_free_space 
        group by tablespace_name, file_id
        ) b 
    where a.file_id = b.file_id(+) 
    group by a.tablespace_name
   ) 
UNION 
select 
   tablespace_name,
   round(max_mb / 1024, 2) as total_gb, 
   round((allocated_mb - free_mb) / 1024, 2) as used_gb, 
   round((max_mb - (allocated_mb - free_mb)) / 1024, 2) as free_gb, 
   round(((max_mb - (allocated_mb - free_mb)) / max_mb) * 100, 2) as percent_free 
from 
   (select a.tablespace_name,
       sum (b.BYTES) / 1048576 free_mb, 
       sum (a.BYTES) / 1048576 allocated_mb, 
       sum (greatest(nvl(a.maxbytes,0), a.BYTES)) / 1048576 max_mb 
    from 
       dba_temp_files a, 
       (select tablespace_name, file_id, SUM (BYTES) BYTES 
        from dba_free_space 
        group by tablespace_name, file_id
       ) b 
    where a.file_id = b.file_id(+) 
    group by a.tablespace_name
   ) 
)
order by 1
/



