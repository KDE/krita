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
import codecs
import sys
import string
import time
import os

###
# If operator element's attributes change in the future, just update this list
# and the attr dictionary below
attr_list = [
	'lspace',
	'rspace',
	'maxsize',
	'minsize',
	'fence',
	'separator',
	'stretchy',
	'symmetric',
	'largeop',
	'movablelimits',
	'accent'
	]
	

def write_header( f ):
	print >> f, '''//
// Created: ''' + time.ctime(time.time()) + '''
//      by: ''' + os.path.basename( sys.argv[0] ) + '''
//    from: ''' + os.path.basename( sys.argv[1] ) + '''
//
// WARNING! All changes made in this file will be lost!

/* This file is part of the KDE project
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
 * Boston, MA 02110-1301, USA.
*/
'''

def write_h( f ):
	print >>f, '''
#ifndef OPERATORDICTIONARY_H
#define OPERATORDICTIONARY_H

#include "kformuladefs.h"

KFORMULA_NAMESPACE_BEGIN
	
struct DictionaryKey
{
    int operator==( const DictionaryKey& right ) const {
        if ( qstrcmp( name, right.name ) || qstrcmp( form, right.form ) ) {
            return false;
        }
        return true;
    }
    const char* name;
    const char* form;
};

struct OperatorDictionary {
    static int size();
    int operator<( const DictionaryKey& right ) const {
        int equal = qstrcmp( key.name, right.name );
        if ( equal != 0 ) {
            return equal < 0;
        }
        return qstrcmp( key.form, right.form ) < 0;
    }
    const DictionaryKey key;
	const char* lspace;
	const char* rspace;
	const char* maxsize;
	const char* minsize;
	bool fence;
	bool separator;
	bool stretchy;
	bool symmetric;
	bool largeop;
	bool movablelimits;
	bool accent;
};
	
extern const OperatorDictionary operators[];

KFORMULA_NAMESPACE_END

#endif // OPERATORDICTIONARY_H
'''

def write_cc( fr, fw ):
	print >> fw, '''
#include "operatordictionary.h"

KFORMULA_NAMESPACE_BEGIN

const OperatorDictionary operators[] = {'''

	entities = get_entities()
	parse( fr, fw, entities )
	
	print >> fw, '''
};

// Needed since sizeof is a macro and we cannot be used until size is known
int OperatorDictionary::size()
{
    return sizeof( operators ) / sizeof( OperatorDictionary );
}

KFORMULA_NAMESPACE_END
	'''

def get_entities():
	# First, read entity list into a dict
	fd = open( 'entity.list' )
	entities = {}
	for line in fd.readlines():
		fields = line.split()
		entities[fields[0]] = string.atoi( fields[1], 16 )
	fd.close()
	return entities

def key_cmp( a, b ):

	if a[0] < b[0]:
		return -1
	if a[0] > b[0]:
		return 1

	if a[1] < b[1]:
		return -1
	if a[1] > b[1]:
		return 1
	print 'WARNING: Same key in operator dictionary: ' + a[0] + ', ' + b[0]
	return 0

def parse( fr, fw, entities ):
	entries = []
	line = fr.readline()
	while line != "":
		if line[0] == '"':
			###
			# If operator element's attributes or default values change in the future,
			# just update this dictionary and the list at the beginning of the file
			attr_dict = {
				attr_list[0]: '"thickmathspace"',
				attr_list[1]: '"thickmathspace"',
				attr_list[2]: '"infinity"',
				attr_list[3]: '"1"',
				attr_list[4]: '"false"',
				attr_list[5]: '"false"',
				attr_list[6]: '"false"',
				attr_list[7]: '"true"',
				attr_list[8]: '"false"',
				attr_list[9]: '"false"',
				attr_list[10]: '"false"'
				}
			fields = line.split()
			name = string.replace( fields[0], '&amp;', '&' )
			fields.pop(0) # Remove name
			entities_found = True
			while True:
				begin = string.find( name, '&' )
				end = string.find( name, ';' )
				if begin == -1 or end == -1:
					break
				###
				# TODO: Support multicharacter entities, should also be supported by
				# application. The best solution would probably to map to a single
				# character provided by the font in the private area of Unicode
				entity_name = name[begin + 1:end]
				if entities.has_key( entity_name ) :
					name = name.replace( '&' + entity_name + ';', unichr(entities[entity_name]));
				else:
					entities_found = False
					break
			if entities_found:
				form = string.split( fields[0], '=' )[1]
				fields.pop(0) # Remove form
				for f in fields:
					attr, value = string.split( f, '=' )
					if not attr_dict.has_key( attr ) :
						print 'Unsupported attribute: ' + attr
						print 'If it is valid, update attribute dictionary'
						sys.exit(-1)
					# Spec has a typo, fix it
					if string.count( value, '"' ) == 3:
						value = value[:-1]
					attr_dict[attr] = value
				entries.append( [name, form, attr_dict] )
		line = fr.readline()
	entries.sort( key_cmp, None, True )

	while True:
		e = entries.pop()
		print >> fw, '     { {' + e[0] + ', ' + e[1] + '},'
		d = e[2]
		for a in attr_list:
			# Convert, at least, bool values
			value = d[a]
			if value == '"true"' or value == '"false"':
				value = string.strip( value, '"' )
			print >> fw, '\t\t' + value,
			if a != attr_list[len(attr_list) - 1]:
				print >> fw, ','
		print >> fw, '}',
		if len( entries ) == 0:
			break
		print >> fw, ',\n'

if __name__ == '__main__':
	fh = open( '../operatordictionary.h', 'w' )
	write_header( fh )
	write_h( fh )
	fh.close()
	fcc = codecs.open( '../operatordictionary.cc', 'w', 'utf-8' )
	write_header( fcc )
	fr = open( sys.argv[1] )
	write_cc( fr , fcc )
	fcc.close()
	fr.close()
	
