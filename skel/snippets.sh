# ---------------------------------------------------------------------------
# Useful shell script snippets or functions 
# ---------------------------------------------------------------------------


# ==========================================================================
#
# These functions are not immediately useful, they are examples that you can
# copy/modify to suit for a particular situation
#
# ==========================================================================


# ---------------------------------------------------------------------------
_cleanup() {
    _say "cleaning up..."
    #cd $BASEDIR
    #rm -fr $tmpdir
}

# ---------------------------------------------------------------------------
_email() {
    msg="$1" # message you want to send

   _say "sending email"
   for e in $email_recipients; do
       #cat <<EOF | /usr/sbin/sendmail -f matt.jeffery@arkansas.gov matt.jeffery@arkansas.gov
       cat <<EOF | /usr/bin/mailx -t
From: matt.jeffery@arkansas.gov
To: $e
Subject: ERROR: some process

$1

EOF

done
}

# ---------------------------------------------------------------------------
_runsql() {
   sqlf=$1
   base=`basename $sqlf .sql`
   _say "running job: $sqlf";
   psql -X -a -U $dbuser -d $dbname -h $dbhost -f $sqlf > logs/$base.log
   _chkerr $? $sqlf
}

# ---------------------------------------------------------------------------
_loadcsv() {
  # spawn subshell so as not to polute environment with .env vars
  (
    f="$1"
    base=$(basename $f .csv)
    _say "loading $f"

    source .env
    export PGPASSWORD="$DB_PASSWORD"
    pgloadcsv -u $DB_USERNAME -d $DB_DATABASE -h $DB_HOST --drop -c -t stg_$base $f
    _chkerr "$?" "pgloadcsv of $f"
  )
}

# ---------------------------------------------------------------------------
_usage()
{
   cat <<EOF
   Description: do something

   Usage: `basename $0` [-h?] [-f filename] [-d YYYYMMDD]

   Options:
      Default is to fetch today's file and load it.  The following options
      change this behavior.

      -f filename    Skips the fetch and instead loads the local filename
      -d YYYYMMDD    Fetches and loads file for YYYYMMDD
      -h -?          Prints this usage message

EOF
}

# ==========================================================================
#
# The remainder are snippets and idioms useful in shell scripts
#
# ==========================================================================


# ------------------------------------------------------------------------
# prompt user for input
# man bash - and search for /^SHELL BUILTIN/ then page down to 'read'
# ------------------------------------------------------------------------
# prompt for password
read -sp "Your password  : " pw

# prompt for non-hidden input
read -p "Do you want to proceed? [yN] " answer
# handle default
if [ -z "$answer" -o "$answer" = "N" ]; then
    _die "Ok, I'm quitting..."
fi

# ------------------------------------------------------------------------
# return a random number between 1 and $1
# ------------------------------------------------------------------------
expr $RANDOM % $1 + 1

# ---------------------------------------------------------------------------
# getopts short example
# ------------------------------------------------------------------------
while getopts "f:d:th" opt; do
   case $opt in
      d) filedate=$OPTARG ;;
      f) inputfile=$OPTARG ;;
      t) TARBALL=TRUE ;;
      h) _usage; exit 0 ;;
      *) echo "invalid option... try running `basename $0` -h"; exit 1 ;;
   esac
done
shift $(($OPTIND - 1))
inputfile="$1"

# required command line parameters
if [ -z "$filedate" ]; then
   _die "Missing required option: -d YYYMMDD"
fi


# ---------------------------------------------------------------------------
# getopts long example
# ------------------------------------------------------------------------
LONG_ARGS_FLAGS=("completions:")
opts=$(
  getopt \
    --longoptions "$(printf "%s:," "${LONG_ARGS_FLAGS[@]}")" \
    --name "$(basename "$0")" \
    --options "lh" \
    -- "$@"
)

eval set --"$opts"

while [[ $# -gt 0 ]]; do
  case $1 in
  --completions)
    _source_completions
    shift
    ;;
  -l)
    _list_available
    exit 0
    ;;
  -h)
    _usage
    exit 0
    ;;
  *)
    echo $1
    echo "invalid option... try running $(basename $0) -h"
    exit 1
    ;;
  esac
  shift
  # shift $(($OPTIND - 1))
done


# ------------------------------------------------------------------------
# while read
# In bash, the while loop spawns a subshell, so the str variable would be empty
# after the loop. This gets around that.
#
# Note, in ksh/Tru64 you can simple do:
#
#   df -h | while read $line
# ------------------------------------------------------------------------

# this works on GNU/Linux
str=
while read line; do
   str="$str $line"
done <<EOF
`df -h`
EOF

echo $str

# ------------------------------------------------------------------------
# grab the specified line from a file
# ------------------------------------------------------------------------
if [ "$filename" = "-" ]
then
   head -n${linenum} | tail -n 1
else
   head -n${linenum} $filename | tail -n 1
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




