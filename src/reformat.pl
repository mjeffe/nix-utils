#!/usr/local/bin/perl
# ------------------------------------------------------------------------
# $Id: reformat.pl 319 2018-07-03 21:39:57Z higuito $
#
# Another bug-ugly utility from Matt's code archive.
#
# this reformats a FIXED WIDTH file from one layout to another.
# ------------------------------------------------------------------------

use warnings;
use strict;
use File::Basename;

my $this = basename($0);

sub read_layout($);


if ( scalar(@ARGV) != 2 ) {
   
   die <<EOF;

Usage: $this existing_layout new_layout < in_file > reformated_file

Description: $this reformats a FIXED WIDTH file from one layout to another.  It
   will use the existing layout as the basis for field names.  Output blanks in
   non existent output fields.  If output field lengths are shorter than input
   the data will be truncated.

EOF
}


my @inlay = read_layout($ARGV[0]);
my @outlay = read_layout($ARGV[1]);
my %outlay = map {(split(/,/,$_))[0] => 1} @outlay;
my %out;
my $outbuff;

# read in input data, parse into input layout, then print in output layout format
while ( my $line = <STDIN> ) {
   chomp($line);

   # run through each field in the input layout and chop the data field out
   # of the line of intput data, and put it into the %out hash
   foreach my $inl ( @inlay  ) {
      my ($field, $start, $end, $len) = split(/,/, $inl);
      if ( exists($outlay{$field}) ) {
         $out{$field} = substr($line, $start - 1, $len); 
      } else {
         $out{$field} = ' ' x $len;
      }
   }

   # now output the line in the output layout, plugging blanks for fields
   # that do not exist on the input
   foreach my $outl ( @outlay ) {
      my ($field, $start, $end, $len) = split(/,/, $outl);
      if ( exists($out{$field}) ) {
         $outbuff .= substr($out{$field}, 0, $len);
      } else {
         $outbuff .= ' ' x $len;
      }
   }

   print "$outbuff\n";
   $outbuff = "";
}



# -------------------------------------------------------------------
sub read_layout($) {
   my ($file) = @_;
   my @out;
   
   open(IN,"<$file") || die "ERROR - can't open layout file $file\n";

   while (my $line = <IN> ) {
      chomp($line);
      next if ( $line =~ m/^\s*$/ );   # strip out blank lines
      next if ( $line =~ m/^\s*#/ );   # skip comment lines
      next if ( $line =~ m/^LINE_TERMINATOR_LF/i );   # skip an acxiread produced layout's final line
      $line =~ s/\s+//g;               # remove any spaces in the line
      push(@out, lc($line));
   }

   close(IN) || die "ERROR - can't close layout file $file\n";

   return @out;
}  


