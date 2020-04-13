spool cr_ts_temp.log
set time on
set timing on

CREATE temporary TABLESPACE temp tempfile
'/u07/oradata/cdi_10/temp_d01.dbf' size 1024M reuse,
'/u02/oradata/cdi_10/temp_d06.dbf' size 1024M reuse,
'/u06/oradata/cdi_10/temp_d02.dbf' size 1024M reuse,
'/u03/oradata/cdi_10/temp_d05.dbf' size 1024M reuse,
'/u05/oradata/cdi_10/temp_d03.dbf' size 1024M reuse,
'/u04/oradata/cdi_10/temp_d04.dbf' size 1024M reuse
extent management local uniform size 1M
;

spool off
exit

