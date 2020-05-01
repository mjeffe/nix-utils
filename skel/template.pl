#!/usr/bin/perl
# ############################################################################
# $Id: template.pl 17 2011-03-09 21:06:25Z mjeffe $
#
# This is my basic perl script starting template.
# ############################################################################

use DBI;
use Getopt::Long;
use File::Basename;
use Data::Dumper;
#use FileHandle;  STDOUT->autoflush(1);
$| = 1;   # autoflush stdout

use strict;
use warnings;

# globals
my $this = basename($0);
my $VERSION = '$Id: template.pl 17 2011-03-09 21:06:25Z mjeffe $' . "\n";
my %opts = ();          # command line options

# protos
sub main();
sub get_opts();
sub usage();
sub get_db_connection();
sub end_db_connection($);
sub do_something();



# ##########
# MAIN
# ##########
&main;
sub main() {

   get_opts();
   do_something();

   exit(0);
}




# ##########
# FUNCTIONS
# ##########

# ------------------------------------------------------------------------
# add your logic
sub do_something() {
   print "This is what we can do so far...\n\n";

   print "Here are my command line opts:\n", Dumper(\%opts);

   my $dbh = get_db_connection();
   my $sth = $dbh->prepare("select 'foo' from dual");
   $sth->execute();
   while (my @row = $sth->fetchrow_array()) {
      print "Here is my db query result = " . uc($row[0]) . "\n";
   }
   $sth->finish;
   end_db_connection($dbh);
}


# ------------------------------------------------------------------------
# connect to db connection
sub get_db_connection() {
   my $dbh = DBI->connect(
      "dbi:Oracle:" . $opts{sid},
      $opts{userpass}, '',
      { PrintError => 0, RaiseError => 1 }
   )  || die $DBI::errstr;

   return $dbh;
}


# ------------------------------------------------------------------------
# disconnect from db
sub end_db_connection($) {
   my ($dbh) = shift;
   $dbh->disconnect || die $DBI::errstr;
}



# ------------------------------------------------------------------------
# get, verify and set defaults on command line options
sub get_opts() {

   $Getopt::Long::ignorecase = 1;
   Getopt::Long::Configure ("bundling");   # allow -vvv
   my $ok = GetOptions(\%opts,
      'help|h',
      'version',
      'verbose|v+',    # the trailing + allows multiple -v 
      'ecr_job_id|j=s',
      'userpass|u=s',
      'sid|s=s',
   );
   exit(1) if not $ok;
   usage() if ( exists($opts{help}) );
   die "$VERSION" if ( exists($opts{version}) );

   #$opts{userpass} = $ENV{ECR_SCHEMA} . '/' . $ENV{ECR_ORAPW} unless(exists($opts{userpass}));
   #$opts{ecr_job_id} = $ENV{ECR_JOB_ID} || "" unless(exists($opts{ecr_job_id}));
   $opts{sid} = "" unless(exists($opts{sid}));  # oracle will default to $ORACLE_SID
   $opts{verbose} = 0 unless(exists($opts{verbose}));
   #print Dumper(\%opts);  exit 1;

   # check for required command line opts
   die "$this: missing required -u|--userpass option\n" unless ( exists($opts{userpass}) );
   if ( $opts{userpass} !~ m?.+/.+? ) {
      die "$this: user/password is not formed correctly\n";
   }
   if (! defined($opts{sid}) && ! defined($ENV{ORACLE_SID}) ) {
      die "No Database SID in environment and not provided on command line\n";
   }

}


# ------------------------------------------------------------------------
sub usage() {
   eval "use Pod::Usage qw( pod2usage )";

   if ( $@ ) {
       print <<"END";

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
* Sorry cannot display help - Pod::Usage was not found.
* Try running "perldoc $this", or just view the bottom of $this.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

END
   } else {
      print "\n";
      pod2usage( -exitval => 'NOEXIT',
                 -verbose => 3,
                 -output  => '-',
                 #-output  => \*STDOUT,
               );
   }

   exit 1;
}




__END__

=pod

=head1 NAME

   $this - does something amazing.  BTW, perl variables will not expand in pod.

=head1 USAGE

   $this [OPTIONAL OPTIONS]

=head1 DESCRIPTION

   $this does something really amazing, and this is a much long description of
   that amazing this it does.

=head1 OPTIONS

=over 4

=item B<-h, --help>

Displays this help message.

=item B<--version>

Prints the version number

=item B<-u, --userpass=user/pass>

Provide the Oracle username and password separated with a "/" character.  
Defaults to $ECR_SCHEMA/$ECR_ORAPW

=item B<-s, --sid=SID>

Provide the Oracle database SID.  Defaults to $ORACLE_SID

=item B<-v, --verbose>

Include more information in the output.  A greater number of -v options present
on the command line will produce greater amounts of output.  Currently a single
-v will include some stuff, a second -v will include more stuff and a third -v
will really give you a buch of stuff.

=back

=head1 PREREQUISITES

Before running this, you should source I<ecr.conf>.

=head1 BUGS

Please report bugs or enhancement requests to Matt Jeffery - I<matt@mattjeffery.dev>.

=cut




