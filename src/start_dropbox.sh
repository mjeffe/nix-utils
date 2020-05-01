#!/bin/bash
#
# start multiple dropbox daemons (work, personal, etc)
#
# This script assumes personal is default
#
# to set up multiple dropboxes:
#
# disable Ubuntu's startup applications for dropbox
# mkdir .dropbox-arc
# HOME=/home/mjeffe/.dropbox-arc dropbox start -i
# once installed, kill the daemon
# create whatever symlink you want to /home/mjeffe/.dropbox-arc/Dropbox (ARC)
# add this script to Ubuntu's startup applications
#

killall dropbox

base=$HOME

# start personal dropbox
$base/.dropbox-dist/dropboxd 2> /dev/null &

# start ARC dropbox
HOME=$base/.dropbox-arc $base/.dropbox-dist/dropboxd 2> /dev/null &

