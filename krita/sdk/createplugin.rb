#!/usr/bin/ruby
#  This file is part of the KDE project
#
#  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

def listavailabletemplates()
  templatesdir = Dir.new( File.dirname( __FILE__  ) + "/templates/")
  templates = []
  templatesdir.each() { |x|
    if( x[0,1] != "." )
      templates << x
    end
  }
  return templates
end

def displayHelp()
  puts "This script creates a plugins for krita from a template."
  puts ""
  puts "Usuage:"
  puts "createplugins.rb type name"
  puts ""
  puts "Where type is: #{listavailabletemplates().join(', ')}"
end

if(ARGV.size != 2)
  displayHelp()
  exit
end

type = ARGV[0]
name = ARGV[1]

templateslist = listavailabletemplates()
if(templateslist.index(type) == nil)
  puts "Unknow templates of type #{type}. Choose in the following list: #{templateslist.join(', ')}"
  exit
end

if(name[/[a-zA-Z]\w+/] != name)
  puts "The name can only contains alphanumeric characters and start by a letter."
  exit
end


namelc = name.downcase
nameuc = name.upcase

if(File.exist?(namelc))
  puts "There is allready a directory/file named #{namelc}, remove it before calling this script."
  exit
end

puts "Creating a plugin of name #{name} and type #{type}"
Dir.mkdir(namelc)

templatedir = Dir.new( File.dirname( __FILE__  ) + "/templates/" + type)

templatedir.each() { |x|
   if( x[0,1] != "." )
      dstname = x.clone
      begin
        dstname[type] = namelc
      rescue Exception
      end
      srcf = File.new( templatedir.path() + "/" + x).readlines
      srcf = srcf.join
      srcf.gsub!(/%\{APPNAME\}/, name)
      srcf.gsub!(/%\{APPNAMELC\}/, namelc)
      srcf.gsub!(/%\{APPNAMEUC\}/, nameuc)
      File.new( namelc + "/" + dstname, "w").write(srcf)
    end
}

