#!/usr/bin/perl -w

# This file is part of Krita
#
#  SPDX-FileCopyrightText: 2005 Sven Langkamp <sven.langkamp@gmail.com>
#
#  SPDX-License-Identifier: GPL-2.0-or-later


use strict;
use warnings;

sub printi18n($$$$) {
  my ($name, $filename, $filename2, $linenum) = @_;
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
        print "// i18n: file: ".$_[2].":".$_[3]."\n";
      }

      print "i18nc(\"".$_[1]."\",\"".$_[0]."\");\n";
    }
}

sub parseFromFilenameAuto($) {
   my $name = $_[0];
   my @extensions = split(/\./, $name);
   my $extension = $extensions[$#extensions];
   return parseFromFilename($name, $extension);
}

sub parseFromFilename($$) {
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


foreach my $filename (@filenames)
{
  unless ( open(FILE, '<'.$filename) )
  {
    next;
  }
  if( $filename =~ /ggr/ || $filename =~ /gpl/ || $filename =~ /gih/ )
  {
    my @lines = <FILE>;
    if( $filename =~ /ggr/ || $filename =~ /gpl/ )
    {
      my @splited = split(/: /, $lines[1]);
      my $name = $splited[1];
      chomp($name);
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
    my @lines = <FILE>;
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
  elsif( $filename =~ /kpp$/ || $filename =~ /kws$/ || $filename =~ /kwl$/ || $filename =~ /kgm$/ || $filename =~ /jpg$/ || $filename =~ /myb$/ || $filename =~ /png$/)
  {
    # all of Krita's default brush presets and other resources with abovementioned extensions
    # are named the same way the file is called
    # so there is no need to parse the file itself to find the name inside of it
    my $extension = split(/\./, $filename);
    my $name = parseFromFilenameAuto($filename);
    printi18n($name, $filename, $filename, -1); # sadly, I'm not sure what the last number means exactly
  }
  else
  {
    if( $filename =~ /gbr/ )
    {
      read(FILE, my $bytes, 4);
      my $size = unpack("N", $bytes);
      read(FILE, $bytes, 4);
      my $version = unpack("N", $bytes);
      if( $version == 1 )
      {
        read(FILE, $bytes, 12);
        read(FILE, my $name, $size - 21);
        printi18n($name, $filename, $filename, -1);
      }
      else
      {
        read(FILE, $bytes, 20);
        read(FILE, my $name, $size - 29);
        printi18n($name, $filename, $filename, -1);
      }
    }
    elsif( $filename =~ /pat$/ )
    {
      read(FILE, my $bytes, 4);
      my $size = unpack("N", $bytes);
      read(FILE, $bytes, 20);
      read(FILE, my $name, $size - 25);
      if( $name eq "" )
      {
        $name = parseFromFilenameAuto($filename);
      }
      printi18n($name, $filename, $filename, -1);
    }
  }
  close(FILE);
}


