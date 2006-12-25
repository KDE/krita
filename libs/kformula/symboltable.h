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

#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <QFont>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVector>

#include "kformuladefs.h"

class KConfig;

namespace KFormula
{

class ContextStyle;
struct UnicodeNameTable;
	
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
class KOFORMULA_EXPORT SymbolTable
{
  public:
         SymbolTable();

    /**
     * Reads the unicode / font tables.
     */
    void init( const QFont& font );

    bool contains( const QString & name ) const;

    /**
     * @returns the char in the symbol font that belongs to
     * the given name.
     */
    QChar unicode( const QString & name ) const;
    QString name( QChar symbol ) const;

    QFont font( QChar symbol, const QFont& f ) const;

    CharClass charClass( QChar symbol ) const;

    /**
     * @returns a string with all greek letters.
     */
    QString greekLetters() const;

    /**
     * @returns the unicode value of the symbol font char.
     */
    QChar unicodeFromSymbolFont( QChar pos ) const;

    /**
     * @returns all known names as strings.
     */
    QStringList allNames() const;

    typedef QMap<QChar, QString> NameTable;
    typedef QMap<QString, QChar> EntryTable;

private:

    QString get_name( UnicodeNameTable entry ) const;

    /**
     * unicode -> name mapping.
     */
    NameTable names;

    /**
     * Name -> unicode mapping.
     */
    EntryTable entries;

    /**
     * Basic symbol font support.
     */
    SymbolFontHelper symbolFontHelper;

    /**
     * Backup font for mathematical operators. We ensure that every symbol in
     * this table is present in this font. If user selected font doesn't have
     * the needed glyph this font will be used instead.
     */
    QFont backupFont;
};

}

#endif // SYMBOLTABLE_H
