/****************************************************************************
**
** Definition of internal rich text classes
**
** Created : 990124
**
** Copyright (C) 1999-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as publish by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef KORICHTEXT_H
#define KORICHTEXT_H

#include <q3ptrlist.h>
#include <qrect.h>
#include <qmap.h>
#include <qstringlist.h>
#include <qcolor.h>
#include <qsize.h>
#include <q3valuelist.h>
#include <qobject.h>
#include <q3stylesheet.h>
#include <qpainter.h>
//Added by qt3to4:
#include <QPixmap>
#include <Q3CString>
#include <Q3GridLayout>

#include <koffice_export.h>
class KoXmlWriter;
class KoGenStyles;
class KoTextParag;
class KoTextString;
class KoTextCursor;
class KoTextCustomItem;
class KoTextFlow;
class KoTextDocument;
//class KoTextPreProcessor;
class KoTextFormatterBase;
class KoTextFormat;
class KoTextFormatCollection;
struct KoBidiContext;

//// kotext additions (needed by the #included headers)
class KCommand;
class QDomElement;
class KoTextZoomHandler;
class KoTextFormatter;
class KoParagVisitor;
class KoTextDocCommand;
class KoXmlWriter;
class KoSavingContext;

#define QT_NO_COMPLEXTEXT

#include <q3memarray.h>
#include "KoParagLayout.h"
////

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class KOTEXT_EXPORT KoTextStringChar
{
    friend class KoTextString;

public:
    // this is never called, initialize variables in KoTextString::insert()!!!
    KoTextStringChar() : lineStart( 0 ), type( Regular ), startOfRun( 0 ) {d.format=0; }
    ~KoTextStringChar();

    QChar c;

    // this is the same struct as in qtextengine_p.h. Don't change!
    uchar softBreak      :1;     // Potential linebreak point
    uchar whiteSpace     :1;     // A unicode whitespace character, except NBSP, ZWNBSP
    uchar charStop       :1;     // Valid cursor position (for left/right arrow)
    uchar wordStop       :1;     // Valid cursor position (for ctrl + left/right arrow) (TODO: use)
    //uchar nobreak        :1;

    enum Type { Regular, Custom };
    uint lineStart : 1;
    Type type : 1;
    uint startOfRun : 1;
    uint rightToLeft : 1;

    // --- added for WYSIWYG ---
    qint8 pixelxadj; // adjustment to apply to lu2pixel(x)
    short int pixelwidth; // width in pixels
    short int width; // width in LU

    int x;
    int height() const;
    int ascent() const;
    int descent() const;
    bool isCustom() const { return type == Custom; }
    KoTextFormat *format() const;
    KoTextCustomItem *customItem() const;
    void setFormat( KoTextFormat *f,bool setFormatAgain=TRUE );
    void setCustomItem( KoTextCustomItem *i );
    void loseCustomItem();
    struct CustomData
    {
	KoTextFormat *format;
	KoTextCustomItem *custom;
    };

    union {
	KoTextFormat* format;
	CustomData* custom;
    } d;

private:
    KoTextStringChar &operator=( const KoTextStringChar & ) {
	//abort();
	return *this;
    }
    KoTextStringChar( const KoTextStringChar & ); // copy-constructor, forbidden
    //friend class KoComplexText;
    friend class KoTextParag;
};

#if defined(Q_TEMPLATEDLL)
#ifndef Q_MOC_RUN
template class Q3MemArray<KoTextStringChar>;
#endif
#endif

class KOTEXT_EXPORT KoTextString
{
public:

    KoTextString();
    KoTextString( const KoTextString &s );
    virtual ~KoTextString();

    QString toString() const;
    static QString toString( const Q3MemArray<KoTextStringChar> &data );
    QString toReverseString() const;

    QString stringToSpellCheck();

    KoTextStringChar &at( int i ) const;
    int length() const;

    //int width( int idx ) const; // moved to KoTextFormat

    void insert( int index, const QString &s, KoTextFormat *f );
    void insert( int index, KoTextStringChar *c );
    void truncate( int index );
    void remove( int index, int len );
    void clear();

    void setFormat( int index, KoTextFormat *f, bool useCollection, bool setFormatAgain = FALSE );

    void setBidi( bool b ) { bidi = b; }
    bool isBidi() const;
    bool isRightToLeft() const;
    QChar::Direction direction() const;
    void setDirection( QChar::Direction d ) { dir = d; bidiDirty = TRUE; }

    /** Set dirty flag for background spell-checking */
    void setNeedsSpellCheck( bool b ) { bNeedsSpellCheck = b; }
    bool needsSpellCheck() const { return bNeedsSpellCheck; }

    Q3MemArray<KoTextStringChar> subString( int start = 0, int len = 0xFFFFFF ) const;
    QString mid( int start = 0, int len = 0xFFFFFF ) const; // kotext addition
    Q3MemArray<KoTextStringChar> rawData() const { return data.copy(); }

    void operator=( const QString &s ) { clear(); insert( 0, s, 0 ); }
    void operator+=( const QString &s );
    void prepend( const QString &s ) { insert( 0, s, 0 ); }

    // return next and previous valid cursor positions.
    bool validCursorPosition( int idx );
    int nextCursorPosition( int idx );
    int previousCursorPosition( int idx );

private:
    void checkBidi() const;

    Q3MemArray<KoTextStringChar> data;
    uint bidiDirty : 1;
    uint bidi : 1; // true when the paragraph has right to left characters
    uint rightToLeft : 1;
    uint dir : 5;
    uint bNeedsSpellCheck : 1;
};

inline bool KoTextString::isBidi() const
{
    if ( bidiDirty )
	checkBidi();
    return bidi;
}

inline bool KoTextString::isRightToLeft() const
{
    if ( bidiDirty )
	checkBidi();
    return rightToLeft;
}

inline QChar::Direction KoTextString::direction() const
{
    return (QChar::Direction) dir;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class KOTEXT_EXPORT KoTextCursor
{
public:
    KoTextCursor( KoTextDocument *d );
    KoTextCursor();
    KoTextCursor( const KoTextCursor &c );
    KoTextCursor &operator=( const KoTextCursor &c );
    virtual ~KoTextCursor() {}

    bool operator==( const KoTextCursor &c ) const;
    bool operator!=( const KoTextCursor &c ) const { return !(*this == c); }

    KoTextDocument *document() const { return doc; }
    void setDocument( KoTextDocument *d );

    KoTextParag *parag() const;
    int index() const;
    void setParag( KoTextParag *s, bool restore = TRUE );

    void gotoLeft();
    void gotoRight();
    void gotoNextLetter();
    void gotoPreviousLetter();
    void gotoUp();
    void gotoDown();
    void gotoLineEnd();
    void gotoLineStart();
    void gotoHome();
    void gotoEnd();
    void gotoPageUp( int visibleHeight );
    void gotoPageDown( int visibleHeight );
    void gotoNextWord();
    void gotoPreviousWord();
    void gotoWordLeft();
    void gotoWordRight();

    void insert( const QString &s, bool checkNewLine, Q3MemArray<KoTextStringChar> *formatting = 0 );
    void splitAndInsertEmptyParag( bool ind = TRUE, bool updateIds = TRUE );
    bool remove();
    bool removePreviousChar();
    void killLine();
    //void indent();

    bool atParagStart() const;
    bool atParagEnd() const;

    void setIndex( int i, bool restore = TRUE );

    //void checkIndex();

    //int offsetX() const { return ox; }
    //int offsetY() const { return oy; }

    bool place( const QPoint &pos, KoTextParag *s, bool link = false, int *customItemIndex = 0 );

    int x() const;
    int y() const;
    void fixCursorPosition();

private:
    KoTextParag *string;
    KoTextDocument *doc;
    int idx, tmpIndex;
    //int ox, oy;
};

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class KoTextDocCommand
{
public:
    enum Commands { Invalid, Insert, Delete, Format, Alignment, ParagType };

    KoTextDocCommand( KoTextDocument *d ) : doc( d ), cursor( d ) {}
    virtual ~KoTextDocCommand() {}
    virtual Commands type() const { return Invalid; };

    virtual KoTextCursor *execute( KoTextCursor *c ) = 0;
    virtual KoTextCursor *unexecute( KoTextCursor *c ) = 0;

protected:
    KoTextDocument *doc;
    KoTextCursor cursor;

};

#if defined(Q_TEMPLATEDLL)
#ifndef Q_MOC_RUN
template class Q3PtrList<KoTextDocCommand>;
#endif
#endif

class KoTextDocCommandHistory
{
public:
    KoTextDocCommandHistory( int s ) : current( -1 ), steps( s ) { history.setAutoDelete( TRUE ); }
    virtual ~KoTextDocCommandHistory() { clear(); }

    void clear() { history.clear(); current = -1; }

    void addCommand( KoTextDocCommand *cmd );
    KoTextCursor *undo( KoTextCursor *c );
    KoTextCursor *redo( KoTextCursor *c );

    bool isUndoAvailable();
    bool isRedoAvailable();

    void setUndoDepth( int d ) { steps = d; }
    int undoDepth() const { return steps; }

    int historySize() const { return history.count(); }
    int currentPosition() const { return current; }

private:
    Q3PtrList<KoTextDocCommand> history;
    int current, steps;

};

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#include "KoTextCustomItem.h"

#if defined(Q_TEMPLATEDLL)
#ifndef Q_MOC_RUN
template class QMap<QString, QString>;
#endif
#endif

#if 0
class KoTextImage : public KoTextCustomItem
{
public:
    KoTextImage( KoTextDocument *p, const QMap<QString, QString> &attr, const QString& context,
		Q3MimeSourceFactory &factory );
    virtual ~KoTextImage();

    Placement placement() const { return place; }
    //void setPainter( QPainter*, bool );
    int widthHint() const { return width; }
    int minimumWidth() const { return width; }

    QString richText() const;

    void draw( QPainter* p, int x, int y, int cx, int cy, int cw, int ch, const QColorGroup& cg, bool selected );

private:
    QRegion* reg;
    QPixmap pm;
    Placement place;
    int tmpwidth, tmpheight;
    QMap<QString, QString> attributes;
    QString imgId;

};
#endif

#if 0
class KoTextHorizontalLine : public KoTextCustomItem
{
public:
    KoTextHorizontalLine( KoTextDocument *p, const QMap<QString, QString> &attr, const QString& context,
			 Q3MimeSourceFactory &factory );
    virtual ~KoTextHorizontalLine();

    //void setPainter( QPainter*, bool );
    void draw(QPainter* p, int x, int y, int cx, int cy, int cw, int ch, const QColorGroup& cg, bool selected );
    QString richText() const;

    bool ownLine() const { return TRUE; }

private:
    int tmpheight;
    QColor color;

};
#endif

#if defined(Q_TEMPLATEDLL)
#ifndef Q_MOC_RUN
template class Q3PtrList<KoTextCustomItem>;
#endif
#endif

class KOTEXT_EXPORT KoTextFlow
{
    friend class KoTextDocument;
    friend class KoTextTableCell;

public:
    KoTextFlow();
    virtual ~KoTextFlow();

    // Called by KoTextDocument::setWidth()
    virtual void setWidth( int width );

    // This is the value returned by KoTextDocument::width()
    int width() const { return w; }

    //virtual void setPageSize( int ps );
    //int pageSize() const { return pagesize; }

    /**
     * Called by the formatter to find out the left and right margin for a paragraph at ( yp, yp+h )
     * @param leftMargin returns the left margin
     * @param rightMargin returns the right margin (from the page width)
     * @param pageWidth returns the page width at that point
     * This method merges QRichText's adjustLMargin and adjustRMargin for efficiency reasons
     */
    virtual void adjustMargins( int yp, int h, int reqMinWidth, int& leftMargin, int& rightMargin, int& pageWidth, KoTextParag* parag );

    virtual void registerFloatingItem( KoTextCustomItem* item );
    virtual void unregisterFloatingItem( KoTextCustomItem* item );
    //virtual QRect boundingRect() const;

    /// kotext addition. Allows the textformatter to stop when it goes too far.
    virtual int availableHeight() const;
    virtual void drawFloatingItems(QPainter* p, int cx, int cy, int cw, int ch, const QColorGroup& cg, bool selected );

    virtual int adjustFlow( int y, int w, int h ); // adjusts y according to the defined pagesize. Returns the shift.

    virtual bool isEmpty();

    void clear();

private:
    int w;
    //int pagesize;

    Q3PtrList<KoTextCustomItem> leftItems;
    Q3PtrList<KoTextCustomItem> rightItems;

};

#ifdef QTEXTTABLE_AVAILABLE
class KoTextTable;

class KoTextTableCell : public QLayoutItem
{
    friend class KoTextTable;

public:
    KoTextTableCell( KoTextTable* table,
		    int row, int column,
		    const QMap<QString, QString> &attr,
		    const Q3StyleSheetItem* style,
		    const KoTextFormat& fmt, const QString& context,
		    Q3MimeSourceFactory &factory, Q3StyleSheet *sheet, const QString& doc );
    KoTextTableCell( KoTextTable* table, int row, int column );
    virtual ~KoTextTableCell();

    QSize sizeHint() const ;
    QSize minimumSize() const ;
    QSize maximumSize() const ;
    QSizePolicy::ExpandData expanding() const;
    bool isEmpty() const;
    void setGeometry( const QRect& ) ;
    QRect geometry() const;

    bool hasHeightForWidth() const;
    int heightForWidth( int ) const;

    void setPainter( QPainter*, bool );

    int row() const { return row_; }
    int column() const { return col_; }
    int rowspan() const { return rowspan_; }
    int colspan() const { return colspan_; }
    int stretch() const { return stretch_; }

    KoTextDocument* richText()  const { return richtext; }
    KoTextTable* table() const { return parent; }

    void draw( int x, int y, int cx, int cy, int cw, int ch, const QColorGroup& cg, bool selected );

    QBrush *backGround() const { return background; }
    virtual void invalidate();

    int verticalAlignmentOffset() const;
    int horizontalAlignmentOffset() const;

private:
    QPainter* painter() const;
    QRect geom;
    KoTextTable* parent;
    KoTextDocument* richtext;
    int row_;
    int col_;
    int rowspan_;
    int colspan_;
    int stretch_;
    int maxw;
    int minw;
    bool hasFixedWidth;
    QBrush *background;
    int cached_width;
    int cached_sizehint;
    QMap<QString, QString> attributes;
    int align;
};

#if defined(Q_TEMPLATEDLL)
#ifndef Q_MOC_RUN
template class Q3PtrList<KoTextTableCell>;
template class QMap<KoTextCursor*, int>;
#endif
#endif

class KoTextTable: public KoTextCustomItem
{
    friend class KoTextTableCell;

public:
    KoTextTable( KoTextDocument *p, const QMap<QString, QString> &attr );
    virtual ~KoTextTable();

    void setPainter( QPainter *p, bool adjust );
    void pageBreak( int  y, KoTextFlow* flow );
    void draw( QPainter* p, int x, int y, int cx, int cy, int cw, int ch,
	       const QColorGroup& cg, bool selected );

    bool noErase() const { return TRUE; }
    bool ownLine() const { return TRUE; }
    Placement placement() const { return place; }
    bool isNested() const { return TRUE; }
    void resize( int nwidth );
    virtual void invalidate();
    /// ## QString anchorAt( QPainter* p, int x, int y );

    virtual bool enter( KoTextCursor *c, KoTextDocument *&doc, KoTextParag *&parag, int &idx, int &ox, int &oy, bool atEnd = FALSE );
    virtual bool enterAt( KoTextCursor *c, KoTextDocument *&doc, KoTextParag *&parag, int &idx, int &ox, int &oy, const QPoint &pos );
    virtual bool next( KoTextCursor *c, KoTextDocument *&doc, KoTextParag *&parag, int &idx, int &ox, int &oy );
    virtual bool prev( KoTextCursor *c, KoTextDocument *&doc, KoTextParag *&parag, int &idx, int &ox, int &oy );
    virtual bool down( KoTextCursor *c, KoTextDocument *&doc, KoTextParag *&parag, int &idx, int &ox, int &oy );
    virtual bool up( KoTextCursor *c, KoTextDocument *&doc, KoTextParag *&parag, int &idx, int &ox, int &oy );

    QString richText() const;

    int minimumWidth() const { return layout ? layout->minimumSize().width() : 0; }
    int widthHint() const { return ( layout ? layout->sizeHint().width() : 0 ) + 2 * outerborder; }

    Q3PtrList<KoTextTableCell> tableCells() const { return cells; }

    QRect geometry() const { return layout ? layout->geometry() : QRect(); }
    bool isStretching() const { return stretch; }

private:
    void format( int &w );
    void addCell( KoTextTableCell* cell );

private:
    Q3GridLayout* layout;
    Q3PtrList<KoTextTableCell> cells;
    QPainter* painter;
    int cachewidth;
    int fixwidth;
    int cellpadding;
    int cellspacing;
    int border;
    int outerborder;
    int stretch;
    int innerborder;
    int us_cp, us_ib, us_b, us_ob, us_cs;
    QMap<QString, QString> attributes;
    QMap<KoTextCursor*, int> currCell;
    Placement place;
    void adjustCells( int y , int shift );
    int pageBreakFor;
};
#endif // QTEXTTABLE_AVAILABLE

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class KoTextTableCell;

struct KoTextDocumentSelection
{
    KoTextCursor startCursor, endCursor;
    bool swapped;
};

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


class KOTEXT_EXPORT KoTextDocDeleteCommand : public KoTextDocCommand
{
public:
    KoTextDocDeleteCommand( KoTextDocument *d, int i, int idx, const Q3MemArray<KoTextStringChar> &str );
    //KoTextDocDeleteCommand( KoTextParag *p, int idx, const QMemArray<KoTextStringChar> &str );
    virtual ~KoTextDocDeleteCommand();

    Commands type() const { return Delete; }
    KoTextCursor *execute( KoTextCursor *c );
    KoTextCursor *unexecute( KoTextCursor *c );

protected:
    int id, index;
    KoTextParag *parag;
    Q3MemArray<KoTextStringChar> text;

};

#if 0
class KoTextDocInsertCommand : public KoTextDocDeleteCommand
{
public:
    KoTextDocInsertCommand( KoTextDocument *d, int i, int idx, const Q3MemArray<KoTextStringChar> &str )
	: KoTextDocDeleteCommand( d, i, idx, str ) {}
    KoTextDocInsertCommand( KoTextParag *p, int idx, const Q3MemArray<KoTextStringChar> &str )
	: KoTextDocDeleteCommand( p, idx, str ) {}
    virtual ~KoTextDocInsertCommand() {}

    Commands type() const { return Insert; }
    KoTextCursor *execute( KoTextCursor *c ) { return KoTextDocDeleteCommand::unexecute( c ); }
    KoTextCursor *unexecute( KoTextCursor *c ) { return KoTextDocDeleteCommand::execute( c ); }

};
#endif

class KoTextDocFormatCommand : public KoTextDocCommand
{
public:
    KoTextDocFormatCommand( KoTextDocument *d, int sid, int sidx, int eid, int eidx, const Q3MemArray<KoTextStringChar> &old, const KoTextFormat *f, int fl );
    virtual ~KoTextDocFormatCommand();

    Commands type() const { return Format; }
    KoTextCursor *execute( KoTextCursor *c );
    KoTextCursor *unexecute( KoTextCursor *c );

protected:
    int startId, startIndex, endId, endIndex;
    KoTextFormat *format;
    Q3MemArray<KoTextStringChar> oldFormats;
    int flags;

};

class KoTextAlignmentCommand : public KoTextDocCommand
{
public:
    KoTextAlignmentCommand( KoTextDocument *d, int fParag, int lParag, int na, const Q3MemArray<int> &oa );
    virtual ~KoTextAlignmentCommand() {}

    Commands type() const { return Alignment; }
    KoTextCursor *execute( KoTextCursor *c );
    KoTextCursor *unexecute( KoTextCursor *c );

private:
    int firstParag, lastParag;
    int newAlign;
    Q3MemArray<int> oldAligns;

};

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct KoTextParagLineStart
{
    KoTextParagLineStart() : y( 0 ), baseLine( 0 ), h( 0 ), lineSpacing( 0 ), hyphenated( false )
#ifndef QT_NO_COMPLEXTEXT
	, bidicontext( 0 )
#endif
    {  }
    KoTextParagLineStart( ushort y_, ushort bl, ushort h_ ) : y( y_ ), baseLine( bl ), h( h_ ),
                                                              lineSpacing( 0 ), hyphenated( false ),
	w( 0 )
#ifndef QT_NO_COMPLEXTEXT
	, bidicontext( 0 )
#endif
    {  }
#ifndef QT_NO_COMPLEXTEXT
    KoTextParagLineStart( KoBidiContext *c, KoBidiStatus s ) : y(0), baseLine(0), h(0),
                                                               lineSpacing( 0 ), hyphenated( false ),
	status( s ), bidicontext( c ) { if ( bidicontext ) bidicontext->ref(); }
#endif

    virtual ~KoTextParagLineStart()
    {
#ifndef QT_NO_COMPLEXTEXT
	if ( bidicontext && bidicontext->deref() )
	    delete bidicontext;
#endif
    }

#ifndef QT_NO_COMPLEXTEXT
    void setContext( KoBidiContext *c ) {
	if ( c == bidicontext )
	    return;
	if ( bidicontext && bidicontext->deref() )
	    delete bidicontext;
	bidicontext = c;
	if ( bidicontext )
	    bidicontext->ref();
    }
    KoBidiContext *context() const { return bidicontext; }
#endif

public:
    ushort y, baseLine, h;
    short lineSpacing;
    bool hyphenated;
#ifndef QT_NO_COMPLEXTEXT
    KoBidiStatus status;
#endif
    int w;

private:
#ifndef QT_NO_COMPLEXTEXT
    KoBidiContext *bidicontext;
#endif
};

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class KOTEXT_EXPORT KoTextFormatterBase
{
public:
    KoTextFormatterBase();
    virtual ~KoTextFormatterBase() {}
    virtual bool format( KoTextDocument *doc, KoTextParag *parag, int start, const QMap<int, KoTextParagLineStart*> &oldLineStarts, int& y, int& widthUsed ) = 0;
    virtual int formatVertically( KoTextDocument* doc, KoTextParag* parag );

    // Called after formatting a paragraph
    virtual void postFormat( KoTextParag* parag ) = 0;

    int wrapAtColumn() const { return wrapColumn;}
    //virtual void setWrapEnabled( bool b ) { wrapEnabled = b; }
    virtual void setWrapAtColumn( int c ) { wrapColumn = c; }
    virtual void setAllowBreakInWords( bool b ) { biw = b; }
    bool allowBreakInWords() const { return biw; }

    // This setting is passed to KoTextParag::fixParagWidth by postFormat()
    void setViewFormattingChars( bool b ) { m_bViewFormattingChars = b; }
    bool viewFormattingChars() const { return m_bViewFormattingChars; }

    /*virtual*/ bool isBreakable( KoTextString *string, int pos ) const;
    /*virtual*/ bool isStretchable( KoTextString *string, int pos ) const;

protected:
    //virtual KoTextParagLineStart *formatLine( KoTextParag *parag, KoTextString *string, KoTextParagLineStart *line, KoTextStringChar *start,
    //					       KoTextStringChar *last, int align = AlignAuto, int space = 0 );
    //KoTextStringChar

#if 0 //ndef QT_NO_COMPLEXTEXT
    virtual KoTextParagLineStart *bidiReorderLine( KoTextParag *parag, KoTextString *string, KoTextParagLineStart *line, KoTextStringChar *start,
						    KoTextStringChar *last, int align, int space );
#endif

private:
    int wrapColumn;
    //bool wrapEnabled;
    bool m_bViewFormattingChars;
    bool biw;
    bool unused; // for future extensions

#ifdef HAVE_THAI_BREAKS
    static Q3CString *thaiCache;
    static KoTextString *cachedString;
    static ThBreakIterator *thaiIt;
#endif
};


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline int KoTextString::length() const
{
    return data.size();
}

inline void KoTextString::operator+=( const QString &s )
{
    insert( length(), s, 0 );
}

inline KoTextParag *KoTextCursor::parag() const
{
    return string;
}

inline int KoTextCursor::index() const
{
    return idx;
}

inline void KoTextCursor::setParag( KoTextParag *s, bool /*restore*/ )
{
    idx = 0;
    string = s;
    tmpIndex = -1;
}

//inline void KoTextCursor::checkIndex()
//{
//    if ( idx >= string->length() )
//	idx = string->length() - 1;
//}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline KoTextStringChar &KoTextString::at( int i ) const
{
    return data[ i ];
}

inline QString KoTextString::toString() const
{
    return toString( data );
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline KoTextFormat *KoTextStringChar::format() const
{
    return (type == Regular) ? d.format : d.custom->format;
}

inline KoTextCustomItem *KoTextStringChar::customItem() const
{
    return isCustom() ? d.custom->custom : 0;
}

#endif
