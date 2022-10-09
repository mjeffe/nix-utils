#!/bin/bash

# NOTE, we could simpley capture the output of `git branch -a` and processes it
# but that is too dangerous for my tastes. I even have a check in place to try
# and prevent disaster, however that is the real problem with this script.  We
# **try** to protect ourselves from deleting important branches, but it's risky.

# run `git branch -a` and paste the list here or call this script, passing it
# all branches on the command line

#branches="
#  remotes/origin/ang-337
#  remotes/origin/ang-356
#"
branches="$@"
DO_NOT_DELETE_BRANCHES='HEAD dev develop stage staging master main'

if [ ${#branches} -lt 1 ]; then
    echo $(basename $0) branch [branch2...]
    exit 1
fi

_get_branch_name() {
    local branch="$1"

    local b=$(echo $f | sed "s/remotes\///" | sed "s/origin\///")

    # this is the risky bit (See notes in header) If we match one of the
    # $DO_NOT_DELETE_BRANCHES, rename it to something unlikely to be a real branch
    if echo "$DO_NOT_DELETE_BRANCHES" | grep -w "$b" > /dev/null; then
        echo 'We-are-no-longer-the-knights-who-say-ni-We-are-now-the-knights-who-say-ekki-ekki-ekki-pitang-zoom-boing'
        return
    fi

    echo $b
}


for f in $branches; do
    b=$(_get_branch_name "$f")
    echo "DELETING REMOTE BRANCH: $b ($f)"
    #git push origin --delete $b
done





