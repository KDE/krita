#! /usr/bin/env python

"""This file is part of the KDE project
   Copyright (C) 2006 Alfredo Beaumont Sainz <alfredo.beaumont@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
 
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
 
   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
"""
import sys
import string
import qt

def decode( fd, font, line ):
	begin = string.find( line, '"' )
	end = string.find( line, '"', begin + 1)
	unicode = line[begin + 2:end] # Remove 'U' from string aswell
	char_list = []
	separation = string.find( unicode, '-' )
	if separation != -1:
		second = unicode
		while separation != -1:
			first = second[0:separation]
			second = second[separation + 2:]
			char_list.append( string.atoi( first, 16 ) )
			separation = string.find( second, '-' )
			if separation == -1:
				char_list.append( string.atoi( second, 16 ) )
	else:
		char_list.append( string.atoi ( unicode, 16 ) )
	fm = qt.QFontMetrics( qt.QFont( font ) )
	in_font = True
	for c in char_list:
		if not fm.inFont( qt.QChar( c ) ):
			in_font = False
	fd.write( unicode + ' ' + str( in_font ) + '\n')

def parse( file, font ):
	fd = open( file )
	fd2 = open( 'mathml.list', 'w' )
	line = fd.readline()
	while line != "":
		if string.find( line, 'name' ) != -1:
			decode( fd2, font, line )
		line = fd.readline()
		
if __name__ == '__main__':
	a = qt.QApplication( sys.argv )
	if len( sys.argv ) == 2:
		sys.argv.append( 'Arev Sans' )
	parse ( sys.argv[1], sys.argv[2] )
	a.quit()
