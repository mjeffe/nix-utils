-- $Id: base_conversion_java.sql 34 2011-03-10 03:58:25Z mjeffe $
-- put together by rreed
create or replace function long_to_string_radix (i in number, radix in number) return varchar2
authid current_user
parallel_enable
deterministic
as
language java
name 'java.lang.Long.toString (long, int) return java.lang.String';
/

create or replace function parse_long_radix (s in varchar2, radix in number) return number
authid current_user
parallel_enable
deterministic
as
language java
name 'java.lang.Long.parseLong (java.lang.String, int) return long';
/


