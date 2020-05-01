-- *********************************************************************
-- ** File:  isdate.sql - Search for Bad Date values
-- **
-- ** Description: 
-- ** 
-- ** Revisions:
-- **
-- ** date          userid   description
-- ** -----------   ------   ----------------------------
-- ** 22-MAR-2000   mjeffe   Original file
-- **
-- *********************************************************************


-- input parameters:
-- 
--  tab_name - table name
--  col_name - column name
--  date_ftm - date format model
--
-- example of how to use this
--
-- SQL> @isdateok my_table birth_date_column mm/dd/yyyy
--
 
set verify off;

declare

   /***** cursors and records *****/

   /* all distinct individual id's */
   cursor date_cur is
      select 
         ind_id,
         &2 date_col
      from
         &1;

   /* count the number of rows processed */
   linenum number(10) := 0;


begin

   dbms_output.enable(1000000);   

   /*
    * get all the date rows, loop thru each one, testing 
    * to see if it is a valid date.  If it is not, trap the error
    * and print out the row number.
    */     
     
   for date_rec in date_cur loop

      declare
         tdate date;
      begin
         /* if this fails then the exception handler should
            print out the rownum and other associated data */
         tdate := to_date(date_rec.date_col,'&3');
      exception
        when OTHERS then
        begin 
           --DBMS_OUTPUT.PUT_LINE('isdateok inside error: SQLERRM => ' || SQLERRM);
           DBMS_OUTPUT.PUT_LINE('record number: '||linenum||
               '	ind_id: ' || date_rec.ind_id|| 
               '	column data: ' || date_rec.date_col);
        end; 
      end;

      linenum := linenum + 1;

   end loop;

exception
   when OTHERS then
      begin
         DBMS_OUTPUT.PUT_LINE('isdateok outside error : SQLERRM => ' || SQLERRM);
         DBMS_OUTPUT.PUT_LINE('rows processed - linenum: ' || linenum);
         if date_cur%ISOPEN then
            close date_cur;
         end if;
      end;
      
end;
/

set verify on;

