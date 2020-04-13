#!/usr/bin/env perl
#
# Another bug-ugly utility from Matt's code archive.
#
# This uses the ImageMagic tools to convert large images into small jpgs
# for use on a web site.  It's just a batch way of reducing image files.
#
# There is virtually no error handeling, and it convert's anything
# indiscriminately, so you could get plenty of garbage.
#

use strict;
use warnings;
use File::Basename;

my $dir = ".";
my $smdir = "$dir/web";
my $factor = $ARGV[0];

if ( ! defined($factor) ) {
   print "usage: ", basename($0), " resize_factor\n";
   exit 1;
}

opendir(DIR, $dir) || die "Can't open dir $dir : $!";
my @images = grep { !/^\./ && -f "$dir/$_" } readdir(DIR);
closedir DIR;

if ( ! -d $smdir ) {
   print "$smdir does not exist, creating...\n";
   mkdir($smdir)  || die "Can't mkdir $smdir : $!";
}

foreach my $f (@images) {
   my $fname = basename($f);
   $fname =~ m/^(\S+)\./;
   $fname = $1;
   my $cmd = "convert $f -resize $factor $smdir/$fname" . "_sm.jpg";
   #print "command: $cmd\n";
   system($cmd);
}

