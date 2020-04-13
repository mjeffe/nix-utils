-- =======================================================================
-- File: pgl_spec.sql
--
-- Description:
--  This package contains functions and procedured used with the P&G DTCM
--
-- Revisions:
--  Date         userid    revision made
--  -----------  ------    -------------------------------------------
--  20-JUN-2000  mjeffe    created
--
-- =======================================================================


create or replace package pgl as
   -- calculates an age based on the DNI document number
   function dni_to_age(doc_type varchar2,doc_num number) return varchar2; 
   function dni_to_age(doc_type varchar2,doc_num varchar2) return varchar2; 
end;
/
