create user odepot_bus
  identified by bus
  default tablespace DATA01
  temporary tablespace TEMP
  quota unlimited on DATA01
  quota unlimited on TEMP
--  profile DBA_PR
;

grant connect to odepot_bus;
grant resource to odepot_bus;
--grant dba to odepot_bus;


