-- show size on disk of largest tables
select nspname || '.' || relname as "relation",
    pg_size_pretty(pg_relation_size(c.oid)) as "size"
from pg_catalog.pg_class c
left join pg_catalog.pg_namespace n on (n.oid = c.relnamespace)
where nspname not in ('pg_catalog', 'information_schema')
order by pg_relation_size(c.oid) desc
limit 20;

