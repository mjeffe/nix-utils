spool dvl_pr.log
set timing on
set echo on
set linesize 132
set pagesize 68
--
--
create profile DVL_PR
  limit
    sessions_per_user          unlimited
    cpu_per_session            unlimited
    cpu_per_call               unlimited
    connect_time               unlimited
    idle_time                  unlimited
    logical_reads_per_session  unlimited
    logical_reads_per_call     unlimited
    composite_limit            unlimited
    private_sga                unlimited
    failed_login_attempts      unlimited
    password_life_time         unlimited
    password_reuse_time        unlimited
    password_reuse_max         unlimited
    password_lock_time         unlimited
    password_grace_time        unlimited
    password_verify_function   default
;
--
--
set timing off
set echo off
spool off
exit

