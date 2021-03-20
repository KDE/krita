#!/usr/bin/perl -w

# This file is part of Krita
#
#  SPDX-FileCopyrightText: 2005 Sven Langkamp <sven.langkamp@gmail.com>
#
#  SPDX-License-Identifier: GPL-2.0-or-later


use strict;
use warnings;

sub printi18n($$$$) {
  if ($_[0] ne "") 
    {
      if ($_[3] > 0)
      {
        print "// i18n: file: ".$_[2].":".$_[3]."\n";
      }
          print "i18nc(\"".$_[1]."\",\"".$_[0]."\");\n";
    }
}

my @filenames = glob("./krita/data/gradients/*.ggr");
push( @filenames, glob("./krita/data/palettes/*.gpl"));
push( @filenames, glob("./krita/data/brushes/*.gih"));
push( @filenames, glob("./krita/data/brushes/*.gbr"));
push( @filenames, glob("./krita/data/patterns/*.pat"));
foreach my $filename (@filenames)
{
  unless ( open(FILE, '<'.$filename) )
  {
    next;
  }
  if( $filename =~ /ggr/ || $filename =~ /gpl/ || $filename =~ /gih/ )
  {
    my @lines = <FILE>;
    close(FILE);
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
    else
    {
      read(FILE, my $bytes, 4);
      my $size = unpack("N", $bytes);
      read(FILE, $bytes, 20);
      read(FILE, my $name, $size - 25);
      printi18n($name, $filename, $filename, -1);
    }
  }
}


