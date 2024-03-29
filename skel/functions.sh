# ---------------------------------------------------------------------------
# Common functions used in my shell scripts.
#
# This is a collection of useful functionality that I have built up over many
# years. These have been developed and used in bash, I cannot vouch for other
# shells.  This file should only contain truly generic functions. I have other
# collections that are useful in certain project environments (Laravel, Vue,
# etc). Please feel free to use and or modify to suit your needs. If you make
# changes or have suggestions for additions, I would love to year from you.
#
#   Matt Jeffery - matt@mattjeffery.dev
#
# NOTES:
#   * I often copy this into a project directory, so it may be modified. My
#      maintained version should be found at:
#
#       https://github.com/mjeffe/nix-utils/blob/master/skel/functions.sh
#
#   * To use these functions, simply source this file into your parent script:
#
#       . utils/functions.sh    # source utils/functions.sh
#
#   * Many functions "return" data. However, returning data from shell
#     functions is a little wonky. Essentially, you can't.  But what we can do
#     is echo it, which means you have to *execute* the function and capture
#     it's output. For example, with the user prompting function _ask:
#
#    answer=$(_ask 'What is your favorite color?')
#    echo "$answer is your final answer"
#
# ---------------------------------------------------------------------------

# run from project root (adjust for your project)
#cd "${0%/*}/.."

#
# Define ANSI escape characters for a few common colors. These are used
# by the _say_color family of functions, but can also be used directly
# to output multiple colors on one line using `echo -e`:
#
#   echo -e "${CRED}Red Warning${CCLEAR}: some plain old text"
#
# For ANSI codes see: https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
#
CCLEAR='\033[0m'  # clear all color
CRED='\033[0;31m'  # red
CYELLOW='\033[1;33m'  # yellow
CBROWN='\033[0;33m'  # brown
CGREEN='\033[0;32m'  # green
CCYAN='\033[0;36m'  # cyan

# ---------------------------------------------------------------------------
# Catch the linux keyboard interrupt signal, usually Ctrl-C and invoke cleanup
# ---------------------------------------------------------------------------
_sigint_handler() {
    echo "Ctrl-C pressed, aborting..."
    _cleanup
    exit 1
}
trap _sigint_handler SIGINT

# ---------------------------------------------------------------------------
# You should redefine this in your script if you need it to actually do
# something.  It is intended to clean up anything temporary your script
# created. We define it here to prevent errors because some of our other
# functions call it (see _sigint_handler)
# ---------------------------------------------------------------------------
_cleanup() {
    _say "stub _cleanup called (doesn't actually do anything)"
    #rm -fr $tmpdir
}

# ---------------------------------------------------------------------------
# print log message preceded by a datetime stamp
#
#   _say [-n] message
#
# The optional '-n' (must be the FIRST parameter) will simply print the message
# without the datetime stamp.
#
# BUG: printf exhibits very strange behavior when using $2 as the message
# parameter (i.e. when called with -n).
# ---------------------------------------------------------------------------
_say() {
    if [ "$1" = '-n' ]; then
        # this ugliness is to handle the bug described above
        msg="$2"
        if [ "${msg:0:1}" = '-' ]; then
            printf '%s\n' $msg
        else
            printf "$msg\n"
        fi
        return
    fi

    local dt=`date | perl -ne 'chomp; print'`
    printf "### $dt: $1\n"
}

# ---------------------------------------------------------------------------
# print message wrapped in ANSI terminal escape codes for a given color
#
# _say_color [-n] message color
#
# See _say usage for -n
# ---------------------------------------------------------------------------
_say_color() {
    local cmd='_say'
    if [ "$1" = '-n' ]; then
        cmd='_say -n'
        shift
    fi

    local msg="$1"
    local color="$2"   # ANSI escape code for the color you want
    if [ -z "$color" ]; then
        $cmd "$msg"
        return
    fi

    $cmd "${color}${msg}${CCLEAR}"
}

# ---------------------------------------------------------------------------
# pre-defined colored message functions
# For example, to print a standard log message in red:
#
#   _say_danger [-n] message
#
# See _say usage for -n
# ---------------------------------------------------------------------------
_say_danger() {
    _say_color "$@" $CRED
}
_say_warning() {
    _say_color "$@" $CYELLOW
}
_say_comment() {
    _say_color "$@" $CBROWN
}
_say_success() {
    _say_color "$@" $CGREEN
}
_say_info() {
    _say_color "$@" $CCYAN
}

# ---------------------------------------------------------------------------
# Prompt the user for confirmation
#
# _confirm prompt_message
#
# If user responds in the affirmative (default) simply return
# If user responds in the negative exit the script (with success)
# ---------------------------------------------------------------------------
_confirm() {
    local prompt="$1"
    read -p "$prompt [Yn]: " ans
    if [ -z "$ans" -o "$ans" = "Y" -o "$ans" = "y" -o "$ans" = "yes" ]; then
        return
    else
       echo "aborting..."
       exit 0
    fi
}

# ---------------------------------------------------------------------------
# Prompt the user for input
#
# _ask prompt_message [default_value]
#
# To use, capture the output of a call to this function
#
#    answer=$(_ask 'What is the password?')
#    # provide a default
#    color=$(_ask 'What color?' 'blue')
#
# ---------------------------------------------------------------------------
_ask() {
    local no_echo=''
    if [ "$1" = '-s' ]; then
        no_echo='-s '
        shift
    fi

    local prompt="$1"
    local default="$2"
    local msg="$prompt"
    if [ -n "$default" ]; then
        msg="$msg [$default]:"
    fi

    read $no_echo -p "$msg " reply

    echo "${reply:-${default}}$nl"
}

# ---------------------------------------------------------------------------
# Print message and exit with error
# See _say usage for -n
# ---------------------------------------------------------------------------
_die() {
    _say_danger "$@"
    exit 1
}

# ---------------------------------------------------------------------------
# Extract the value of a .env file variable.  Note, it simply returns
# everything after the '=' sign. Specifically, it doesn't do any work to trim
# spaces. While *most* .env files don't allow spaces surrounding the '=', some
# do, so you may have to deal with that.
#
# To use, capture the output of a call to this function
#
#   local app_env=$(_env_val APP_ENV /path/to/.env)
# ---------------------------------------------------------------------------
_env_val() {
    local var="$1"
    local file="$2"

    echo $(egrep "^${var}\s*=" "${file}" | cut -d '=' -f 2)
}

# ---------------------------------------------------------------------------
# Get the given variable from the system's /etc/os-release file. This works for
# many modern distributions that support the /etc/os-release file
#
# To use, capture the output of a call to this function
#
#   echo Running on $(_os_release PRETTY_NAME)
#   local os_version=$(_os_release VERSION_ID)
# ---------------------------------------------------------------------------
_os_release() {
    local var="$1"

    # note use of bash specific indirect variable reference ${!var}
    (. /etc/os-release && echo ${!var})
}

# ---------------------------------------------------------------------------
# Get the string name of os distribution. This works for many modern
# distributions that support the /etc/os-release file
#
# To use, capture the output of a call to this function
#
#   local os_distribution=$(_get_os_distro)
# ---------------------------------------------------------------------------
_get_os_distro() {
    _os_release ID
}

# ---------------------------------------------------------------------------
# Checks the value passed as $1 and exits with that value if it is non-zero.
# Intended to simplify error checking and exit with a useful message on error.
#
#   _chkerr <code|array_of_codes> <description_of_what_was_called>
#
# This supports receiving an array of exit codes, for pipelines (obtained from
# $PIPESTATUS in bash).  Note that they must be passed as a single argument,
# typically by quoting.  In such a case, make sure you're using bash and
# passing all elements of the array, using the syntax: "${PIPESTATUS[*]}". The
# quotes, braces, and * are all required for it to work properly. See example
# below.
#
# Note, older versions of bash only set $PIPESTATUS for pipelines, however
# current versions seem to set it for every command executed.
#
# For example:
#
#   my_script.sh
#   _chkerr $? "my_script.sh"
#
#   scripts/abupgrade.sh 1.2.3
#   _chkerr $? 'abupgrade for version 1.2.3'
#
# Or like this after running a pipeline:
#
#   warnings=`cat foo.log | grep -i "warning:" | awk '{print $2}'`
#   _chkerr "${PIPESTATUS[*]}" "extract foo warnings"
# ---------------------------------------------------------------------------
#typeset -xf chkerr
_chkerr() {
    local rc_list=($1)      # return codes, convert to array
    local msg="$2"          # description of what was called

    idx=1
    for rc in "${rc_list[@]}"; do
        if [ "$rc" -ne 0 ]; then
            if [ "${#rc_list[@]}" -gt 1 ]; then
                msg="$msg pipeline element $idx"
            fi

            _say_danger "$msg exited with error (rc=$rc)"
            exit $rc
        fi
        idx=$((idx + 1))
    done
}
#export chkerr

# ===========================================================================
# The remaining functions are recent additions with limited testing...
#
# Since being forced to work on a bletcherous (new to me) platform recently, I
# have had to implement a few new os checking functions. Eventually one will
# win out...
# ===========================================================================

# ---------------------------------------------------------------------------
# Die if not running on the given OS. There may be a better way to do this...
# Parameters are one of 'mac' or 'linux'.
#
# For example:
#
#   # die if not running on a Mac
#   _os_check mac
# ---------------------------------------------------------------------------
_os_check() {
    local os="$1"

    if [[ $os = 'linux' && ! "$OSTYPE" =~ linux* ]]; then
        echo Should only be run on Linux
        exit 1
    fi
    if [[ $os = 'mac' && ! "$OSTYPE" =~ darwin* ]]; then
        echo Should only be run on a Mac
        exit 1
    fi
}

# ---------------------------------------------------------------------------
# Die if docker desktop is not running
# ---------------------------------------------------------------------------
_docker_is_running_check() {
    docker ps > /dev/null 2>&1
    local rc=$?

    if [ "$rc" -ne 0 ]; then
        _die -n "Docker Desktop does not appear to be running. Please start it then try again"
    fi
}

# ---------------------------------------------------------------------------
# Simply returns one of 'mac', 'linux', or 'other'
#
# I could have added this to one of my older get_os() functions, but so many of
# the checks are obsolete (True64, DEC, etc), or I just don't use anymore
# (AIX, HP-UX, etc) so I have no way to test.  Let's just keep it simple.
# ---------------------------------------------------------------------------
_get_os() {
    if [[ "$OSTYPE" =~ linux* ]]; then
        echo 'linux'
    elif [[ "$OSTYPE" =~ darwin* ]]; then
        echo 'mac'
    else
        echo 'other'
    fi
}

# ---------------------------------------------------------------------------
_spawn_linux_terminal() {
    gnome-terminal --tab -- $@
}

# ---------------------------------------------------------------------------
_spawn_mac_terminal() {
    local cmd="$@"
    osascript -e "tell app \"Terminal\"
        do script \"$cmd\"
    end tell"
}

# ---------------------------------------------------------------------------
_spawn_terminal() {
    local cmd="$@"

    if [ $(_get_os) = 'linux' ]; then
        _spawn_linux_terminal "$cmd"
    elif [ $(_get_os) = 'mac' ]; then
        _spawn_mac_terminal "$cmd"
    else
        _die "Unsupported os"
    fi
}

