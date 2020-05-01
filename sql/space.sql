select a.tablespace_name, sum(b.bytes)/1048576 freemb, sum(a.bytes)/1048576 totmb
from dba_data_files a, 
(select tablespace_name, file_id, sum(bytes) bytes 
from dba_free_space group by tablespace_name, file_id) b where a.file_id = b.file_id(+)
group by a.tablespace_name
order by (totmb - freemb) / totmb
/

