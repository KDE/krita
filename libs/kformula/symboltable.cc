/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
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

#include <QFile>
#include <QRegExp>
#include <QTextStream>
#include <QFontMetrics>

#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <koffice_export.h>

#include "symboltable.h"
#include "contextstyle.h"
#include "unicodetable.cc"


KFORMULA_NAMESPACE_BEGIN

#include "symbolfontmapping.cc"

SymbolFontHelper::SymbolFontHelper()
    : greek("abgdezhqiklmnxpvrstufjcywGDQLXPSUFYVW")
{
    for ( uint i = 0; symbolMap[ i ].unicode != 0; i++ ) {
        compatibility[ symbolMap[ i ].pos ] = symbolMap[ i ].unicode;
    }
}

QChar SymbolFontHelper::unicodeFromSymbolFont( QChar pos ) const
{		
    if ( compatibility.contains( pos ) ) {
        return compatibility[ pos.toLatin1() ];
    }
    return QChar::Null;
}


SymbolTable::SymbolTable()
{
}


void SymbolTable::init( const QFont& font )
{
    backupFont = font;
    for ( int i=0; operatorTable[i].unicode != 0; ++i ) {
        names[QChar( operatorTable[i].unicode )] = get_name( operatorTable[i] );
        entries[get_name( operatorTable[i] )] = QChar( operatorTable[i].unicode );
    }
    for ( int i=0; arrowTable[i].unicode != 0; ++i ) {
        names[QChar( arrowTable[i].unicode )] = get_name( arrowTable[i] );
        entries[get_name( arrowTable[i] )] = QChar( arrowTable[i].unicode );
    }
    for ( int i=0; greekTable[i].unicode != 0; ++i ) {
        names[QChar( greekTable[i].unicode )] = get_name( greekTable[i] );
        entries[get_name( greekTable[i] )] = QChar( greekTable[i].unicode );
    }
}


bool SymbolTable::contains(const QString & name) const
{
    return entries.find( name ) != entries.end();
}

QChar SymbolTable::unicode(const QString & name) const
{
    return entries[ name ];
}


QString SymbolTable::name( QChar symbol ) const
{
    return names[symbol];
}


QFont SymbolTable::font( QChar symbol, const QFont& f ) const
{
    QFontMetrics fm( f );
    if ( fm.inFont( symbol ) ) {
        return f;
    }
    return QFont("Arev Sans");
}


QChar SymbolTable::unicodeFromSymbolFont( QChar pos ) const
{
    return symbolFontHelper.unicodeFromSymbolFont( pos );
}


QString SymbolTable::greekLetters() const
{
    return symbolFontHelper.greekLetters();
}


QStringList SymbolTable::allNames() const
{
    QStringList list;

    for ( int i=0; operatorTable[i].unicode != 0; ++i ) {
        list.append( get_name( operatorTable[i] ));
    }
    for ( int i=0; arrowTable[i].unicode != 0; ++i ) {
        list.append( get_name( arrowTable[i] ));
    }
    for ( int i=0; greekTable[i].unicode != 0; ++i ) {
        list.append( get_name( greekTable[i] ) );
    }
    return list;
}


QString SymbolTable::get_name( struct UnicodeNameTable entry ) const
{
    if ( !*entry.name ) {
        return 'U' + QString( "%1" ).arg( entry.unicode, 4, 16 ).upper();
    }
    return entry.name;
}


KFORMULA_NAMESPACE_END
