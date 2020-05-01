-- =======================================================================
-- File: pgl_body.sql
--
-- Description:
--  This package contains functions and procedures which are used in the
--  P&G DTCM
--
-- Revisions:
--  Date         userid    revision made
--  -----------  ------    -------------------------------------------
--  30-JUN-2000  mjeffe    created
-- =======================================================================


create or replace package body pgl is

   -- ===================================================================
   -- Method:      function dni_to_age
   -- Description: 
   --  NOTE: This function is overloaded.  The number version handles the
   --        conversion of doc_num from a varchar2 to a number.  It then
   --        calls this version which accepts a number.  This version
   --        actually calculates the range.
   --
   --  This function returns a calculated age range based on the dni number.
   --  In Argentina, an official government document is issued to each person
   --  called a DNI Document, similar to the US SSN (Social Security Number).  
   --  These numbers are issued consecutivley, which allows us to calculate a 
   --  persons age based on their DNI number.
   --  This is an Argentina specific function.
   -- Parameters:
   --  doc_type   from the table ind_info_valid.  Used to find DNO doc types
   --  doc_num    the actual document number, used to make the calculation
   -- Revisions:   
   --  Date         userid    revision made
   --  -----------  ------    -------------------------------------------
   --  19-JUN-2000  mjeffe    created
   --  30-JUN-2000  mjeffe    made doc_num parameter a number and removed
   --                         all to_number conversion functions.
   -- ===================================================================
   
   function dni_to_age(doc_type varchar2,doc_num number) return varchar2 is
   begin
      if doc_type != 'DNI' then return null;
      elsif ( doc_num >= 41541000 ) then return                         '0-1  ';
      elsif ( doc_num <= 41540000 and doc_num >= 40040000 ) then return '2-3  ';
      elsif ( doc_num <= 40039000 and doc_num >= 38540000 ) then return '4-5  ';
      elsif ( doc_num <= 38539000 and doc_num >= 37040000 ) then return '6-7  ';
      elsif ( doc_num <= 37039000 and doc_num >= 35540000 ) then return '8-9  ';
      elsif ( doc_num <= 35539000 and doc_num >= 34810000 ) then return '10   ';
      elsif ( doc_num <= 34809000 and doc_num >= 34080000 ) then return '11   ';
      elsif ( doc_num <= 34079000 and doc_num >= 33350000 ) then return '12   ';
      elsif ( doc_num <= 33349000 and doc_num >= 32620000 ) then return '13   ';
      elsif ( doc_num <= 32619000 and doc_num >= 31890000 ) then return '14   ';
      elsif ( doc_num <= 31889000 and doc_num >= 31160000 ) then return '15   ';
      elsif ( doc_num <= 31159000 and doc_num >= 30430000 ) then return '16   ';
      elsif ( doc_num <= 30429000 and doc_num >= 29700000 ) then return '17   ';
      elsif ( doc_num <= 29699000 and doc_num >= 28970000 ) then return '18   ';
      elsif ( doc_num <= 28969000 and doc_num >= 28240000 ) then return '19   ';
      elsif ( doc_num <= 28239000 and doc_num >= 27510000 ) then return '20   ';
      elsif ( doc_num <= 27509000 and doc_num >= 26780000 ) then return '21   ';
      elsif ( doc_num <= 26779000 and doc_num >= 26050000 ) then return '22   ';
      elsif ( doc_num <= 26049000 and doc_num >= 25318000 ) then return '23   ';
      elsif ( doc_num <= 25317000 and doc_num >= 24585000 ) then return '24   ';
      elsif ( doc_num <= 24584000 and doc_num >= 23850000 ) then return '25   ';
      elsif ( doc_num <= 23849000 and doc_num >= 23150000 ) then return '26   ';
      elsif ( doc_num <= 23149000 and doc_num >= 22650000 ) then return '27   ';
      elsif ( doc_num <= 22649000 and doc_num >= 22100000 ) then return '28   ';
      elsif ( doc_num <= 22099000 and doc_num >= 21550000 ) then return '29   ';
      elsif ( doc_num <= 21549000 and doc_num >= 20900000 ) then return '30   ';
      elsif ( doc_num <= 20899000 and doc_num >= 20000000 ) then return '31   ';
      elsif ( doc_num <= 19999000 and doc_num >= 18300000 ) then return '32   ';
      elsif ( doc_num <= 18299000 and doc_num >= 17800000 ) then return '33   ';
      elsif ( doc_num <= 17799000 and doc_num >= 17400000 ) then return '34   ';
      elsif ( doc_num <= 17399000 and doc_num >= 16900000 ) then return '35   ';
      elsif ( doc_num <= 16899000 and doc_num >= 16250000 ) then return '36   ';
      elsif ( doc_num <= 16249000 and doc_num >= 14950000 ) then return '37   ';
      elsif ( doc_num <= 14949000 and doc_num >= 14400000 ) then return '38   ';
      elsif ( doc_num <= 14399000 and doc_num >= 13850000 ) then return '39   ';
      elsif ( doc_num <= 13849000 and doc_num >= 11700000 ) then return '40-44';
      elsif ( doc_num <= 11699000 and doc_num >=  9700000 ) then return '45-49';
      elsif ( doc_num <=  9699000 and doc_num >=  7900000 ) then return '50-54';
      elsif ( doc_num <=  7899000 and doc_num >=  6350000 ) then return '55-59';
      elsif ( doc_num <=  6349000 ) then return                         '+69  ';
      else return 'OUT_OF_RANGE';  -- should never get here
      end if;
   
   exception
      when OTHERS then
         DBMS_OUTPUT.PUT_LINE('unknown error in overloaded function dni_to_age - number version');
   
   end;



   -- ===================================================================
   -- Method:      function dni_to_age
   -- Description: 
   --  NOTE: This function is overloaded.  The number version handles the
   --        conversion of doc_num from a varchar2 to a number.  It then
   --        calls this version which accepts a number.  This version
   --        actually calculates the range.
   --  This function returns a calculated age range based on the dni number.
   --  In Argentina, an official government document is issued to each person
   --  called a DNI Document, similar to the US SSN (Social Security Number).  
   --  These numbers are issued consecutivley, which allows us to calculate a 
   --  persons age based on their DNI number.
   --  This is an Argentina specific function.
   -- Parameters:
   --  doc_type   from the table ind_info_valid.  Used to find DNO doc types
   --  doc_num    the actual document number, used to make the calculation
   -- Revisions:
   --  Date         userid    revision made
   --  -----------  ------    -------------------------------------------
   --  20-JUN-2000  mjeffe    created.  This function was overloaded to fix
   --                         a bug.  This is the description from the
   --                         number version of this function: 
   --                         overloaded this function.  changed doc_num
   --                         parameter to accept a number and eliminated
   --                         the to_number conversion in this function.
   --                         Then created a version which accepts varchars
   --                         and attempts to do the to_number conversion
   --                         before calling this function.
   --                         Thanks to rreed for finding that bug!
   --
   -- =======================================================================
   
   function dni_to_age(doc_type varchar2,doc_num varchar2) return varchar2 is
   begin
      return pgl.dni_to_age(doc_type,to_number(doc_num));
   exception
      when VALUE_ERROR then
         -- vlaue error is when the to_number fails to convert the varchar
         -- to a number.  This will be due to garbage in the doc_num field.
         return null;
      when OTHERS then
         DBMS_OUTPUT.PUT_LINE('unknown error in overloaded function dni_to_age - varchar2 version');
   
   end;
  


end pgl;
/
