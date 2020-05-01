# ---------------------------------------------------------------------------
# Shell script helper functions I commonly use. Source this near the top
# of your script.
#
# There are also functions that I commonly use, but need to be customized for
# each application. These live in nix-utils/skel/snippets.sh
# ---------------------------------------------------------------------------

# ---------------------------------------------------------------------------
# you should override this AFTER you source this file
# ---------------------------------------------------------------------------
_cleanup() {
    _say "cleaning up..."
    _say "This is a generic cleanup script and it doesn't do anything!"

    # do your cleanup
    #cd $BASEDIR
    #rm -fr $tmpdir
}


# ---------------------------------------------------------------------------
_sigint_handler() {
   echo "Ctrl-C pressed, aborting..."
   _cleanup   # try to run your cleanup handler if you have one
   exit 1
}
trap _sigint_handler SIGINT

# ---------------------------------------------------------------------------
_die() {
    msg="$1"   # optional error message you want printed
    rc=${2:-1} # optional return code to die with

    if [ -n "$msg" ]; then
        _say "$msg"
    fi
    exit $rc
}

# ---------------------------------------------------------------------------
_say() {
   dt=`date | perl -ne 'chomp; print'`
   echo "$dt: $1"
}

# ---------------------------------------------------------------------------
# simple version, just accepts $?
# see below for more complex version
# ---------------------------------------------------------------------------
_chkerr_simple() {
   if [ $1 -ne 0 ]; then
      _say "$2 exited with error (rc=$1)"
      #_email "ERROR: $2"
      exit $1
   fi
}

# ---------------------------------------------------------------------------
# check the return code for non zero status
#
# Call like this after running a command:
#
#   my_script.sh
#   chkerr $? "something went wrong" my_script.sh
#
# Or like this after running a pipeline:
#
#   `foo | bar | baz`
#   chkerr "${PIPESTATUS[*]} $?"
# ---------------------------------------------------------------------------
#typeset -xf chkerr
_chkerr() {
    rc_list="$1"      # return codes
    msg="$2"          # error message you want displayed if rc > 0
    script="$3"       # OPTIONAL name of the script or command who's return code we are checking

    # Process $rc in a loop, to support receiving an array of exit codes, for
    # pipelines (obtained from $PIPESTATUS in bash).  Note that they must be
    # passed as a single argument, typically by quoting.  In such a case, make
    # sure you're using bash, and use this syntax: chkerr "${PIPESTATUS[*]} $?"
    # The quotes, braces, and * are all required for it to work properly.
    # $PIPESTATUS is only set for pipelines, so the $? at the end there ensures
    # that if you convert a pipeline to a single command and don't convert the
    # chkerr line, the error will still be caught properly

    for rc in $rc_list; do
        if [ "$rc" -ne 0 ]; then
            _say "ERROR: $msg"
            echo -n "ERROR during $this: return code $rc"
            if [ -n "$script" ]; then echo -n " returned by $script"; fi
            #_email "ERROR: $2"
            _die
        fi
    done
}
#export chkerr

# ---------------------------------------------------------------------------
# fetch variables's value from .env key=value type file
#
# my_var=`_read_dot_env_var DBNAME`
# ---------------------------------------------------------------------------
_read_dot_env_var() {
  if [ -z "$1" ]; then
    _say "missing required variable argumentr"
    exit 1
  fi

  ENV_VAL=$(egrep "^${1}=" .env | cut -d '=' -f 2)

  echo ${ENV_VAL}
}

