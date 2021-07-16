#!/usr/bin/perl -w

# This file is part of Krita
#
#  SPDX-FileCopyrightText: 2005 Sven Langkamp <sven.langkamp@gmail.com>
#
#  SPDX-License-Identifier: GPL-2.0-or-later


use strict;
use warnings;
use Archive::Zip qw( :ERROR_CODES );
use Archive::Zip::MemberRead;


sub printi18n($$$$) {
  # function that prints the actual output
  my ($name, $filename, $filename2, $linenum) = @_;
  chomp($name); # chomp out the null bytes at the end if relevant
  $name =~ s/\0*//;
  if ($name ne "")
    {
      if ($filename =~ /myb$/)
      {
        print "// i18n: Display name of resource, see context: MyPaint brush [path-to-file]/[resource-filename]\n";
      }
      else
      {
        print "// i18n: Display name of resource, see context: [path-to-resources]/[resource-type]/[resource-filename]\n";
      }

      if ($name =~ /^\w\)/)
      {
        print "// i18n: 'a)', 'b)' etc. in resource names are used to keep resources in a specific order ";
        print "when Krita sorts them alphabetically. The order will be kept only using the original/untranslated names\n";
      }
      if ($name =~ /DITH\b/)
      {
        print "// i18n: DITH probably means 'dithering'\n";
      }

      if ($linenum > 0)
      {
        print "// i18n: file: ".$filename.":".$linenum."\n";
      }

      print "i18nc(\"".$filename."\",\"".$name."\");\n";
    }
}

sub parseFromFilenameAuto($) {
  # function that prettifies filenames without needing to define the extension
  # because it figures out the extension on its own
  my $name = $_[0];
  my @extensions = split(/\./, $name);
  my $extension = $extensions[$#extensions];
  return parseFromFilename($name, $extension);
}

sub parseFromFilename($$) {
  # function that prettifies filenames
  # it extracts the filename from the full path, cuts off the extension and replaces '_  with ' '
  my $name = $_[0];
  my $extension = $_[1];
  chomp($name);
  my @path = split(/\//, $name);
  $name = $path[$#path];
  $name =~ s/_/ /g;
  if( $extension ne "" ) {
    $name =~ s/\.${extension}$//g;
  }
  return $name;
}

sub readGeneric($$$$) {
  # function that reads all the lines from either the proper perl file handle
  # or from the special buffer made out of the contents of the file in the bundle
  my ($fh, $zipSpecialBuffer, $bufferref, $bytesnum) = @_;

  my $response = undef;
  my $buffer = ${$bufferref};
  if(defined $fh)
  {
    $response = read($fh, $buffer, $bytesnum);
  }
  elsif(defined $zipSpecialBuffer)
  {
    my $where = $zipSpecialBuffer->{"pos"};
    $buffer = unpack("x$where a${bytesnum}", $zipSpecialBuffer->{"buffer"});
    $response = length($buffer);
    $zipSpecialBuffer->{"pos"} = $zipSpecialBuffer->{"pos"} + $bytesnum;
  }
  ${$bufferref} = $buffer;
  return $response;
}

sub readAllLinesGeneric($$) {
  # function that reads all the lines from either the proper perl file handle
  # or from the MemberRead from the Archive::Zip module file handle
  my ($fh, $ziph) = @_;
  my @response = undef;
  if(defined $fh)
  {
    @response = <$fh>;
  }
  elsif(defined $ziph)
  {
    my $i = 0;
    @response = ();
    while (defined(my $line = $ziph->getline()))
    {
      push(@response, $line);
      $i += 1;
    }
  }
  return @response;
}


sub readGbrBytesWise($$) {
  # function that extracts a name from a gbr file
  # it's in the function to allow easier error checking
  # (early returning)
  my ($fh, $ziph) = @_;

  my $success = 1;
  my ($bytes, $size, $version);
  $success = readGeneric($fh, $ziph, \$bytes, 4) == 4;

  return "" if not $success;

  $size = unpack("N", $bytes);
  $success = readGeneric($fh, $ziph, \$bytes, 4) == 4;

  return "" if not $success;

  $version = unpack("N", $bytes);
  if( $version == 1 )
  {
    $success = readGeneric($fh, $ziph, \$bytes, 12) == 12;
    return "" if not $success;

    my $name;
    $success = readGeneric($fh, $ziph, \$name, $size - 21) == $size - 21;
    return "" if not $success;

    return $name;
  }
  else
  {
    $success = readGeneric($fh, $ziph, \$bytes, 20) == 20;
    return "" if not $success;

    my $name;
    $success = readGeneric($fh, $ziph, \$name, $size - 29) == $size - 29;
    return "" if not $success;

    return $name;
  }

  return "";

}

sub readZipSpecialBuffer($)
{
  # this is a hack
  # $ziph->read($buffer, $bytes) didn't work but ->getline() did
  # this works for all binary files in the bundle that are supported at the moment
  my ($ziph) = @_;
  my $buffer = {};
  $buffer->{"pos"} = 0;
  my @array = readAllLinesGeneric(undef, $ziph);
  $buffer->{"buffer"} = join("\n", @array);
  return $buffer;
}




my @filenames = glob("./krita/data/gradients/*.ggr");
push( @filenames, glob("./krita/data/palettes/*.gpl"));
push( @filenames, glob("./krita/data/brushes/*.gih"));
push( @filenames, glob("./krita/data/brushes/*.gbr"));
push( @filenames, glob("./krita/data/brushes/*.svg"));
push( @filenames, glob("./krita/data/patterns/*.pat"));
push( @filenames, glob("./krita/data/patterns/*.png"));
push( @filenames, glob("./krita/data/paintoppresets/*.kpp"));
push( @filenames, glob("./krita/data/workspaces/*.kws"));
push( @filenames, glob("./krita/data/windowlayouts/*.kwl"));
push( @filenames, glob("./krita/data/gamutmasks/*.kgm"));
push( @filenames, glob("./plugins/paintops/mypaint/brushes/*.myb"));
push( @filenames, glob("./krita/data/symbols/*.svg"));


my %bundleForResource;
my %internalFilenameForResource;

# get the filename from the bundle
my @bundlenames = glob("./krita/data/bundles/*.bundle");
foreach my $bundlename (@bundlenames)
{
  my $bundle = Archive::Zip->new();
  unless ( $bundle->read( $bundlename ) == AZ_OK )
  {
    next;
  }
  my @memberNames = $bundle->memberNames();
  foreach my $member (@memberNames)
  {
    unless ($member =~ /xml$/ or $member eq "mimetype" or $member eq "preview.png") {
      my $newFilename = "$bundlename:$member";
      push(@filenames, $newFilename);
      $bundleForResource{$newFilename} = $bundle;
      $internalFilenameForResource{$newFilename} = $member;
    }
  }
}



my $isZip = 0;

my $i = 0;

foreach my $filename (@filenames)
{
  $i = $i + 1;

  my $fh = undef;
  my $ziph = undef;

  unless ( open($fh, '<'.$filename) )
  {
    $fh = undef;
    unless ($filename =~ /\.bundle/) {
      next;
    }
    $ziph = Archive::Zip::MemberRead->new($bundleForResource{$filename}, $internalFilenameForResource{$filename});

    unless (defined $ziph) {
      next;
    }
    $isZip = 1;
  }

  if( $filename =~ /ggr/ || $filename =~ /gpl/ || $filename =~ /gih/ )
  {
    my @lines = readAllLinesGeneric($fh, $ziph);

    if( $filename =~ /ggr/ || $filename =~ /gpl/ )
    {
      my @splited = split(/: /, $lines[1]);
      my $name = $splited[1];
      chomp($name);
      $name =~ s/\t$//; # Trim trailing \t, fixes the name for "swatche.gpl"
      printi18n($name, $filename, $filename, 2);
    }
    else
    {
      my $name = $lines[0];
      chomp($name);
      printi18n($name, $filename, $filename, 1);
    }
  }
  elsif( $filename =~ /svg$/ )
  {
    my @lines = readAllLinesGeneric($fh, $ziph);
    my $svg = join('', @lines);
    my $name = "";
    if( $svg =~ m:(<title.*?</title>):s )
    {
      my $titlesvg = $1;
      if( $titlesvg =~ m:>(.*?)</title>:s ) #not perfect, but should usually do a good job, at least for existing libraries
      {
        $name = $1;
        chomp($name);
        $name =~ s/&amp;/&/; # 'Pepper & Carrot Speech Bubbles' needs it
      }
    }
    if ($name eq "")
    {
      $name = parseFromFilenameAuto($filename);
    }
    printi18n($name, $filename, $filename, -1);
  }
  elsif( $filename =~ /kpp$/ || $filename =~ /kws$/ || $filename =~ /kwl$/ || $filename =~ /kgm$/ || $filename =~ /jpg$/ || $filename =~ /myb$/ || $filename =~ /png$/ || $filename =~ /kse$/)
  {
    # all of Krita's default brush presets and other resources with abovementioned extensions
    # are named the same way the file is called
    # so there is no need to parse the file itself to find the name inside of it
    my $extension = split(/\./, $filename);
    my $name = parseFromFilenameAuto($filename);
    printi18n($name, $filename, $filename, -1); # sadly, I'm not sure what the last number means exactly
  }
  elsif($filename =~ /gbr|pat/)
  {
    my $zipSpecialBuffer = undef;
    if(defined($ziph)) {
      $zipSpecialBuffer = readZipSpecialBuffer($ziph);
    }

    if( $filename =~ /gbr/ )
    {

      my $name = readGbrBytesWise($fh, $zipSpecialBuffer);
      if($name eq "")
      {
        $name = parseFromFilenameAuto($filename);
      }

      printi18n($name, $filename, $filename, -1);

    }
    elsif( $filename =~ /pat$/ )
    {
      my $bytes;
      my $name;
      readGeneric($fh, $zipSpecialBuffer, \$bytes, 4);
      my $size = unpack("N", $bytes);
      readGeneric($fh, $zipSpecialBuffer, \$bytes, 20);
      readGeneric($fh, $zipSpecialBuffer, \$name, $size - 25);
      if( $name eq "" )
      {
        $name = parseFromFilenameAuto($filename);
      }
      printi18n($name, $filename, $filename, -1);
    }
  }

  close($fh) if defined $fh;

}


