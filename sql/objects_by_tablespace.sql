set pagesize 999
set linesize 999

select b.created, ',', substr(a.owner,1,12) owner, ',',  substr(a.segment_name,1,35) object,  ',', a.tablespace_name,  ',',  MB 
from 
(select owner, segment_name, tablespace_name, sum(blocks) * 32768 / 1048576 MB 
from dba_segments
where tablespace_name like  'DATA01'
group by owner, segment_name, tablespace_name) a, 
(select owner, object_name, max(created) created from dba_objects group by owner, object_name) b
where a.owner = b.owner 
and a.segment_name = b.object_name
order by mb desc, a.owner, created        
/
 
exit


