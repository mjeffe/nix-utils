-- $Id: base_conversion_plsql.sql 34 2011-03-10 03:58:25Z mjeffe $

-- found by bgille on the net
create or replace function base36_to_dec (p_str in varchar2, p_from_base in number default 36) return number
parallel_enable  
is
   l_num number default 0;
   l_hex varchar2(36) default '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ';
begin
   for i in 1 .. length(p_str) loop
      l_num := l_num * p_from_base + instr(l_hex,upper(substr(p_str,i,1)))-1;
   end loop;
   return l_num;
end base36_to_dec;
/


-- coded by rreed
create or replace function dec_to_base36 (p_num in number, p_to_base in number default 36) return varchar2
parallel_enable  
is
   l_num    number;
   l_remain number;
   l_str    varchar2(36) default '';
   l_hex    varchar2(36) default '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ';
begin
   l_num := p_num;
   loop
      l_remain := mod(l_num,p_to_base);
      l_num := trunc(l_num/p_to_base);
      l_str := substr(l_hex,l_remain+1,1) || l_str;
      exit when l_num <= 0; 
   end loop;
   return l_str;
end dec_to_base36;
/



