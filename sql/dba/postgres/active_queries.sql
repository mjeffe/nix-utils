-- show running queries (>= v9.2)
select pid, age(clock_timestamp(), query_start), usename, query 
from pg_stat_activity 
where query not ilike 'idle%' and query not ilike '%pg_stat_activity%' 
order by query_start desc

-- get a given application's queries
select usename, application_name, query
from pg_stat_activity
where state = 'active' and application_name = '';

-- kill running query
select pg_cancel_backend(pid)

-- kill idle query
select pg_terminate_backend(pid)


