# ---------------------------------------------------------------------------
# Useful shell script helper functions. You could source this directly but
# probably don't want to because there are a number of functions that really
# need to be customized for your specific use case.
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

