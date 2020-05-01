#!/usr/local/bin/perl -w
#
# Another bug-ugly utility from Matt's code archive.
#
# Find location of perl modules
#

sub myrequire {
    my($filename) = @_;
    $filename =~ s/::/\//g;
    $filename .= '.pm';
    my($realfilename,$result);
    ITER: {
        foreach $prefix (@INC) {
            $realfilename = "$prefix/$filename";
            if (-f $realfilename) {
                print STDOUT "Found $filename in $prefix\n";
                last ITER;
            }
        }
        die "Can't find $filename in \@INC";
    }
}

my $file = shift;
print STDOUT "Looking for $file\n";
myrequire($file);

