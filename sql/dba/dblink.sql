--drop public database link PGLATIN.WORLD;

--create public database link PGLATIN.WORLD
--  connect to PGLATIN identified by RICOSUAVE
--  using 'PGLATIN';

drop public database link odie9i;

create public database link odie9i
connect to ccity identified by hifi using 'ODIE9I.CONWAY.ACXIOM.COM';

-- syntax is as follows:
-- create public database link <dblink_name>
-- connect to <username> identified by <password>
-- using '<tnsnames.ora connection name>'

-- to use the dblink do this:
-- select * from user.table@dblink_name

