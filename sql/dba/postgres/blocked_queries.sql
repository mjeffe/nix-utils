--
-- Show blocked and blocking queries
--
with active as (
    select
        pid,
        pg_blocking_pids(pid) as blocking_pids
    from pg_stat_activity
  where state = 'active'
        and usename not in ('rdsrepladmin', 'datadog-agent')
        and age(now(), query_start) > '1 second'::interval
),
blocking_pids as (
  select
    elements as pid
  from active, unnest(active.blocking_pids) as elements
)
select
    pid,
    age(now(), query_start),
    usename,
    application_name,
    state,
    substring(regexp_replace(regexp_replace(query, '^\s+', '', 'g'), '[\r\n]+|\s{2,}', ' ', 'g'), 0, 80) || '...' as query,
    pg_blocking_pids(pid) as blocking_pids
from pg_stat_activity
where pid in (
    select pid from active
    union
    select pid from blocking_pids
  )
order by 2 desc;

