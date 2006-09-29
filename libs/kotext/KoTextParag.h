#ifndef KOTEXTPARAG_H
#define KOTEXTPARAG_H

/* This file is part of the KDE project
   Copyright (C) 2001-2005 David Faure <faure@kde.org>

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

// -*- c++ -*-

#include "KoParagLayout.h"

#include "KoTextFormat.h"
#include "KoRichText.h" // for KoTextString
#include "KoTabulator.h"
//Added by qt3to4:
#include <Q3PtrList>
#include <Q3MemArray>
class KoTextFormatterBase;
class KoTextParagLineStart;
class KoTextString;
class KoTextDocument;
class KoParagCounter;
class KoParagStyle;
class KoTextCustomItem;
class KoOasisContext;
class KoSavingContext;
class KoStyleCollection;

struct KoTextParagSelection
{
    int start, end;
};

#if defined(Q_TEMPLATEDLL)
#ifndef Q_MOC_RUN
template class QMap<int, KoTextParagSelection>;
template class QMap<int, KoTextParagLineStart*>;
#endif
#endif

class KOTEXT_EXPORT KoTextParag
{
    friend class KoTextDocument;
    friend class KoTextCursor;

public:
    KoTextParag( KoTextDocument *d, KoTextParag *pr = 0, KoTextParag *nx = 0, bool updateIds = true );
    virtual ~KoTextParag();

    KoTextString *string() const;
    KoTextStringChar *at( int i ) const;
    int leftGap() const;
    int length() const;

    // Abstraction over the trailing-space thing, so that it can be removed later
    int lastCharPos() const { return str->length()-2; }

    void setFormat( KoTextFormat *fm );
    KoTextFormat *paragFormat() const;

    KoTextDocument *document() const;

    QRect rect() const;
    void setRect( const QRect& rect ) { r = rect; }
    void setHeight( int h ) { r.setHeight( h ); }
    void setWidth( int w ) { r.setWidth( w ); }
    void show();
    void hide();
    bool isVisible() const { return visible; }

    KoTextParag *prev() const;
    KoTextParag *next() const;
    void setPrev( KoTextParag *s );
    void setNext( KoTextParag *s );

    void insert( int index, const QString &s );
    void append( const QString &s, bool reallyAtEnd = false );
    void truncate( int index );
    void remove( int index, int len );

    void move( int &dy );
    void format( int start = -1, bool doMove = true );

    /// Call this to ensure that format() will be called on this paragraph later on
    void invalidate( int chr /*ignored*/ = 0 );
    /// Returns false if format() needs to be called on this paragraph
    bool isValid() const;

    /// 'changed' tells the painting code what it needs to paint
    bool hasChanged() const;
    void setChanged( bool b, bool recursive = false );
    short int lineChanged(); // first line that has been changed.
    void setLineChanged( short int line );

    int lineHeightOfChar( int i, int *bl = 0, int *y = 0 ) const;
    KoTextStringChar *lineStartOfChar( int i, int *index = 0, int *line = 0 ) const;
    int lines() const;
    KoTextStringChar *lineStartOfLine( int line, int *index = 0 ) const;
    int lineY( int l ) const;
    int lineBaseLine( int l ) const;
    int lineHeight( int l ) const;
    void lineInfo( int l, int &y, int &h, int &bl ) const;

    void setSelection( int id, int start, int end );
    void removeSelection( int id );
    int selectionStart( int id ) const;
    int selectionEnd( int id ) const;
    bool hasSelection( int id ) const;
    bool hasAnySelection() const;
    bool fullSelected( int id ) const;

    //void setEndState( int s );
    //int endState() const;

    void setParagId( int i );
    int paragId() const;

    QMap<int, KoTextParagLineStart*> &lineStartList();

    void setFormat( int index, int len, const KoTextFormat *f, bool useCollection = true, int flags = -1 );

    void setAlignment( uint a );
    void setAlignmentDirect( uint a ) { align = a; }
    uint alignment() const;

    virtual void paint( QPainter &painter, const QColorGroup &cg, KoTextCursor *cursor, bool drawSelections,
                       int clipx, int clipy, int clipw, int cliph ); // kotextparag.cc


    int topMargin() const;
    int bottomMargin() const;
    int leftMargin() const;
    int firstLineMargin() const;
    int rightMargin() const;
    int lineSpacing( int line ) const;
    int calculateLineSpacing( int line, int start, int last ) const;

    void registerFloatingItem( KoTextCustomItem *i );
    void unregisterFloatingItem( KoTextCustomItem *i );

    void setFullWidth( bool b ) { fullWidth = b; }
    bool isFullWidth() const { return fullWidth; }

    int customItems() const;

    void setDocumentRect( const QRect &r );
    int documentWidth() const;
    //int documentVisibleWidth() const;
    int documentX() const;
    int documentY() const;
    KoTextFormatCollection *formatCollection() const;
    //void setFormatter( KoTextFormatterBase *f );
    KoTextFormatterBase *formatter() const;
    //int minimumWidth() const;
    int widthUsed() const;

    int nextTabDefault( int i, int x );
    int nextTab( int i, int x, int availableWidth );
    int *tabArray() const;
    void setTabArray( int *a );
    void setTabStops( int tw );

    /// Set whether '\n' should break the paragraph into multiple lines
    /// Not used
    void setNewLinesAllowed( bool b );
    /// Return whether '\n' should break the paragraph into multiple lines
    bool isNewLinesAllowed() const;

    virtual void join( KoTextParag *s );
    virtual void copyParagData( KoTextParag *parag );

    //void setBreakable( bool b ) { breakable = b; }
    //bool isBreakable() const { return breakable; }

    void setMovedDown( bool b ) { movedDown = b; }
    bool wasMovedDown() const { return movedDown; }

    void setDirection( QChar::Direction d );
    QChar::Direction direction() const;

    /// Mark a paragraph as being part of the table of contents (kword only)
    void setPartOfTableOfContents( bool b ) { m_toc = b; }
    bool partOfTableOfContents() const { return m_toc; }

    // For KoTextFormatter only
    void insertLineStart( int index, KoTextParagLineStart *ls );

protected:
    void drawLabel( QPainter* p, int x, int y, int w, int h, int base, const QColorGroup& cg );
    void drawCursorDefault( QPainter &painter, KoTextCursor *cursor, int curx, int cury, int curh, const QColorGroup &cg );
    void drawCursor( QPainter &painter, KoTextCursor *cursor, int curx, int cury, int curh, const QColorGroup &cg );

/**
 * We extend KoTextParag with more (zoom-aware) features,
 * like linespacing, borders, counter, tabulators, etc.
 * This also implements WYSIWYG text drawing.
 */
public:
    KoTextDocument * textDocument() const { return document(); }

    KoTextFormat * paragraphFormat() const
    { return static_cast<KoTextFormat *>( paragFormat() ); }

    /** Sets all or some parameters from a paragLayout struct.
     * @param flags selects which settings to apply, see KoParagLayout's enum. */
    virtual void setParagLayout( const KoParagLayout &layout, int flags = KoParagLayout::All,
                                 int marginIndex = -1 );

    const KoParagLayout & paragLayout() { return m_layout; }

    // Margins
    double margin( Q3StyleSheetItem::Margin m ) { return m_layout.margins[m]; }
    const double * margins() const { return m_layout.margins; }
    void setMargin( Q3StyleSheetItem::Margin m, double _i );
    void setMargins( const double * _i );

    /** Line spacing in pt if >=0, can also be one of the LS_* values */
    double kwLineSpacing() const { return m_layout.lineSpacingValue(); }

    void setLineSpacing( double _i );

    KoParagLayout::SpacingType kwLineSpacingType() const { return m_layout.lineSpacingType; }

    void setLineSpacingType( KoParagLayout::SpacingType _type );


    /** Use this to change the paragraph alignment, not KoTextParag::setAlignment ! */
    void setAlign( int align );
    /** Return the real alignment: Auto is resolved to either Left or Right */
    int resolveAlignment() const;

    /// The part of the top margin that can be broken by a page break
    /// Obviously the non-breakable part (e.g. border width) is topMargin()-breakableTopMargin()
    int breakableTopMargin() const;

    // Borders
    KoBorder leftBorder() const { return m_layout.leftBorder; }
    KoBorder rightBorder() const { return m_layout.rightBorder; }
    KoBorder topBorder() const { return m_layout.topBorder; }
    KoBorder bottomBorder() const { return m_layout.bottomBorder; }
    bool hasBorder() const { return m_layout.hasBorder(); }
    bool joinBorder() const { return m_layout.joinBorder; }

    void setLeftBorder( const KoBorder & _brd ) { m_layout.leftBorder = _brd; }
    void setRightBorder( const KoBorder & _brd ) { m_layout.rightBorder = _brd; }
    void setTopBorder( const KoBorder & _brd );
    void setBottomBorder( const KoBorder & _brd );
    void setJoinBorder( bool join );

    // Paragraph background
    QColor backgroundColor() { return m_layout.backgroundColor; }
    void setBackgroundColor( const QColor& color);

    // Counters are used to implement list and heading numbering/bullets.
    void setCounter( const KoParagCounter & counter );
    void setNoCounter();
    void setCounter( const KoParagCounter * pCounter );
    KoParagCounter *counter();

    /** The space required to draw the complete counter label (i.e. the Counter for this
     * paragraph, as well as the Counters for any paragraphs above us in the numbering
     * hierarchy). @see drawLabel(). */
    int counterWidth() const;

    /** Style used by this paragraph */
    KoParagStyle *style() const { return m_layout.style; }
    /** Sets the style in this paragraph, but doesn't _apply_ it, only sets a reference */
    void setStyle( KoParagStyle *style ) { m_layout.style = style; }
    /** Applies the style directly (without undo/redo! See KoTextObject for the full command) */
    void applyStyle( KoParagStyle *style );

    /** Get tabulator positions */
    const KoTabulatorList& tabList() const { return m_layout.tabList(); }
    /** Set tabulator positions */
    void setTabList( const KoTabulatorList &tabList );

    /** Return the X for the shadow distance in pixels (zoomed) */
    int shadowX( KoTextZoomHandler *zh ) const;
    /** Return the Y for the shadow distance in pixels (zoomed) */
    int shadowY( KoTextZoomHandler *zh ) const;
    /** Return the Y for the shadow distance in pt */
    double shadowDistanceY() const;

    /** Set a @p custom item at position @p index, with format @p currentFormat (convenience method) */
    void setCustomItem( int index, KoTextCustomItem * custom, KoTextFormat * currentFormat );
    /** Remove the custom item from position @p index, but doesn't delete it */
    void removeCustomItem( int index );

    /** Find a custom item that we know is somewhere in this paragraph
     * Returns the index in the paragraph */
    int findCustomItem( const KoTextCustomItem * custom ) const;

    /** Cache to find a tab by char index, QMap<char index, tab index> */
    QMap<int, int>& tabCache() { return m_tabCache; }

    /** @return the parag rect, in pixels. This takes care of some rounding problems */
    QRect pixelRect( KoTextZoomHandler* zh ) const;

    /** draw underline and double underline. Static because it's used
     *  for draw double/simple in variable.
     */
     static void drawFontEffects( QPainter * p, KoTextFormat *format, KoTextZoomHandler *zh, QFont font, const QColor & color, int startX, int baseLine, int bw, int y, int h, QChar firstChar );

    /** a bit more clever than KoTextString::toString, e.g. with numbered lists */
    QString toString( int from = 0, int length = 0xffffffff) const;

    /// The app should call this during formatting - e.g. in formatVertically
    void fixParagWidth( bool viewFormattingChars );

    /// Load from XML
    virtual void loadOasis( const QDomElement& e, KoOasisContext& context, KoStyleCollection *styleCollection, uint& pos );
    /// Save to XML
    /// By default the whole paragraph is saved. from/to allow to save only a portion of it.
    /// The 'from' and 'to' characters are both included.
    virtual void saveOasis( KoXmlWriter& writer, KoSavingContext& context,
                            int from, int to, bool saveAnchorsFramesets = false ) const;

    void loadOasisSpan( const QDomElement& parent, KoOasisContext& context, uint& pos );

    void applyListStyle( KoOasisContext& context, int restartNumbering, bool orderedList, bool heading, int level );

#ifndef NDEBUG
    void printRTDebug( int );
#endif

protected:
    void invalidateCounters();
    bool lineHyphenated( int l ) const;

    void paintLines( QPainter &painter, const QColorGroup &cg, KoTextCursor *cursor, bool drawSelections,
                     int clipx, int clipy, int clipw, int cliph );

    void drawParagString( QPainter &painter, const QString &str, int start, int len, int startX,
                          int lastY, int baseLine, int bw, int h, bool drawSelections,
                          KoTextFormat *lastFormat, const Q3MemArray<int> &selectionStarts,
                          const Q3MemArray<int> &selectionEnds, const QColorGroup &cg, bool rightToLeft, int line );
    void drawParagStringInternal( QPainter &painter, const QString &s, int start, int len, int startX,
                                  int lastY, int baseLine, int bw, int h, bool drawSelections,
                                  KoTextFormat *lastFormat, const Q3MemArray<int> &selectionStarts,
                                  const Q3MemArray<int> &selectionEnds, const QColorGroup &cg, bool rightToLeft, int line, KoTextZoomHandler* zh, bool drawingShadow );

    /// Bitfield for drawFormattingChars's "whichFormattingChars" param
    enum { FormattingSpace = 1, FormattingBreak = 2, FormattingEndParag = 4, FormattingTabs = 8,
           AllFormattingChars = FormattingSpace | FormattingBreak | FormattingEndParag | FormattingTabs };

    /// Called by drawParagStringInternal to draw the formatting characters, if the
    /// kotextdocument drawingflag for it was set.
    /// The last arg is a bit special: drawParagStringInternal always sets it to "all",
    /// but reimplementations can change its value.
    virtual void drawFormattingChars( QPainter &painter, int start, int len,
                                      int lastY_pix, int baseLine_pix, int h_pix, // in pixels
                                      bool drawSelections,
                                      KoTextFormat *format, const Q3MemArray<int> &selectionStarts,
                                      const Q3MemArray<int> &selectionEnds, const QColorGroup &cg,
                                      bool rightToLeft, int line, KoTextZoomHandler* zh,
                                      int whichFormattingChars );

protected:
    KoParagLayout m_layout;
    QMap<int, int> m_tabCache;

private:
    KoParagLayout loadParagLayout( KoOasisContext& context, KoStyleCollection *styleCollection, bool findStyle );



    /////// End of kotext-specific additions
private:
    QMap<int, KoTextParagSelection> &selections() const;
    Q3PtrList<KoTextCustomItem> &floatingItems() const;
    /// Returns the height of the biggest character in that line
    int heightForLineSpacing( int startChar, int lastChar ) const;

    QMap<int, KoTextParagLineStart*> lineStarts;
    QRect r;
    KoTextParag *p, *n;
    KoTextDocument *doc;
    bool m_invalid : 1;
    bool changed : 1;
    bool fullWidth : 1;
    bool newLinesAllowed : 1;
    bool visible : 1;
    bool movedDown : 1;
    bool m_toc : 1;
    uint align : 4;
    short int m_lineChanged;
    int id;
    int m_wused;
    KoTextString *str;
    QMap<int, KoTextParagSelection> *mSelections;
    Q3PtrList<KoTextCustomItem> *mFloatingItems;
    KoTextFormat *defFormat; // is this really used?
    int *tArray;

    // Those things are used by QRT for the case of a paragraph without document
    // We don't use this currently, and it's not worth making EVERY parag bigger
    // just for a special case that's rarely used. Better have lightweight KoTextDocument
    // replacement (with common base class), if we ever want efficient single-parag docs...
    //int tabStopWidth;
    //QRect docRect;
    //KoTextFormatterBase *pFormatter;
    //KoTextDocCommandHistory *commandHistory;
};

inline int KoTextParag::length() const
{
    return str->length();
}

inline QRect KoTextParag::rect() const
{
    return r;
}

inline KoTextStringChar *KoTextParag::at( int i ) const
{
    return &str->at( i );
}

inline bool KoTextParag::isValid() const
{
    return !m_invalid;
}

inline bool KoTextParag::hasChanged() const
{
    return changed;
}

inline short int KoTextParag::lineChanged()
{
    return m_lineChanged;
}

inline void KoTextParag::append( const QString &s, bool reallyAtEnd )
{
    if ( reallyAtEnd )
	insert( str->length(), s );
    else
	insert( qMax( str->length() - 1, 0 ), s );
}

inline KoTextParag *KoTextParag::prev() const
{
    return p;
}

inline KoTextParag *KoTextParag::next() const
{
    return n;
}

inline bool KoTextParag::hasAnySelection() const
{
    return mSelections ? !selections().isEmpty() : false;
}

/*inline void KoTextParag::setEndState( int s )
{
    if ( s == state )
	return;
    state = s;
}

inline int KoTextParag::endState() const
{
    return state;
}*/

inline void KoTextParag::setParagId( int i )
{
    id = i;
}

inline int KoTextParag::paragId() const
{
    //if ( id == -1 )
    //	kWarning() << "invalid parag id!!!!!!!! (" << (void*)this << ")" << endl;
    return id;
}

inline QMap<int, KoTextParagLineStart*> &KoTextParag::lineStartList()
{
    return lineStarts;
}

inline KoTextString *KoTextParag::string() const
{
    return str;
}

inline KoTextDocument *KoTextParag::document() const
{
    return doc;
}

inline void KoTextParag::setAlignment( uint a )
{
    if ( a == align )
	return;
    align = a;
    invalidate( 0 );
}

/*inline void KoTextParag::setListStyle( QStyleSheetItem::ListStyle ls )
{
    lstyle = ls;
    invalidate( 0 );
}

inline QStyleSheetItem::ListStyle KoTextParag::listStyle() const
{
    return lstyle;
}*/

inline KoTextFormat *KoTextParag::paragFormat() const
{
    return defFormat;
}

inline void KoTextParag::registerFloatingItem( KoTextCustomItem *i )
{
    floatingItems().append( i );
}

inline void KoTextParag::unregisterFloatingItem( KoTextCustomItem *i )
{
    floatingItems().removeRef( i );
}

/*inline void KoTextParag::addCustomItem()
{
    numCustomItems++;
}

inline void KoTextParag::removeCustomItem()
{
    numCustomItems--;
}*/

inline int KoTextParag::customItems() const
{
    return mFloatingItems ? mFloatingItems->count() : 0;
    // was numCustomItems, but no need for a separate count
}

inline void KoTextParag::setNewLinesAllowed( bool b )
{
    newLinesAllowed = b;
}

inline bool KoTextParag::isNewLinesAllowed() const
{
    return newLinesAllowed;
}

#endif
