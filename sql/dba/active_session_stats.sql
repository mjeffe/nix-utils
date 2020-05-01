-- -----------------------------------------------------------------------
-- $Id: active_session_stats.sql 34 2011-03-10 03:58:25Z mjeffe $
-- -----------------------------------------------------------------------

set echo on
set pagesize 999

-- use this to get all the stats on a sid
select a.sid, b.name, a.value
from v$sesstat a, v$statname b
where a.statistic# = b.statistic#
order by a.sid, b.name;

-- use this to find out how long Oracle thinks something will take to run
-- QCSID is the parallel query server controller, so use it in the where
-- clause if you want to limit to a given query.
--select * from v$session_longops order by last_update_time desc;

