#!/bin/bash
# ------------------------------------------------------------------------
# $Id: shell.sh 32 2011-03-10 03:53:55Z mjeffe $
# 
# Usefull shell code snippets
# NOTE: see perl.pl for perl one liners that can be used in shell scripts.
# ------------------------------------------------------------------------



# ------------------------------------------------------------------------
# while read
# build a variable in a while loop
# ------------------------------------------------------------------------

# this works on GNU/Linux
str=
while read line
do
   str="$str $line"
done <<EOF
`df -h`
EOF

echo $str

# This works in ksh/Tru64
#df -h | while read $line


# ------------------------------------------------------------------------
# displays the specified line from a file
# ------------------------------------------------------------------------
if [ ! "$1" ] || [ ! "$2" ]
then
   echo "Description:"
   echo "  `basename $0` displays line_number from the input_file or - (stdin)."
   echo "Usage: "
   echo "   `basename $0` <line_number> <input_file|->"
   echo
   exit 1
fi

if [ "$2" = "-" ]
then
   head -n${1} | tail -n 1
else
   head -n${1} $2 | tail -n 1
fi


# ------------------------------------------------------------------------
# create a string of repeated characters
# ------------------------------------------------------------------------
# you can do it in line like this
num=6
str=D
myvar=`perl -e "print \"$str\" x $num"`
echo $myvar

# or you can create a function and call it
makestr() {
   num=$1
   str=$2

   myvar=`perl -e "print \"$str\" x $num"`
}

makestr 4 B
echo $myvar

makestr 2 AV
echo $myvar

# ------------------------------------------------------------------------
# remove blank lines from a file
# ------------------------------------------------------------------------
if [ ! "$1" ]
then
   echo "missing parameter..."
   echo "Usage: `basename $0` source_file"
   exit 1
fi

awk '

# skip empty rows
$0 == "" {
   next
}

# print all non blank rows
{
   print
}' $1


# ------------------------------------------------------------------------
# return a random number between 1 and $1
# ------------------------------------------------------------------------
expr $RANDOM % $1 + 1


# ------------------------------------------------------------------------
# Color ORACLE_SID portion of your promp RED for production SID.
# ------------------------------------------------------------------------
# make sure this script is in your path, preferably ~/bin
# then add the following line to your .bashrc
#
#   export PS1='`set_prompt.sh`'
#

if [ "$ORACLE_SID" = "cdi" ]; then
   echo "[$USER@$MYHOSTNAME..^[[31m$ORACLE_SID^[[0m] $PWD => "
else
   echo "[$USER@$MYHOSTNAME..$ORACLE_SID] $PWD => "
fi

