#!/usr/bin/perl -w

use strict;

# A small script to convert current-style KOffice tar storages to storages
# compatible with KOffice 1.0 and KOffice 1.1(.1)

# Note to developers:
#  Add the PID (in Perl: $$ ) to all fixed temporary directory/file names,
#  so that this script can be run multiple times at once.

# Holds the directory tree
my @rootdir;
my $tmpdir = "/tmp/kofficeconverter$$" ;
print "Using temporary directory... $tmpdir\n";
# Holds the source/dest of the files to fix
my @needFixing;

# Walk the whole archive and collect information about the files
# This creates one array, containing another array for every directory
# we found (recursively). Additionally this array holding a directory
# holds the name of the directory and the path.
sub explore {
  my($path) = @_;
  my(@dir);

  print "   Exploring: $path\n";
  chdir($path);
  opendir(DIR, $path) || die "Couldn't open the directory: $!";
  my @contents = readdir(DIR);
  my $i = 0;
  foreach(@contents) {
    if($_ eq "." || $_ eq "..") {
      next;      # we're not intersted in . and ..
    }
    if(-d $_) {
      $dir[$i] = [ $_, $path, [ explore($path . '/' . $_) ] ];
      chdir($path);    # back to the directory where we come from
    }
    else {
      $dir[$i] = $_;
    }
    $i = $i + 1;
  }
  closedir(DIR);
  return @dir;
}

# Dumps the scary datastructure we built
sub dumpTree {
  my(@dir) = @_;
  foreach(@dir) {
    if(ref($_) eq 'ARRAY') {
      print $_->[0], "   (", $_->[1], ")\n";
      dumpTree(@{$_->[2]});
    }
    else {
      print $_ . "\n";
    }
  }
}

# Finds the files where we have to fix part references (->maindoc.xml)
sub findCandidates {
  my($dref, $currentdir, $parentdir) = @_;
  my @dir = @{$dref};
  #print "current: $currentdir, parentdir: $parentdir\n";
  foreach(@dir) {
    if(ref($_) eq 'ARRAY') {
      #print $_->[0], "   (", $_->[1], ")\n";
      findCandidates(\@{$_->[2]}, $_->[0], $_->[1]);
    }
    else {
      if($_ =~ m/maindoc\.xml/) {
	my $source = $parentdir . '/' . $currentdir . "/maindoc.xml";
	my $dest = $parentdir . '/' . $currentdir . ".xml";
	push(@needFixing, [ $source, $dest ]);
      }
    }
  }
}

# No need to move around elements of the root directory, these are handled
# separately anyway. Therefore we call findCandidates only on subdirs
sub findMainDocuments {
  foreach(@rootdir) {
    if(ref($_) eq 'ARRAY') {
      findCandidates(\@{$_->[2]}, $_->[0], $_->[1]);
    }
  }
}

# Factorizes the common regexp code between maindoc.xml and parts
sub fixLine {
  my($line, $prefix) = @_;

  if($line =~ m/(\s*\<object\s+mime=\"[^\"]*\"\s+url=\")([^\"]*)(\".*)/) {
    return $1 . $prefix . $2 . $3 . "\n";
  }
  elsif($line =~ m/(\s*\<OBJECT\s+mime=\"[^\"]*\"\s+url=\")([^\"]*)(\".*)/) {
    return $1 . $prefix . $2 . $3 . "\n";
  }
  elsif($line =~ m/(\s*\<KEY\s+.*\s+)filename(=\"[^\"]*\".*)/) {
    my($tmp) = $1 . "key" . $2 . "\n";
    if($tmp =~ m/(\s*\<KEY\s+.*\s+name=\")([^\"]*)(\".*)/) {
      return $1 . $prefix . $2 . $3 . "\n";
    }
    return $tmp;
  }
# Replace pictures by images, as cliparts will never work with only this script.
  elsif($line =~ m%\s*\<PICTURE%) {
      $line =~ s%\<PICTURES%\<PIXMAPS% ;
      $line =~ s%\<PICTURE%\<IMAGE% ;
  }
  elsif($line =~ m%\s*\</PICTURE%) {
      $line =~ s%\</PICTURES%\</PIXMAPS% ;
      $line =~ s%\</PICTURE%\</IMAGE% ;
  }
  elsif($line =~ m%\s*\<BACKPICTUREKEY%) {
      $line =~ s%\<BACKPICTUREKEY%\<BACKPIXKEY% ;
  }
  return $line;
}

# Walks through all the documents and fixes links. "Fixes" all the
# candidates we found
sub fixLinks {
  for my $item (@needFixing) {
    my $prefix = substr $item->[0], length($tmpdir)+1;
    $prefix =~ m,^(.*?)(maindoc\.xml),;
    $prefix = "tar:/" . $1;
    open(SOURCE, "<$item->[0]") || die "Couldn't open the source file: $!\n";
    open(DEST, ">$item->[1]") || die "Couldn't open the destination file: $!\n";
    while(<SOURCE>) {
      print DEST fixLine($_, $prefix);
    }
    close(SOURCE);
    close(DEST);
  }
}

# Get rid of the moved files
sub removeOldFiles {
  foreach(@needFixing) {
    system("rm -rf $_->[0]");
  }
}

# Special case for the main document as we have to use a temporary
# file and stuff like that. We only have to fix part references here.
sub fixMainDocument {
  open(SOURCE, "<$tmpdir/maindoc.xml");
  open(DEST, ">$tmpdir/tmp.xml");
  while(<SOURCE>) {
    print DEST fixLine($_, "tar:/");
  }
  close(SOURCE);
  close(DEST);
  system("mv $tmpdir/tmp.xml $tmpdir/maindoc.xml");
}

##################################################
# The execution starts here
##################################################
if($#ARGV != 1) {
    print "Script to convert current storages to KOffice 1.0/1.1.x compatible ones.\n";
    print "Usage: perl fix_storage.pl <inputfile> <outputfile>\n";
    exit(1);
}

# remember where we came from
chomp(my $cwd = `pwd`);

# clean up properly
system("rm -rf $tmpdir");
mkdir $tmpdir || die "Couldn't create tmp directory: $!\n";


print "Trying to detect the type of archive... ";
my($mime) = `file -i -z $ARGV[0]`;

if($mime =~ m,application/x-tar,) {
  print "tar.gz\n";
  print "Uncompressing the archive...\n";
  system("tar -C $tmpdir -xzf $ARGV[0]");
}
elsif($mime =~ m,application/x-zip,) {
  print "zip\n";
  print "Uncompressing the archive...\n";
  system("unzip -qq -d $tmpdir $ARGV[0]");
}

print "Browsing the directory structure...\n";
@rootdir = explore($tmpdir);

# debugging
#dumpTree(@rootdir);

print "Find candidates for moving...\n";
findMainDocuments();

print "Moving and fixing relative links...\n";
fixLinks();
removeOldFiles();
fixMainDocument();

print "Creating the archive...\n";
chdir($tmpdir);
system("tar czf tmp$$.tgz *");
chdir ($cwd);
system("mv $tmpdir/tmp$$.tgz $ARGV[1]");

print "Cleaning up...\n";
# clean up properly
system("rm -rf $tmpdir");

print "Done.\n";
