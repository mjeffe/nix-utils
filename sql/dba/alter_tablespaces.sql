spool alter_mex_ts_02.log
set echo on
set timing on
set linesize 132
set pagesize 68
--
--
-- Alter tablespace.
--
alter tablespace MEX_TS
  add datafile
            '/u06/oradata/dvl/mex02.dbf' size 2048M reuse
;
--
--
set echo off
set timing off
spool off
exit

