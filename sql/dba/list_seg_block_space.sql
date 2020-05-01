-- ---------------------------------------------------------------------------
-- $Id: list_seg_block_space.sql 34 2011-03-10 03:58:25Z mjeffe $
-- source: http://www.dba-oracle.com/t_alter_table_shrink_space_command.htm
--
-- I had to mess with it a bit to get it to work at all.
-- Still needs work for the output...
-- ---------------------------------------------------------------------------
set echo on
set timing on
set pagesize 999
set linesize 120

drop type BlckFreeSpaceSet;
drop type BlckFreeSpace;

create type BlckFreeSpace as object
(
 seg_owner varchar2(30),
 seg_type varchar2(30),
 seg_name varchar2(100),
 fs1 number,
 fs2 number,
 fs3 number,
 fs4 number,
 fb  number
 );
/
 
create type BlckFreeSpaceSet as table of  BlckFreeSpace;
/
 
create or replace function BlckFreeSpaceFunc (seg_owner IN varchar2, seg_type in varchar2 default null) 
   return BlckFreeSpaceSet
pipelined
is
   outRec BlckFreeSpace := BlckFreeSpace(null,null,null,null,null,null,null,null);
   fs1_b number;
   fs2_b number;
   fs3_b number;
   fs4_b number;
   fs1_bl number;
   fs2_bl number;
   fs3_bl number;
   fs4_bl number;
   fulb number;
   fulbl number;
   u_b number;
   u_bl number;

begin
  for rec in 
   (select s.owner,s.segment_name,s.segment_type 
    from dba_segments s 
    where owner = seg_owner 
      and segment_type = nvl(seg_type,segment_type)
   )
   loop

   dbms_space.space_usage(
      segment_owner      => rec.owner,
      segment_name       => rec.segment_name,
      segment_type       => rec.segment_type,
      fs1_bytes          => fs1_b,
      fs1_blocks         => fs1_bl,
      fs2_bytes          => fs2_b,
      fs2_blocks         => fs2_bl,
      fs3_bytes          => fs3_b,
      fs3_blocks         => fs3_bl,
      fs4_bytes          => fs4_b,
      fs4_blocks         => fs4_bl,
      full_bytes         => fulb,
      full_blocks        => fulbl,
      unformatted_blocks => u_bl,
      unformatted_bytes  => u_b
   );
 
   outRec.seg_owner := rec.owner;
   outRec.seg_type := rec.segment_type;
   outRec.seg_name := rec.segment_name;
  
   outRec.fs1 := fs1_bl;
   outRec.fs2 := fs2_bl;
   outRec.fs3 := fs3_bl;
   outRec.fs4 := fs4_bl;
   outRec.fb  := fulbl;

   Pipe Row (outRec);
  end loop;

  return;

end;
/

-- call it like this?
--select BlckFreeSpaceFunc('<SCHEMA_NAME>','TABLE') from dual;

exit




