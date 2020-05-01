-- ----------------------------------------------------------------------
-- $Id: active_sessions.sql 34 2011-03-10 03:58:25Z mjeffe $
--
-- I obtained this from rreed's Orac Perl db tool
-- ----------------------------------------------------------------------

set pagesize 999
set linesize 160

/* Thanks to Andre Seesink for Sid,Serial change to ease session control */
select rpad(s.username,14) "User", 
       rpad(s.osuser,14) "OS User",
       rpad(s.sid||','||s.serial#,14) "Sid,Serial",           
       --decode(s.type, 'USER', 'User', 'BACKGROUND', 'Backgd', s.type) "Type",
       --decode(s.status,'ACTIVE', 'Active', s.status) "Status",     
       --decode(s.status,'INACTIVE','Inact ' || round((s.last_call_et/60),0) || ' min', 
       --                'ACTIVE', 'Active', s.status) "Status",   
       --to_char(s.logon_time,'dd/mm hh24:mi') "Logged On",
       p.spid "Spid",
       rpad(s.program,40) "OS Program", 
       rpad(s.module,30) "Module",
       --s.server "Server", 
       rpad(s.machine,25) "Machine",   
       rpad(s.terminal,14) "Terminal",
       rpad(decode(s.command, 0,'',                 1,'Create Table',
                         2,'Insert',           3,'Select',
                         4,'Create Cluster',   5,'Alter Cluster',
                         6,'Update',           7,'Delete',
                         8,'Drop',             9,'Create Index',
                         10,'Drop Index',      11,'Alter Index',
                         12,'Drop Table',      15,'Alter Table',
                         17,'Grant',           18,'Revoke',
                         19,'Create Synonym',  20,'Drop Synonym',
                         21,'Create View',     22,'Drop View',
                         26,'Lock Table',
                         28,'Rename',          29,'Comment',
                         30,'Audit',           31,'Noaudit',
                         32,'Cre Ext Data',    33,'Drop Ext Dat',
                         34,'Create Data',     35,'Alter Data',
                         36,'Create Rollback Segment',
                         37,'Alter Rollback Segment',
                         38,'Drop Rollback Segment',
                         39,'Create Tablespace',
                         40,'Alter Tablespace',
                         41,'Drop Tablespace',
                         42,'Alter Session',   43,'Alter User',
                         44,'Commit',          45,'Rollback',
                         46,'Save Point',      47,'PL/SQL',
                         to_char(command)),25)     "Command Type",
       decode(s.lockwait,'','','Yes') "Lock Wait?"
from   v$session s, v$process p, v$px_session x
where  s.paddr = p.addr
  -- Don't clutter the report with parallel query slaves
  and  s.sid = x.sid (+) and s.serial# = x.serial# (+)
  and  (s.sid = x.qcsid or x.qcsid is null)
  -- And exclude inactive or background processes
  and s.type = 'USER'
  and s.status = 'ACTIVE'
order by 1, 2, 3, 4, 5
/

