#! /usr/local/bin/perl
# ===================================================================
# Another bug-ugly utility from Matt's code archive.
#
# Description: 
#   Cleans out the archive directory.  If scheduled in cron then
#   this script will delete or report on files older than the 
#   age parameter. 
#   
# Revisions:
#  Date         userid    revision made
#  -----------  ------    -------------------------------------------
#  1-DEC-1999  mjeffe    created
# ===================================================================


# -----------------------
# import modules
# -----------------------

use Cwd;
use Getopt::Long;
   $Getopt::Long::ignorecase = 1;

# -----------------------
# global variables and
# constants
# -----------------------

#$this = $0;
$this = `basename $0`;
chomp($this);

$USAGE = "
usage:
  $this --dir=/absolute/path/to/directory
        [--age=number_of_days] [--verbose] [--report] [--rename=file_prefix]

description:
  $this is intended to be used to clean up old files in a log or
  archive directory, but it will clean any directory passed in the --dir= 
  parameter.

parameters:
  --dir=      The absolute path for the root directory to clean.  $this 
              decends into all sub directories of the root directory 
              and cleans those as well.
  --age=      Action will be taken for all files older than this number 
              of days.  The default is 31 days, and the default action is 
              to delete the file.
  --rename=   This option takes a string argument which is used as a
              prefix to rename old files rather than the default action
              which is to delete the file.
  --report    Generates verbose output for all old files, but does not alter
              or delete them.
  --verbose   This option causes $this to output messages about
              the directories and files it is processing.

";



# -----------------------
# verify command line 
# parameters
# -----------------------

%opts = ();
$result = GetOptions(\%opts,'age:i','dir:s','rename=s',
                      'recurse','report','verbose');
verify_opts();



####################################################################
#                  MAIN 
####################################################################

print "\n" if ( defined($opts{"verbose"}) || defined($opts{"report"}) ); 
   


# root dir to clean
$home = $opts{"dir"};
chdir $home || die "$this: can't cd into $home...\n$!";

# get a list of ALL directories below this directory - the lazy way...
@dirs = `find . -type d`;

# process each directory
foreach $dir (@dirs)
{
   chomp($dir);

   #step into dir
   chdir $dir || die "$this: can't cd into $dir...\n$!";
   ++$dirnum;
   if ( defined($opts{"verbose"}) || defined($opts{"report"}) ) 
   {
      print "processing files in $dir\n";
   }

   # get a list of all files in this directory
   @files = `ls`;

   # process each file in this directory
   foreach $file (@files)
   {
      chomp($file);

      # only process files with creation date older than age parameter
      if ( (-f $file) && (-M $file > $opts{"age"}) )
      {
         ++$filenum;

         if ( defined($opts{"report"}) )
         {
            print "  $file is ",int(-M $file)," days old\n"; 
         }
         elsif ( defined($opts{"rename"}) ) 
         {
            $prefix = $opts{"rename"};

            if ( defined($opts{"verbose"}) ) 
            {
               print "  $file is ",int(-M $file)," days old, ";
               print "renaming to $prefix$file...\n";
            }

            #system "cp $file $prefix$file" || die "can't rename $file...\n";
            rename($file, $prefix.$file) || die "can't rename $file...\n$!";
         }
         else 
         {
            if ( defined($opts{"verbose"}) ) 
            {
               print "  $file is ",int(-M $file)," days old, deleting...\n";
            }

            unlink $file || die "can't delete $file...\n$!";
         }
      }
   }
   
   # go back to archive home
   chdir $home || die "$this: can't cd into $home...\n";

   # if we are not going to recurse into subdirectories then break out now
   if ( ! $opts{"recurse"} ) {
      last;
   }
}

if ( defined($opts{"verbose"}) || defined($opts{"report"}) ) {
   print "\n$this: $filenum files processed in $dirnum directories.\n\n";
}


####################################################################
#                          SUBROUTINES                             #
####################################################################


sub verify_opts
{
   # ---------------------------
   # check for required command
   # line options
   # ---------------------------

   if ( $result == 0 ) 
   {
      #print "$this: problem with command line arguments...\n";
      print $USAGE;
      exit 1; 
   }
   
   # root directory for archive
   elsif ( !defined($opts{"dir"}) )
   {
      print "$this: option --dir is required...\n";
      print $USAGE;
      exit 1;
   }

   # ---------------------------
   # now set defaults for
   # optional args
   # ---------------------------
   
   # age in days
   if ( defined($opts{"age"}) )
   {
      if ( $opts{"age"} <= 0 ) 
      {
         print "$this: --age= must be greater than 0...\n";
         print $USAGE;
         exit 1;
      }
   }
   else {
      $opts{"age"} = 31;
   }

}

# ---------------------------------------------------------------


