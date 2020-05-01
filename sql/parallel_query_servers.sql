--
-- determines who is using all the parallel query servers
--
-- Remember that if the max parallel query servers for an Oracle instance is reached, 
-- any new queries submitted will simply run serially (unless the parallel_min_percent 
-- parm is set, which it usually is not).
--
-- there are other colums of interest in v$session that you may want to add
--

select pqs_cnt, schemaname, osuser, program, status, module, LOGON_TIME, machine
from v$session s,
(select qcsid, count(*) pqs_cnt from 
(select sid, qcsid, degree, req_degree 
from v$px_session
order by qcsid, sid)
group by qcsid) p
where qcsid = s.sid
order by 1 desc
/
