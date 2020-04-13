spool resize_temp.log
set time on
set timing on

-- in order to distribute equally across for example, six mount points where dbf files are located
-- formula = (#Gig you want) * 1024 / 6
-- for example 100 * 1024 / 6 = 17067M
--
-- 13652M = about 80  Gig
-- 17067M = about 100 Gig
-- 20480M = about 120 Gig

alter database tempfile '/u07/oradata/cdi_10/temp_d01.dbf' resize 2048M;
alter database tempfile '/u02/oradata/cdi_10/temp_d06.dbf' resize 2048M;
alter database tempfile '/u06/oradata/cdi_10/temp_d02.dbf' resize 2048M;
alter database tempfile '/u03/oradata/cdi_10/temp_d05.dbf' resize 2048M;
alter database tempfile '/u05/oradata/cdi_10/temp_d03.dbf' resize 2048M;
alter database tempfile '/u04/oradata/cdi_10/temp_d04.dbf' resize 2048M;

spool off
exit

