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

#ifndef FORMULADEFS_H
#define FORMULADEFS_H

#include <memory>

#include <QPoint>
#include <qrect.h>
#include <QString>

#include <KoPoint.h>
#include <KoRect.h>


#define KFORMULA_NAMESPACE_BEGIN namespace KFormula {
#define KFORMULA_NAMESPACE_END }

KFORMULA_NAMESPACE_BEGIN

const int DEBUGID = 40000;

// to make kDebug a litte more interessting...
//#define TERM_RESET "[0m"
//#define TERM_ERROR "[40;31;m"

/**
 * The type to be used for points.
 */
typedef double pt;
typedef KoPoint PtPoint;
typedef KoRect PtRect;
//typedef KoSize PtSize;

/**
 * Pixels. At any zoom level.
 */
typedef int pixel;
typedef QPoint PixelPoint;
typedef QRect PixelRect;
typedef QSize PixelSize;

/**
 * Layout Unit. That's the values we store to get
 * wysiwyg right.
 */
typedef int luPt;
typedef QPoint LuPtPoint;
typedef QRect LuPtRect;
typedef QSize LuPtSize;

typedef int luPixel;
typedef QPoint LuPixelPoint;
typedef QRect LuPixelRect;
typedef QSize LuPixelSize;


/**
 * The symbols that are supported by our artwork.
 */
enum SymbolType {
    LeftSquareBracket = '[',
    RightSquareBracket = ']',
    LeftCurlyBracket = '{',
    RightCurlyBracket = '}',
    LeftCornerBracket = '<',
    RightCornerBracket = '>',
    LeftRoundBracket = '(',
    RightRoundBracket = ')',
    SlashBracket = '/',
    BackSlashBracket = '\\',
    LeftLineBracket = 256,
    RightLineBracket,
    EmptyBracket = 1000,
    Integral,
    Sum,
    Product
};


/**
 * Flag for cursor movement functions.
 * Select means move selecting the text (usually Shift key)
 * Word means move by whole words  (usually Control key)
 */
enum MoveFlag { NormalMovement = 0, SelectMovement = 1, WordMovement = 2 };

inline MoveFlag movementFlag( int state )
{
    int flag = NormalMovement;
    if ( state & Qt::ControlModifier )
        flag |= WordMovement;
    if ( state & Qt::ShiftModifier )
        flag |= SelectMovement;
    return static_cast<MoveFlag>( flag );
}



/**
 * TeX like char classes
 */
enum CharClass {
    ORDINARY = 0,
    BINOP = 1,
    RELATION = 2,
    PUNCTUATION = 3,

    NUMBER, NAME, ELEMENT, INNER, BRACKET, SEQUENCE, SEPARATOR, END
};

typedef CharClass TokenType;


// there are four bits needed to store this
enum CharStyle {
    normalChar,
    boldChar,
    italicChar,
    boldItalicChar, // is required to be (boldChar | italicChar)!
    //slantChar,
    anyChar
};


enum CharFamily {
    normalFamily,
    scriptFamily,
    frakturFamily,
    doubleStruckFamily,
    anyFamily
};


/**
 * The struct used to store static font data.
 */
struct InternFontTable {
    short unicode;
    QChar pos;
    CharClass cl;
    CharStyle style;
};


/**
 * Wether we want to insert to the left of the cursor
 * or right of it.
 * The same for deletion.
 */
enum Direction { beforeCursor, afterCursor };

/**
 * The types of space we know.
 */
enum SpaceWidth { THIN, MEDIUM, THICK, QUAD, NEGTHIN };

/**
 * each index has its own number.
 */
enum IndexPosition {
    upperLeftPos,
    lowerLeftPos,
    upperMiddlePos,
    contentPos,
    lowerMiddlePos,
    upperRightPos,
    lowerRightPos,
    parentPos
};


class BasicElement;
class FormulaCursor;

/**
 * A type that describes an index. You can get one of those
 * for each index from an element that owns indexes.
 *
 * This type is used to work on indexes in a generic way.
 */
class ElementIndex {
public:

    virtual ~ElementIndex() { /*cerr << "ElementIndex destroyed.\n";*/ }

    /**
     * Moves the cursor inside the index. The index has to exist.
     */
    virtual void moveToIndex(FormulaCursor*, Direction) = 0;

    /**
     * Sets the cursor to point to the place where the index normaly
     * is. These functions are only used if there is no such index and
     * we want to insert them.
     */
    virtual void setToIndex(FormulaCursor*) = 0;

    /**
     * Tells whether we own those indexes.
     */
    virtual bool hasIndex() const = 0;

    /**
     * Tells to which element the index belongs.
     */
    virtual BasicElement* getElement() = 0;
};

typedef std::auto_ptr<ElementIndex> ElementIndexPtr;

enum RequestID {
    req_addBracket,
    req_addOverline,
    req_addUnderline,
    req_addFraction,
    req_addIndex,
    req_addMatrix,
    req_addMultiline,
    req_addNameSequence,
    req_addNewline,
    req_addOneByTwoMatrix,
    req_addRoot,
    req_addSpace,
    req_addSymbol,
    req_addTabMark,
    req_addText,
    req_addTextChar,
    req_addEmptyBox,
    req_appendColumn,
    req_appendRow,
    req_compactExpression,
    req_copy,
    req_cut,
    req_insertColumn,
    req_insertRow,
    req_makeGreek,
    req_paste,
    req_remove,
    req_removeEnclosing,
    req_removeColumn,
    req_removeRow,
    req_formatBold,
    req_formatItalic,
    req_formatFamily
};


class Request {
    RequestID id;
public:
    Request( RequestID _id ) : id( _id ) {}
    virtual ~Request() {}
    operator RequestID() const { return id;}
};


class BracketRequest : public Request {
    SymbolType m_left, m_right;
public:
    BracketRequest( SymbolType l, SymbolType r ) : Request( req_addBracket ), m_left( l ), m_right( r ) {}
    SymbolType left() const { return m_left; }
    SymbolType right() const { return m_right; }
};

class SymbolRequest : public Request {
    SymbolType m_type;
public:
    SymbolRequest( SymbolType t ) : Request( req_addSymbol ), m_type( t ) {}
    SymbolType type() const { return m_type; }
};

class IndexRequest : public Request {
    IndexPosition m_index;
public:
    IndexRequest( IndexPosition i ) : Request( req_addIndex ), m_index( i ) {}
    IndexPosition index() const { return m_index; }
};

class SpaceRequest : public Request {
    SpaceWidth m_space;
public:
    SpaceRequest( SpaceWidth s ) : Request( req_addSpace ), m_space( s ) {}
    SpaceWidth space() const { return m_space; }
};

class DirectedRemove : public Request {
    Direction m_direction;
public:
    DirectedRemove( RequestID id, Direction d ) : Request( id ), m_direction( d ) {}
    Direction direction() const { return m_direction; }
};

class TextCharRequest : public Request {
    QChar m_ch;
    bool m_isSymbol;
public:
    TextCharRequest( QChar ch, bool isSymbol=false ) : Request( req_addTextChar ), m_ch( ch ), m_isSymbol( isSymbol ) {}
    QChar ch() const { return m_ch; }
    bool isSymbol() const { return m_isSymbol; }
};

class TextRequest : public Request {
    QString m_text;
public:
    TextRequest( QString text ) : Request( req_addText ), m_text( text ) {}
    QString text() const { return m_text; }
};

class MatrixRequest : public Request {
    uint m_rows, m_columns;
public:
    MatrixRequest( uint rows, uint columns ) : Request( req_addMatrix ), m_rows( rows ), m_columns( columns ) {}
    uint rows() const { return m_rows; }
    uint columns() const { return m_columns; }
};

class CharStyleRequest : public Request {
    bool m_bold;
    bool m_italic;
public:
    CharStyleRequest( RequestID id, bool bold, bool italic ) : Request( id ), m_bold( bold ), m_italic( italic ) {}
    bool bold() const { return m_bold; }
    bool italic() const { return m_italic; }
};

class CharFamilyRequest : public Request {
    CharFamily m_charFamily;
public:
    CharFamilyRequest( CharFamily cf ) : Request( req_formatFamily ), m_charFamily( cf ) {}
    CharFamily charFamily() const { return m_charFamily; }
};


KFORMULA_NAMESPACE_END

#endif // FORMULADEFS_H
