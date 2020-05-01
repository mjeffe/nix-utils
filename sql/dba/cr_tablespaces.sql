-- old way
create tablespace MEX_TS
  datafile
            '/u02/oradata/dvl/mex01.dbf' size 2048M reuse
default storage (
  INITIAL        4M
  NEXT           4M
  MINEXTENTS     1
  MAXEXTENTS     2147483645
  PCTINCREASE    0
  )
online;

-- new way
create tablespace ECR_ATT
  datafile '/u02/oradata/ecrtest1/ecr_att_01.dbf' size 65535M reuse,
 '/u03/oradata/ecrtest1/ecr_att_02.dbf' size 65535M reuse,
 '/u04/oradata/ecrtest1/ecr_att_03.dbf' size 65535M reuse,
 '/u05/oradata/ecrtest1/ecr_att_04.dbf' size 65535M reuse
Extent management local uniform size 4m;


-- new autoextend way
create tablespace ECR_ATT
  datafile '/u02/oradata/ecrtest1/ecr_att_01.dbf' size 129M autoextend on next 128M maxsize 65535M reuse,
 '/u03/oradata/ecrtest1/ecr_att_02.dbf'  size 129M autoextend on next 128M maxsize 65535M reuse,
 '/u04/oradata/ecrtest1/ecr_att_03.dbf'  size 129M autoextend on next 128M maxsize 65535M reuse,
 '/u05/oradata/ecrtest1/ecr_att_04.dbf'  size 129M autoextend on next 128M maxsize 65535M reuse
Extent management local uniform size 4m;


