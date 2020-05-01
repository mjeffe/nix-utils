-- =======================================================================
-- File: dni_to_age_code.sql
--
-- Description:
--  This function returns a calculated age range CODE based on the dni 
--  number.  This code, along with the associated age ranges, upper 
--  and lower dni numbers and the upper and lower years are stored in the
--  table DNI_AGE_RANGES.
--  In Argentina, an official government document is issued to each person
--  called a DNI Document, similar to the US SSN (Social Security Number).  
--  These numbers are issued consecutivley, which allows us to calculate a 
--  persons age based on their DNI number.
--  This is an Argentina specific function.
--
-- Parameters:
--  doc_type   from the table ind_info_valid.  Used to find DNO doc types
--  doc_num    the actual document number, used to make the calculation
--
-- Revisions:
--  Date         userid    revision made
--  -----------  ------    -------------------------------------------
--  19-JUN-2000  mjeffe    created
--
-- =======================================================================

create or replace 
function dni_to_age(doc_type varchar2,doc_num number) return varchar2 is
begin
   if doc_type != 'DNI' then return null;
   elsif ( to_number(doc_num) >= 41541000 ) then return 'A40';
   elsif ( to_number(doc_num) <= 41540000 and to_number(doc_num) >= 40040000 ) then return 'A39';
   elsif ( to_number(doc_num) <= 40039000 and to_number(doc_num) >= 38540000 ) then return 'A38';
   elsif ( to_number(doc_num) <= 38539000 and to_number(doc_num) >= 37040000 ) then return 'A37';
   elsif ( to_number(doc_num) <= 37039000 and to_number(doc_num) >= 35540000 ) then return 'A36';
   elsif ( to_number(doc_num) <= 35539000 and to_number(doc_num) >= 34810000 ) then return 'A35';
   elsif ( to_number(doc_num) <= 34809000 and to_number(doc_num) >= 34080000 ) then return 'A34';
   elsif ( to_number(doc_num) <= 34079000 and to_number(doc_num) >= 33350000 ) then return 'A33';
   elsif ( to_number(doc_num) <= 33349000 and to_number(doc_num) >= 32620000 ) then return 'A32';
   elsif ( to_number(doc_num) <= 32619000 and to_number(doc_num) >= 31890000 ) then return 'A31';
   elsif ( to_number(doc_num) <= 31889000 and to_number(doc_num) >= 31160000 ) then return 'A30';
   elsif ( to_number(doc_num) <= 31159000 and to_number(doc_num) >= 30430000 ) then return 'A29';
   elsif ( to_number(doc_num) <= 30429000 and to_number(doc_num) >= 29700000 ) then return 'A28';
   elsif ( to_number(doc_num) <= 29699000 and to_number(doc_num) >= 28970000 ) then return 'A27';
   elsif ( to_number(doc_num) <= 28969000 and to_number(doc_num) >= 28240000 ) then return 'A26';
   elsif ( to_number(doc_num) <= 28239000 and to_number(doc_num) >= 27510000 ) then return 'A25';
   elsif ( to_number(doc_num) <= 27509000 and to_number(doc_num) >= 26780000 ) then return 'A24';
   elsif ( to_number(doc_num) <= 26779000 and to_number(doc_num) >= 26050000 ) then return 'A23';
   elsif ( to_number(doc_num) <= 26049000 and to_number(doc_num) >= 25318000 ) then return 'A22';
   elsif ( to_number(doc_num) <= 25317000 and to_number(doc_num) >= 24585000 ) then return 'A21';
   elsif ( to_number(doc_num) <= 24584000 and to_number(doc_num) >= 23850000 ) then return 'A20';
   elsif ( to_number(doc_num) <= 23849000 and to_number(doc_num) >= 23150000 ) then return 'A19';
   elsif ( to_number(doc_num) <= 23149000 and to_number(doc_num) >= 22650000 ) then return 'A18';
   elsif ( to_number(doc_num) <= 22649000 and to_number(doc_num) >= 22100000 ) then return 'A17';
   elsif ( to_number(doc_num) <= 22099000 and to_number(doc_num) >= 21550000 ) then return 'A16';
   elsif ( to_number(doc_num) <= 21549000 and to_number(doc_num) >= 20900000 ) then return 'A15';
   elsif ( to_number(doc_num) <= 20899000 and to_number(doc_num) >= 20000000 ) then return 'A14';
   elsif ( to_number(doc_num) <= 19999000 and to_number(doc_num) >= 18300000 ) then return 'A13';
   elsif ( to_number(doc_num) <= 18299000 and to_number(doc_num) >= 17800000 ) then return 'A12';
   elsif ( to_number(doc_num) <= 17799000 and to_number(doc_num) >= 17400000 ) then return 'A11';
   elsif ( to_number(doc_num) <= 17399000 and to_number(doc_num) >= 16900000 ) then return 'A10';
   elsif ( to_number(doc_num) <= 16899000 and to_number(doc_num) >= 16250000 ) then return 'A9';
   elsif ( to_number(doc_num) <= 16249000 and to_number(doc_num) >= 14950000 ) then return 'A8';
   elsif ( to_number(doc_num) <= 14949000 and to_number(doc_num) >= 14400000 ) then return 'A7';
   elsif ( to_number(doc_num) <= 14399000 and to_number(doc_num) >= 13850000 ) then return 'A6';
   elsif ( to_number(doc_num) <= 13849000 and to_number(doc_num) >= 11700000 ) then return 'A5';
   elsif ( to_number(doc_num) <= 11699000 and to_number(doc_num) >=  9700000 ) then return 'A4';
   elsif ( to_number(doc_num) <=  9699000 and to_number(doc_num) >=  7900000 ) then return 'A3';
   elsif ( to_number(doc_num) <=  7899000 and to_number(doc_num) >=  6350000 ) then return 'A2';
   elsif ( to_number(doc_num) <=  6349000 ) then return 'A1';
   else return null;
   end if;

exception
   when VALUE_ERROR then
      return null;
   when OTHERS then
      DBMS_OUTPUT.PUT_LINE('unknown error in function dni_to_age');

end dni_to_age;
/
