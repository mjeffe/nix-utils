-- -----------------------------------------------------------------------
-- $Id: show_hidden_parms.sql 34 2011-03-10 03:58:25Z mjeffe $
-- Show Oracle hidden parameters
-- -----------------------------------------------------------------------

set pagesize 9999
set linesize 90
col parameter_name for a50
col session_value for a15
col instance_value for a15

SELECT a.ksppinm parameter_name, b.ksppstvl session_value, c.ksppstvl instance_value
FROM x$ksppi a, x$ksppcv b, x$ksppsv c
WHERE a.indx = b.indx AND a.indx = c.indx
AND substr(ksppinm,1,1) = '_'
ORDER BY a.ksppinm;

exit

