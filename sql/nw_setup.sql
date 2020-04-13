-- This script needs to be run as sys or "internal"
--
create user nw identified by nw;
grant connect to nw;
grant select on sys.gv_$process to nw;
grant select on sys.gv_$session to nw;
grant select on sys.gv_$sqlarea to nw;
grant select on sys.gv_$session_wait to nw;
grant select on sys.gv_$pq_slave to nw;
grant select on sys.v_$process to nw;
grant select on sys.gv_$sqltext to nw;
grant select on sys.gv_$session_wait to nw;

connect nw/nw

CREATE OR REPLACE VIEW RUNNERS
( inst_id,
  CLASS,
  OWNER_INSTANCE,
  OWNER_SESSION,
  SQL_TEXT,
  SQL_STMT_TRUNCATED,
  PROCESS_PID,
  PROCESS_USERNAME,
  PROCESS_SERIAL#,
  PROCESS_TERMINAL,
  PROCESS_PROGRAM,
  SESS_SID,
  SESS_SERIAL#,
  AUDSID,
  SESS_USER#,
  SESS_USERNAME,
  SESS_COMMAND,
  SESS_PROGRAM,
  SESS_OWNERID,
  SESS_STATUS,
  SESS_SERVER,
  SESS_SCHEMA#,
  SESS_SCHEMANAME,
  SESS_OSUSER,
  SESS_PROCESS,
  SESS_MACHINE,
  SESS_TERMINAL,
  SESS_TYPE,
  SQL_ADDRESS,
  SESS_MODULE,
  SESS_CLIENT_INFO,
  SESS_LOGON_TIME,
  SORTS,
  EXECUTIONS,
  USERS_EXECUTING,
  LOADS,
  FIRST_LOAD_TIME,
  INVALIDATIONS,
  PARSE_CALLS,
  DISK_READS,
  BUFFER_GETS,
  ROWS_PROCESSED,
  COMMAND_TYPE,
  OPTIMIZER_MODE,
  PARSING_USER_ID,
  PARSING_SCHEMA_ID,
  MODULE,
  SESSION_WAIT,
  IDLE_TIME_CUR,
  BUSY_TIME_CUR
)
AS
select
  s.inst_id,
  decode(s.ownerid, 2147483644, 'MASTER', 'SLAVE') class,
  decode(s.ownerid, 2147483644, s.inst_id, trunc(s.ownerid/65536, 0))
      owner_instance,
  decode(s.ownerid, 2147483644, s.sid,
  mod(s.ownerid, 256)) owner_session,
  sql.sql_text,
  decode(length(sql.sql_text), 999, 'TRUNCATED', 'FULL') SQL_STMT_TRUNCATED,
  p.PID process_pid,
  p.USERNAME process_username,
  p.SERIAL# process_serial#,
  p.TERMINAL process_terminal,
  p.PROGRAM process_program,
  s.SID sess_sid,
  s.SERIAL# sess_serial#,
  s.AUDSID,
  s.USER# sess_user#,
  s.USERNAME sess_username,
  s.COMMAND sess_command,
  s.PROGRAM sess_program,
  s.OWNERID sess_ownerid,
  s.STATUS sess_status,
  s.SERVER sess_server,
  s.SCHEMA# sess_schema#,
  s.SCHEMANAME sess_schemaname,
  s.OSUSER sess_osuser,
  s.PROCESS sess_process,
  s.MACHINE sess_machine,
  s.TERMINAL sess_terminal,
  s.TYPE sess_type,
  s.SQL_ADDRESS,
  s.MODULE sess_module,
  s.CLIENT_INFO sess_client_info,
  s.LOGON_TIME sess_logon_time,
  sql.SORTS,
  sql.EXECUTIONS,
  sql.USERS_EXECUTING,
  sql.LOADS,
  sql.FIRST_LOAD_TIME,
  sql.INVALIDATIONS,
  sql.PARSE_CALLS,
  sql.DISK_READS,
  sql.BUFFER_GETS,
  sql.ROWS_PROCESSED,
  sql.COMMAND_TYPE,
  sql.OPTIMIZER_MODE,
  sql.PARSING_USER_ID,
  sql.PARSING_SCHEMA_ID,
  sql.MODULE,
  sw.event session_wait,
  pq.idle_time_cur,
  pq.busy_time_cur
from
  sys.gv_$process p,
  sys.gv_$session s,
  sys.gv_$sqlarea sql,
  sys.gv_$session_wait sw,
  sys.gv_$PQ_SLAVE pq
where
    s.paddr       = p.addr
and s.inst_id     = p.inst_id
and s.inst_id     = sql.inst_id(+)
and s.sql_address = sql.address(+)
and s.inst_id     = sw.inst_id(+)
and s.sid         = sw.sid(+)
and s.inst_id     = pq.inst_id(+)
and substr(s.program, length(s.program) - 4, 4) = pq.slave_name(+)
/
