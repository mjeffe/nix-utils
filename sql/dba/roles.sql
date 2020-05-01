spool roles.log
set timing on
set echo on
set linesize 132
set pagesize 68


create role DATA_ADMIN
  not identified
;

create role DEVELOPER
  not identified
;

grant connect to DATA_ADMIN;

grant connect to DEVELOPER;

--  This script will grant the plustrace role to all users loggin on 
--  with the developer role.  This role will allow developers to run
--  traces in sqlplus.  Before this script is run, plustrce.sql
--  must be run to create the plustrace role.  (The script resides in
--  the $ORACLE_HOME/sqlplus/admin directory.)
--
--
grant plustrace to DEVELOPER;

set timing off
set echo off
spool off
exit

