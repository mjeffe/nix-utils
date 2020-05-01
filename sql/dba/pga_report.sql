-- -----------------------------------------------------------------------
-- $Id: pga_report.sql 34 2011-03-10 03:58:25Z mjeffe $
-- from: http://www.dba-oracle.com/tips_pga_aggregate_target.htm
-- -----------------------------------------------------------------------
set pagesize 999

-- value of pga_aggregate_target and the high water marks for all RAM memory areas
column name format a40
column value format 999,999,999,999,999
select name,value from v$pgastat order by value desc;


-- information about workarea executions
-- optimal   = query could obtain RAM immediately
-- onepass   = query could obtain RAM after one pass
-- multipass = query could obtain RAM after multiple passes - waiting for other queries
col c1 heading 'Workarea|Profile' format a35           
col c2 heading 'Count' format 999,999,999,999,999
col c3 heading 'Percentage' format 99
select name c1,cnt c2,decode(total, 0, 0, round(cnt*100/total)) c3
from (
   select name,value cnt,(sum(value) over ()) total
   from v$sysstat
   where name like 'workarea exec%'
);

