#!/usr/local/bin/perl
# ------------------------------------------------------------------------
# $Id: perl.pl 33 2011-03-10 03:54:26Z mjeffe $
# 
# Usefull Perl code snippets
# ------------------------------------------------------------------------

# ------------------------------------------------------------------------
# Adds commas to a number for display purposes.
# ------------------------------------------------------------------------
sub commify {
   local $_  = shift;
   1 while s/^([-+]?\d+)(\d{3})/$1,$2/;
   return $_;
}

# example usage
print "number: ", commify("456300.45"), "\n";

# ------------------------------------------------------------------------
# prints the number of minutes since the file was modified. seconds are
# truncated by the int function.
# ------------------------------------------------------------------------
# full version
use File::stat;
use File::Basename;

$me = basename $0;
die "usage: $me file_name\n\n$me prints the number of minutes since file_name was last modified\n" if $ARGV[0] eq "";

$fs = stat($ARGV[0]);
$age_in_seconds = time() - $fs->mtime;
print int($age_in_seconds / 60) . "\n";

# this one liner alternative would print "stale" if the last modification time is greater than 1 hour ago
#print "stale\n" if -M $ARGV[0] > 1/24; 


# one line version
print (int((-M $ARGV[0]) * 24 * 60), "\n");

# can just use this line in a shell script directly
#age=`perl -e 'print (int((-M $ARGV[0]) * 24 * 60), "\n");' $file_name`



# -----------------------------------------------------------------------------
# strips dashes, spaces, and () from phone numbers
# -----------------------------------------------------------------------------
my $phone =~ s/\s|-|\(|\)//g;

# -----------------------------------------------------------------------------
# usefull to quote an non alphanumeric characters in a string
# from a shell script where something like sed or grep would complain.
# -----------------------------------------------------------------------------
# myval=`echo $1 | perl -ne 'print(quotemeta($_));'`
# sed "s/$val/$myval/" thefile


# -----------------------------------------------------------------------------
# add CR LF to the end of a file
# -----------------------------------------------------------------------------
# someproces | perl -lne 'print $_ . "\r";' > MSDOS_outfile.txt


# -----------------------------------------------------------------------------
# upload binary or text data in a cgi
# -----------------------------------------------------------------------------
my($bytesread,$buffer);
while($bytesread=read($upload_filehandle,$buffer,1024))
{
   print UPLOADFILE $buffer;
}

# -----------------------------------------------------------------------------
# function to open a file and return the filehandle (w/o resorting to sym ref)
# credit to Charlie Reddit
# -----------------------------------------------------------------------------
sub get_handle($) {
   # from an article by Randal Schwartz, "Perl of Wisdom" Linux Mgzn March 2000
   # call this function like this: my $fh = get_handle{$filename};

   # Note: if you build an array of file handles, you will need to dereference
   # them with a specific syntax in order to use them properly.  Like this:
   # print {$my_fh_array[$i]}, "printing to file $my_file_names[$i]\n";

   my $filename = shift;
   local *IN;
   open IN, $filename or die "$0:Can't open \"$filename\": $!";
   return *IN{IO};
}


# -----------------------------------------------------------------------------
# convert unix to msdos text file
# -----------------------------------------------------------------------------
#! /usr/local/bin/perl -n
chomp; print; print "\r\n";

# -----------------------------------------------------------------------------
# convert msdos to unix text file.  strips all \r (octal 012) and \n (octal
# 012) from each line then prints the line out with a single \n
# -----------------------------------------------------------------------------
perl -ne 's/\015//g; s/\012//g; print; print "\n";'

# -----------------------------------------------------------------------------
# simple convert csv to pipe delimited
# -----------------------------------------------------------------------------
use Text::ParseWords;

while (<>) {
   print join "|", quotewords(",", 0, $_);
}

