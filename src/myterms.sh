#!/bin/bash

# start up terminals in the initial positions I like.

# DUAL MONITOR NOTES
# These are set to work with dual monitors at the office.
# Origin (+0+0) is in the top left corner of the left monitor.
#
# There is probably a way to query the OS to find out if we are working with
# single or dual monitors and behave accordingly. Future enhancement...


# ------------- right monitor -------------
# top left
gnome-terminal --window-with-profile=mjeffe --geometry=120x24+1024+0
# top right 
#gnome-terminal --window-with-profile=mjeffe --geometry=120x24-310+0
# middle right 
gnome-terminal --window-with-profile=mjeffe --geometry=120x24-310+170
# bottom right
gnome-terminal --window-with-profile=mjeffe --geometry=120x24-0-0
# bottom left
gnome-terminal --window-with-profile=mjeffe --geometry=120x24+1024-0

# ------------- left monitor -------------
# bottom right
gnome-terminal --window-with-profile=mjeffe --geometry=120x24+300-0


