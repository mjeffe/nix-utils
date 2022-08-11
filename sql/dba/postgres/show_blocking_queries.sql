-- show which query pids are causing blocks
select distinct blocking_pid, blocking_user, current_statement_in_blocking_process
from monitor_queries_blocked
where blocking_pid not in (select blocked_pid from monitor_queries_blocked);
