# ---------------------------------------------------------------------------
# Useful shell script helper functions. You should not source this directly
# because there are a number of bare code snippets. Copy what is useful into
# your own project and modify to suit.
# ---------------------------------------------------------------------------


# ==========================================================================
#
# This group of functions *could* be used as is
#
# ==========================================================================

# ---------------------------------------------------------------------------
_sigint_handler() {
   echo "Ctrl-C pressed, aborting..."
   _cleanup
   exit 1
}
trap _sigint_handler SIGINT

# ---------------------------------------------------------------------------
_say() {
   dt=`date | perl -ne 'chomp; print'`
   echo "$dt: $1"
}


# ---------------------------------------------------------------------------
_chkerr() {
   if [ $1 -ne 0 ]; then
      _say "$2 exited with error (rc=$1)"
      #_email "ERROR: $2"
      exit $1
   fi
}

# ---------------------------------------------------------------------------
_die() {
  _say "$1"
  exit 1
}

# ---------------------------------------------------------------------------
_cleanup() {
  # cleanup
  _say "cleaning up"
  #rm -fr $tmpdir
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
_read_dot_env_var() {
  if [ -z "$1" ]; then
    _say "Need an argument of the variable name you are looking for"
    exit 1
  fi

  ENV_VAL=$(egrep "^${1}=" .env | cut -d '=' -f 2)

  echo ${ENV_VAL}
}

# ==========================================================================
#
# Functions in this group are not immediately useful, they are examples that
# you can copy/modify to suit for a particular situation
#
# ==========================================================================

# ---------------------------------------------------------------------------
_email() {
   _say "sending email"
   for e in $email_recipients; do
       #cat <<EOF | /usr/sbin/sendmail -f matt.jeffery@arkansas.gov matt.jeffery@arkansas.gov
       cat <<EOF | /usr/bin/mailx -t
From: matt.jeffery@arkansas.gov
To: $e
Subject: ERROR: GED daily gedts.sh script

$1

The `pwd`/gedts.sh script controls the daily update. Header notes
in this script may be helpful for debugging.
EOF

done
}

# ---------------------------------------------------------------------------
_runsql_example()
{
   sqlf=$1
   base=`basename $sqlf .sql`
   _say "running job: $sqlf";
   psql -X -a -U $dbuser -d $dbname -h $dbhost -f $sqlf > logs/$base.log
   _chkerr $? $sqlf
}

# ---------------------------------------------------------------------------
_usage_example()
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
# The remainder are code snippets of useful stuff
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
# build a variable in a while loop
# ------------------------------------------------------------------------

# this works on GNU/Linux
str=
while read line; do
   str="$str $line"
done <<EOF
`df -h`
EOF

echo $str

# This works in ksh/Tru64
#df -h | while read $line

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




