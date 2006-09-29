// -*- c++ -*-
/* This file is part of the KDE project
   Copyright (C) 2001 David Faure <faure@kde.org>

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

#ifndef KOTEXTDOCUMENT_H
#define KOTEXTDOCUMENT_H

#include "KoRichText.h"
#include <koffice_export.h>
//Added by qt3to4:
#include <QPixmap>
#include <Q3MemArray>
#include <Q3PtrList>
#include <Q3ValueList>
#if defined(Q_TEMPLATEDLL)
#ifndef Q_MOC_RUN
template class Q_EXPORT QMap<int, QColor>;
template class Q_EXPORT QMap<int, bool>;
template class Q_EXPORT QMap<int, KoTextDocumentSelection>;
template class Q_EXPORT Q3PtrList<KoTextDocument>;
#endif
#endif

class KoStyleCollection;
class KoXmlWriter;
class KoGenStyles;
class KoTextZoomHandler;
class KoTextFormatCollection;
class KoParagVisitor;
class KoTextFormatter;
class KoTextParag;
class CustomItemsMap;

class KOTEXT_EXPORT KoTextDocument : public QObject
{
    Q_OBJECT

    friend class KoTextCursor;
    friend class KoTextParag;

public:
    /** Identifiers for possible selections. */
    enum SelectionId {
	Standard = 0,
        InputMethodPreedit = 1,
        HighlightSelection = 2, // used to highlight during search/replace
	Temp = 32000 // This selection must not be drawn, it's used e.g. by undo/redo to
	// remove multiple lines with removeSelectedText()
    };

    //KoTextDocument( KoTextDocument *p );
    //KoTextDocument( KoTextDocument *d, KoTextFormatCollection *f );
    // see below for constructor
    virtual ~KoTextDocument();

    //KoTextDocument *parent() const { return par; }

    void setText( const QString &text, const QString &context );

    //QString text() const;
    //QString text( int parag ) const;
    //QString originalText() const;

    int x() const;
    int y() const;
    int width() const;
    //int widthUsed() const;
    //int visibleWidth() const;
    int height() const;
    void setWidth( int w );
    //int minimumWidth() const;
    //virtual bool setMinimumWidth( int w, KoTextParag *parag );

    void setY( int y );
    int leftMargin() const;
    void setLeftMargin( int lm );
    int rightMargin() const;
    void setRightMargin( int rm );

    KoTextParag *firstParag() const;
    KoTextParag *lastParag() const;
    void setFirstParag( KoTextParag *p );
    void setLastParag( KoTextParag *p );

    void invalidate();

    //void setPreProcessor( KoTextPreProcessor *sh );
    //KoTextPreProcessor *preProcessor() const;

    void setFormatter( KoTextFormatterBase *f );
    KoTextFormatterBase *formatter() const;

    QColor selectionColor( int id ) const;
    bool invertSelectionText( int id ) const;
    void setSelectionColor( int id, const QColor &c );
    void setInvertSelectionText( int id, bool b );
    bool hasSelection( int id, bool visible = false ) const;
    bool isSelectionSwapped( int id ); //// kotext
    void setSelectionStart( int id, KoTextCursor *cursor );
    bool setSelectionEnd( int id, KoTextCursor *cursor );
    void selectAll( int id );
    bool removeSelection( int id );
    void selectionStart( int id, int &paragId, int &index );
    KoTextCursor selectionStartCursor( int id );
    KoTextCursor selectionEndCursor( int id );
    void selectionEnd( int id, int &paragId, int &index );
    void setFormat( int id, const KoTextFormat *f, int flags );
    KoTextParag *selectionStart( int id );
    KoTextParag *selectionEnd( int id );
    int numSelections() const { return nSelections; }
    void addSelection( int id );

    QString selectedText( int id, bool withCustom = true ) const;
    //void copySelectedText( int id );
    void removeSelectedText( int id, KoTextCursor *cursor );

    KoTextParag *paragAt( int i ) const;

    void addCommand( KoTextDocCommand *cmd );
    KoTextCursor *undo( KoTextCursor *c = 0 );
    KoTextCursor *redo( KoTextCursor *c  = 0 );
    KoTextDocCommandHistory *commands() const { return commandHistory; }

    KoTextFormatCollection *formatCollection() const;

    bool find( const QString &expr, bool cs, bool wo, bool forward, int *parag, int *index, KoTextCursor *cursor );

    //void setTextFormat( Qt::TextFormat f );
    //Qt::TextFormat textFormat() const;

    bool inSelection( int selId, const QPoint &pos ) const;

    void setUnderlineLinks( bool b ) { underlLinks = b; }
    bool underlineLinks() const { return underlLinks; }

    void setPaper( QBrush *brush ) { if ( backBrush ) delete backBrush; backBrush = brush; }
    QBrush *paper() const { return backBrush; }

    //void doLayout( QPainter *p, int w );
#if 0 // see KoTextDocument
    void draw( QPainter *p, const QRect& rect, const QColorGroup &cg, const QBrush *paper = 0 );
    void drawParag( QPainter *p, KoTextParag *parag, int cx, int cy, int cw, int ch,
		    QPixmap *&doubleBuffer, const QColorGroup &cg,
		    bool drawCursor, KoTextCursor *cursor, bool resetChanged = true );
    KoTextParag *draw( QPainter *p, int cx, int cy, int cw, int ch, const QColorGroup &cg,
		      bool onlyChanged = false, bool drawCursor = false, KoTextCursor *cursor = 0,
		      bool resetChanged = true );
#endif

    //void setDefaultFont( const QFont &f );

    void registerCustomItem( KoTextCustomItem *i, KoTextParag *p );
    void unregisterCustomItem( KoTextCustomItem *i, KoTextParag *p );
    const Q3PtrList<KoTextCustomItem> & allCustomItems() const { return customItems; }

    void setFlow( KoTextFlow *f );
    void takeFlow();
    KoTextFlow *flow() const { return flow_; }
    bool isPageBreakEnabled() const { return m_pageBreakEnabled; }
    void setPageBreakEnabled( bool b ) { m_pageBreakEnabled = b; }

    void setWithoutDoubleBuffer( bool b ) { withoutDoubleBuffer = b; }
    bool isWithoutDoubleBuffer() const { return withoutDoubleBuffer; } // added for KWTextDocument

    void setUseFormatCollection( bool b ) { useFC = b; }
    bool useFormatCollection() const { return useFC; }

#ifdef QTEXTTABLE_AVAILABLE
    KoTextTableCell *tableCell() const { return tc; }
    void setTableCell( KoTextTableCell *c ) { tc = c; }
#endif

    void setPlainText( const QString &text );
    //void setRichText( const QString &text, const QString &context );
    //QString richText( KoTextParag *p = 0 ) const;
    QString plainText() const;

    //bool focusNextPrevChild( bool next );

    int alignment() const;
    void setAlignment( int a );

    int *tabArray() const;
    int tabStopWidth() const;
    void setTabArray( int *a );
    void setTabStops( int tw );

    void setUndoDepth( int d ) { commandHistory->setUndoDepth( d ); }
    int undoDepth() const { return commandHistory->undoDepth(); }

    int length() const;
    void clear( bool createEmptyParag = false );

    KoTextParag* loadList( const QDomElement& list, KoOasisContext& context, KoTextParag* lastParagraph, KoStyleCollection * styleColl, KoTextParag* nextParagraph );

    // For normal loading nextParagraph and pos are 0.
    KoTextParag* loadOasisText( const QDomElement &bodyElem, KoOasisContext& context, KoTextParag* lastParagraph, KoStyleCollection * styleColl, KoTextParag* nextParagraph );

    QString copySelection( KoXmlWriter& writer, KoSavingContext& context, int selectionId );

    void saveOasisContent( KoXmlWriter& writer, KoSavingContext& context ) const;

    virtual KoTextParag *createParag( KoTextDocument *d, KoTextParag *pr = 0, KoTextParag *nx = 0, bool updateIds = true );

    // Whether margins are added or max'ed.
    int addMargins() const { return true; }

    void informParagraphDeleted( KoTextParag* parag );

signals:
    //void minimumWidthChanged( int );

    /** Emitted when a paragraph is deleted (kotext addition) */
    void paragraphDeleted( KoTextParag* parag );

private:
    void init();
    QPixmap *bufferPixmap( const QSize &s );

    //// Beginning of kotext additions

public:
    /**
     * Construct a text document, i.e. a set of paragraphs
     *
     * @param zoomHandler The KoTextZoomHandler instance, to handle the zooming, as the name says :)
     * We need one here because KoTextFormatter needs one for formatting, currently.
     *
     * @param fc a format collection for this document. Ownership is transferred to the document. ###
     * @param formatter a text formatter for this document. If 0L, a KoTextFormatter is created.
     *  If not, ownership of the given one is transferred to the document.
     * @param createInitialParag if true, an initial KoTextParag is created. Set to false if you reimplement createParag,
     *  since the constructor can't call the reimplementation. In that case, make sure to call
     *  clear(true) in your constructor; QRT doesn't support documents without paragraphs.
     */
    KoTextDocument( KoTextZoomHandler *zoomHandler,
                    KoTextFormatCollection *fc, KoTextFormatter *formatter = 0L,
                    bool createInitialParag = true );

    /** Return the zoom handler associated with this document,
     * used when formatting. Don't use for any other purpose, it might disappear. */
    KoTextZoomHandler * formattingZoomHandler() const { return m_zoomHandler; }

    /**
     * Return the zoom handler currently used for drawing.
     * (This means, at a particular zoom level).
     * Don't call this in a method that isn't called by drawWYSIWYG, it will be 0L !
     * (a different one than zoomHandler(), in case it disappears one day,
     * to have different zoom levels in different views)
     */
    KoTextZoomHandler * paintingZoomHandler() const { return m_zoomHandler; }


    /** Visit all the parts of a selection.
     * Returns true, unless canceled. See KoParagVisitor. */
    bool visitSelection( int selectionId, KoParagVisitor *visitor, bool forward = true );

    /** Visit all paragraphs of the document.
     * Returns true, unless canceled. See KoParagVisitor. */
    bool visitDocument( KoParagVisitor *visitor, bool forward = true );

    /** Visit the document between those two point.
     * Returns true, unless canceled. See KoParagVisitor. */
    bool visitFromTo( KoTextParag *firstParag, int firstIndex, KoTextParag* lastParag, int lastIndex, KoParagVisitor* visitor, bool forw = true );

    /**
     * Used by ~KoTextParag to know if it should die quickly
     */
    bool isDestroying() const { return m_bDestroying; }

    /**
     * Flags for drawWYSIWYG and drawParagWYSIWYG
     */
    enum DrawingFlags {
        DrawMisspelledLine = 1,
        DrawFormattingChars = 2,
        DrawSelections = 4,
        DontDrawNoteVariable = 8,
        TransparentBackground = 16
    };
    /** The main drawing method. Equivalent to KoTextDocument::draw, but reimplemented
     * for wysiwyg */
    KoTextParag *drawWYSIWYG( QPainter *p, int cx, int cy, int cw, int ch, const QColorGroup &cg,
                              KoTextZoomHandler* zoomHandler, bool onlyChanged = false,
                              bool drawCursor = false, KoTextCursor *cursor = 0,
                              bool resetChanged = true, uint drawingFlags = KoTextDocument::DrawSelections );

    /** Draw a single paragraph (used by drawWYSIWYG and by KWTextFrameSet::drawCursor).
     * Equivalent to KoTextDocument::draw, but modified for wysiwyg */
    void drawParagWYSIWYG( QPainter *p, KoTextParag *parag, int cx, int cy, int cw, int ch,
                           QPixmap *&doubleBuffer, const QColorGroup &cg,
                           KoTextZoomHandler* zoomHandler,
                           bool drawCursor, KoTextCursor *cursor,
                           bool resetChanged = true,
                           uint drawingFlags = KoTextDocument::DrawSelections );

    /** Set by drawParagWYSIWYG, used by KoTextParag::drawParagString */
    bool drawFormattingChars() const { return (m_drawingFlags & DrawFormattingChars); }
    /** Set by drawParagWYSIWYG, used by KoTextParag::drawParagStringInternal */
    bool drawingMissingSpellLine() const { return (m_drawingFlags & DrawMisspelledLine); }

    /** Set by drawParagWYSIWYG, used by KoTextParag::drawParagStringInternal */
    bool dontDrawingNoteVariable() const { return (m_drawingFlags & DontDrawNoteVariable); }

    virtual KoTextDocCommand *deleteTextCommand( KoTextDocument *textdoc, int id, int index, const Q3MemArray<KoTextStringChar> & str, const CustomItemsMap & customItemsMap, const Q3ValueList<KoParagLayout> & oldParagLayouts );

    void emitNewCommand(KCommand *cmd) {
        emit newCommand( cmd );
    }
    void emitRepaintChanged() {
        emit repaintChanged();
    }
signals:
    /**
     * Emitted when a new command has been created and should be added to
     * the main list of commands (usually in the KoDocument).
     * KoTextObject connects (and forwards) that one.
     */
    void newCommand( KCommand *cmd );
    /**
     * Tell the world that we'd like some repainting to happen.
     * KoTextObject connects (and forwards) that one.
     */
    void repaintChanged();

protected:
    void drawWithoutDoubleBuffer( QPainter *p, const QRect &rect, const QColorGroup &cg,
                                  KoTextZoomHandler* zoomHandler, const QBrush *paper = 0 );

    /**
     * Called by loadOasisText. This allows to extend the loading mechanism
     * for special tags no handled by kotext (images, textboxes, tables, etc.)
     * @return true if @p tag was handled.
     */
    virtual bool loadOasisBodyTag( const QDomElement& /*tag*/, KoOasisContext& /*context*/,
                                   KoTextParag* & /*lastParagraph*/, KoStyleCollection* /*styleColl*/,
                                   KoTextParag* /*nextParagraph*/ ) {
        return false;
    }

    /**
     * Called by KoTextParag::loadOasisSpan. This allows to extend the loading mechanism
     * for special tags no handled by kotext (bookmarks, image, textbox, link, footnotes etc.)
     * This method is here instead of in KoTextParag because it's easier to derive from
     * KoTextDocument.
     * @return true (and optionally @p textData and @p customItem) if @p tag was handled.
     */
    virtual bool loadSpanTag( const QDomElement& /*tag*/, KoOasisContext& /*context*/,
                              KoTextParag* /*parag*/, uint /*pos*/,
                              QString& /*textData*/, KoTextCustomItem* & /*customItem*/ ) {
        return false;
    }

private:
    // The zoom handler used when formatting
    // (due to the pixelx/pixelww stuff in KoTextFormatter)
    KoTextZoomHandler * m_zoomHandler;
    bool m_bDestroying;
    uint m_drawingFlags;

    //// End of kotext additions

private:
    /*struct Q_EXPORT Focus {
	KoTextParag *parag;
	int start, len;
	QString href;
    };*/

    int cx, cy; //, cw, vw;
    KoTextParag *fParag, *lParag;
    //KoTextPreProcessor *pProcessor;
    QMap<int, QColor> selectionColors;
    QMap<int, KoTextDocumentSelection> selections;
    QMap<int, bool> selectionText;
    KoTextDocCommandHistory *commandHistory;
    KoTextFormatterBase *pFormatter;
    KoTextFormatCollection *fCollection;
    //Qt::TextFormat txtFormat;
    //bool preferRichText : 1;
    bool m_pageBreakEnabled : 1;
    bool useFC : 1;
    bool withoutDoubleBuffer : 1;
    bool underlLinks : 1;
    //bool nextDoubleBuffered : 1;
    bool addMargs : 1;
    int nSelections;
    KoTextFlow *flow_;
    Q3PtrList<KoTextCustomItem> customItems;
    QBrush *backBrush;
    QPixmap *buf_pixmap;
    //Focus focusIndicator;
    //int minw;
    int leftmargin;
    int rightmargin;
    //KoTextParag *minwParag;
    int align;
    int *tArray;
    int tStopWidth;
};

inline int KoTextDocument::x() const
{
    return cx;
}

inline int KoTextDocument::y() const
{
    return cy;
}

inline int KoTextDocument::width() const
{
    return flow_->width();
    //return qMax( cw, flow_->width() );
}

//inline int KoTextDocument::visibleWidth() const
//{
//    return vw;
//}

inline KoTextParag *KoTextDocument::firstParag() const
{
    return fParag;
}

inline KoTextParag *KoTextDocument::lastParag() const
{
    return lParag;
}

inline void KoTextDocument::setFirstParag( KoTextParag *p )
{
    fParag = p;
}

inline void KoTextDocument::setLastParag( KoTextParag *p )
{
    lParag = p;
}

inline void KoTextDocument::setWidth( int w )
{
    //cw = qMax( w, minw );
    flow_->setWidth( w );
    //vw = w;
}

//inline int KoTextDocument::minimumWidth() const
//{
//    return minw;
//}

inline void KoTextDocument::setY( int y )
{
    cy = y;
}

inline int KoTextDocument::leftMargin() const
{
    return leftmargin;
}

inline void KoTextDocument::setLeftMargin( int lm )
{
    leftmargin = lm;
}

inline int KoTextDocument::rightMargin() const
{
    return rightmargin;
}

inline void KoTextDocument::setRightMargin( int rm )
{
    rightmargin = rm;
}

/*inline KoTextPreProcessor *KoTextDocument::preProcessor() const
{
    return pProcessor;
}

inline void KoTextDocument::setPreProcessor( KoTextPreProcessor * sh )
{
    pProcessor = sh;
}*/

inline void KoTextDocument::setFormatter( KoTextFormatterBase *f )
{
    delete pFormatter;
    pFormatter = f;
}

inline KoTextFormatterBase *KoTextDocument::formatter() const
{
    return pFormatter;
}

inline QColor KoTextDocument::selectionColor( int id ) const
{
    return selectionColors[ id ];
}

inline bool KoTextDocument::invertSelectionText( int id ) const
{
    return selectionText[ id ];
}

inline void KoTextDocument::setSelectionColor( int id, const QColor &c )
{
    selectionColors[ id ] = c;
}

inline void KoTextDocument::setInvertSelectionText( int id, bool b )
{
    selectionText[ id ] = b;
}

inline KoTextFormatCollection *KoTextDocument::formatCollection() const
{
    return fCollection;
}

inline int KoTextDocument::alignment() const
{
    return align;
}

inline void KoTextDocument::setAlignment( int a )
{
    align = a;
}

inline int *KoTextDocument::tabArray() const
{
    return tArray;
}

inline int KoTextDocument::tabStopWidth() const
{
    return tStopWidth;
}

inline void KoTextDocument::setTabArray( int *a )
{
    tArray = a;
}

inline void KoTextDocument::setTabStops( int tw )
{
    tStopWidth = tw;
}

/*inline QString KoTextDocument::originalText() const
{
    if ( oTextValid )
	return oText;
    return text();
}*/

inline void KoTextDocument::setFlow( KoTextFlow *f )
{
    if ( flow_ )
	delete flow_;
    flow_ = f;
}

inline void KoTextDocument::takeFlow()
{
    flow_ = 0L;
}

/**
 * Base class for "visitors". Visitors are a well-designed way to
 * apply a given operation to all the paragraphs in a selection, or
 * in a document. The visitor needs to inherit KoParagVisitor, and implement visit().
 */
class KoParagVisitor
{
protected:
    /** protected since this is an abstract base class */
    KoParagVisitor() {}
    virtual ~KoParagVisitor() {}
public:
    /** Visit the paragraph @p parag, from index @p start to index @p end */
    virtual bool visit( KoTextParag *parag, int start, int end ) = 0;
};

class KCommand;
class QDomElement;
class KMacroCommand;

/** A CustomItemsMap associates a custom item to an index
 * Used in the undo/redo info for insert/delete text. */
class CustomItemsMap : public QMap<int, KoTextCustomItem *>
{
public:

    /** Insert all the items from the map, into the existing text */
    void insertItems( const KoTextCursor & startCursor, int size );

    /** Delete all the items from the map, adding their commands into macroCmd */
    void deleteAll( KMacroCommand *macroCmd );
};

#endif
