/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>

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
#include <QString>
#include <QStringList>
#include <QTextStream>

#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include "symboltable.h"
#include "contextstyle.h"


KFORMULA_NAMESPACE_BEGIN

/* -- moved to "symbolfontstyle.cc" to have access to the symbolMap.
SymbolFontHelper::SymbolFontHelper()
    : greek("abgdezhqiklmnxpvrstufjcywGDQLXPSUFYVW")
{
    for ( uint i = 0; symbolMap[ i ].unicode != 0; i++ ) {
        compatibility[ symbolMap[ i ].pos ] = symbolMap[ i ].unicode;
    }
}
*/

QChar SymbolFontHelper::unicodeFromSymbolFont( QChar pos ) const
{		
    if ( compatibility.contains( pos ) ) {
        return compatibility[ pos.latin1() ];
    }
    return QChar::Null;
}


SymbolTable::SymbolTable()
{
}


void SymbolTable::init( ContextStyle* /*context*/ )
{
    normalChars.clear();
    boldChars.clear();
    italicChars.clear();
    boldItalicChars.clear();
    entries.clear();
    fontTable.clear();
}


void SymbolTable::initFont( const InternFontTable* table,
                            const char* fontname,
                            const NameTable& tempNames )
{
    uint fontnr = fontTable.size();
    fontTable.push_back( QFont( fontname ) );
    for ( uint i = 0; table[ i ].unicode != 0; ++i ) {
        QChar uc = table[ i ].unicode;
        unicodeTable( table[ i ].style )[ uc ] =
            CharTableEntry( table[ i ].cl,
                            static_cast<char>( fontnr ),
                            table[ i ].pos );

        if ( tempNames.contains( uc ) ) {
            entries[ tempNames[uc] ] = uc;
            names[uc] = tempNames[uc];
        }
    }
}


bool SymbolTable::contains(QString name) const
{
    return entries.find( name ) != entries.end();
}

QChar SymbolTable::unicode(QString name) const
{
    return entries[ name ];
}


QString SymbolTable::name( QChar symbol ) const
{
    return names[symbol];
}


const CharTableEntry& SymbolTable::entry( QChar symbol, CharStyle style ) const
{
    const UnicodeTable& table = unicodeTable( style );
    if ( table.contains( symbol ) ) {
        return table[symbol];
    }
    if ( ( style != normalChar ) && ( style != anyChar ) ) {
        if ( normalChars.contains( symbol ) ) {
            return normalChars[symbol];
        }
    }
    if ( style != boldChar ) {
        if ( boldChars.contains( symbol ) ) {
            return boldChars[symbol];
        }
    }
    if ( style != italicChar ) {
        if ( italicChars.contains( symbol ) ) {
            return italicChars[symbol];
        }
    }
    if ( style != boldItalicChar ) {
        if ( boldItalicChars.contains( symbol ) ) {
            return boldItalicChars[symbol];
        }
    }
    return dummyEntry;
}


QFont SymbolTable::font( QChar symbol, CharStyle style ) const
{
    char f = entry( symbol, style ).font();
    return fontTable[f];
}


QChar SymbolTable::character( QChar symbol, CharStyle style ) const
{
    return entry( symbol, style ).character();
}


CharClass SymbolTable::charClass( QChar symbol, CharStyle style ) const
{
    return entry( symbol, style ).charClass();
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

    for ( EntryTable::const_iterator iter = entries.begin();
          iter != entries.end();
          ++iter ) {
        if ( QChar( character( iter.data() ) ) != QChar::Null ) {
            list.append( iter.key() );
        }
    }
    list.sort();
    return list;
}


bool SymbolTable::inTable( QChar ch, CharStyle style ) const
{
    if ( style == anyChar ) {
        return normalChars.contains( ch ) ||
            boldChars.contains( ch ) ||
            italicChars.contains( ch ) ||
            boldItalicChars.contains( ch );
    }
    return unicodeTable( style ).contains( ch );
}


SymbolTable::UnicodeTable& SymbolTable::unicodeTable( CharStyle style )
{
    switch ( style ) {
    case boldChar: return boldChars;
    case italicChar: return italicChars;
    case boldItalicChar: return boldItalicChars;
    default: break;
    }
    return normalChars;
}

const SymbolTable::UnicodeTable& SymbolTable::unicodeTable( CharStyle style ) const
{
    switch ( style ) {
    case boldChar: return boldChars;
    case italicChar: return italicChars;
    case boldItalicChar: return boldItalicChars;
    default: break;
    }
    return normalChars;
}


KFORMULA_NAMESPACE_END
