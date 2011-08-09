#! /usr/bin/env python

"""This file is part of the KDE project
   Copyright (C) 2007 Martin Pfeiffer <hubipete@gmx.net>

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

"""
This script generates the Dictionaty.cpp file which serves as an operator dictionary.
The source of information for generation is the operator.list file. This is copied
from the appendix of the MathML spec with whitespace striped. The url is:
http://www.w3.org/TR/2003/REC-MathML2-20031021/appendixf.html .
Further this script generates the entity mapping which maps MathML entities to unicode
characters. The raw data in entities.list is taken from the MathML specification
http://www.w3.org/TR/2003/REC-MathML2-20031021/byalpha.html .
"""

import codecs
import time
	
'''
Write the standart KDE file header with copyright and time signature. Write also the
constructor of the Dictionary class.
'''
def write_file_header( file ):
	print >> file,'''// Created: ''' + time.ctime( time.time() ) + '''
// WARNING! All changes made in this file will be lost!

/* This file is part of the KDE project
   Copyright (C) 2007 <hubipete@gmx.net>

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
*/

#include "Dictionary.h"

Dictionary::Dictionary()
{
    m_lspace = "thickmathspace";
    m_rspace = "thickmathspace";
    m_maxsize = "infinity";
    m_minsize = "1";
    m_fence = false;
    m_separator = false;
    m_stretchy = false;
    m_symmetric = true;
    m_largeop = false;
    m_movablelimits = false;
    m_accent = false;
}
'''

def write_entity_mapping( file ):
	print >> file, 'QChar Dictionary::mapEntity( const QString& entity )'
	print >> file, '{\n    if( entity.isEmpty() ) return QChar();'
	entity_list = open( 'entities.list' )
	for line in entity_list:
		tokens = line.split( ',' )
		if tokens[ 1 ].find( '-' ) > -1 :
			continue
                file.write( '    else if( entity == "' + tokens[ 0 ] + '" ) return QChar( 0x' )
                file.write( tokens[ 1 ].strip()[1:]  +  ' );\n' )
	print >> file, '    else return QChar();\n}\n'

def write_operator_dictionary( file ):
	print >> file, 'bool Dictionary::queryOperator( const QString& queriedOperator, Form form )'
	print >> file, '{\n    if( queriedOperator.isEmpty() || queriedOperator.isNull() )\n        return false;'
	operator_list = open( 'operator.list' )
	for line in operator_list:
		for token in line.split():
			if token.startswith( '"' ) and token.endswith( '"' ):
				file.write( '    else if( queriedOperator == ' + token + ' && ' )
			elif token.find( 'form=' ) > -1:
				print >> file, 'form == ' + token.strip( '"' )[6:].capitalize() + ' ) {'
			else:
				print >> file, parse_token( token )
		print >> file, '        return true;'
   		print >> file, '    }'
	print >> file, '\n    return false;'
	print >> file, '}'
	operator_list.close()


def parse_token( token ):
	subtokens = token.split( '=' )
	if token.find( 'true' ) > -1 or token.find( 'false' ) > -1:
		return '        m_' + subtokens[0] + ' = ' + subtokens[1].strip( '"' ) + ';'
	else:
		return '        m_' + subtokens[0] + ' = ' + subtokens[1] + ';'
	

if __name__ == '__main__':
	source_file = codecs.open( '../Dictionary.cpp', 'w', 'utf-8' )
	write_file_header( source_file )
        write_entity_mapping( source_file )
	write_operator_dictionary( source_file )
	source_file.close()
