set timing on
set echo on
set linesize 132
set pagesize 68

spool jpaxto.log

drop user jpaxto cascade;

create user jpaxto
  identified by jpaxto
  default tablespace USERS
  temporary tablespace TEMP
  quota unlimited on DATA01
  quota unlimited on TEMP
  quota unlimited on USERS
--  profile DBA_PR
;

grant CONNECT to jpaxto;

--grant DBA to jpaxto;

--alter user jpaxto default role DATA_ADMIN;

set timing off
set echo off
spool off


--create user mjeffe identified by mjeffe        
--default tablespace data01 temporary tablespace temp
--quota unlimited on data01 quota unlimited on temp;
--
--grant connect to mjeffe;
--grant resource to mjeffe;
--grant dba to mjeffe;

