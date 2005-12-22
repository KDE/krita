#!/usr/bin/perl -w

# This file is part of Krita
#
#  Copyright (c) 2005 Sven Langkamp <longamp@reallygood.de>
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.


use strict;
use warnings;

sub printi18n {
  print "i18n(\"".$_[0]."\");\n";
}

my @filenames = glob("./data/gradients/*.ggr");
push( @filenames, glob("./data/palettes/*.gpl"));
push( @filenames, glob("./data/brushes/*.gih"));
push( @filenames, glob("./data/brushes/*.gbr"));
push( @filenames, glob("./data/patterns/*.pat"));
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
      printi18n $name;
    }
    else
    {
      my $name = $lines[0];
      chomp($name);
      printi18n $name;
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
        printi18n $name;
      }
      else
      {
        read(FILE, $bytes, 20);
        read(FILE, my $name, $size - 29);
        printi18n $name;
      }
    }
    else
    {
      read(FILE, my $bytes, 4);
      my $size = unpack("N", $bytes);
      read(FILE, $bytes, 20);
      read(FILE, my $name, $size - 25);
      printi18n $name;
    }
  }
}


