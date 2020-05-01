spool cr_ts_data01_addfile.log
set time on
set timing on
set echo on

alter TABLESPACE data01 add DATAFILE 
'/u05/oradata/cdi/data01_d02.dbf' size 24576M reuse,
'/u03/oradata/cdi/data01_d03.dbf' size 24576M reuse,
'/u06/oradata/cdi/data01_d04.dbf' size 24576M reuse,
'/u04/oradata/cdi/data01_d05.dbf' size 24576M reuse,
'/u07/oradata/cdi/data01_d06.dbf' size 24576M reuse,
'/m02/oradata/cdi/data01_d07.dbf' size 24576M reuse,
'/m03/oradata/cdi/data01_d08.dbf' size 24576M reuse,
'/u04/oradata/cdi/data01_d09.dbf' size 24576M reuse,
'/u07/oradata/cdi/data01_d10.dbf' size 24576M reuse,
'/u03/oradata/cdi/data01_d11.dbf' size 24576M reuse,
'/u06/oradata/cdi/data01_d12.dbf' size 24576M reuse,
'/u02/oradata/cdi/data01_d13.dbf' size 24576M reuse,
'/u05/oradata/cdi/data01_d14.dbf' size 24576M reuse
;
spool off
exit

