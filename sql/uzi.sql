set feedback off
set heading off
set verify off
select count(*), osuser || ' on as ' || schemaname from v$session group by osuser, schemaname order by osuser, schemaname; 
select 'Uzi whom?' from dual;
select to_char(logon_time, 'YYYYMMDD HH24:MI:SS') when, sid, serial#, substr(machine,1,20) machine, osuser, command from v$session where osuser = '&&uzi'
order by logon_time;
spool uzi_session.sql
select 'alter system kill session ''' || sid || ',' || serial# || ''';' from v$session where osuser || ' on as ' || schemaname = '&&uzi' order by logon_time;
spool off
set feedback on 
@uzi_session.sql
!rm uzi_session.sql
exit

