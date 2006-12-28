//
// Created: Thu Dec 28 21:25:48 2006
//      by: oper-dict.py
//    from: appendixf.html
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

