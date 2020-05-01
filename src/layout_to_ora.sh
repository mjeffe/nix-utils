#!/usr/local/bin/bash
#
# Another bug-ugly utility from Matt's code archive.
#

this=`basename $0`

usage="
Description:
   Given a comma delimited layout file with the following structure:
      field_name, start, end, length[, oracle_data_type]
   $this creates:
      1. table_name.sql - create table sql script
      2. table_name.fixed.ctl - sqlldr ctl file which assumes a fixed 
         width input data file
      3. table_name.delim.ctl - sqlldr ctl file which assumes a delimited 
         input data file
   If the layout file is missing any of the required fields, then nulls 
   will be pluged in place of those elements.  If the optional 
   oracle_data_type field is missing (or if it is \"character\"), then
   varchar2 is used.

   NOTE: 
    - These files give you someting to start with, they are not intended
      to be used as is! 
    - This script uses jlemle's oranames program to \"oraclize\" the
      field names.  If oranames is not installed then the script will
      abort.
    - This script will strip any field named FILLER[some number].  For
      example the if the fields filler, filler2 and filler_3 are in the 
      layout, they will not be included in any of the output scripts.

Usage:
   $this table_name layout_file
"


if [ ! $# -eq 2 ]; then
   echo "$usage"
   exit 1
fi

table_name=$1
layout_file=$2

# I may add a command line option for this later, but for now, this means
# if I encounter a field name of filler[some number] then don't output it
# This is common on fixed width files, and I don't want to create oracle
# fields for these.
rm_filler=YES

# cheesy substitute for jlemle's oranames
oranameit() {
   awk -F, '{s=$1; gsub("  *","_",s); printf("%s,%s\n",s,$0);}'
}

# try to find jlemle's oranames
oran=`which oranames`
if [ ! -x "$oran" ]; then
   # assume it's in the same bin directory
   oran="`dirname $0`/oranames"
fi

if [ ! -x $oran ]; then
   echo can't find jlemle's oranames program, using a cheesy substitue...
   oran=oranameit
fi


# oraclize the field names using jlemle's oranames then create the sql
# NOTE: oranames prepends the oraclized name to the file, thereby shifing
#   everyting to the right one column

cat $layout_file | $oran |
awk -F, -v table_name=$table_name ' 
BEGIN {
   printf("create table %s (\n",table_name)
}

# skip any field names called filler
toupper($1) ~ /FILLER[_0-9]*/ { 
   next 
}

# print the oraclized field name and the field length, taking care
# not to put a comma after the last field
NR == 1 {
   if ( $6 == "" || toupper($6) == "CHARACTER" ) {
      printf("   %s varchar2(%d)",$1,$5)
   } else {
      printf("   %s %s(%d)",$1,$6,$5)
   }
   next
}

# all other lines in the file
{
   printf(",\n")
   if ( $6 == "" || toupper($6) == "CHARACTER" ) {
      printf("   %s varchar2(%d)",$1,$5)
   } else {
      printf("   %s %s(%d)",$1,$6,$5)
   }
}

END {
   printf("\n);\n\n")
} ' > $table_name.sql


# next create the ctl, assuming a fixed width file
(
echo "
--
-- This ctl file is meant to give you something to start with, it
-- is not intended to be used as is.
--
-- !!!!   NOTE     !!!!
-- Change /path/to/file to the appropriate path and file names.
-- Change or add whatever sqlldr options you may need.
-- !!!!   NOTE     !!!!
-- 

OPTIONS (ERRORS=1000, DIRECT=TRUE)
LOAD DATA
INFILE '/path/to/file'
BADFILE '/path/to/file.bad'
DISCARDFILE '/path/to/file.dsc'
REPLACE
INTO TABLE $table_name ("
) > $table_name.fixed.ctl

cat $layout_file | $oran |
awk -v table_name=$table_name '
BEGIN {
   FS=",";
}

# skip any field names called filler
toupper($1) ~ /FILLER[_0-9]*/ { 
   next 
}

# print the oraclized field name, taking care not to put a comma 
# after the last field
NR == 1 {
   printf("   %s POSITION(%s:%s) char NULLIF %s=BLANKS",$1,$3,$4,$1)
   next
}

{
   printf(",\n");
   printf("   %s POSITION(%s:%s) char NULLIF %s=BLANKS",$1,$3,$4,$1)
}
 
END {
   printf("\n)\n\n")
} ' >> $table_name.fixed.ctl



# next create the ctl, assuming a delimited input file
(
echo "
--
-- This ctl file is meant to give you something to start with, it
-- is not intended to be used as is.
--
-- !!!!   NOTE     !!!!
-- Change /path/to/file to the appropriate path and file names.
-- Change the delimiter to the appropriate character.
-- Change or add whatever sqlldr options you may need.
-- !!!!   NOTE     !!!!
-- 

OPTIONS (ERRORS=1000, DIRECT=TRUE)
LOAD DATA
INFILE '/path/to/file'
BADFILE '/path/to/file.bad'
DISCARDFILE '/path/to/file.dsc'
REPLACE
INTO TABLE $table_name 
FIELDS TERMINATED BY '|' ("
) > $table_name.delim.ctl

cat $layout_file | $oran |
cat $layout_file | awk -F, '{s=$1; gsub("  *","_",s); printf("%s,%s\n",s,$0);}' |
awk -v table_name=$table_name '
BEGIN {
   FS=",";
}

# skip any field names called filler
toupper($1) ~ /FILLER[_0-9]*/ { 
   next 
}

# print the oraclized field name, taking care not to put a comma 
# after the last field
NR == 1 {
   printf("   %s NULLIF %s=BLANKS",$1,$1)
   next
}

# all other fields
{
   printf(",\n");
   printf("   %s NULLIF %s=BLANKS",$1,$1)
}
 
END {
   printf("\n)\n\n")
} ' >> $table_name.delim.ctl



