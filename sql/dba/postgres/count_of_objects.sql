select
    object_type, count(*) as object_count
from (
    select
        n.nspname as schema_name,
        c.relname as object_name,
        -- see where clause
        case c.relkind
            when 'r' then 'TABLE'
            --when 't' then 'TOAST TABLE'
            when 'p' then 'PARTITIONED TABLE'
            when 'v' then 'VIEW'
            when 'm' then 'MATERIALIZED_VIEW'
            --else c.relkind::text
        end as object_type
    from pg_class c
    join pg_namespace n
    on n.oid = c.relnamespace
    where n.nspname not in ('information_schema', 'pg_catalog')
    and n.nspname not like 'pg_toast%'
    and c.relkind in ('r','p','m','v')  -- 't'
    UNION ALL
    select
        routine_schema as schema_name,
        routine_name as object_name,
        -- we don't distinguish between functions and procedures
        'FUNCTION' as objecty_type
        --coalesce(routine_type, 'FUNCTION') as object_type
    from information_schema.routines
    where routine_schema not in ('information_schema', 'pg_catalog')
) a
group by object_type
order by object_type
;

