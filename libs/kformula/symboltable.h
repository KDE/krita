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

#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <QFont>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVector>

#include "kformuladefs.h"

class KConfig;

KFORMULA_NAMESPACE_BEGIN

class ContextStyle;

/**
 * What we know about a unicode char. The char value itself
 * is a key inside the symbol table. Here we have
 * the char class and which font to use.
 */
class CharTableEntry {
public:

    /**
     * Defaults for all arguments are provided so it can be used in a QMap.
     */
    CharTableEntry( CharClass cl=ORDINARY, char font=0, QChar ch=0 )
        : m_charClass( static_cast<char>( cl ) ), m_font( font ), m_character( ch ) {}

    char font() const { return m_font; }
    QChar character() const { return m_character; }
    CharClass charClass() const { return static_cast<CharClass>( m_charClass ); }

private:

    char m_charClass;
    char m_font;
    QChar m_character;
};


/**
 * We expect to always have the symbol font.
 */
class SymbolFontHelper {
public:

    SymbolFontHelper();

    /**
     * @returns a string with all greek letters.
     */
    QString greekLetters() const { return greek; }

    /**
     * @returns the unicode value of the symbol font char.
     */
    QChar unicodeFromSymbolFont( QChar pos ) const;

private:

    /**
     * symbol font char -> unicode mapping.
     */
    QMap<QChar, QChar> compatibility;

    /**
     * All greek letters that are known.
     */
    QString greek;
};


/**
 * The symbol table.
 *
 * It contains all names that are know to the system.
 */
class KOFORMULA_EXPORT SymbolTable {
public:

    SymbolTable();

    /**
     * Reads the unicode / font tables.
     */
    void init( ContextStyle* context );

    bool contains( QString name ) const;

    /**
     * @returns the char in the symbol font that belongs to
     * the given name.
     */
    QChar unicode( QString name ) const;
    QString name( QChar symbol ) const;

    const CharTableEntry& entry( QChar symbol, CharStyle style=normalChar ) const;
    QFont font( QChar symbol, CharStyle style=normalChar ) const;
    QChar character( QChar symbol, CharStyle style=normalChar ) const;
    CharClass charClass( QChar symbol, CharStyle style=normalChar ) const;

    /**
     * @returns the unicode value of the symbol font char.
     */
    QChar unicodeFromSymbolFont( QChar pos ) const;

    /**
     * @returns a string with all greek letters.
     */
    QString greekLetters() const;

    /**
     * @returns all known names as strings.
     */
    QStringList allNames() const;

    typedef QMap<QChar, CharTableEntry> UnicodeTable;
    typedef QMap<QChar, QString> NameTable;
    typedef QMap<QString, QChar> EntryTable;
    typedef QVector<QFont> FontTable;

    bool inTable( QChar ch, CharStyle style=anyChar ) const;

    void initFont( const InternFontTable* table,
                   const char* fontname,
                   const NameTable& tempNames );

private:

    UnicodeTable& unicodeTable( CharStyle style );
    const UnicodeTable& unicodeTable( CharStyle style ) const;

    /**
     * The chars from unicode.
     */
    UnicodeTable normalChars;
    UnicodeTable boldChars;
    UnicodeTable italicChars;
    UnicodeTable boldItalicChars;

    /**
     * unicode -> name mapping.
     */
    NameTable names;

    /**
     * Name -> unicode mapping.
     */
    EntryTable entries;

    /**
     * Symbol fonts in use.
     * There must not be more than 256 fonts.
     */
    FontTable fontTable;

    /**
     * Basic symbol font support.
     */
    SymbolFontHelper symbolFontHelper;

    // An empty entry that's used if nothing was found.
    CharTableEntry dummyEntry;
};

KFORMULA_NAMESPACE_END

#endif // SYMBOLTABLE_H
