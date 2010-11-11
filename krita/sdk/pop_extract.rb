#!/usr/bin/ruby
#  This file is part of the KDE project
#
#  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

require 'Qt4'

def displayHelp()
  puts "This script extract paintop settings from paintop presents."
  puts ""
  puts "Usuage:"
  puts "pop_extract.rb --get paintop.kpp            extract the settings"
  puts "pop_extract.rb --set filename paintop.kpp   replace the settings by the content of filename"
  puts ""
  puts "Where type is: #{listavailabletemplates().join(', ')}"
end

if(ARGV.size == 2 and ARGV[0] == "--get")
  filename = ARGV[1]
  reader = Qt::ImageReader.new(filename, Qt::ByteArray.new("PNG"))
  puts reader.text("preset")
elsif(ARGV.size == 3 and ARGV[0] == "--set")
  filename = ARGV[2]
  preset_filename = ARGV[1]
  reader = Qt::ImageReader.new(filename, Qt::ByteArray.new("PNG"))
  version = reader.text("version")
  img = Qt::Image.new
  exit(-1) unless (reader.read(img))
  lines = File.open(preset_filename).collect.join("\n")
  
  writer = Qt::ImageWriter.new(filename, Qt::ByteArray.new("PNG"))
  writer.setText("version", version)
  writer.setText("preset", lines)
  writer.write(img)
  
else
  displayHelp()
end
