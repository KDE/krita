//
// Created: Thu Dec 28 21:14:19 2006
//      by: bynames.py
//    from: byalpha.html
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


#ifndef ENTITIES_H
#define ENTITIES_H

#include "kformuladefs.h"

KFORMULA_NAMESPACE_BEGIN
	
struct entityMap {
    static int size();
    int operator<( const char* right ) const {
	    return qstrcmp( name, right ) < 0;
    }
	const char* name;
	const uint unicode;
};
	
extern const entityMap entities[];

KFORMULA_NAMESPACE_END

#endif // ENTITIES_H

