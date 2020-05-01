spool resize_ts_data01.log
set time on
set timing on
set echo on

--
-- to increase the tablespace and distribute it evenly over the existing files
-- use the following formula:
--
--   G * 1024 / F
--
--   where:
--      G = # of Gigs you want
--      F = # of datafiles, in this case 6
--


alter database datafile '/u02/oradata/cdi_10/data01_d01.dbf' resize 12288M;
alter database datafile '/u03/oradata/cdi_10/data01_d02.dbf' resize 12288M; 
alter database datafile '/u04/oradata/cdi_10/data01_d03.dbf' resize 12288M; 
alter database datafile '/u05/oradata/cdi_10/data01_d04.dbf' resize 12288M; 
alter database datafile '/u06/oradata/cdi_10/data01_d05.dbf' resize 12288M; 
alter database datafile '/u07/oradata/cdi_10/data01_d06.dbf' resize 12288M; 

spool off
exit

