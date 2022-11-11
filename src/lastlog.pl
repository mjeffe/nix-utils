#!/usr/local/bin/perl
# -------------------------------------------------------------------------
# $Id: lastlog.pl 18 2011-03-09 21:47:56Z mjeffe $
#
# Another bug-ugly utility from Matt's code archive.
#
# ! ! ! NOTE this needs to be run as root so it can read the lastlog file.
#
# CREDITS:
#
# I wrote this from information I found here:
#
#   http://www.unix.com.ua/orelly/networking/puis/ch10_01.htm
#   http://www.hcidata.info/lastlog.htm
#
# DESCRIPTION:
#
# Display entries in the binary file /log/var/lastlog (or the equivalent on
# your system).  The reason I wrote this is that the lastlog command on linux
# truncates the host.  This gives me control over output.  And I wanted to
# learn how to read binary data in perl...
#
# On Linux, the lastlog record structures are defined in
# /usr/include/bits/utmp.h and on my machine look like this:
#
# #define UT_LINESIZE	32
# #define UT_HOSTSIZE	256
#
# struct lastlog
#   {
#     __time_t ll_time;
#     char ll_line[UT_LINESIZE];
#     char ll_host[UT_HOSTSIZE];
#   };
#
# ---------------------
# This is an alternative solution that worked on one box, but not on another.
# I need study it a bit more when I have a chance.  This reads from stdin, so
# redirect /var/log/lastlog into this.  Slurping all of stdin may be a bit of
# a problem...
#
# my $recs = "";
# while (<>) {
#    $recs .= $_
# };
# 
# my $uid = 0;
# foreach (split(/(.{292})/s,$recs)) {
#    next if length($_) == 0;
#    my ($binTime,$line,$host) = $_ =~/(.{4})(.{32})(.{256})/;
#    if (defined $line && $line =~ /\w/) {
#       $line =~ s/\x00+//g;
#       $host =~ s/\x00+//g;
#       printf("%5d %s %8s %s\n",$uid,scalar(gmtime(unpack("I4",$binTime))),$line,$host)
#    }
#    $uid++
# }
# print"\n";
#
# 
# -------------------------------------------------------------------------

use strict;
use warnings;


my $fname = (shift || "/var/log/lastlog");

# build a uid->username lookup hash from information in the password file.
# NOTE: on LDAP controlled machines, this may not be available
my %names;
my ($name, $junk, $uid);
setpwent;
while ( ($name, $junk, $uid) = getpwent ) {
   $names{$uid} = $name;
}
endpwent;

# print header
print "User    Date                       Port     From\n";
print "-" x 79 . "\n";

#use vars qw($user $date $time $port $host);
my ($user,$date,$time,$port,$host);
format STDOUT =
#@<<<<<  @<<<<<<<<<<<<<<<<<<<<<<<<  @<<<<<<  @<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<...
@<<<<<  @<<<<<<<<<<<<<<<<<<<<<<<<  @<<<<<<  @*
$user,  $date,                     $port,   $host
.

# read a record at a time (292 bytes - see above record structure)
open(LASTL, $fname) or die "Can't open $fname: $!\n";
for ($uid = 0; read(LASTL, my $record, 292); $uid++) {
   ($time, $port, $host) = unpack('l A32 A256', $record);
   next unless $time;

   $date = gmtime($time);

   $user = $uid;
   if ( exists($names{$uid}) ) {
      $user = $names{$uid};
   }

   write;

}
close LASTL;

