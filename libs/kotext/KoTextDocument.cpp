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

#include "KoTextDocument.h"
#include "KoTextParag.h"
#include "KoTextZoomHandler.h"
#include "KoTextFormatter.h"
#include "KoTextFormat.h"
#include "KoParagCounter.h"
#include "KoTextCommand.h"
#include "KoOasisContext.h"
#include "KoVariable.h"
#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include <KoDom.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <QApplication>
//Added by qt3to4:
#include <QPixmap>
#include <Q3MemArray>
#include <Q3PtrList>
#include <Q3ValueList>
#include <assert.h>

//#define DEBUG_PAINTING

//// Note that many methods are implemented in korichtext.cpp
//// Those are the ones that come from Qt, and that mostly work :)

KoTextDocument::KoTextDocument( KoTextZoomHandler *zoomHandler, KoTextFormatCollection *fc,
                                KoTextFormatter *formatter, bool createInitialParag )
    : m_zoomHandler( zoomHandler ),
      m_bDestroying( false ),
#ifdef QTEXTTABLE_AVAILABLE
      par( 0L /*we don't use parent documents */ ),
      tc( 0 ),
#endif
      tArray( 0 ), tStopWidth( 0 )
{
    fCollection = fc;
    init(); // see korichtext.cpp

    m_drawingFlags = 0;
    if ( !formatter )
        formatter = new KoTextFormatter;
    setFormatter( formatter );

    setY( 0 );
    setLeftMargin( 0 );
    setRightMargin( 0 );

    // Delete the KoTextParag created by KoTextDocument::init() if createInitialParag is false.
    if ( !createInitialParag )
        clear( false );
}

void KoTextDocument::init()
{
    //pProcessor = 0;
    useFC = TRUE;
    pFormatter = 0;
    fParag = 0;
    m_pageBreakEnabled = false;
    //minw = 0;
    align = Qt::AlignLeft;
    nSelections = 2;

    underlLinks = TRUE;
    backBrush = 0;
    buf_pixmap = 0;
    //nextDoubleBuffered = FALSE;

    //if ( par )
//	withoutDoubleBuffer = par->withoutDoubleBuffer;
//    else
	withoutDoubleBuffer = FALSE;

    lParag = fParag = createParag( this, 0, 0 );

    //cx = 0;
    //cy = 2;
    //if ( par )
	cx = cy = 0;
    //cw = 600; // huh?
    //vw = 0;
    flow_ = new KoTextFlow;
    //flow_->setWidth( cw );

    leftmargin = 0; // 4 in QRT
    rightmargin = 0; // 4 in QRT

    selectionColors[ Standard ] = QApplication::palette().color( QPalette::Active, QColorGroup::Highlight );
    selectionText[ Standard ] = TRUE;
    assert( Standard < nSelections );
    selectionText[ InputMethodPreedit ] = FALSE;
    assert( InputMethodPreedit < nSelections );
    commandHistory = new KoTextDocCommandHistory( 100 );
    tStopWidth = formatCollection()->defaultFormat()->width( 'x' ) * 8;
}

KoTextDocument::~KoTextDocument()
{
    //if ( par )
//	par->removeChild( this );
    //// kotext
    m_bDestroying = true;
    clear( false );
    ////
    delete commandHistory;
    delete flow_;
    //if ( !par )
	delete pFormatter;
    delete fCollection;
    //delete pProcessor;
    delete buf_pixmap;
    delete backBrush;
    if ( tArray )
	delete [] tArray;
}

void KoTextDocument::clear( bool createEmptyParag )
{
    if ( flow_ )
	flow_->clear();
    while ( fParag ) {
	KoTextParag *p = fParag->next();
	fParag->string()->clear(); // avoid the "unregister custom items" code, not needed
	delete fParag;
	fParag = p;
    }
    fParag = lParag = 0;
    if ( createEmptyParag )
	fParag = lParag = createParag( this );
    selections.clear();
    customItems.clear();
}

/*
   // Looks slow!
int KoTextDocument::widthUsed() const
{
    KoTextParag *p = fParag;
    int w = 0;
    while ( p ) {
	int a = p->alignment();
	p->setAlignment( Qt::AlignLeft );
	p->invalidate( 0 );
	p->format();
	w = qMax( w, p->rect().width() );
	p->setAlignment( a );
	p->invalidate( 0 );
	p = p->next();
    }
    return w;
}
*/

int KoTextDocument::height() const
{
    int h = 0;
    if ( lParag )
	h = lParag->rect().top() + lParag->rect().height() + 1;
    //int fh = flow_->boundingRect().height();
    //return qMax( h, fh );
    return h;
}


KoTextParag *KoTextDocument::createParag( KoTextDocument *d, KoTextParag *pr, KoTextParag *nx, bool updateIds )
{
    return new KoTextParag( d, pr, nx, updateIds );
}

void KoTextDocument::setPlainText( const QString &text )
{
    clear();
    //preferRichText = FALSE;
    //oTextValid = TRUE;
    //oText = text;

    int lastNl = 0;
    int nl = text.find( '\n' );
    if ( nl == -1 ) {
	lParag = createParag( this, lParag, 0 );
	if ( !fParag )
	    fParag = lParag;
	QString s = text;
	if ( !s.isEmpty() ) {
	    if ( s[ (int)s.length() - 1 ] == '\r' )
		s.remove( s.length() - 1, 1 );
	    lParag->append( s );
	}
    } else {
	for (;;) {
	    lParag = createParag( this, lParag, 0 );
	    if ( !fParag )
		fParag = lParag;
	    QString s = text.mid( lastNl, nl - lastNl );
	    if ( !s.isEmpty() ) {
		if ( s[ (int)s.length() - 1 ] == '\r' )
		    s.remove( s.length() - 1, 1 );
		lParag->append( s );
	    }
	    if ( nl == 0xffffff )
		break;
	    lastNl = nl + 1;
	    nl = text.find( '\n', nl + 1 );
	    if ( nl == -1 )
		nl = 0xffffff;
	}
    }
    if ( !lParag )
	lParag = fParag = createParag( this, 0, 0 );
}

void KoTextDocument::setText( const QString &text, const QString & /*context*/ )
{
    selections.clear();
    setPlainText( text );
}

QString KoTextDocument::plainText() const
{
    QString buffer;
    QString s;
    KoTextParag *p = fParag;
    while ( p ) {
        s = p->string()->toString();
        s.remove( s.length() - 1, 1 );
        if ( p->next() )
            s += "\n";
        buffer += s;
        p = p->next();
    }
    return buffer;
}

void KoTextDocument::invalidate()
{
    KoTextParag *s = fParag;
    while ( s ) {
	s->invalidate( 0 );
	s = s->next();
    }
}

void KoTextDocument::informParagraphDeleted( KoTextParag* parag )
{
    QMap<int, KoTextDocumentSelection>::Iterator it = selections.begin();
    for ( ; it != selections.end(); ++it )
    {
        if ( (*it).startCursor.parag() == parag ) {
            if ( parag->prev() ) {
                KoTextParag* prevP = parag->prev();
                (*it).startCursor.setParag( prevP );
                (*it).startCursor.setIndex( prevP->length()-1 );
            } else
                (*it).startCursor.setParag( parag->next() ); // sets index to 0
        }
        if ( (*it).endCursor.parag() == parag ) {
            if ( parag->prev() ) {
                KoTextParag* prevP = parag->prev();
                (*it).endCursor.setParag( prevP );
                (*it).endCursor.setIndex( prevP->length()-1 );
            } else
                (*it).endCursor.setParag( parag->next() ); // sets index to 0
        }
    }
    emit paragraphDeleted( parag );
}

void KoTextDocument::selectionStart( int id, int &paragId, int &index )
{
    QMap<int, KoTextDocumentSelection>::Iterator it = selections.find( id );
    if ( it == selections.end() )
	return;
    KoTextDocumentSelection &sel = *it;
    paragId = !sel.swapped ? sel.startCursor.parag()->paragId() : sel.endCursor.parag()->paragId();
    index = !sel.swapped ? sel.startCursor.index() : sel.endCursor.index();
}

KoTextCursor KoTextDocument::selectionStartCursor( int id)
{
    QMap<int, KoTextDocumentSelection>::Iterator it = selections.find( id );
    if ( it == selections.end() )
	return KoTextCursor( this );
    KoTextDocumentSelection &sel = *it;
    if ( sel.swapped )
	return sel.endCursor;
    return sel.startCursor;
}

KoTextCursor KoTextDocument::selectionEndCursor( int id)
{
    QMap<int, KoTextDocumentSelection>::Iterator it = selections.find( id );
    if ( it == selections.end() )
	return KoTextCursor( this );
    KoTextDocumentSelection &sel = *it;
    if ( !sel.swapped )
	return sel.endCursor;
    return sel.startCursor;
}

void KoTextDocument::selectionEnd( int id, int &paragId, int &index )
{
    QMap<int, KoTextDocumentSelection>::Iterator it = selections.find( id );
    if ( it == selections.end() )
	return;
    KoTextDocumentSelection &sel = *it;
    paragId = sel.swapped ? sel.startCursor.parag()->paragId() : sel.endCursor.parag()->paragId();
    index = sel.swapped ? sel.startCursor.index() : sel.endCursor.index();
}

bool KoTextDocument::isSelectionSwapped( int id )
{
    QMap<int, KoTextDocumentSelection>::Iterator it = selections.find( id );
    if ( it == selections.end() )
	return false;
    KoTextDocumentSelection &sel = *it;
    return sel.swapped;
}

KoTextParag *KoTextDocument::selectionStart( int id )
{
    QMap<int, KoTextDocumentSelection>::Iterator it = selections.find( id );
    if ( it == selections.end() )
	return 0;
    KoTextDocumentSelection &sel = *it;
    if ( sel.startCursor.parag()->paragId() <  sel.endCursor.parag()->paragId() )
	return sel.startCursor.parag();
    return sel.endCursor.parag();
}

KoTextParag *KoTextDocument::selectionEnd( int id )
{
    QMap<int, KoTextDocumentSelection>::Iterator it = selections.find( id );
    if ( it == selections.end() )
	return 0;
    KoTextDocumentSelection &sel = *it;
    if ( sel.startCursor.parag()->paragId() >  sel.endCursor.parag()->paragId() )
	return sel.startCursor.parag();
    return sel.endCursor.parag();
}

void KoTextDocument::addSelection( int id )
{
    nSelections = qMax( nSelections, id + 1 );
}

static void setSelectionEndHelper( int id, KoTextDocumentSelection &sel, KoTextCursor &start, KoTextCursor &end )
{
    KoTextCursor c1 = start;
    KoTextCursor c2 = end;
    if ( sel.swapped ) {
	c1 = end;
	c2 = start;
    }

    c1.parag()->removeSelection( id );
    c2.parag()->removeSelection( id );
    if ( c1.parag() != c2.parag() ) {
	c1.parag()->setSelection( id, c1.index(), c1.parag()->length() - 1 );
	c2.parag()->setSelection( id, 0, c2.index() );
    } else {
	c1.parag()->setSelection( id, qMin( c1.index(), c2.index() ), qMax( c1.index(), c2.index() ) );
    }

    sel.startCursor = start;
    sel.endCursor = end;
    if ( sel.startCursor.parag() == sel.endCursor.parag() )
	sel.swapped = sel.startCursor.index() > sel.endCursor.index();
}

bool KoTextDocument::setSelectionEnd( int id, KoTextCursor *cursor )
{
    QMap<int, KoTextDocumentSelection>::Iterator it = selections.find( id );
    if ( it == selections.end() )
	return FALSE;
    KoTextDocumentSelection &sel = *it;

    KoTextCursor start = sel.startCursor;
    KoTextCursor end = *cursor;

    if ( start == end ) {
	removeSelection( id );
	setSelectionStart( id, cursor );
	return TRUE;
    }

    if ( sel.endCursor.parag() == end.parag() ) {
	setSelectionEndHelper( id, sel, start, end );
	return TRUE;
    }

    bool inSelection = FALSE;
    KoTextCursor c( this );
    KoTextCursor tmp = sel.startCursor;
    if ( sel.swapped )
	tmp = sel.endCursor;
    KoTextCursor tmp2 = *cursor;
    c.setParag( tmp.parag()->paragId() < tmp2.parag()->paragId() ? tmp.parag() : tmp2.parag() );
    KoTextCursor old;
    bool hadStart = FALSE;
    bool hadEnd = FALSE;
    bool hadStartParag = FALSE;
    bool hadEndParag = FALSE;
    bool hadOldStart = FALSE;
    bool hadOldEnd = FALSE;
    bool leftSelection = FALSE;
    sel.swapped = FALSE;
    for ( ;; ) {
	if ( c == start )
	    hadStart = TRUE;
	if ( c == end )
	    hadEnd = TRUE;
	if ( c.parag() == start.parag() )
	    hadStartParag = TRUE;
	if ( c.parag() == end.parag() )
	    hadEndParag = TRUE;
	if ( c == sel.startCursor )
	    hadOldStart = TRUE;
	if ( c == sel.endCursor )
	    hadOldEnd = TRUE;

	if ( !sel.swapped &&
	     ( hadEnd && !hadStart ||
	       hadEnd && hadStart && start.parag() == end.parag() && start.index() > end.index() ) )
	    sel.swapped = TRUE;

	if ( c == end && hadStartParag ||
	     c == start && hadEndParag ) {
	    KoTextCursor tmp = c;
	    if ( tmp.parag() != c.parag() ) {
		int sstart = tmp.parag()->selectionStart( id );
		tmp.parag()->removeSelection( id );
		tmp.parag()->setSelection( id, sstart, tmp.index() );
	    }
	}

	if ( inSelection &&
	     ( c == end && hadStart || c == start && hadEnd ) )
	     leftSelection = TRUE;
	else if ( !leftSelection && !inSelection && ( hadStart || hadEnd ) )
	    inSelection = TRUE;

	bool noSelectionAnymore = hadOldStart && hadOldEnd && leftSelection && !inSelection && !c.parag()->hasSelection( id ) && c.atParagEnd();
	c.parag()->removeSelection( id );
	if ( inSelection ) {
	    if ( c.parag() == start.parag() && start.parag() == end.parag() ) {
		c.parag()->setSelection( id, qMin( start.index(), end.index() ), qMax( start.index(), end.index() ) );
	    } else if ( c.parag() == start.parag() && !hadEndParag ) {
		c.parag()->setSelection( id, start.index(), c.parag()->length() - 1 );
	    } else if ( c.parag() == end.parag() && !hadStartParag ) {
		c.parag()->setSelection( id, end.index(), c.parag()->length() - 1 );
	    } else if ( c.parag() == end.parag() && hadEndParag ) {
		c.parag()->setSelection( id, 0, end.index() );
	    } else if ( c.parag() == start.parag() && hadStartParag ) {
		c.parag()->setSelection( id, 0, start.index() );
	    } else {
		c.parag()->setSelection( id, 0, c.parag()->length() - 1 );
	    }
	}

	if ( leftSelection )
	    inSelection = FALSE;

	old = c;
	c.gotoNextLetter();
	if ( old == c || noSelectionAnymore )
	    break;
    }

    if ( !sel.swapped )
	sel.startCursor.parag()->setSelection( id, sel.startCursor.index(), sel.startCursor.parag()->length() - 1 );

    sel.startCursor = start;
    sel.endCursor = end;
    if ( sel.startCursor.parag() == sel.endCursor.parag() )
	sel.swapped = sel.startCursor.index() > sel.endCursor.index();

    setSelectionEndHelper( id, sel, start, end );

    return TRUE;
}

void KoTextDocument::selectAll( int id )
{
    removeSelection( id );

    KoTextDocumentSelection sel;
    sel.swapped = FALSE;
    KoTextCursor c( this );

    c.setParag( fParag );
    c.setIndex( 0 );
    sel.startCursor = c;

    c.setParag( lParag );
    c.setIndex( lParag->length() - 1 );
    sel.endCursor = c;

    KoTextParag *p = fParag;
    while ( p ) {
	p->setSelection( id, 0, p->length() - 1 );
#ifdef QTEXTTABLE_AVAILABLE
	for ( int i = 0; i < (int)p->length(); ++i ) {
	    if ( p->at( i )->isCustom() && p->at( i )->customItem()->isNested() ) {
		KoTextTable *t = (KoTextTable*)p->at( i )->customItem();
		Q3PtrList<KoTextTableCell> tableCells = t->tableCells();
		for ( KoTextTableCell *c = tableCells.first(); c; c = tableCells.next() )
		    c->richText()->selectAll( id );
	    }
	}
#endif
	p = p->next();
    }

    selections.insert( id, sel );
}

bool KoTextDocument::removeSelection( int id )
{
    QMap<int, KoTextDocumentSelection>::Iterator it = selections.find( id );
    if ( it == selections.end() )
	return FALSE;

    KoTextDocumentSelection &sel = *it;

    KoTextCursor c( this );
    KoTextCursor tmp = sel.startCursor;
    if ( sel.swapped )
	tmp = sel.endCursor;
    c.setParag( tmp.parag() );
    KoTextCursor old;
    bool hadStart = FALSE;
    bool hadEnd = FALSE;
    KoTextParag *lastParag = 0;
    bool leftSelection = FALSE;
    bool inSelection = FALSE;
    sel.swapped = FALSE;
    for ( ;; ) {
	if ( !hadStart && c.parag() == sel.startCursor.parag() )
	    hadStart = TRUE;
	if ( !hadEnd && c.parag() == sel.endCursor.parag() )
	    hadEnd = TRUE;

        if ( !leftSelection && !inSelection && ( c.parag() == sel.startCursor.parag() || c.parag() == sel.endCursor.parag() ) )
	    inSelection = TRUE;

	if ( inSelection &&
	     ( c == sel.endCursor && hadStart || c == sel.startCursor && hadEnd ) ) {
	     leftSelection = TRUE;
             inSelection = FALSE;
        }

	bool noSelectionAnymore = leftSelection && !inSelection && !c.parag()->hasSelection( id ) && c.atParagEnd();

	if ( lastParag != c.parag() )
	    c.parag()->removeSelection( id );

	old = c;
	lastParag = c.parag();
	c.gotoNextLetter();
	if ( old == c || noSelectionAnymore )
	    break;
    }

    selections.remove( id );
    return TRUE;
}

QString KoTextDocument::selectedText( int id, bool withCustom ) const
{
    // ######## TODO: look at textFormat() and return rich text or plain text (like the text() method!)
    QMap<int, KoTextDocumentSelection>::ConstIterator it = selections.find( id );
    if ( it == selections.end() )
	return QString::null;

    KoTextDocumentSelection sel = *it;


    KoTextCursor c1 = sel.startCursor;
    KoTextCursor c2 = sel.endCursor;
    if ( sel.swapped ) {
	c2 = sel.startCursor;
	c1 = sel.endCursor;
    }

    if ( c1.parag() == c2.parag() ) {
	QString s;
	KoTextParag *p = c1.parag();
	int end = c2.index();
	if ( p->at( qMax( 0, end - 1 ) )->isCustom() )
	    ++end;
	if ( !withCustom || !p->customItems() ) {
	    s += p->string()->toString().mid( c1.index(), end - c1.index() );
	} else {
	    for ( int i = c1.index(); i < end; ++i ) {
		if ( p->at( i )->isCustom() ) {
#ifdef QTEXTTABLE_AVAILABLE
		    if ( p->at( i )->customItem()->isNested() ) {
			s += "\n";
			KoTextTable *t = (KoTextTable*)p->at( i )->customItem();
			Q3PtrList<KoTextTableCell> cells = t->tableCells();
			for ( KoTextTableCell *c = cells.first(); c; c = cells.next() )
			    s += c->richText()->plainText() + "\n";
			s += "\n";
		    }
#endif
		} else {
		    s += p->at( i )->c;
		}
		s += "\n";
	    }
	}
	return s;
    }

    QString s;
    KoTextParag *p = c1.parag();
    int start = c1.index();
    while ( p ) {
	int end = p == c2.parag() ? c2.index() : p->length() - 1;
	if ( p == c2.parag() && p->at( qMax( 0, end - 1 ) )->isCustom() )
	    ++end;
	if ( !withCustom || !p->customItems() ) {
	    s += p->string()->toString().mid( start, end - start );
	    if ( p != c2.parag() )
		s += "\n";
	} else {
	    for ( int i = start; i < end; ++i ) {
		if ( p->at( i )->isCustom() ) {
#ifdef QTEXTTABLE_AVAILABLE
		    if ( p->at( i )->customItem()->isNested() ) {
			s += "\n";
			KoTextTable *t = (KoTextTable*)p->at( i )->customItem();
			Q3PtrList<KoTextTableCell> cells = t->tableCells();
			for ( KoTextTableCell *c = cells.first(); c; c = cells.next() )
			    s += c->richText()->plainText() + "\n";
			s += "\n";
		    }
#endif
		} else {
		    s += p->at( i )->c;
		}
		s += "\n";
	    }
	}
	start = 0;
	if ( p == c2.parag() )
	    break;
	p = p->next();
    }
    return s;
}

QString KoTextDocument::copySelection( KoXmlWriter& writer, KoSavingContext& context, int selectionId )
{
    KoTextCursor c1 = selectionStartCursor( selectionId );
    KoTextCursor c2 = selectionEndCursor( selectionId );
    QString text;
    if ( c1.parag() == c2.parag() )
    {
        text = c1.parag()->toString( c1.index(), c2.index() - c1.index() );

        c1.parag()->saveOasis( writer, context, c1.index(), c2.index()-1, true );
    }
    else
    {
        text += c1.parag()->toString( c1.index() ) + "\n";

        c1.parag()->saveOasis( writer, context, c1.index(), c1.parag()->length()-2, true );
        KoTextParag *p = c1.parag()->next();
        while ( p && p != c2.parag() ) {
            text += p->toString() + "\n";
            p->saveOasis( writer, context, 0, p->length()-2, true );
            p = p->next();
        }
        text += c2.parag()->toString( 0, c2.index() );
        c2.parag()->saveOasis( writer, context, 0, c2.index() - 1, true );
    }
    return text;
}

void KoTextDocument::setFormat( int id, const KoTextFormat *f, int flags )
{
    QMap<int, KoTextDocumentSelection>::ConstIterator it = selections.find( id );
    if ( it == selections.end() )
	return;

    KoTextDocumentSelection sel = *it;

    KoTextCursor c1 = sel.startCursor;
    KoTextCursor c2 = sel.endCursor;
    if ( sel.swapped ) {
	c2 = sel.startCursor;
	c1 = sel.endCursor;
    }

    if ( c1.parag() == c2.parag() ) {
	c1.parag()->setFormat( c1.index(), c2.index() - c1.index(), f, TRUE, flags );
	return;
    }

    c1.parag()->setFormat( c1.index(), c1.parag()->length() - c1.index(), f, TRUE, flags );
    KoTextParag *p = c1.parag()->next();
    while ( p && p != c2.parag() ) {
	p->setFormat( 0, p->length(), f, TRUE, flags );
	p = p->next();
    }
    c2.parag()->setFormat( 0, c2.index(), f, TRUE, flags );
}

/*void KoTextDocument::copySelectedText( int id )
{
#ifndef QT_NO_CLIPBOARD
    if ( !hasSelection( id ) )
	return;

    QApplication::clipboard()->setText( selectedText( id ) );
#endif
}*/

void KoTextDocument::removeSelectedText( int id, KoTextCursor *cursor )
{
    QMap<int, KoTextDocumentSelection>::Iterator it = selections.find( id );
    if ( it == selections.end() )
	return;

    KoTextDocumentSelection sel = *it;

    KoTextCursor c1 = sel.startCursor;
    KoTextCursor c2 = sel.endCursor;
    if ( sel.swapped ) {
	c2 = sel.startCursor;
	c1 = sel.endCursor;
    }

    *cursor = c1;
    removeSelection( id );

    if ( c1.parag() == c2.parag() ) {
	c1.parag()->remove( c1.index(), c2.index() - c1.index() );
	return;
    }

    // ## Qt has a strange setValid/isValid on QTextCursor, only used in the few lines below !?!?
    bool valid = true;
    if ( c1.parag() == fParag && c1.index() == 0 &&
         c2.parag() == lParag && c2.index() == lParag->length() - 1 )
        valid = FALSE;

    bool didGoLeft = FALSE;
    if (  c1.index() == 0 && c1.parag() != fParag ) {
	cursor->gotoPreviousLetter();
        if ( valid )
            didGoLeft = TRUE;
    }

    c1.parag()->remove( c1.index(), c1.parag()->length() - 1 - c1.index() );
    KoTextParag *p = c1.parag()->next();
    int dy = 0;
    KoTextParag *tmp;
    while ( p && p != c2.parag() ) {
	tmp = p->next();
	dy -= p->rect().height();
        //emit paragraphDeleted( p ); // done by KoTextParag dtor already
	delete p;
	p = tmp;
    }
    c2.parag()->remove( 0, c2.index() );
    while ( p ) {
	p->move( dy );
        //// kotext
        if ( p->paragLayout().counter )
            p->paragLayout().counter->invalidate();
        ////
	p->invalidate( 0 );
	//p->setEndState( -1 );
	p = p->next();
    }

    c1.parag()->join( c2.parag() );

    if ( didGoLeft )
	cursor->gotoNextLetter();
}

void KoTextDocument::addCommand( KoTextDocCommand *cmd )
{
    commandHistory->addCommand( cmd );
}

KoTextCursor *KoTextDocument::undo( KoTextCursor *c )
{
    return commandHistory->undo( c );
}

KoTextCursor *KoTextDocument::redo( KoTextCursor *c )
{
    return commandHistory->redo( c );
}

bool KoTextDocument::find( const QString &expr, bool cs, bool wo, bool forward,
			  int *parag, int *index, KoTextCursor *cursor )
{
    KoTextParag *p = forward ? fParag : lParag;
    if ( parag )
	p = paragAt( *parag );
    else if ( cursor )
	p = cursor->parag();
    bool first = TRUE;

    while ( p ) {
	QString s = p->string()->toString();
	s.remove( s.length() - 1, 1 ); // get rid of trailing space
	int start = forward ? 0 : s.length() - 1;
	if ( first && index )
	    start = *index;
	else if ( first )
	    start = cursor->index();
	if ( !forward && first ) {
	    start -= expr.length() + 1;
	    if ( start < 0 ) {
		first = FALSE;
		p = p->prev();
		continue;
	    }
	}
	first = FALSE;

	for ( ;; ) {
	    int res = forward ? s.find( expr, start, cs ) : s.findRev( expr, start, cs );
	    if ( res == -1 )
		break;

	    bool ok = TRUE;
	    if ( wo ) {
		int end = res + expr.length();
		if ( ( res == 0 || s[ res - 1 ].isSpace() || s[ res - 1 ].isPunct() ) &&
		     ( end == (int)s.length() || s[ end ].isSpace() || s[ end ].isPunct() ) )
		    ok = TRUE;
		else
		    ok = FALSE;
	    }
	    if ( ok ) {
		cursor->setParag( p );
		cursor->setIndex( res );
		setSelectionStart( Standard, cursor );
		cursor->setIndex( res + expr.length() );
		setSelectionEnd( Standard, cursor );
		if ( parag )
		    *parag = p->paragId();
		if ( index )
		    *index = res;
		return TRUE;
	    }
	    if ( forward ) {
		start = res + 1;
	    } else {
		if ( res == 0 )
		    break;
		start = res - 1;
	    }
	}
	p = forward ? p->next() : p->prev();
    }

    return FALSE;
}

bool KoTextDocument::inSelection( int selId, const QPoint &pos ) const
{
    QMap<int, KoTextDocumentSelection>::ConstIterator it = selections.find( selId );
    if ( it == selections.end() )
	return FALSE;

    KoTextDocumentSelection sel = *it;
    KoTextParag *startParag = sel.startCursor.parag();
    KoTextParag *endParag = sel.endCursor.parag();
    if ( sel.startCursor.parag() == sel.endCursor.parag() &&
	 sel.startCursor.parag()->selectionStart( selId ) == sel.endCursor.parag()->selectionEnd( selId ) )
	return FALSE;
    if ( sel.endCursor.parag()->paragId() < sel.startCursor.parag()->paragId() ) {
	endParag = sel.startCursor.parag();
	startParag = sel.endCursor.parag();
    }

    KoTextParag *p = startParag;
    while ( p ) {
	if ( p->rect().contains( pos ) ) {
	    bool inSel = FALSE;
	    int selStart = p->selectionStart( selId );
	    int selEnd = p->selectionEnd( selId );
	    int y = 0;
	    int h = 0;
	    for ( int i = 0; i < p->length(); ++i ) {
		if ( i == selStart )
		    inSel = TRUE;
		if ( i == selEnd )
		    break;
		if ( p->at( i )->lineStart ) {
		    y = (*p->lineStarts.find( i ))->y;
		    h = (*p->lineStarts.find( i ))->h;
		}
		if ( pos.y() - p->rect().y() >= y && pos.y() - p->rect().y() <= y + h ) {
		    if ( inSel && pos.x() >= p->at( i )->x &&
			 pos.x() <= p->at( i )->x + p->at( i )->width /*p->at( i )->format()->width( p->at( i )->c )*/ )
			return TRUE;
		}
	    }
	}
	if ( pos.y() < p->rect().y() )
	    break;
	if ( p == endParag )
	    break;
	p = p->next();
    }

    return FALSE;
}

QPixmap *KoTextDocument::bufferPixmap( const QSize &s )
{
    if ( !buf_pixmap ) {
	int w = QABS( s.width() );
	int h = QABS( s.height() );
	buf_pixmap = new QPixmap( w, h );
    } else {
	if ( buf_pixmap->width() < s.width() ||
	     buf_pixmap->height() < s.height() ) {
	    buf_pixmap->resize( qMax( s.width(), buf_pixmap->width() ),
				qMax( s.height(), buf_pixmap->height() ) );
	}
    }

    return buf_pixmap;
}

void KoTextDocument::registerCustomItem( KoTextCustomItem *i, KoTextParag *p )
{
    if ( i && i->placement() != KoTextCustomItem::PlaceInline )
	flow_->registerFloatingItem( i );
    p->registerFloatingItem( i );
    i->setParagraph( p );
    //kDebug(32500) << "KoTextDocument::registerCustomItem " << (void*)i << endl;
    customItems.append( i );
}

void KoTextDocument::unregisterCustomItem( KoTextCustomItem *i, KoTextParag *p )
{
    flow_->unregisterFloatingItem( i );
    p->unregisterFloatingItem( i );
    i->setParagraph( 0 );
    customItems.removeRef( i );
}

int KoTextDocument::length() const
{
    int l = 0;
    KoTextParag *p = fParag;
    while ( p ) {
	l += p->length() - 1; // don't count trailing space
	p = p->next();
    }
    return l;
}

bool KoTextDocument::visitSelection( int selectionId, KoParagVisitor* visitor, bool forward )
{
    KoTextCursor c1 = selectionStartCursor( selectionId );
    KoTextCursor c2 = selectionEndCursor( selectionId );
    if ( c1 == c2 )
        return true;
    return visitFromTo( c1.parag(), c1.index(), c2.parag(), c2.index(), visitor, forward );
}

bool KoTextDocument::hasSelection( int id, bool visible ) const
{
    return ( selections.find( id ) != selections.end() &&
             ( !visible ||
               ( (KoTextDocument*)this )->selectionStartCursor( id ) !=
               ( (KoTextDocument*)this )->selectionEndCursor( id ) ) );
}

void KoTextDocument::setSelectionStart( int id, KoTextCursor *cursor )
{
    KoTextDocumentSelection sel;
    sel.startCursor = *cursor;
    sel.endCursor = *cursor;
    sel.swapped = FALSE;
    selections[ id ] = sel;
}

KoTextParag *KoTextDocument::paragAt( int i ) const
{
    KoTextParag *s = fParag;
    while ( s ) {
	if ( s->paragId() == i )
	    return s;
	s = s->next();
    }
    return 0;
}

bool KoTextDocument::visitDocument( KoParagVisitor *visitor, bool forward )
{
    return visitFromTo( firstParag(), 0, lastParag(), lastParag()->length()-1, visitor, forward );
}

bool KoTextDocument::visitFromTo( KoTextParag *firstParag, int firstIndex, KoTextParag* lastParag, int lastIndex, KoParagVisitor* visitor, bool forw )
{
    if ( firstParag == lastParag )
    {
        return visitor->visit( firstParag, firstIndex, lastIndex );
    }
    else
    {
        bool ret = true;
        if ( forw )
        {
            // the -1 is for the trailing space
            ret = visitor->visit( firstParag, firstIndex, firstParag->length() - 1 );
            if (!ret) return false;
        }
        else
        {
            ret = visitor->visit( lastParag, 0, lastIndex );
            if (!ret) return false;
        }

        KoTextParag* currentParag = forw ? firstParag->next() : lastParag->prev();
        KoTextParag * endParag = forw ? lastParag : firstParag;
        while ( currentParag && currentParag != endParag )
        {
            ret = visitor->visit( currentParag, 0, currentParag->length() - 1 );
            if (!ret) return false;
            currentParag = forw ? currentParag->next() : currentParag->prev();
        }
        Q_ASSERT( currentParag );
        Q_ASSERT( endParag == currentParag );
        if ( forw )
            ret = visitor->visit( lastParag, 0, lastIndex );
        else
            ret = visitor->visit( currentParag, firstIndex, currentParag->length() - 1 );
        return ret;
    }
}

static bool is_printer( QPainter *p )
{
    return p && p->device() && p->device()->devType() == QInternal::Printer;
}

KoTextParag *KoTextDocument::drawWYSIWYG( QPainter *p, int cx, int cy, int cw, int ch, const QColorGroup &cg,
                                          KoTextZoomHandler* zoomHandler, bool onlyChanged,
                                          bool drawCursor, KoTextCursor *cursor,
                                          bool resetChanged, uint drawingFlags )
{
    m_drawingFlags = drawingFlags;
    // We need to draw without double-buffering if
    // 1) printing (to send text and not bitmaps to the printer)
    // 2) drawing a transparent embedded document
    //
    if ( is_printer( p ) || ( drawingFlags & TransparentBackground ) ) {
    // This stuff relies on doLayout()... simpler to just test for Printer.
    // If someone understand doLayout() please tell me (David)
    /*if ( isWithoutDoubleBuffer() || par && par->withoutDoubleBuffer ) { */
	//setWithoutDoubleBuffer( TRUE );
	QRect crect( cx, cy, cw, ch );
	drawWithoutDoubleBuffer( p, crect, cg, zoomHandler );
	return 0;
    }
    //setWithoutDoubleBuffer( FALSE );

    if ( !firstParag() )
        return 0;

    KoTextParag *lastFormatted = 0;
    KoTextParag *parag = firstParag();

    QPixmap *doubleBuffer = 0;
    QPainter painter;
    // All the coordinates in this method are in view pixels
    QRect crect( cx, cy, cw, ch );
    Q_ASSERT( ch > 0 );
#ifdef DEBUG_PAINTING
    kDebug(32500) << "\nKoTextDocument::drawWYSIWYG crect=" << crect << endl;
#endif

    // Space above first parag
    QRect pixelRect = parag->pixelRect( zoomHandler );
    if ( isPageBreakEnabled() && parag && cy <= pixelRect.y() && pixelRect.y() > 0 ) {
        QRect r( 0, 0,
                 zoomHandler->layoutUnitToPixelX( parag->document()->x() + parag->document()->width() ),
                 pixelRect.y() );
        r &= crect;
        if ( !r.isEmpty() ) {
#ifdef DEBUG_PAINTING
            kDebug(32500) << " drawWYSIWYG: space above first parag: " << r << " (pixels)" << endl;
#endif
            p->fillRect( r, cg.brush( QColorGroup::Base ) );
        }
    }

    while ( parag ) {
	lastFormatted = parag;
	if ( !parag->isValid() )
	    parag->format();

	QRect ir = parag->pixelRect( zoomHandler );
#ifdef DEBUG_PAINTING
        kDebug(32500) << " drawWYSIWYG: ir=" << ir << endl;
#endif
	if ( isPageBreakEnabled() && parag->next() && ( drawingFlags & TransparentBackground ) == 0 )
        {
            int nexty = parag->next()->pixelRect(zoomHandler).y();
            // Test ir.y+ir.height, which is the first pixel _under_ the parag
            // (as opposed ir.bottom() which is the last pixel of the parag)
	    if ( ir.y() + ir.height() < nexty ) {
		QRect r( 0, ir.y() + ir.height(),
			 zoomHandler->layoutUnitToPixelX( parag->document()->x() + parag->document()->width() ),
			 nexty - ( ir.y() + ir.height() ) );
		r &= crect;
		if ( !r.isEmpty() )
                {
#ifdef DEBUG_PAINTING
                    kDebug(32500) << " drawWYSIWYG: space between parag " << parag->paragId() << " and " << parag->next()->paragId() << " : " << r << " (pixels)" << endl;
#endif
		    p->fillRect( r, cg.brush( QColorGroup::Base ) );
                }
	    }
        }

        if ( !ir.intersects( crect ) ) {
            // Paragraph is not in the crect - but let's check if the area on its right is.
	    ir.setWidth( zoomHandler->layoutUnitToPixelX( parag->document()->width() ) );
	    if ( ir.intersects( crect ) && ( drawingFlags & TransparentBackground ) == 0 )
		p->fillRect( ir.intersect( crect ), cg.brush( QColorGroup::Base ) );
	    if ( ir.y() > cy + ch ) {
                goto floating;
	    }
	}
        else if ( parag->hasChanged() || !onlyChanged ) {
            // lineChanged() only makes sense if we're drawing with onlyChanged=true
            // otherwise, call setChanged() to make sure we'll paint it all (lineChanged=-1).
            // (this avoids having to send onlyChanged to drawParagWYSIWYG)
            if ( !onlyChanged && parag->lineChanged() > 0 )
                parag->setChanged( false );
            drawParagWYSIWYG( p, parag, cx, cy, cw, ch, doubleBuffer, cg,
                              zoomHandler, drawCursor, cursor, resetChanged, drawingFlags );
        }
	parag = parag->next();
    }

    parag = lastParag();

floating:
    pixelRect = parag->pixelRect(zoomHandler);
    int docheight = zoomHandler->layoutUnitToPixelY( parag->document()->height() );
    if ( pixelRect.y() + pixelRect.height() < docheight ) {
        int docwidth = zoomHandler->layoutUnitToPixelX( parag->document()->width() );
        if ( ( drawingFlags & TransparentBackground ) == 0 ) {
            p->fillRect( 0, pixelRect.y() + pixelRect.height(),
                         docwidth, docheight - ( pixelRect.y() + pixelRect.height() ),
                         cg.brush( QColorGroup::Base ) );
        }
	if ( !flow()->isEmpty() ) {
	    QRect cr( cx, cy, cw, ch );
	    cr = cr.intersect( QRect( 0, pixelRect.y() + pixelRect.height(), docwidth,
				      docheight - ( pixelRect.y() + pixelRect.height() ) ) );
	    flow()->drawFloatingItems( p, cr.x(), cr.y(), cr.width(), cr.height(), cg, FALSE );
	}
    }

    if ( buf_pixmap && buf_pixmap->height() > 300 ) {
	delete buf_pixmap;
	buf_pixmap = 0;
    }

    return lastFormatted;
}


// Used for printing
void KoTextDocument::drawWithoutDoubleBuffer( QPainter *p, const QRect &cr, const QColorGroup &cg,
                                              KoTextZoomHandler* zoomHandler, const QBrush *paper )
{
    if ( !firstParag() )
        return;

    Q_ASSERT( (m_drawingFlags & DrawSelections) == 0 );
    if (m_drawingFlags & DrawSelections)
           kDebug() << kBacktrace();
    if ( paper && ( m_drawingFlags & TransparentBackground ) == 0 ) {
        p->setBrushOrigin(  -(int)p->translationX(),  -(int)p->translationY() );
        p->fillRect( cr, *paper );
    }

    KoTextParag *parag = firstParag();
    while ( parag ) {
        if ( !parag->isValid() )
            parag->format();

        QRect pr( parag->pixelRect( zoomHandler ) );
        pr.setLeft( 0 );
        pr.setWidth( QWIDGETSIZE_MAX );
        // The cliprect is checked in layout units, in KoTextParag::paint
        QRect crect_lu( parag->rect() );

        if ( !cr.isNull() && !cr.intersects( pr ) ) {
            parag = parag->next();
            continue;
        }

        p->translate( 0, pr.y() );

        // No need to brush plain white on a printer. Brush all
        // other cases (except "full transparent" case).
        QBrush brush = cg.brush( QColorGroup::Base );;
        bool needBrush = brush.style() != Qt::NoBrush &&
                         !(brush.style() == Qt::SolidPattern &&
                           brush.color() == Qt::white &&
                           is_printer(p));
        if ( needBrush && ( m_drawingFlags & TransparentBackground ) == 0 )
            p->fillRect( QRect( 0, 0, pr.width(), pr.height() ), brush );

        //p->setBrushOrigin( p->brushOrigin() + QPoint( 0, pr.y() ) );
        parag->paint( *p, cg, 0, FALSE,
                      crect_lu.x(), crect_lu.y(),
                      crect_lu.width(), crect_lu.height() );
        p->translate( 0, -pr.y() );
        //p->setBrushOrigin( p->brushOrigin() - QPoint( 0, pr.y() ) );
        parag = parag->next();
    }
}

// Used for screen display (and also printing?)
// Called by drawWYSIWYG and the app's drawCursor
void KoTextDocument::drawParagWYSIWYG( QPainter *p, KoTextParag *parag, int cx, int cy, int cw, int ch,
                                       QPixmap *&doubleBuffer, const QColorGroup &cg,
                                       KoTextZoomHandler* zoomHandler, bool drawCursor,
                                       KoTextCursor *cursor, bool resetChanged, uint drawingFlags )
{
    if ( cw <= 0 || ch <= 0 ) { Q_ASSERT( cw > 0 ); Q_ASSERT( ch > 0 ); return; }
#ifdef DEBUG_PAINTING
    kDebug(32500) << "KoTextDocument::drawParagWYSIWYG " << (void*)parag << " id:" << parag->paragId() << endl;
#endif
    m_drawingFlags = drawingFlags;
    QPainter *painter = 0;
    // Those three rects are in pixels, in the document coordinates (0,0 == topleft of first parag)
    QRect rect = parag->pixelRect( zoomHandler ); // the parag rect

    int offsetY = 0;
    // Start painting from a given line number.
    if ( parag->lineChanged() > -1 )
    {
        offsetY = zoomHandler->layoutUnitToPixelY( parag->lineY( parag->lineChanged() ) - parag->topMargin() );
#ifdef DEBUG_PAINTING
        kDebug(32500) << " Repainting from lineChanged=" << parag->lineChanged() << " -> adding " << offsetY << " to rect" << endl;
#endif
        // Skip the lines that are not repainted by moving Top. The bottom doesn't change.
        rect.rTop() += offsetY;
    }

    QRect crect( cx, cy, cw, ch ); // the overall crect
    QRect ir( rect ); // will be the rect to be repainted

    QBrush brush = cg.brush( QColorGroup::Base );

    // No need to brush plain white on a printer. Brush all
    // other cases (except "full transparent" case).
    bool needBrush = brush.style() != Qt::NoBrush &&
                     ( drawingFlags & TransparentBackground ) == 0 &&
                     !(brush.style() == Qt::SolidPattern &&
                       brush.color() == Qt::white &&
                       is_printer(p));

    bool useDoubleBuffer = !parag->document()->parent();
    if ( is_printer(p) )
        useDoubleBuffer = FALSE;
    // Can't handle transparency using double-buffering, in case of rotation/scaling (due to bitBlt)
    // The test on mat is almost like isIdentity(), but allows for translation.
    //// ##### The way to fix this: initialize the pixmap to be fully transparent instead
    // of being white.
    QMatrix mat = p->worldMatrix();
    if ( ( mat.m11() != 1.0 || mat.m22() != 1.0 || mat.m12() != 0.0 || mat.m21() != 0.0 )
         && brush.style() != Qt::SolidPattern )
        useDoubleBuffer = FALSE;

#ifdef DEBUG_PAINTING
    kDebug(32500) << "KoTextDocument::drawParagWYSIWYG parag->rect=" << parag->rect()
                   << " pixelRect(ir)=" << ir
                   << " crect (pixels)=" << crect
                   << " useDoubleBuffer=" << useDoubleBuffer << endl;
#endif

    if ( useDoubleBuffer  ) {
	painter = new QPainter;
	if ( cx >= 0 && cy >= 0 )
	    ir = ir.intersect( crect );
	if ( !doubleBuffer ||
	     ir.width() > doubleBuffer->width() ||
	     ir.height() > doubleBuffer->height() )
        {
	    doubleBuffer = bufferPixmap( ir.size() );
        }
        painter->begin( doubleBuffer );

    } else {
        p->save();
	painter = p;
	painter->translate( ir.x(), ir.y() );
    }
    // Until the next translate(), (0,0) in the painter will be at ir.topLeft() in reality
    //kDebug() << "KoTextDocument::drawParagWYSIWYG ir=" << ir << endl;


    // Cumulate ir.x(), ir.y() with the current brush origin
    //painter->setBrushOrigin( painter->brushOrigin() + ir.topLeft() );

    if ( useDoubleBuffer || is_printer( painter ) ) {
        // Transparent -> grab background from p's device
        if ( brush.style() != Qt::SolidPattern ) {
            bitBlt( doubleBuffer, 0, 0, p->device(),
                    ir.x() + (int)p->translationX(), ir.y() + (int)p->translationY(),
                    ir.width(), ir.height() );
        }
    }

    if ( needBrush )
        painter->fillRect( QRect( 0, 0, ir.width(), ir.height() ), brush );

    // Now revert the previous painter translation, and instead make (0,0) the topleft of the PARAGRAPH
    painter->translate( rect.x() - ir.x(), rect.y() - ir.y() );
#ifdef DEBUG_PAINTING
    kDebug(32500) << "KoTextDocument::drawParagWYSIWYG translate " << rect.x() - ir.x() << "," << rect.y() - ir.y() << endl;
#endif
    //painter->setBrushOrigin( painter->brushOrigin() + rect.topLeft() - ir.topLeft() );

    // The cliprect is checked in layout units, in KoTextParag::paint
    QRect crect_lu( zoomHandler->pixelToLayoutUnit( crect ) );
#ifdef DEBUG_PAINTING
    kDebug(32500) << "KoTextDocument::drawParagWYSIWYG crect_lu=" << crect_lu << endl;
#endif

    // paintDefault will paint line 'lineChanged' at its normal Y position.
    // But the buffer-pixmap below starts at Y. We need to translate by -Y
    // so that the painting happens at the right place.
    painter->translate( 0, -offsetY );

    parag->paint( *painter, cg, drawCursor ? cursor : 0, (m_drawingFlags & DrawSelections),
                  crect_lu.x(), crect_lu.y(), crect_lu.width(), crect_lu.height() );


    if ( useDoubleBuffer ) {
	delete painter;
	painter = 0;
	p->drawPixmap( ir.topLeft(), *doubleBuffer, QRect( QPoint( 0, 0 ), ir.size() ) );
#if 0 // for debug!
        p->save();
        p->setPen( Qt::blue );
        p->drawRect( ir.x(), ir.y(), ir.width(), ir.height() );
        p->restore();
#endif
    } else {
        // undo previous translations, painter is 'p', i.e. will be used later on
        p->restore();
	//painter->translate( -ir.x(), -ir.y() );
        //painter->translate( 0, +offsetY );
        //painter->setBrushOrigin( painter->brushOrigin() - ir.topLeft() );
    }

    if ( needBrush ) {
        int docright = zoomHandler->layoutUnitToPixelX( parag->document()->x() + parag->document()->width() );
#ifdef DEBUG_PAINTING
//        kDebug(32500) << "KoTextDocument::drawParagWYSIWYG my rect is: " << rect << endl;
#endif
        if ( rect.x() + rect.width() < docright ) {
#ifdef DEBUG_PAINTING
            kDebug(32500) << "KoTextDocument::drawParagWYSIWYG rect doesn't go up to docright=" << docright << endl;
#endif
            p->fillRect( rect.x() + rect.width(), rect.y(),
                         docright - ( rect.x() + rect.width() ),
                         rect.height(), cg.brush( QColorGroup::Base ) );
        }
    }

    if ( resetChanged )
	parag->setChanged( FALSE );
}


KoTextDocCommand *KoTextDocument::deleteTextCommand( KoTextDocument *textdoc, int id, int index, const Q3MemArray<KoTextStringChar> & str, const CustomItemsMap & customItemsMap, const Q3ValueList<KoParagLayout> & oldParagLayouts )
{
    return new KoTextDeleteCommand( textdoc, id, index, str, customItemsMap, oldParagLayouts );
}

KoTextParag* KoTextDocument::loadOasisText( const QDomElement& bodyElem, KoOasisContext& context, KoTextParag* lastParagraph, KoStyleCollection* styleColl, KoTextParag* nextParagraph )
{
    // was OoWriterImport::parseBodyOrSimilar
    QDomElement tag;
    forEachElement( tag, bodyElem )
    {
        context.styleStack().save();
        const QString localName = tag.localName();
        const bool isTextNS = tag.namespaceURI() == KoXmlNS::text;
        uint pos = 0;
        if ( isTextNS && localName == "p" ) {  // text paragraph
            context.fillStyleStack( tag, KoXmlNS::text, "style-name", "paragraph" );

            KoTextParag *parag = createParag( this, lastParagraph, nextParagraph );
            parag->loadOasis( tag, context, styleColl, pos );
            if ( !lastParagraph )        // First parag
                setFirstParag( parag );
            lastParagraph = parag;
        }
        else if ( isTextNS && localName == "h" ) // heading
        {
            //kDebug(32500) << " heading " << endl;
            context.fillStyleStack( tag, KoXmlNS::text, "style-name", "paragraph" );
            int level = tag.attributeNS( KoXmlNS::text, "outline-level", QString::null ).toInt();
            bool listOK = false;
            // When a heading is inside a list, it seems that the list prevails.
            // Example:
            //    <text:list text:style-name="Numbering 1">
            //      <text:list-item text:start-value="5">
            //        <text:h text:style-name="P2" text:level="4">The header</text:h>
            // where P2 has list-style-name="something else"
            // Result: the numbering of the header follows "Numbering 1".
            // So we use the style for the outline level only if we're not inside a list:
            //if ( !context.atStartOfListItem() )
            // === The new method for this is that we simply override it after loading.
            listOK = context.pushOutlineListLevelStyle( level );
            int restartNumbering = -1;
            if ( tag.hasAttributeNS( KoXmlNS::text, "start-value" ) )
                // OASIS extension http://lists.oasis-open.org/archives/office/200310/msg00033.html
                restartNumbering = tag.attributeNS( KoXmlNS::text, "start-value", QString::null ).toInt();

            KoTextParag *parag = createParag( this, lastParagraph, nextParagraph );
            parag->loadOasis( tag, context, styleColl, pos );
            if ( !lastParagraph )        // First parag
                setFirstParag( parag );
            lastParagraph = parag;
            if ( listOK ) {
                parag->applyListStyle( context, restartNumbering, true /*ordered*/, true /*heading*/, level );
                context.listStyleStack().pop();
            }
        }
        else if ( isTextNS &&
                  ( localName == "unordered-list" || localName == "ordered-list" // OOo-1.1
                    || localName == "list" || localName == "numbered-paragraph" ) )  // OASIS
        {
            lastParagraph = loadList( tag, context, lastParagraph, styleColl, nextParagraph );
        }
        else if ( isTextNS && localName == "section" ) // Temporary support (###TODO)
        {
            kDebug(32500) << "Section found!" << endl;
            context.fillStyleStack( tag, KoXmlNS::text, "style-name", "section" );
            lastParagraph = loadOasisText( tag, context, lastParagraph, styleColl, nextParagraph );
        }
        else if ( isTextNS && localName == "variable-decls" )
        {
            // We don't parse variable-decls since we ignore var types right now
            // (and just storing a list of available var names wouldn't be much use)
        }
        else if ( isTextNS && localName == "user-field-decls" )
        {
            QDomElement fd;
            forEachElement( fd, tag )
            {
                if ( fd.namespaceURI() == KoXmlNS::text && fd.localName() == "user-field-decl" )
                {
                    const QString name = fd.attributeNS( KoXmlNS::text, "name", QString::null );
                    const QString value = fd.attributeNS( KoXmlNS::office, "value", QString::null );
                    if ( !name.isEmpty() )
                        context.variableCollection().setVariableValue( name, value );
                }
            }
        }
        else if ( isTextNS && localName == "number" ) // text:number
        {
            // This is the number in front of a numbered paragraph,
            // written out to help export filters. We can ignore it.
        }
        else if ( !loadOasisBodyTag( tag, context, lastParagraph, styleColl, nextParagraph ) )
        {
            kWarning(32500) << "Unsupported body element '" << localName << "'" << endl;
        }

        context.styleStack().restore(); // remove the styles added by the paragraph or list
        //use signal slot ?
        //m_doc->progressItemLoaded(); // ## check
    }
    return lastParagraph;
}

KoTextParag* KoTextDocument::loadList( const QDomElement& list, KoOasisContext& context, KoTextParag* lastParagraph, KoStyleCollection * styleColl, KoTextParag* nextParagraph )
{
    //kDebug(32500) << "loadList: " << list.attributeNS( KoXmlNS::text, "style-name", QString::null ) << endl;

    const QString oldListStyleName = context.currentListStyleName();
    if ( list.hasAttributeNS( KoXmlNS::text, "style-name" ) )
        context.setCurrentListStyleName( list.attributeNS( KoXmlNS::text, "style-name", QString::null ) );
    bool listOK = !context.currentListStyleName().isEmpty();
    int level;
    if ( list.localName() == "numbered-paragraph" )
        level = list.attributeNS( KoXmlNS::text, "level", "1" ).toInt();
    else
        level = context.listStyleStack().level() + 1;
    if ( listOK )
        listOK = context.pushListLevelStyle( context.currentListStyleName(), level );

    const QDomElement listStyle = context.listStyleStack().currentListStyle();
    // The tag is either list-level-style-number or list-level-style-bullet
    const bool orderedList = listStyle.localName() == "list-level-style-number";

    if ( list.localName() == "numbered-paragraph" )
    {
        // A numbered-paragraph contains paragraphs directly (it's both a list and a list-item)
        int restartNumbering = -1;
        if ( list.hasAttributeNS( KoXmlNS::text, "start-value" ) )
            restartNumbering = list.attributeNS( KoXmlNS::text, "start-value", QString::null ).toInt();
        KoTextParag* oldLast = lastParagraph;
        lastParagraph = loadOasisText( list, context, lastParagraph, styleColl, nextParagraph );
        KoTextParag* firstListItem = oldLast ? oldLast->next() : firstParag();
        // Apply list style to first paragraph inside numbered-parag - there's only one anyway
        // Keep the "is outline" property though
        bool isOutline = firstListItem->counter() && firstListItem->counter()->numbering() == KoParagCounter::NUM_CHAPTER;
        firstListItem->applyListStyle( context, restartNumbering, orderedList,
                                       isOutline, level );
    }
    else
    {
        // Iterate over list items
        for ( QDomNode n = list.firstChild(); !n.isNull(); n = n.nextSibling() )
        {
            QDomElement listItem = n.toElement();
            int restartNumbering = -1;
            if ( listItem.hasAttributeNS( KoXmlNS::text, "start-value" ) )
                restartNumbering = listItem.attributeNS( KoXmlNS::text, "start-value", QString::null ).toInt();
            bool isListHeader = listItem.localName() == "list-header" || listItem.attributeNS( KoXmlNS::text, "is-list-header", QString::null ) == "is-list-header";
            KoTextParag* oldLast = lastParagraph;
            lastParagraph = loadOasisText( listItem, context, lastParagraph, styleColl, nextParagraph );
            KoTextParag* firstListItem = oldLast ? oldLast->next() : firstParag();
            KoTextParag* p = firstListItem;
            // It's either list-header (normal text on top of list) or list-item
            if ( !isListHeader && firstListItem ) {
                // Apply list style to first paragraph inside list-item
                bool isOutline = firstListItem->counter() && firstListItem->counter()->numbering() == KoParagCounter::NUM_CHAPTER;
                firstListItem->applyListStyle( context, restartNumbering, orderedList, isOutline, level );
                p = p->next();
            }
            // Make text:h inside list-item (as non first child) unnumbered.
            while ( p && p != lastParagraph->next() ) {
                if ( p->counter() )
                    p->counter()->setNumbering( KoParagCounter::NUM_NONE );
                p = p->next();
            }
        }
    }
    if ( listOK )
        context.listStyleStack().pop();
    context.setCurrentListStyleName( oldListStyleName );
    return lastParagraph;
}

void KoTextDocument::saveOasisContent( KoXmlWriter& writer, KoSavingContext& context ) const
{
    // Basically just call saveOasis on every paragraph.
    // KWord doesn't use this method because it does table-of-contents-handling in addition.
    KoTextParag* parag = firstParag();
    while ( parag ) {
        // Save the whole parag, without the trailing space.
        parag->saveOasis( writer, context, 0, parag->lastCharPos() );
        parag = parag->next();
    }
}

#include "KoTextDocument.moc"
