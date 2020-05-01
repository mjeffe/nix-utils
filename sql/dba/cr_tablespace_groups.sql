-- --------------------------------------------------------------------------
-- $Id: cr_tablespace_groups.sql 34 2011-03-10 03:58:25Z mjeffe $
-- create tablespace group named ecr_temp_tbsgrp
--
-- A tablespace group is a grouping of tablespaces, which can be used by
-- oracle as a single temporary tablespace.  I don't know all the
-- implications, but one of them seems to be that in traditional temporary
-- tablespaces oracle does not seem to use more than one datafile at a time,
-- even if they are spread across many mount points.  With tablespace groups,
-- it appears that oracle can use each of the tablespaces with the group in
-- parallel.  This has show significant performance improvement on joins,
-- sorts, etc.
-- 
-- One caveat seems to be that certain types of queries can only use one
-- tablespace within the group, so each individual tablespace needs to be
-- large enough to handle these types of queries.  The only queries I'm aware
-- of like this are ones that use windowing functions.
-- --------------------------------------------------------------------------

set echo on
set timing on
spool cr_ecr_temp_tbsgrp.log

CREATE TEMPORARY TABLESPACE ecr_temp1_tbs  TEMPFILE
   '/u02/nbu/oradata/ecrp006/ecr_temp1_1.dbf' SIZE 24G REUSE,
   '/u03/nbu/oradata/ecrp006/ecr_temp1_2.dbf' SIZE 24G REUSE
   TABLESPACE GROUP ecr_temp_tbsgrp;

CREATE TEMPORARY TABLESPACE ecr_temp2_tbs  TEMPFILE
   '/u04/nbu/oradata/ecrp006/ecr_temp2_1.dbf' SIZE 24G REUSE,
   '/u05/nbu/oradata/ecrp006/ecr_temp2_2.dbf' SIZE 24G REUSE
   TABLESPACE GROUP ecr_temp_tbsgrp;

CREATE TEMPORARY TABLESPACE ecr_temp3_tbs  TEMPFILE
   '/u06/nbu/oradata/ecrp006/ecr_temp3_1.dbf' SIZE 24G REUSE,
   '/u07/nbu/oradata/ecrp006/ecr_temp3_2.dbf' SIZE 24G REUSE
   TABLESPACE GROUP ecr_temp_tbsgrp;

CREATE TEMPORARY TABLESPACE ecr_temp4_tbs  TEMPFILE
   '/u08/nbu/oradata/ecrp006/ecr_temp4_1.dbf' SIZE 24G REUSE,
   '/u09/nbu/oradata/ecrp006/ecr_temp4_2.dbf' SIZE 24G REUSE
   TABLESPACE GROUP ecr_temp_tbsgrp;

exit

