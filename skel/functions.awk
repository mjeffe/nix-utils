# ****************************************************
# $Id: functions.awk 162 2002-01-31 15:47:52Z mjeffe $
#
# useful functions that can be included in an awk script.
# See also shell.sh for awk one liners.
# ****************************************************

function ltrim(s) {
  if ( match(s, /^ */) ) {
     # grab everyting after the spaces
     return substr(s, RSTART + RLENGTH)
  }
  return s
}


function ljustify(s) {
  if ( match(s, /^ */) ) {
     # grab everything after the spaces, then concat the spaces back on the end
     return substr(s, RSTART + RLENGTH) substr(s,RSTART, RLENGTH)
  }
  return s
}


