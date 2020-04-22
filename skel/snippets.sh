# ---------------------------------------------------------------------------
# Useful shell script snippets
# ---------------------------------------------------------------------------

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




