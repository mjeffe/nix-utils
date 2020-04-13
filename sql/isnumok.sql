-- *********************************************************************
-- ** File:  snn.sql - Search for Non Numeric values
-- **
-- ** Description: 
-- **  will search for non numeric values in a field and print out
-- **  the unique_key for that row.
-- ** 
-- ** Revisions:
-- **
-- ** date          userid   description
-- ** -----------   ------   ----------------------------
-- ** 20-MAR-2000   mjeffe   Original file
-- ** 06-OCT-2000   mjeffe   changed the output format to a single line
-- **                        and added a descriptive header row.
-- **
-- *********************************************************************


-- input parameters:
-- 
--  tab_name - table name
--  col_name - column name
--  unique_key - a unique row column such as primary key
--
-- example of how to use this
--
-- SQL> @isnumok my_table some_column unique_key
--

set verify off;
 
declare
   
   /***** cursors and records *****/

   /* all distinct individual id's */
   cursor num_cur is
      select 
         &unique_key unique_key,
         &num_col num_col
      from
         &from_table;

   /* count the number of rows processed */
   linenum number(10) := 0;


begin

   dbms_output.enable(10000000);   
   -- output the header line
   DBMS_OUTPUT.PUT_LINE('unique_key: (num_column data)');

   /*
    * get all the num_doc rows, loop thru each one, testing 
    * to see if it is numeric.  If it is not, trap the error
    * and print out the row number.
    */     
     
   for num_rec in num_cur loop

      declare
         tnum number(10);
      begin
         /* if this fails then the exception handler should
            print out the rownum and other associated data */
         tnum := to_number(num_rec.num_col);
      exception
        when OTHERS then
        begin 
           --DBMS_OUTPUT.PUT_LINE('isnumok inside error: SQLERRM => ' || SQLERRM);
           DBMS_OUTPUT.PUT_LINE(num_rec.unique_key || ': (' || num_rec.num_col || ')');
        end; 
      end;

      linenum := linenum + 1;

   end loop;

exception
   when OTHERS then
      begin
         DBMS_OUTPUT.PUT_LINE('isnumok outside error : SQLERRM => ' || SQLERRM);
         DBMS_OUTPUT.PUT_LINE('rows processed - linenum: ' || linenum);
         if num_cur%ISOPEN then
            close num_cur;
         end if;
      end;
      
end isnumok;
/

set verify on;

