/* This file is part of the KDE project
   Copyright (C) 2001-2006 David Faure <faure@kde.org>
   Copyright (C) 2005-2006 Martin Ellis <martin.ellis@kdemail.net>

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

#include "KoTextParag.h"
#include "KoTextDocument.h"
#include "KoParagCounter.h"
#include "KoTextZoomHandler.h"
#include "KoStyleCollection.h"
#include "KoVariable.h"
#include <KoOasisContext.h>
#include <KoXmlWriter.h>
#include <KoGenStyles.h>
#include <KoDom.h>
#include <KoXmlNS.h>
#include <KoUnit.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <assert.h>
//Added by qt3to4:
#include <Q3MemArray>
#include <Q3PtrList>

//#define DEBUG_PAINT

KoTextParag::KoTextParag( KoTextDocument *d, KoTextParag *pr, KoTextParag *nx, bool updateIds )
    : p( pr ), n( nx ), doc( d ),
      m_invalid( true ),
      changed( FALSE ),
      fullWidth( TRUE ),
      newLinesAllowed( TRUE ), // default in kotext
      visible( TRUE ), //breakable( TRUE ),
      movedDown( FALSE ),
      m_toc( false ),
      align( 0 ),
      m_lineChanged( -1 ),
      m_wused( 0 ),
      mSelections( 0 ),
      mFloatingItems( 0 ),
      tArray( 0 )
{
    defFormat = formatCollection()->defaultFormat();
    /*if ( !doc ) {
	tabStopWidth = defFormat->width( 'x' ) * 8;
	commandHistory = new KoTextDocCommandHistory( 100 );
    }*/

    if ( p ) {
	p->n = this;
    }
    if ( n ) {
	n->p = this;
    }

    if ( !p && doc )
	doc->setFirstParag( this );
    if ( !n && doc )
	doc->setLastParag( this );

    //firstFormat = TRUE; //// unused
    //firstPProcess = TRUE;
    //state = -1;
    //needPreProcess = FALSE;

    if ( p )
	id = p->id + 1;
    else
	id = 0;
    if ( n && updateIds ) {
	KoTextParag *s = n;
	while ( s ) {
	    s->id = s->p->id + 1;
	    //s->lm = s->rm = s->tm = s->bm = -1, s->flm = -1;
	    s = s->n;
	}
    }

    str = new KoTextString();
    str->insert( 0, " ", formatCollection()->defaultFormat() );
    setJoinBorder( true );
}

KoTextParag::~KoTextParag()
{
    //kDebug(32500) << "KoTextParag::~KoTextParag " << this << " id=" << paragId() << endl;

    // #107961: unregister custom items; KoTextString::clear() will delete them
    const int len = str->length();
    for ( int i = 0; i < len; ++i ) {
	KoTextStringChar *c = at( i );
	if ( doc && c->isCustom() ) {
	    doc->unregisterCustomItem( c->customItem(), this );
	    //removeCustomItem();
	}
    }

    delete str;
    str = 0;
//    if ( doc && p == doc->minwParag ) {
//	doc->minwParag = 0;
//	doc->minw = 0;
//    }
    if ( !doc ) {
	//delete pFormatter;
	//delete commandHistory;
    }
    delete [] tArray;
    //delete eData;
    QMap<int, KoTextParagLineStart*>::Iterator it = lineStarts.begin();
    for ( ; it != lineStarts.end(); ++it )
	delete *it;
    if ( mSelections ) delete mSelections;
    if ( mFloatingItems ) delete mFloatingItems;

    if (p)
       p->setNext(n);
    if (n)
       n->setPrev(p);

    //// kotext
    if ( doc && !doc->isDestroying() )
    {
        doc->informParagraphDeleted( this );
    }
    //kDebug(32500) << "KoTextParag::~KoTextParag " << this << " done" << endl;
    ////
}

void KoTextParag::setNext( KoTextParag *s )
{
    n = s;
    if ( !n && doc )
	doc->setLastParag( this );
}

void KoTextParag::setPrev( KoTextParag *s )
{
    p = s;
    if ( !p && doc )
	doc->setFirstParag( this );
}

void KoTextParag::invalidate( int /*chr, ignored*/ )
{
    m_invalid = true;
#if 0
    if ( invalid < 0 )
	invalid = chr;
    else
	invalid = qMin( invalid, chr );
#endif
}

void KoTextParag::setChanged( bool b, bool /*recursive*/ )
{
    changed = b;
    m_lineChanged = -1; // all
}

void KoTextParag::setLineChanged( short int line )
{
    if ( m_lineChanged == -1 ) {
        if ( !changed ) // only if the whole parag wasn't "changed" already
            m_lineChanged = line;
    }
    else
        m_lineChanged = qMin( m_lineChanged, line ); // also works if line=-1
    changed = true;
    //kDebug(32500) << "KoTextParag::setLineChanged line=" << line << " -> m_lineChanged=" << m_lineChanged << endl;
}

void KoTextParag::insert( int index, const QString &s )
{
    str->insert( index, s, formatCollection()->defaultFormat() );
    invalidate( index );
    //needPreProcess = TRUE;
}

void KoTextParag::truncate( int index )
{
    str->truncate( index );
    insert( length(), " " );
    //needPreProcess = TRUE;
}

void KoTextParag::remove( int index, int len )
{
    if ( index + len - str->length() > 0 )
	return;
    for ( int i = index; i < index + len; ++i ) {
	KoTextStringChar *c = at( i );
	if ( doc && c->isCustom() ) {
	    doc->unregisterCustomItem( c->customItem(), this );
	    //removeCustomItem();
	}
    }
    str->remove( index, len );
    invalidate( 0 );
    //needPreProcess = TRUE;
}

void KoTextParag::join( KoTextParag *s )
{
    //kDebug(32500) << "KoTextParag::join this=" << paragId() << " (length " << length() << ") with " << s->paragId() << " (length " << s->length() << ")" << endl;
    int oh = r.height() + s->r.height();
    n = s->n;
    if ( n )
	n->p = this;
    else if ( doc )
	doc->setLastParag( this );

    int start = str->length();
    if ( length() > 0 && at( length() - 1 )->c == ' ' ) {
	remove( length() - 1, 1 );
	--start;
    }
    append( s->str->toString(), TRUE );

    for ( int i = 0; i < s->length(); ++i ) {
	if ( !doc || doc->useFormatCollection() ) {
	    s->str->at( i ).format()->addRef();
	    str->setFormat( i + start, s->str->at( i ).format(), TRUE );
	}
	if ( s->str->at( i ).isCustom() ) {
	    KoTextCustomItem * item = s->str->at( i ).customItem();
	    str->at( i + start ).setCustomItem( item );
	    s->str->at( i ).loseCustomItem();
	    doc->unregisterCustomItem( item, s ); // ### missing in QRT
	    doc->registerCustomItem( item, this );
	}
    }
    Q_ASSERT(str->at(str->length()-1).c == ' ');

    /*if ( !extraData() && s->extraData() ) {
	setExtraData( s->extraData() );
	s->setExtraData( 0 );
    } else if ( extraData() && s->extraData() ) {
	extraData()->join( s->extraData() );
        }*/
    delete s;
    invalidate( 0 );
    //// kotext
    invalidateCounters();
    ////
    r.setHeight( oh );
    //needPreProcess = TRUE;
    if ( n ) {
	KoTextParag *s = n;
	while ( s ) {
	    s->id = s->p->id + 1;
	    //s->state = -1;
	    //s->needPreProcess = TRUE;
	    s->changed = TRUE;
	    s = s->n;
	}
    }
    format();
    //state = -1;
}

void KoTextParag::move( int &dy )
{
    //kDebug(32500) << "KoTextParag::move paragId=" << paragId() << " dy=" << dy << endl;
    if ( dy == 0 )
	return;
    changed = TRUE;
    r.moveBy( 0, dy );
    if ( mFloatingItems ) {
	for ( KoTextCustomItem *i = mFloatingItems->first(); i; i = mFloatingItems->next() ) {
		i->finalize();
	}
    }
    //if ( p )
    //    p->lastInFrame = TRUE; // Qt does this, but the loop at the end of format() calls move a lot!

    movedDown = FALSE;

    // do page breaks if required
    if ( doc && doc->isPageBreakEnabled() ) {
	int shift;
	if ( ( shift = doc->formatter()->formatVertically(  doc, this ) ) ) {
	    if ( p )
		p->setChanged( TRUE );
	    dy += shift;
	}
    }
}

void KoTextParag::format( int start, bool doMove )
{
    if ( !str || str->length() == 0 || !formatter() )
	return;

    if ( isValid() )
	return;

    //kDebug(32500) << "KoTextParag::format " << this << " id:" << paragId() << endl;

    r.moveTopLeft( QPoint( documentX(), p ? p->r.y() + p->r.height() : documentY() ) );
    //if ( p )
    //    p->lastInFrame = FALSE;

    movedDown = FALSE;
    bool formattedAgain = FALSE;

 formatAgain:
    r.setWidth( documentWidth() );

    // Not really useful....
    if ( doc && mFloatingItems ) {
	for ( KoTextCustomItem *i = mFloatingItems->first(); i; i = mFloatingItems->next() ) {
	    if ( i->placement() == KoTextCustomItem::PlaceRight )
		i->move( r.x() + r.width() - i->width, r.y() );
	    else
		i->move( i->x(), r.y() );
	}
    }
    QMap<int, KoTextParagLineStart*> oldLineStarts = lineStarts;
    lineStarts.clear();
    int y;
    bool formatterWorked = formatter()->format( doc, this, start, oldLineStarts, y, m_wused );

    // It can't happen that width < minimumWidth -- hopefully.
    //r.setWidth( qMax( r.width(), formatter()->minimumWidth() ) );
    //m_minw = formatter()->minimumWidth();

    QMap<int, KoTextParagLineStart*>::Iterator it = oldLineStarts.begin();

    for ( ; it != oldLineStarts.end(); ++it )
	delete *it;

/*    if ( hasBorder() || str->isRightToLeft() )
        ////kotext: border extends to doc width
        ////        and, bidi parags might have a counter, which will be right-aligned...
    {
        setWidth( textDocument()->width() - 1 );
    }
    else*/
    {
        if ( lineStarts.count() == 1 ) { //&& ( !doc || doc->flow()->isEmpty() ) ) {
// kotext: for proper parag borders, we want all parags to be as wide as linestart->w
/*            if ( !str->isBidi() ) {
                KoTextStringChar *c = &str->at( str->length() - 1 );
                r.setWidth( c->x + c->width );
            } else*/ {
                r.setWidth( lineStarts[0]->w );
            }
        }
        if ( newLinesAllowed ) {
            it = lineStarts.begin();
            int usedw = 0; int lineid = 0;
            for ( ; it != lineStarts.end(); ++it, ++lineid ) {
                usedw = qMax( usedw, (*it)->w );
            }
            if ( r.width() <= 0 ) {
                // if the user specifies an invalid rect, this means that the
                // bounding box should grow to the width that the text actually
                // needs
                r.setWidth( usedw );
            } else {
                r.setWidth( qMin( usedw, r.width() ) );
            }
        }
    }

    if ( y != r.height() )
	r.setHeight( y );

    if ( !visible )
	r.setHeight( 0 );

    // do page breaks if required
    if ( doc && doc->isPageBreakEnabled() ) {
        int shift = doc->formatter()->formatVertically( doc, this );
        //kDebug(32500) << "formatVertically returned shift=" << shift << endl;
        if ( shift && !formattedAgain ) {
            formattedAgain = TRUE;
            goto formatAgain;
        }
    }

    if ( doc )
        doc->formatter()->postFormat( this );

    if ( n && doMove && n->isValid() && r.y() + r.height() != n->r.y() ) {
        //kDebug(32500) << "r=" << r << " n->r=" << n->r << endl;
	int dy = ( r.y() + r.height() ) - n->r.y();
	KoTextParag *s = n;
	bool makeInvalid = false; //p && p->lastInFrame;
	//kDebug(32500) << "might move of dy=" << dy << ". previous's lastInFrame (=makeInvalid): " << makeInvalid << endl;
	while ( s && dy ) {
            if ( s->movedDown ) { // (not in QRT) : moved down -> invalidate and stop moving down
                s->invalidate( 0 ); // (there is no point in moving down a parag that has a frame break...)
                break;
            }
	    if ( !s->isFullWidth() )
		makeInvalid = TRUE;
	    if ( makeInvalid )
		s->invalidate( 0 );
	    s->move( dy );
	    //if ( s->lastInFrame )
            //    makeInvalid = TRUE;
  	    s = s->n;
	}
    }

//#define DEBUG_CI_PLACEMENT
    if ( mFloatingItems ) {
#ifdef DEBUG_CI_PLACEMENT
        kDebug(32500) << lineStarts.count() << " lines" << endl;
#endif
        // Place custom items - after the formatting is finished
        int len = length();
        int line = -1;
        int lineY = 0; // the one called "cy" in other algos
        int baseLine = 0;
        QMap<int, KoTextParagLineStart*>::Iterator it = lineStarts.begin();
        for ( int i = 0 ; i < len; ++i ) {
            KoTextStringChar *chr = &str->at( i );
            if ( chr->lineStart ) {
                ++line;
                if ( line > 0 )
                    ++it;
                lineY = (*it)->y;
                baseLine = (*it)->baseLine;
#ifdef DEBUG_CI_PLACEMENT
                kDebug(32500) << "New line (" << line << "): lineStart=" << (*it) << " lineY=" << lineY << " baseLine=" << baseLine << " height=" << (*it)->h << endl;
#endif
            }
            if ( chr->isCustom() ) {
                int x = chr->x;
                KoTextCustomItem* item = chr->customItem();
                Q_ASSERT( baseLine >= item->ascent() ); // something went wrong in KoTextFormatter if this isn't the case
                int y = lineY + baseLine - item->ascent();
#ifdef DEBUG_CI_PLACEMENT
                kDebug(32500) << "Custom item: i=" << i << " x=" << x << " lineY=" << lineY << " baseLine=" << baseLine << " ascent=" << item->ascent() << " -> y=" << y << endl;
#endif
                item->move( x, y );
                item->finalize();
            }
        }
    }

    //firstFormat = FALSE; //// unused
    if ( formatterWorked ) // only if it worked, i.e. we had some width to format it
    {
        m_invalid = false;
    }
    changed = TRUE;
    //####   str->setTextChanged( FALSE );
}

int KoTextParag::lineHeightOfChar( int i, int *bl, int *y ) const
{
    if ( !isValid() )
	( (KoTextParag*)this )->format();

    QMap<int, KoTextParagLineStart*>::ConstIterator it = lineStarts.end();
    --it;
    for ( ;; ) {
	if ( i >= it.key() ) {
	    if ( bl )
		*bl = ( *it )->baseLine;
	    if ( y )
		*y = ( *it )->y;
	    return ( *it )->h;
	}
	if ( it == lineStarts.begin() )
	    break;
	--it;
    }

    kWarning(32500) << "KoTextParag::lineHeightOfChar: couldn't find lh for " << i << endl;
    return 15;
}

KoTextStringChar *KoTextParag::lineStartOfChar( int i, int *index, int *line ) const
{
    if ( !isValid() )
	( (KoTextParag*)this )->format();

    int l = (int)lineStarts.count() - 1;
    QMap<int, KoTextParagLineStart*>::ConstIterator it = lineStarts.end();
    --it;
    for ( ;; ) {
	if ( i >= it.key() ) {
	    if ( index )
		*index = it.key();
	    if ( line )
		*line = l;
	    return &str->at( it.key() );
	}
	if ( it == lineStarts.begin() )
	    break;
	--it;
	--l;
    }

    kWarning(32500) << "KoTextParag::lineStartOfChar: couldn't find " << i << endl;
    return 0;
}

int KoTextParag::lines() const
{
    if ( !isValid() )
	( (KoTextParag*)this )->format();

    return (int)lineStarts.count();
}

KoTextStringChar *KoTextParag::lineStartOfLine( int line, int *index ) const
{
    if ( !isValid() )
	( (KoTextParag*)this )->format();

    if ( line >= 0 && line < (int)lineStarts.count() ) {
	QMap<int, KoTextParagLineStart*>::ConstIterator it = lineStarts.begin();
	while ( line-- > 0 )
	    ++it;
	int i = it.key();
	if ( index )
	    *index = i;
	return &str->at( i );
    }

    kWarning(32500) << "KoTextParag::lineStartOfLine: couldn't find " << line << endl;
    return 0;
}

int KoTextParag::leftGap() const
{
    if ( !isValid() )
	( (KoTextParag*)this )->format();

    int line = 0;
    int x = str->at(0).x;  /* set x to x of first char */
    if ( str->isBidi() ) {
	for ( int i = 1; i < str->length(); ++i )
	    x = qMin(x, str->at(i).x);
	return x;
    }

    QMap<int, KoTextParagLineStart*>::ConstIterator it = lineStarts.begin();
    while (line < (int)lineStarts.count()) {
	int i = it.key(); /* char index */
	x = qMin(x, str->at(i).x);
	++it;
	++line;
    }
    return x;
}

void KoTextParag::setFormat( int index, int len, const KoTextFormat *_f, bool useCollection, int flags )
{
    Q_ASSERT( useCollection ); // just for info
    if ( index < 0 )
	index = 0;
    if ( index > str->length() - 1 )
	index = str->length() - 1;
    if ( index + len >= str->length() )
	len = str->length() - index;

    KoTextFormatCollection *fc = 0;
    if ( useCollection )
	fc = formatCollection();
    KoTextFormat *of;
    for ( int i = 0; i < len; ++i ) {
	of = str->at( i + index ).format();
	if ( !changed && _f->key() != of->key() )
	    changed = TRUE;
        // Check things that need the textformatter to run
        // (e.g. not color changes)
        // ######## Is this test exhaustive?
	if ( m_invalid == false &&
	     ( _f->font().family() != of->font().family() ||
	       _f->pointSize() != of->pointSize() ||
	       _f->font().weight() != of->font().weight() ||
	       _f->font().italic() != of->font().italic() ||
	       _f->vAlign() != of->vAlign() ||
               _f->relativeTextSize() != of->relativeTextSize() ||
               _f->offsetFromBaseLine() != of->offsetFromBaseLine() ||
               _f->wordByWord() != of->wordByWord()  ||
               _f->attributeFont() != of->attributeFont() ||
               _f->language() != of->language() ||
               _f->hyphenation() != of->hyphenation() ||
               _f->shadowDistanceX() != of->shadowDistanceX() ||
               _f->shadowDistanceY() != of->shadowDistanceY()
                 ) ) {
	    invalidate( 0 );
	}
	if ( flags == -1 || flags == KoTextFormat::Format || !fc ) {
#ifdef DEBUG_COLLECTION
	    kDebug(32500) << " KoTextParag::setFormat, will use format(f) " << f << " " << _f->key() << endl;
#endif
            KoTextFormat* f = fc ? fc->format( _f ) : const_cast<KoTextFormat *>( _f );
	    str->setFormat( i + index, f, useCollection, true );
	} else {
#ifdef DEBUG_COLLECTION
	    kDebug(32500) << " KoTextParag::setFormat, will use format(of,f,flags) of=" << of << " " << of->key() << ", f=" << _f << " " << _f->key() << endl;
#endif
	    KoTextFormat *fm = fc->format( of, _f, flags );
#ifdef DEBUG_COLLECTION
	    kDebug(32500) << " KoTextParag::setFormat, format(of,f,flags) returned " << fm << " " << fm->key() << " " << endl;
#endif
	    str->setFormat( i + index, fm, useCollection );
	}
    }
}

void KoTextParag::drawCursorDefault( QPainter &painter, KoTextCursor *cursor, int curx, int cury, int curh, const QColorGroup &cg )
{
    painter.fillRect( QRect( curx, cury, 1, curh ), cg.color( QColorGroup::Text ) );
    painter.save();
    if ( str->isBidi() ) {
        const int d = 4;
        if ( at( cursor->index() )->rightToLeft ) {
            painter.setPen( Qt::black );
            painter.drawLine( curx, cury, curx - d / 2, cury + d / 2 );
            painter.drawLine( curx, cury + d, curx - d / 2, cury + d / 2 );
        } else {
            painter.setPen( Qt::black );
            painter.drawLine( curx, cury, curx + d / 2, cury + d / 2 );
            painter.drawLine( curx, cury + d, curx + d / 2, cury + d / 2 );
        }
    }
    painter.restore();
}

int *KoTextParag::tabArray() const
{
    int *ta = tArray;
    if ( !ta && doc )
	ta = doc->tabArray();
    return ta;
}

int KoTextParag::nextTabDefault( int, int x )
{
    int *ta = tArray;
    //if ( doc ) {
	if ( !ta )
	    ta = doc->tabArray();
	int tabStopWidth = doc->tabStopWidth();
    //}
    if ( tabStopWidth != 0 )
	return tabStopWidth*(x/tabStopWidth+1);
    else
        return x;
}

KoTextFormatCollection *KoTextParag::formatCollection() const
{
    if ( doc )
	return doc->formatCollection();
    //if ( !qFormatCollection )
    //    qFormatCollection = new KoTextFormatCollection;
    //return qFormatCollection;
    return 0L;
}

void KoTextParag::show()
{
    if ( visible || !doc )
	return;
    visible = TRUE;
}

void KoTextParag::hide()
{
    if ( !visible || !doc )
	return;
    visible = FALSE;
}

void KoTextParag::setDirection( QChar::Direction d )
{
    if ( str && str->direction() != d ) {
	str->setDirection( d );
	invalidate( 0 );
        //// kotext
        m_layout.direction = d;
	invalidateCounters(); // #47178
        ////
    }
}

QChar::Direction KoTextParag::direction() const
{
    return (str ? str->direction() : QChar::DirON );
}

void KoTextParag::setSelection( int id, int start, int end )
{
    QMap<int, KoTextParagSelection>::ConstIterator it = selections().find( id );
    if ( it != mSelections->end() ) {
	if ( start == ( *it ).start && end == ( *it ).end )
	    return;
    }

    KoTextParagSelection sel;
    sel.start = start;
    sel.end = end;
    (*mSelections)[ id ] = sel;
    setChanged( TRUE, TRUE );
}

void KoTextParag::removeSelection( int id )
{
    if ( !hasSelection( id ) )
	return;
    if ( mSelections )
	mSelections->remove( id );
    setChanged( TRUE, TRUE );
}

int KoTextParag::selectionStart( int id ) const
{
    if ( !mSelections )
	return -1;
    QMap<int, KoTextParagSelection>::ConstIterator it = mSelections->find( id );
    if ( it == mSelections->end() )
	return -1;
    return ( *it ).start;
}

int KoTextParag::selectionEnd( int id ) const
{
    if ( !mSelections )
	return -1;
    QMap<int, KoTextParagSelection>::ConstIterator it = mSelections->find( id );
    if ( it == mSelections->end() )
	return -1;
    return ( *it ).end;
}

bool KoTextParag::hasSelection( int id ) const
{
    if ( !mSelections )
	return FALSE;
    QMap<int, KoTextParagSelection>::ConstIterator it = mSelections->find( id );
    if ( it == mSelections->end() )
	return FALSE;
    return ( *it ).start != ( *it ).end || length() == 1;
}

bool KoTextParag::fullSelected( int id ) const
{
    if ( !mSelections )
	return FALSE;
    QMap<int, KoTextParagSelection>::ConstIterator it = mSelections->find( id );
    if ( it == mSelections->end() )
	return FALSE;
    return ( *it ).start == 0 && ( *it ).end == str->length() - 1;
}

int KoTextParag::lineY( int l ) const
{
    if ( l > (int)lineStarts.count() - 1 ) {
	kWarning(32500) << "KoTextParag::lineY: line " << l << " out of range!" << endl;
	return 0;
    }

    if ( !isValid() )
	( (KoTextParag*)this )->format();

    QMap<int, KoTextParagLineStart*>::ConstIterator it = lineStarts.begin();
    while ( l-- > 0 )
	++it;
    return ( *it )->y;
}

int KoTextParag::lineBaseLine( int l ) const
{
    if ( l > (int)lineStarts.count() - 1 ) {
	kWarning(32500) << "KoTextParag::lineBaseLine: line " << l << " out of range!" << endl;
	return 10;
    }

    if ( !isValid() )
	( (KoTextParag*)this )->format();

    QMap<int, KoTextParagLineStart*>::ConstIterator it = lineStarts.begin();
    while ( l-- > 0 )
	++it;
    return ( *it )->baseLine;
}

int KoTextParag::lineHeight( int l ) const
{
    if ( l > (int)lineStarts.count() - 1 ) {
	kWarning(32500) << "KoTextParag::lineHeight: line " << l << " out of range!" << endl;
	return 15;
    }

    if ( !isValid() )
	( (KoTextParag*)this )->format();

    QMap<int, KoTextParagLineStart*>::ConstIterator it = lineStarts.begin();
    while ( l-- > 0 )
	++it;
    return ( *it )->h;
}

void KoTextParag::lineInfo( int l, int &y, int &h, int &bl ) const
{
    if ( l > (int)lineStarts.count() - 1 ) {
	kWarning(32500) << "KoTextParag::lineInfo: line " << l << " out of range!" << endl;
	kDebug(32500) << (int)lineStarts.count() - 1 << " " << l << endl;
	y = 0;
	h = 15;
	bl = 10;
	return;
    }

    if ( !isValid() )
	( (KoTextParag*)this )->format();

    QMap<int, KoTextParagLineStart*>::ConstIterator it = lineStarts.begin();
    while ( l-- > 0 )
	++it;
    y = ( *it )->y;
    h = ( *it )->h;
    bl = ( *it )->baseLine;
}

uint KoTextParag::alignment() const
{
    return align;
}

void KoTextParag::setFormat( KoTextFormat *fm )
{
#if 0
    bool doUpdate = FALSE;
    if (defFormat && (defFormat != formatCollection()->defaultFormat()))
       doUpdate = TRUE;
#endif
    defFormat = formatCollection()->format( fm );
#if 0
    if ( !doUpdate )
	return;
    for ( int i = 0; i < length(); ++i ) {
	if ( at( i )->format()->styleName() == defFormat->styleName() )
	    at( i )->format()->updateStyle();
    }
#endif
}

KoTextFormatterBase *KoTextParag::formatter() const
{
    if ( doc )
	return doc->formatter();
    return 0;
}

/*int KoTextParag::minimumWidth() const
{
    //return doc ? doc->minimumWidth() : 0;
    return m_minw;
}*/

int KoTextParag::widthUsed() const
{
    return m_wused;
}

void KoTextParag::setTabArray( int *a )
{
    delete [] tArray;
    tArray = a;
}

void KoTextParag::setTabStops( int tw )
{
    if ( doc )
	doc->setTabStops( tw );
    //else
    //    tabStopWidth = tw;
}

QMap<int, KoTextParagSelection> &KoTextParag::selections() const
{
    if ( !mSelections )
	((KoTextParag *)this)->mSelections = new QMap<int, KoTextParagSelection>;
    return *mSelections;
}

Q3PtrList<KoTextCustomItem> &KoTextParag::floatingItems() const
{
    if ( !mFloatingItems )
	((KoTextParag *)this)->mFloatingItems = new Q3PtrList<KoTextCustomItem>;
    return *mFloatingItems;
}

void KoTextCursor::setIndex( int i, bool /*restore*/ )
{
// Note: QRT doesn't allow to position the cursor at string->length
// However we need it, when applying a style to a paragraph, so that
// the trailing space gets the style change applied as well.
// Obviously "right of the trailing space" isn't a good place for a real
// cursor, but this needs to be checked somewhere else.
    if ( i < 0 || i > string->length() ) {
#if defined(QT_CHECK_RANGE)
	kWarning(32500) << "KoTextCursor::setIndex: " << i << " out of range" << endl;
        //abort();
#endif
	i = i < 0 ? 0 : string->length() - 1;
    }

    tmpIndex = -1;
    idx = i;
}

///////////////////////////////////// kotext extensions ///////////////////////

// Return the counter associated with this paragraph.
KoParagCounter *KoTextParag::counter()
{
    if ( !m_layout.counter )
        return 0L;

    // Garbage collect un-needed counters.
    if ( m_layout.counter->numbering() == KoParagCounter::NUM_NONE
        // [keep it for unnumbered outlines (the depth is useful)]
         && ( !m_layout.style || !m_layout.style->isOutline() ) )
        setNoCounter();
    return m_layout.counter;
}

void KoTextParag::setMargin( Q3StyleSheetItem::Margin m, double _i )
{
    //kDebug(32500) << "KoTextParag::setMargin " << m << " margin " << _i << endl;
    m_layout.margins[m] = _i;
    if ( m == Q3StyleSheetItem::MarginTop && prev() )
        prev()->invalidate(0);     // for top margin (post-1.1: remove this, not necessary anymore)
    invalidate(0);
}

void KoTextParag::setMargins( const double * margins )
{
    for ( int i = 0 ; i < 5 ; ++i )
        m_layout.margins[i] = margins[i];
    invalidate(0);
}

void KoTextParag::setAlign( int align )
{
    Q_ASSERT( align <= Qt::AlignJustify );
    align &= Qt::AlignHorizontal_Mask;
    setAlignment( align );
    m_layout.alignment = align;
}

int KoTextParag::resolveAlignment() const
{
    if ( (int)m_layout.alignment == Qt::AlignLeft )
        return str->isRightToLeft() ? Qt::AlignRight : Qt::AlignLeft;
    return m_layout.alignment;
}

void KoTextParag::setLineSpacing( double _i )
{
    m_layout.setLineSpacingValue(_i);
    invalidate(0);
}

void KoTextParag::setLineSpacingType( KoParagLayout::SpacingType _type )
{
    m_layout.lineSpacingType = _type;
    invalidate(0);
}

void KoTextParag::setTopBorder( const KoBorder & _brd )
{
    m_layout.topBorder = _brd;
    invalidate(0);
}

void KoTextParag::setBottomBorder( const KoBorder & _brd )
{
    m_layout.bottomBorder = _brd;
    invalidate(0);
}

void KoTextParag::setJoinBorder( bool join )
{
    m_layout.joinBorder = join;
    invalidate(0);
}

void KoTextParag::setBackgroundColor ( const QColor& color )
{
    m_layout.backgroundColor = color;
    invalidate(0);
}

void KoTextParag::setNoCounter()
{
    delete m_layout.counter;
    m_layout.counter = 0L;
    invalidateCounters();
}

void KoTextParag::setCounter( const KoParagCounter * pCounter )
{
    // Preserve footnote numbering when applying a style
    const bool isFootNote = m_layout.counter &&
                            m_layout.counter->numbering() == KoParagCounter::NUM_FOOTNOTE;
    if ( isFootNote ) {
        const QString footNotePrefix = m_layout.counter->prefix(); // this is where the number is
        delete m_layout.counter;
        m_layout.counter = pCounter ? new KoParagCounter( *pCounter ) : new KoParagCounter();
        m_layout.counter->setNumbering( KoParagCounter::NUM_FOOTNOTE );
        m_layout.counter->setStyle( KoParagCounter::STYLE_NONE ); // no number after the 'prefix'
        m_layout.counter->setPrefix( footNotePrefix );
        m_layout.counter->setSuffix( QString::null );
        invalidateCounters();
    } else {
        if ( pCounter )
            setCounter( *pCounter );
        else
            setNoCounter();
    }
}

void KoTextParag::setCounter( const KoParagCounter & counter )
{
    // Garbage collect unnneeded counters.
    if ( counter.numbering() == KoParagCounter::NUM_NONE
         // [keep it for unnumbered outlines (the depth is useful)]
         && ( !m_layout.style || !m_layout.style->isOutline() ) )
    {
        setNoCounter();
    }
    else
    {
        delete m_layout.counter;
        m_layout.counter = new KoParagCounter( counter );

        // Invalidate the counters
        invalidateCounters();
    }
}

void KoTextParag::invalidateCounters()
{
    // Invalidate this paragraph and all the following ones
    // (Numbering may have changed)
    invalidate( 0 );
    if ( m_layout.counter )
        m_layout.counter->invalidate();
    KoTextParag *s = next();
    // #### Possible optimization: since any invalidation propagates down,
    // it's enough to stop at the first paragraph with an already-invalidated counter, isn't it?
    // This is only true if nobody else calls counter->invalidate...
    while ( s ) {
        if ( s->m_layout.counter )
            s->m_layout.counter->invalidate();
        s->invalidate( 0 );
        s = s->next();
    }
}

int KoTextParag::counterWidth() const
{
    if ( !m_layout.counter )
        return 0;

    return m_layout.counter->width( this );
}

// Draw the complete label (i.e. heading/list numbers/bullets) for this paragraph.
// This is called by KoTextParag::paint.
void KoTextParag::drawLabel( QPainter* p, int xLU, int yLU, int /*wLU*/, int /*hLU*/, int baseLU, const QColorGroup& /*cg*/ )
{
    if ( !m_layout.counter ) // shouldn't happen
        return;

    if ( m_layout.counter->numbering() == KoParagCounter::NUM_NONE )
        return;

    int counterWidthLU = m_layout.counter->width( this );

    // We use the formatting of the first char as the formatting of the counter
    KoTextFormat counterFormat( *KoParagCounter::counterFormat( this ) );
    if ( !m_layout.style || !m_layout.style->isOutline() )
    {
      // But without bold/italic for normal lists, since some items could be bold and others not.
      // For headings we must keep the bold when the heading is bold.
      counterFormat.setBold( false );
      counterFormat.setItalic( false );
    }
    KoTextFormat* format = &counterFormat;
    p->save();

    QColor textColor( format->color() );
    if ( !textColor.isValid() ) // Resolve the color at this point
        textColor = KoTextFormat::defaultTextColor( p );
    p->setPen( QPen( textColor ) );

    KoTextZoomHandler * zh = textDocument()->paintingZoomHandler();
    assert( zh );
    //bool forPrint = ( p->device()->devType() == QInternal::Printer );

    bool rtl = str->isRightToLeft(); // when true, we put suffix+counter+prefix at the RIGHT of the paragraph.
    int xLeft = zh->layoutUnitToPixelX( xLU - (rtl ? 0 : counterWidthLU) );
    int y = zh->layoutUnitToPixelY( yLU );
    //int h = zh->layoutUnitToPixelY( yLU, hLU );
    int base = zh->layoutUnitToPixelY( yLU, baseLU );
    int counterWidth = zh->layoutUnitToPixelX( xLU, counterWidthLU );
    int height = zh->layoutUnitToPixelY( yLU, format->height() );

    QFont font( format->screenFont( zh ) );
    // Footnote numbers are in superscript (in WP and Word, not in OO)
    if ( m_layout.counter->numbering() == KoParagCounter::NUM_FOOTNOTE )
    {
        int pointSize = ( ( font.pointSize() * 2 ) / 3 );
        font.setPointSize( pointSize );
        y -= ( height - QFontMetrics(font).height() );
    }
    p->setFont( font );

    // Now draw any bullet that is required over the space left for it.
    if ( m_layout.counter->isBullet() )
    {
	int xBullet = xLeft + zh->layoutUnitToPixelX( m_layout.counter->bulletX() );

        //kDebug(32500) << "KoTextParag::drawLabel xLU=" << xLU << " counterWidthLU=" << counterWidthLU << endl;
	// The width and height of the bullet is the width of one space
        int width = zh->layoutUnitToPixelX( xLeft, format->width( ' ' ) );

        //kDebug(32500) << "Pix: xLeft=" << xLeft << " counterWidth=" << counterWidth
        //          << " xBullet=" << xBullet << " width=" << width << endl;

        QString prefix = m_layout.counter->prefix();
        if ( !prefix.isEmpty() )
        {
            if ( rtl )
                prefix.prepend( ' ' /*the space before the bullet in RightToLeft mode*/ );
            KoTextParag::drawFontEffects( p, format, zh, format->screenFont( zh ), textColor, xLeft, base, width, y, height, prefix[0] );

            int posY =y + base - format->offsetFromBaseLine();
            //we must move to bottom text because we create
            //shadow to 'top'.
            int sy = format->shadowY( zh );
            if ( sy < 0)
                posY -= sy;

            p->drawText( xLeft, posY, prefix );
        }

        QRect er( xBullet + (rtl ? width : 0), y + height / 2 - width / 2, width, width );
        // Draw the bullet.
        int posY = 0;
        switch ( m_layout.counter->style() )
        {
            case KoParagCounter::STYLE_DISCBULLET:
                p->setBrush( QBrush(textColor) );
                p->drawEllipse( er );
                p->setBrush( Qt::NoBrush );
                break;
            case KoParagCounter::STYLE_SQUAREBULLET:
                p->fillRect( er, QBrush(textColor) );
                break;
            case KoParagCounter::STYLE_BOXBULLET:
                p->drawRect( er );
                break;
            case KoParagCounter::STYLE_CIRCLEBULLET:
                p->drawEllipse( er );
                break;
            case KoParagCounter::STYLE_CUSTOMBULLET:
            {
                // The user has selected a symbol from a special font. Override the paragraph
                // font with the given family. This conserves the right size etc.
                if ( !m_layout.counter->customBulletFont().isEmpty() )
                {
                    QFont bulletFont( p->font() );
                    bulletFont.setFamily( m_layout.counter->customBulletFont() );
                    p->setFont( bulletFont );
                }
                KoTextParag::drawFontEffects( p, format, zh, format->screenFont( zh ), textColor, xBullet, base, width, y, height, ' ' );

                posY = y + base- format->offsetFromBaseLine();
                //we must move to bottom text because we create
                //shadow to 'top'.
                int sy = format->shadowY( zh );
                if ( sy < 0)
                    posY -= sy;

                p->drawText( xBullet, posY, m_layout.counter->customBulletCharacter() );
                break;
            }
            default:
                break;
        }

        QString suffix = m_layout.counter->suffix();
        if ( !suffix.isEmpty() )
        {
            if ( !rtl )
                suffix += ' ' /*the space after the bullet*/;

            KoTextParag::drawFontEffects( p, format, zh, format->screenFont( zh ), textColor, xBullet + width, base, counterWidth, y,height, suffix[0] );

            int posY =y + base- format->offsetFromBaseLine();
            //we must move to bottom text because we create
            //shadow to 'top'.
            int sy = format->shadowY( zh );
            if ( sy < 0)
                posY -= sy;

            p->drawText( xBullet + width, posY, suffix, -1 );
        }
    }
    else
    {
        QString counterText = m_layout.counter->text( this );
        // There are no bullets...any parent bullets have already been suppressed.
        // Just draw the text! Note: one space is always appended.
        if ( !counterText.isEmpty() )
        {
            KoTextParag::drawFontEffects( p, format, zh, format->screenFont( zh ), textColor, xLeft, base, counterWidth, y, height, counterText[0] );

            counterText += ' ' /*the space after the bullet (before in RightToLeft mode)*/;

            int posY =y + base - format->offsetFromBaseLine();
            //we must move to bottom text because we create
            //shadow to 'top'.
            int sy = format->shadowY( zh );
            if ( sy < 0)
                posY -= sy;

            p->drawText( xLeft, posY , counterText, -1 );
        }
    }
    p->restore();
}

int KoTextParag::breakableTopMargin() const
{
    KoTextZoomHandler * zh = textDocument()->formattingZoomHandler();
    return zh->ptToLayoutUnitPixY(
        m_layout.margins[ Q3StyleSheetItem::MarginTop ] );
}

int KoTextParag::topMargin() const
{
    KoTextZoomHandler * zh = textDocument()->formattingZoomHandler();
    return zh->ptToLayoutUnitPixY(
        m_layout.margins[ Q3StyleSheetItem::MarginTop ]
        + ( ( prev() && prev()->joinBorder() && prev()->bottomBorder() == m_layout.bottomBorder &&
        prev()->topBorder() == m_layout.topBorder && prev()->leftBorder() == m_layout.leftBorder &&
        prev()->rightBorder() == m_layout.rightBorder) ? 0 : m_layout.topBorder.width() ) );
}

int KoTextParag::bottomMargin() const
{
    KoTextZoomHandler * zh = textDocument()->formattingZoomHandler();
    return zh->ptToLayoutUnitPixY(
        m_layout.margins[ Q3StyleSheetItem::MarginBottom ]
        + ( ( joinBorder() && next() && next()->bottomBorder() == m_layout.bottomBorder &&
        next()->topBorder() == m_layout.topBorder && next()->leftBorder() == m_layout.leftBorder &&
        next()->rightBorder() == m_layout.rightBorder) ? 0 : m_layout.bottomBorder.width() ) );
}

int KoTextParag::leftMargin() const
{
    KoTextZoomHandler * zh = textDocument()->formattingZoomHandler();
    return zh->ptToLayoutUnitPixX(
        m_layout.margins[ Q3StyleSheetItem::MarginLeft ]
        + m_layout.leftBorder.width() );
}

int KoTextParag::rightMargin() const
{
    KoTextZoomHandler * zh = textDocument()->formattingZoomHandler();
    int cw=0;
    if( m_layout.counter && str->isRightToLeft() &&
    	(( m_layout.counter->alignment() == Qt::AlignRight ) || ( m_layout.counter->alignment() == Qt::AlignLeft )))
	    cw = counterWidth();

    return zh->ptToLayoutUnitPixX(
        m_layout.margins[ Q3StyleSheetItem::MarginRight ]
        + m_layout.rightBorder.width() )
        + cw; /* in layout units already */
}

int KoTextParag::firstLineMargin() const
{
    KoTextZoomHandler * zh = textDocument()->formattingZoomHandler();
    return zh->ptToLayoutUnitPixY(
        m_layout.margins[ Q3StyleSheetItem::MarginFirstLine ] );
}

int KoTextParag::lineSpacing( int line ) const
{
    Q_ASSERT( isValid() );
    if ( m_layout.lineSpacingType == KoParagLayout::LS_SINGLE )
        return 0; // or shadow, see calculateLineSpacing
    else {
        if( line >= (int)lineStarts.count() )
        {
            kError() << "KoTextParag::lineSpacing assert(line<lines) failed: line=" << line << " lines=" << lineStarts.count() << endl;
            return 0;
        }
        QMap<int, KoTextParagLineStart*>::ConstIterator it = lineStarts.begin();
        while ( line-- > 0 )
            ++it;
        return (*it)->lineSpacing;
    }
}

// Called by KoTextFormatter
int KoTextParag::calculateLineSpacing( int line, int startChar, int lastChar ) const
{
    KoTextZoomHandler * zh = textDocument()->formattingZoomHandler();
    // TODO add shadow in KoTextFormatter!
    int shadow = 0; //QABS( zh->ptToLayoutUnitPixY( shadowDistanceY() ) );
    if ( m_layout.lineSpacingType == KoParagLayout::LS_SINGLE )
        return shadow;
    else if ( m_layout.lineSpacingType == KoParagLayout::LS_CUSTOM )
        return zh->ptToLayoutUnitPixY( m_layout.lineSpacingValue() ) + shadow;
    else {
        if( line >= (int)lineStarts.count() )
        {
            kError() << "KoTextParag::lineSpacing assert(line<lines) failed: line=" << line << " lines=" << lineStarts.count() << endl;
            return 0+shadow;
        }
        QMap<int, KoTextParagLineStart*>::ConstIterator it = lineStarts.begin();
        while ( line-- > 0 )
            ++it;

        //kDebug(32500) << " line spacing type: " << m_layout.lineSpacingType << " value:" << m_layout.lineSpacingValue() << " line_height=" << (*it)->h << " startChar=" << startChar << " lastChar=" << lastChar << endl;
        switch ( m_layout.lineSpacingType )
        {
        case KoParagLayout::LS_MULTIPLE:
        {
            double n = m_layout.lineSpacingValue() - 1.0; // yes, can be negative
            return shadow + qRound( n * heightForLineSpacing( startChar, lastChar ) );
        }
        case KoParagLayout::LS_ONEANDHALF:
        {
            // Special case of LS_MULTIPLE, with n=1.5
            return shadow + heightForLineSpacing( startChar, lastChar ) / 2;
        }
        case KoParagLayout::LS_DOUBLE:
        {
            // Special case of LS_MULTIPLE, with n=1
            return shadow + heightForLineSpacing( startChar, lastChar );
        }
        case KoParagLayout::LS_AT_LEAST:
        {
            int atLeast = zh->ptToLayoutUnitPixY( m_layout.lineSpacingValue() );
            const int lineHeight = ( *it )->h;
            int h = qMax( lineHeight, atLeast );
            // height is now the required total height
            return shadow + h - lineHeight;
        }
        case KoParagLayout::LS_FIXED:
        {
            const int lineHeight = ( *it )->h;
            return shadow + zh->ptToLayoutUnitPixY( m_layout.lineSpacingValue() ) - lineHeight;
        }
        // Silence compiler warnings
        case KoParagLayout::LS_SINGLE:
        case KoParagLayout::LS_CUSTOM:
            break;
        }
    }
    kWarning() << "Unhandled linespacing type : " << m_layout.lineSpacingType << endl;
    return 0+shadow;
}

QRect KoTextParag::pixelRect( KoTextZoomHandler *zh ) const
{
    QRect rct( zh->layoutUnitToPixel( rect() ) );
    //kDebug(32500) << "   pixelRect for parag " << paragId()
    //               << ": rect=" << rect() << " pixelRect=" << rct << endl;

    // After division we almost always end up with the top overwriting the bottom of the parag above
    if ( prev() )
    {
        QRect prevRect( zh->layoutUnitToPixel( prev()->rect() ) );
        if ( rct.top() < prevRect.bottom() + 1 )
        {
            //kDebug(32500) << "   pixelRect: rct.top() adjusted to " << prevRect.bottom() + 1 << " (was " << rct.top() << ")" << endl;
            rct.setTop( prevRect.bottom() + 1 );
        }
    }
    return rct;
}

// Paint this paragraph. This is called by KoTextDocument::drawParagWYSIWYG
// (KoTextDocument::drawWithoutDoubleBuffer when printing)
void KoTextParag::paint( QPainter &painter, const QColorGroup &cg, KoTextCursor *cursor, bool drawSelections,
                         int clipx, int clipy, int clipw, int cliph )
{
#ifdef DEBUG_PAINT
    kDebug(32500) << "KoTextParag::paint =====  id=" << paragId() << " clipx=" << clipx << " clipy=" << clipy << " clipw=" << clipw << " cliph=" << cliph << endl;
    kDebug(32500) << " clipw in pix (approx) : " << textDocument()->paintingZoomHandler()->layoutUnitToPixelX( clipw ) << " cliph in pix (approx) : " << textDocument()->paintingZoomHandler()->layoutUnitToPixelX( cliph ) << endl;
#endif

    KoTextZoomHandler * zh = textDocument()->paintingZoomHandler();
    assert(zh);

    QRect paraRect = pixelRect( zh );

    // Find left margin size, first line offset and right margin in pixels
    int leftMarginPix = zh->layoutUnitToPixelX( leftMargin() );
    int firstLineOffset = zh->layoutUnitToPixelX( firstLineMargin() );

    // The furthest left and right x-coords of the paragraph,
    // including the bullet/counter, but not the borders.
    int leftExtent = qMin ( leftMarginPix,  leftMarginPix + firstLineOffset );
    int rightExtent = paraRect.width() - zh->layoutUnitToPixelX( rightMargin() );

    // Draw the paragraph background color
    if ( backgroundColor().isValid() )
    {
        // Render background from either left margin indent, or first line indent,
        // whichever is nearer the left.
        int backgroundWidth = rightExtent - leftExtent + 1;
        int backgroundHeight = pixelRect( zh ).height();
        painter.fillRect( leftExtent, 0,
                          backgroundWidth, backgroundHeight,
                          backgroundColor() );
    }

    // Let's call drawLabel ourselves, rather than having to deal with QStyleSheetItem to get paintLines to call it!
    if ( m_layout.counter && m_layout.counter->numbering() != KoParagCounter::NUM_NONE && m_lineChanged <= 0 )
    {
        int cy, h, baseLine;
        lineInfo( 0, cy, h, baseLine );
        int xLabel = at(0)->x;
        if ( str->isRightToLeft() )
            xLabel += at(0)->width;
        drawLabel( &painter, xLabel, cy, 0, 0, baseLine, cg );
    }

    paintLines( painter, cg, cursor, drawSelections, clipx, clipy, clipw, cliph );

    // Now draw paragraph border
    if ( m_layout.hasBorder() )
    {
        bool const drawTopBorder = !prev() || !prev()->joinBorder() || prev()->bottomBorder() != bottomBorder() || prev()->topBorder() != topBorder() || prev()->leftBorder() != leftBorder() || prev()->rightBorder() != rightBorder();
        bool const drawBottomBorder = !joinBorder() || !next() || next()->bottomBorder() != bottomBorder() || next()->topBorder() != topBorder() || next()->leftBorder() != leftBorder() || next()->rightBorder() != rightBorder();

        // Paragraph borders surround the paragraph and its
        // counters/bullets, but they only touch the frame border if
        // the paragraph margins are of non-zero length.
        // This is what OpenOffice does (no really, it is this time).
        //
        // drawBorders paints outside the give rect, so for the
        // x-coords, it just needs to know the left and right extent
        // of the paragraph.
        QRect r;
        r.setLeft( leftExtent );
        r.setRight( rightExtent );
        r.setTop( zh->layoutUnitToPixelY(lineY( 0 )) );

        int lastLine = lines() - 1;
        // We need to start from the pixelRect, to make sure the bottom border is entirely painted.
        // This is a case where we DO want to subtract pixels to pixels...
        int paragBottom = pixelRect(zh).height()-1;
        // If we don't have a bottom border, we need go as low as possible ( to touch the next parag's border ).
        // If we have a bottom border, then we rather exclude the linespacing. Looks nicer. OO does that too.
        if ( m_layout.bottomBorder.width() > 0 && drawBottomBorder)
            paragBottom -= zh->layoutUnitToPixelY( lineSpacing( lastLine ) );
        paragBottom -= KoBorder::zoomWidthY( m_layout.bottomBorder.width(), zh, 0 );
        //kDebug(32500) << "Parag border: paragBottom=" << paragBottom
        //               << " bottom border width = " << KoBorder::zoomWidthY( m_layout.bottomBorder.width(), zh, 0 ) << endl;
        r.setBottom( paragBottom );

        //kDebug(32500) << "KoTextParag::paint documentWidth=" << documentWidth() << " LU (" << zh->layoutUnitToPixelX(documentWidth()) << " pixels) bordersRect=" << r << endl;
        KoBorder::drawBorders( painter, zh, r,
                               m_layout.leftBorder, m_layout.rightBorder, m_layout.topBorder, m_layout.bottomBorder,
                               0, QPen(), drawTopBorder, drawBottomBorder );
    }
}


void KoTextParag::paintLines( QPainter &painter, const QColorGroup &cg, KoTextCursor *cursor, bool drawSelections,
			int clipx, int clipy, int clipw, int cliph )
{
    if ( !visible )
	return;
    //KoTextStringChar *chr = at( 0 );
    //if (!chr) { kDebug(32500) << "paragraph " << (void*)this << " " << paragId() << ", can't paint, EMPTY !" << endl;

    // This is necessary with the current code, but in theory it shouldn't
    // be necessary, if Xft really gives us fully proportionnal chars....
#define CHECK_PIXELXADJ

    int curx = -1, cury = 0, curh = 0, curline = 0;
    int xstart, xend = 0;

    QString qstr = str->toString();
    qstr.replace( QChar(0x00a0U), ' ' ); // Not all fonts have non-breakable-space glyph

    const int nSels = doc ? doc->numSelections() : 1;
    Q3MemArray<int> selectionStarts( nSels );
    Q3MemArray<int> selectionEnds( nSels );
    if ( drawSelections ) {
	bool hasASelection = FALSE;
	for ( int i = 0; i < nSels; ++i ) {
	    if ( !hasSelection( i ) ) {
		selectionStarts[ i ] = -1;
		selectionEnds[ i ] = -1;
	    } else {
		hasASelection = TRUE;
		selectionStarts[ i ] = selectionStart( i );
		int end = selectionEnd( i );
		if ( end == length() - 1 && n && n->hasSelection( i ) )
		    end++;
		selectionEnds[ i ] = end;
	    }
	}
	if ( !hasASelection )
	    drawSelections = FALSE;
    }

    // Draw the lines!
    int line = m_lineChanged;
    if (line<0) line = 0;

    int numLines = lines();
#ifdef DEBUG_PAINT
    kDebug(32500) << " paintLines: from line " << line << " to " << numLines-1 << endl;
#endif
    for( ; line<numLines ; line++ )
    {
	// get the start and length of the line
	int nextLine;
        int startOfLine;
    	lineStartOfLine(line, &startOfLine);
	if (line == numLines-1 )
            nextLine = length();
	else
            lineStartOfLine(line+1, &nextLine);

	// init this line
        int cy, h, baseLine;
	lineInfo( line, cy, h, baseLine );
	if ( clipy != -1 && cy > clipy - r.y() + cliph ) // outside clip area, leave
	    break;

        // Vars related to the current "run of text"
	int paintStart = startOfLine;
	KoTextStringChar* chr = at(startOfLine);
        KoTextStringChar* nextchr = chr;

	// okay, paint the line!
	for(int i=startOfLine;i<nextLine;i++)
	{
            chr = nextchr;
            if ( i < nextLine-1 )
                nextchr = at( i+1 );

            // we flush at end of line
            bool flush = ( i == nextLine - 1 );
            // Optimization note: QRT uses "flush |=", which doesn't have shortcut optimization

            // we flush on format changes
	    flush = flush || ( nextchr->format() != chr->format() );
	    // we flush on link changes
	    //flush = flush || ( nextchr->isLink() != chr->isLink() );
            // we flush on small caps changes
            if ( !flush && chr->format()->attributeFont() == KoTextFormat::ATT_SMALL_CAPS )
            {
                bool isLowercase = chr->c.upper() != chr->c;
                bool nextLowercase = nextchr->c.upper() != nextchr->c;
                flush = isLowercase != nextLowercase;
            }
	    // we flush on start of run
	    flush = flush || nextchr->startOfRun;
	    // we flush on bidi changes
	    flush = flush || ( nextchr->rightToLeft != chr->rightToLeft );
#ifdef CHECK_PIXELXADJ
            // we flush when the value of pixelxadj changes
            // [unless inside a ligature]
            flush = flush || ( nextchr->pixelxadj != chr->pixelxadj && nextchr->charStop );
#endif
	    // we flush before and after tabs
	    flush = flush || ( chr->c == '\t' || nextchr->c == '\t' );
	    // we flush on soft hyphens
	    flush = flush || ( chr->c.unicode() == 0xad );
	    // we flush on custom items
	    flush = flush || chr->isCustom();
	    // we flush before custom items
	    flush = flush || nextchr->isCustom();
	    // when painting justified we flush on spaces
	    if ((alignment() & Qt::AlignJustify) == Qt::AlignJustify )
		//flush = flush || QTextFormatter::isBreakable( str, i );
                flush = flush || chr->whiteSpace;
	    // when underlining or striking "word by word" we flush before/after spaces
	    if (!flush && chr->format()->wordByWord() && chr->format()->isStrikedOrUnderlined())
                flush = flush || chr->whiteSpace || nextchr->whiteSpace;
	    // we flush when the string is getting too long
	    flush = flush || ( i - paintStart >= 256 );
	    // we flush when the selection state changes
	    if ( drawSelections ) {
                // check if selection state changed - TODO update from QRT
		bool selectionChange = FALSE;
		if ( drawSelections ) {
		    for ( int j = 0; j < nSels; ++j ) {
			selectionChange = selectionStarts[ j ] == i+1 || selectionEnds[ j ] == i+1;
			if ( selectionChange )
			    break;
		    }
		}
                flush = flush || selectionChange;
            }

            // check for cursor mark
            if ( cursor && this == cursor->parag() && i == cursor->index() ) {
                curx = cursor->x();
                curline = line;
                KoTextStringChar *c = chr;
                if ( i > 0 )
                    --c;
                curh = c->height();
                cury = cy + baseLine - c->ascent();
            }

            if ( flush ) {  // something changed, draw what we have so far

                KoTextStringChar* cStart = at( paintStart );
                if ( chr->rightToLeft ) {
                    xstart = chr->x;
                    xend = cStart->x + cStart->width;
                } else {
                    xstart = cStart->x;
                        if ( i < length() - 1 && !str->at( i + 1 ).lineStart &&
                         str->at( i + 1 ).rightToLeft == chr->rightToLeft )
                        xend = str->at( i + 1 ).x;
                    else
                        xend = chr->x + chr->width;
                }

                if ( (clipx == -1 || clipw == -1) || (xend >= clipx && xstart <= clipx + clipw) ) {
                    if ( !chr->isCustom() ) {
                        drawParagString( painter, qstr, paintStart, i - paintStart + 1, xstart, cy,
                                         baseLine, xend-xstart, h, drawSelections,
                                         chr->format(), selectionStarts, selectionEnds,
                                         cg, chr->rightToLeft, line );
                    }
                    else
                        if ( chr->customItem()->placement() == KoTextCustomItem::PlaceInline ) {
                            chr->customItem()->draw( &painter, chr->x, cy + baseLine - chr->customItem()->ascent(),
                                                     clipx - r.x(), clipy - r.y(), clipw, cliph, cg,
                                                     drawSelections && nSels && selectionStarts[ 0 ] <= i && selectionEnds[ 0 ] > i );
                    }
                }
                paintStart = i+1;
            }
        } // end of character loop
    } // end of line loop

    // if we should draw a cursor, draw it now
    if ( curx != -1 && cursor ) {
        drawCursor( painter, cursor, curx, cury, curh, cg );
    }
}

// Called by KoTextParag::paintLines
// Draw a set of characters with the same formattings.
// Reimplemented here to convert coordinates first, and call @ref drawFormattingChars.
void KoTextParag::drawParagString( QPainter &painter, const QString &str, int start, int len, int startX,
                                   int lastY, int baseLine, int bw, int h, bool drawSelections,
                                   KoTextFormat *format, const Q3MemArray<int> &selectionStarts,
                                   const Q3MemArray<int> &selectionEnds, const QColorGroup &cg, bool rightToLeft, int line )
{
    KoTextZoomHandler * zh = textDocument()->paintingZoomHandler();
    assert(zh);

#ifdef DEBUG_PAINT
    kDebug(32500) << "KoTextParag::drawParagString drawing from " << start << " to " << start+len << endl;
    kDebug(32500) << " startX in LU: " << startX << " lastY in LU:" << lastY
                   << " baseLine in LU:" << baseLine << endl;
#endif

    // Calculate offset (e.g. due to shadow on left or top)
    // Important: don't use the 2-args methods here, offsets are not heights
    // (0 should be 0, not 1) (#63256)
    int shadowOffsetX_pix = zh->layoutUnitToPixelX( format->offsetX() );
    int shadowOffsetY_pix = zh->layoutUnitToPixelY( format->offsetY() );

    // Calculate startX in pixels
    int startX_pix = zh->layoutUnitToPixelX( startX ) /* + at( rightToLeft ? start+len-1 : start )->pixelxadj */;
#ifdef DEBUG_PAINT
    kDebug(32500) << "KoTextParag::drawParagString startX in pixels : " << startX_pix /*<< " adjustment:" << at( rightToLeft ? start+len-1 : start )->pixelxadj*/ << " bw=" << bw << endl;
#endif

    int bw_pix = zh->layoutUnitToPixelX( startX, bw );
    int lastY_pix = zh->layoutUnitToPixelY( lastY );
    int baseLine_pix = zh->layoutUnitToPixelY( lastY, baseLine ); // 2 args=>+1. Is that correct?
    int h_pix = zh->layoutUnitToPixelY( lastY, h );
#ifdef DEBUG_PAINT
    kDebug(32500) << "KoTextParag::drawParagString h(LU)=" << h << " lastY(LU)=" << lastY
                   << " h(PIX)=" << h_pix << " lastY(PIX)=" << lastY_pix
                   << " baseLine(PIX)=" << baseLine_pix << endl;
#endif

    if ( format->textBackgroundColor().isValid() )
        painter.fillRect( startX_pix, lastY_pix, bw_pix, h_pix, format->textBackgroundColor() );

    // don't want to draw line breaks but want them when drawing formatting chars
    int draw_len = len;
    int draw_startX = startX;
    int draw_bw = bw_pix;
    if ( at( start + len - 1 )->c == '\n' )
    {
        draw_len--;
        draw_bw -= at( start + len - 1 )->pixelwidth;
        if ( rightToLeft && draw_len > 0 )
            draw_startX = at( start + draw_len - 1 )->x;
    }

    // Draw selection (moved here to do it before applying the offset from the shadow)
    // (and because it's not part of the shadow drawing)
    if ( drawSelections ) {
        bool inSelection = false;
	const int nSels = doc ? doc->numSelections() : 1;
	for ( int j = 0; j < nSels; ++j ) {
	    if ( start >= selectionStarts[ j ] && start < selectionEnds[ j ] ) {
                inSelection = true;
                switch (j) {
                case KoTextDocument::Standard:
                    painter.fillRect( startX_pix, lastY_pix, bw_pix, h_pix, cg.color( QColorGroup::Highlight ) );
                    break;
                case KoTextDocument::InputMethodPreedit:
                    // no highlight
                    break;
                default:
                    painter.fillRect( startX_pix, lastY_pix, bw_pix, h_pix, doc ? doc->selectionColor( j ) : cg.color( QColorGroup::Highlight ) );
                    break;
                }
	    }
	}
        if ( !inSelection )
            drawSelections = false; // save time in drawParagStringInternal
    }

    // Draw InputMethod Preedit Underline
    const int nSels = doc ? doc->numSelections() : 1;
    if ( KoTextDocument::InputMethodPreedit < nSels
         && doc->hasSelection( KoTextDocument::InputMethodPreedit )
         && start >= selectionStarts[ KoTextDocument::InputMethodPreedit ]
         && start < selectionEnds[ KoTextDocument::InputMethodPreedit ] )
    {
        QColor textColor( format->color() );
        painter.setPen( QPen( textColor ) );

        QPoint p1( startX_pix, lastY_pix + h_pix - 1 );
        QPoint p2( startX_pix + bw_pix, lastY_pix + h_pix - 1 );
        painter.drawLine( p1, p2 );
    }

    if ( draw_len > 0 )
    {
        int draw_startX_pix = zh->layoutUnitToPixelX( draw_startX ) /* + at( rightToLeft ? start+draw_len-1 : start )->pixelxadj*/;
        draw_startX_pix += shadowOffsetX_pix;
        lastY_pix += shadowOffsetY_pix;

        if ( format->shadowDistanceX() != 0 || format->shadowDistanceY() != 0 ) {
            int sx = format->shadowX( zh );
            int sy = format->shadowY( zh );
            if ( sx != 0 || sy != 0 )
            {
                painter.save();
                painter.translate( sx, sy );
                drawParagStringInternal( painter, str, start, draw_len, draw_startX_pix,
                                         lastY_pix, baseLine_pix,
                                         draw_bw,
                                         h_pix, FALSE /*drawSelections*/,
                                         format, selectionStarts,
                                         selectionEnds, cg, rightToLeft, line, zh, true );
                painter.restore();
            }
        }

        drawParagStringInternal( painter, str, start, draw_len, draw_startX_pix,
                                 lastY_pix, baseLine_pix,
                                 draw_bw,
                                 h_pix, drawSelections, format, selectionStarts,
                                 selectionEnds, cg, rightToLeft, line, zh, false );
    }

    bool forPrint = ( painter.device()->devType() == QInternal::Printer );
    if ( textDocument()->drawFormattingChars() && !forPrint )
    {
        drawFormattingChars( painter, start, len,
                             lastY_pix, baseLine_pix, h_pix,
                             drawSelections,
                             format, selectionStarts,
                             selectionEnds, cg, rightToLeft,
                             line, zh, AllFormattingChars );
    }
}

// Copied from the original KoTextParag
// (we have to copy it here, so that color & font changes don't require changing
// a local copy of the text format)
// And we have to keep it separate from drawParagString to avoid s/startX/startX_pix/ etc.
void KoTextParag::drawParagStringInternal( QPainter &painter, const QString &s, int start, int len, int startX,
                                   int lastY, int baseLine, int bw, int h, bool drawSelections,
                                   KoTextFormat *format, const Q3MemArray<int> &selectionStarts,
                                   const Q3MemArray<int> &selectionEnds, const QColorGroup &cg, bool rightToLeft, int line, KoTextZoomHandler* zh, bool drawingShadow )
{
#ifdef DEBUG_PAINT
    kDebug(32500) << "KoTextParag::drawParagStringInternal start=" << start << " len=" << len << " : '" << s.mid(start,len) << "'" << endl;
    kDebug(32500) << "In pixels:  startX=" << startX << " lastY=" << lastY << " baseLine=" << baseLine
                   << " bw=" << bw << " h=" << h << " rightToLeft=" << rightToLeft << endl;
#endif
    if ( drawingShadow && format->shadowDistanceX() == 0 && format->shadowDistanceY() == 0 )
        return;
    // 1) Sort out the color
    QColor textColor( drawingShadow ? format->shadowColor() : format->color() );
    if ( !textColor.isValid() ) // Resolve the color at this point
        textColor = KoTextFormat::defaultTextColor( &painter );

    // 2) Sort out the font
    QFont font( format->screenFont( zh ) );
    if ( format->attributeFont() == KoTextFormat::ATT_SMALL_CAPS && s[start].upper() != s[start] )
        font = format->smallCapsFont( zh, true );

#if 0
    QFontInfo fi( font );
    kDebug(32500) << "KoTextParag::drawParagStringInternal requested font " << font.pointSizeF() << " using font " << fi.pointSize() << "pt (format font: " << format->font().pointSizeFloat() << "pt)" << endl;
    QFontMetrics fm( font );
    kDebug(32500) << "Real font: " << fi.family() << ". Font height in pixels: " << fm.height() << endl;
#endif

    // 3) Paint
    QString str( s );
    if ( str[ (int)str.length() - 1 ].unicode() == 0xad )
        str.remove( str.length() - 1, 1 );
    painter.setPen( QPen( textColor ) );
    painter.setFont( font );

    KoTextDocument* doc = document();

    if ( drawSelections ) {
	const int nSels = doc ? doc->numSelections() : 1;
	for ( int j = 0; j < nSels; ++j ) {
	    if ( start >= selectionStarts[ j ] && start < selectionEnds[ j ] ) {
		if ( !doc || doc->invertSelectionText( j ) )
		    textColor = cg.color( QColorGroup::HighlightedText );
		    painter.setPen( QPen( textColor ) );
                    break;
            }
        }
    }

    Qt::LayoutDirection dir = rightToLeft ? Qt::RightToLeft : Qt::LeftToRight;

    if ( dir != Qt::RightToLeft && start + len == length() ) // don't draw the last character (trailing space)
    {
       len--;
       if ( len <= 0 )
           return;
       bw-=at(length()-1)->pixelwidth;
    }
    KoTextParag::drawFontEffects( &painter, format, zh, font, textColor, startX, baseLine, bw, lastY, h, str[start] );

    if ( str[ start ] != '\t' && str[ start ].unicode() != 0xad ) {
        str = format->displayedString( str ); // #### This converts the whole string, instead of from start to start+len!
	if ( format->vAlign() == KoTextFormat::AlignNormal ) {
            int posY = lastY + baseLine;
            //we must move to bottom text because we create
            //shadow to 'top'.
            int sy = format->shadowY( zh );
            if ( sy < 0)
                posY -= sy;
	    // TODO RTL painter.drawText( startX, posY, str, start, len, dir );
            painter.drawText( startX, posY, str, start, len );
#ifdef BIDI_DEBUG
	    painter.save();
	    painter.setPen ( Qt::red );
	    painter.drawLine( startX, lastY, startX, lastY + baseLine );
	    painter.drawLine( startX, lastY + baseLine/2, startX + 10, lastY + baseLine/2 );
	    int w = 0;
	    int i = 0;
	    while( i < len )
		w += painter.fontMetrics().charWidth( str, start + i++ );
	    painter.setPen ( Qt::blue );
	    painter.drawLine( startX + w - 1, lastY, startX + w - 1, lastY + baseLine );
	    painter.drawLine( startX + w - 1, lastY + baseLine/2, startX + w - 1 - 10, lastY + baseLine/2 );
	    painter.restore();
#endif
	} else if ( format->vAlign() == KoTextFormat::AlignSuperScript ) {
            int posY =lastY + baseLine - ( painter.fontMetrics().height() / 2 );
            //we must move to bottom text because we create
            //shadow to 'top'.
            int sy = format->shadowY( zh );
            if ( sy < 0)
                posY -= sy;
	    painter.drawText( startX, posY, str, start, len /*TODO , dir*/ );
	} else if ( format->vAlign() == KoTextFormat::AlignSubScript ) {
            int posY =lastY + baseLine + ( painter.fontMetrics().height() / 6 );
            //we must move to bottom text because we create
            //shadow to 'top'.
            int sy = format->shadowY( zh );
            if ( sy < 0)
                posY -= sy;
	    painter.drawText( startX, posY, str, start, len /*TODO , dir*/ );
	} else if ( format->vAlign() == KoTextFormat::AlignCustom ) {
            int posY = lastY + baseLine - format->offsetFromBaseLine();
            //we must move to bottom text because we create
            //shadow to 'top'.
            int sy = format->shadowY( zh );
            if ( sy < 0)
                posY -= sy;
	    painter.drawText( startX, posY, str, start, len /*TODO , dir */ );
	}
    }
    if ( str[ start ] == '\t' && m_tabCache.contains( start ) ) {
	painter.save();
	KoTextZoomHandler * zh = textDocument()->paintingZoomHandler();
	const KoTabulator& tab = m_layout.tabList()[ m_tabCache[ start ] ];
	int lineWidth = zh->zoomItYOld( tab.ptWidth );
	switch ( tab.filling ) {
	    case TF_DOTS:
		painter.setPen( QPen( textColor, lineWidth, Qt::DotLine ) );
		painter.drawLine( startX, lastY + baseLine, startX + bw, lastY + baseLine );
		break;
	    case TF_LINE:
		painter.setPen( QPen( textColor, lineWidth, Qt::SolidLine ) );
		painter.drawLine( startX, lastY + baseLine, startX + bw, lastY + baseLine );
                break;
            case TF_DASH:
		painter.setPen( QPen( textColor, lineWidth, Qt::DashLine ) );
		painter.drawLine( startX, lastY + baseLine, startX + bw, lastY + baseLine );
		break;
            case TF_DASH_DOT:
		painter.setPen( QPen( textColor, lineWidth, Qt::DashDotLine ) );
		painter.drawLine( startX, lastY + baseLine, startX + bw, lastY + baseLine );
		break;
            case TF_DASH_DOT_DOT:
		painter.setPen( QPen( textColor, lineWidth, Qt::DashDotDotLine ) );
		painter.drawLine( startX, lastY + baseLine, startX + bw, lastY + baseLine );
		break;

            default:
                break;
	}
	painter.restore();
    }

    if ( start+len < length() && at( start+len )->lineStart )
    {
#ifdef DEBUG_PAINT
        //kDebug(32500) << "we are drawing the end of line " << line << ". Auto-hyphenated: " << lineHyphenated( line ) << endl;
#endif
        bool drawHyphen = at( start+len-1 )->c.unicode() == 0xad;
        drawHyphen = drawHyphen || lineHyphenated( line );
        if ( drawHyphen ) {
#ifdef DEBUG_PAINT
            kDebug(32500) << "drawing hyphen at x=" << startX+bw << endl;
#endif
            painter.drawText( startX + bw, lastY + baseLine, "-" ); // \xad gives squares with some fonts (!?)
        }
    }

    // Paint a zigzag line for "wrong" background spellchecking checked words:
    if(
		painter.device()->devType() != QInternal::Printer &&
		format->isMisspelled() &&
		!drawingShadow &&
		textDocument()->drawingMissingSpellLine() )
	{
		painter.save();
		painter.setPen( QPen( Qt::red, 1 ) );

		// Draw 3 pixel lines with increasing offset and distance 4:
		for( int zigzag_line = 0; zigzag_line < 3; ++zigzag_line )
		{
			for( int zigzag_x = zigzag_line; zigzag_x < bw; zigzag_x += 4 )
			{
				painter.drawPoint(
					startX + zigzag_x,
					lastY + baseLine + h/12 - 1 + zigzag_line );
			}
		}

		// "Double" the pixel number for the middle line:
		for( int zigzag_x = 3; zigzag_x < bw; zigzag_x += 4 )
		{
			painter.drawPoint(
				startX + zigzag_x,
				lastY + baseLine + h/12 );
		}

		painter.restore();
	}
}

bool KoTextParag::lineHyphenated( int l ) const
{
    if ( l > (int)lineStarts.count() - 1 ) {
	kWarning() << "KoTextParag::lineHyphenated: line " << l << " out of range!" << endl;
	return false;
    }

    if ( !isValid() )
	const_cast<KoTextParag*>(this)->format();

    QMap<int, KoTextParagLineStart*>::ConstIterator it = lineStarts.begin();
    while ( l-- > 0 )
	++it;
    return ( *it )->hyphenated;
}

/** Draw the cursor mark. Reimplemented from KoTextParag to convert coordinates first. */
void KoTextParag::drawCursor( QPainter &painter, KoTextCursor *cursor, int curx, int cury, int curh, const QColorGroup &cg )
{
    KoTextZoomHandler * zh = textDocument()->paintingZoomHandler();
    int x = zh->layoutUnitToPixelX( curx ) /*+ cursor->parag()->at( cursor->index() )->pixelxadj*/;
    //kDebug(32500) << "  drawCursor: LU: [cur]x=" << curx << ", cury=" << cury << " -> PIX: x=" << x << ", y=" << zh->layoutUnitToPixelY( cury ) << endl;
    KoTextParag::drawCursorDefault( painter, cursor, x,
                            zh->layoutUnitToPixelY( cury ),
                            zh->layoutUnitToPixelY( cury, curh ), cg );
}

// Reimplemented from KoTextParag
void KoTextParag::copyParagData( KoTextParag *parag )
{
    // Style of the previous paragraph
    KoParagStyle * style = parag->style();
    // Obey "following style" setting
    bool styleApplied = false;
    if ( style )
    {
        KoParagStyle * newStyle = style->followingStyle();
        if ( newStyle && style != newStyle ) // if same style, keep paragraph-specific changes as usual
        {
            setParagLayout( newStyle->paragLayout() );
            KoTextFormat * format = &newStyle->format();
            setFormat( format );
            format->addRef();
            str->setFormat( 0, format, true ); // prepare format for text insertion
            styleApplied = true;
        }
    }
    // This should never happen in KWord, but it happens in KPresenter
    //else
    //    kWarning() << "Paragraph has no style " << paragId() << endl;

    // No "following style" setting, or same style -> copy layout & format of previous paragraph
    if (!styleApplied)
    {
        setParagLayout( parag->paragLayout() );
        // Remove pagebreak flags from initial parag - they got copied to the new parag
        parag->m_layout.pageBreaking &= ~KoParagLayout::HardFrameBreakBefore;
        parag->m_layout.pageBreaking &= ~KoParagLayout::HardFrameBreakAfter;
        // Remove footnote counter text from second parag
        if ( m_layout.counter && m_layout.counter->numbering() == KoParagCounter::NUM_FOOTNOTE )
            setNoCounter();
        // Do not copy 'restart numbering at this paragraph' option (would be silly)
        if ( m_layout.counter )
            m_layout.counter->setRestartCounter(false);

        // set parag format to the format of the trailing space of the previous parag
        setFormat( parag->at( parag->length()-1 )->format() );
        // KoTextCursor::splitAndInsertEmptyParag takes care of setting the format
        // for the chars in the new parag
    }

    // Note: we don't call the original KoTextParag::copyParagData on purpose.
    // We don't want setListStyle to get called - it ruins our stylesheetitems
    // And we don't care about copying the stylesheetitems directly,
    // applying the parag layout will create them
}

void KoTextParag::setTabList( const KoTabulatorList &tabList )
{
    KoTabulatorList lst( tabList );
    m_layout.setTabList( lst );
    if ( !tabList.isEmpty() )
    {
        KoTextZoomHandler* zh = textDocument()->formattingZoomHandler();
        int * tabs = new int[ tabList.count() + 1 ]; // will be deleted by ~KoTextParag
        KoTabulatorList::Iterator it = lst.begin();
        unsigned int i = 0;
        for ( ; it != lst.end() ; ++it, ++i )
            tabs[i] = zh->ptToLayoutUnitPixX( (*it).ptPos );
        tabs[i] = 0;
        assert( i == tabList.count() );
        setTabArray( tabs );
    } else
    {
        setTabArray( 0 );
    }
    invalidate( 0 );
}

/** "Reimplemented" (compared to nextTabDefault) to implement non-left-aligned tabs */
int KoTextParag::nextTab( int chnum, int x, int availableWidth )
{
    if ( !m_layout.tabList().isEmpty() )
    {
        // Fetch the zoomed and sorted tab positions from KoTextParag
        // We stored them there for faster access
        int * tArray = tabArray();
        int i = 0;
        if ( str->isRightToLeft() )
            i = m_layout.tabList().size() - 1;
        KoTextZoomHandler* zh = textDocument()->formattingZoomHandler();

        while ( i >= 0 && i < (int)m_layout.tabList().size() ) {
            //kDebug(32500) << "KoTextParag::nextTab tArray[" << i << "]=" << tArray[i] << " type " << m_layout.tabList()[i].type << endl;
            int tab = tArray[ i ];

            // If a right-aligned tab is after the right edge then assume
            // that it -is- on the right edge, otherwise the last letters will fall off.
            // This is compatible with OOo's behavior.
            if ( tab > availableWidth ) {
                //kDebug(32500) << "Tab position adjusted to availableWidth=" << availableWidth << endl;
                tab = availableWidth;
            }

            if ( str->isRightToLeft() )
                tab = availableWidth - tab;

            if ( tab > x ) {
                int type = m_layout.tabList()[i].type;

                // fix the tab type for right to left text
                if ( str->isRightToLeft() )
                    if ( type == T_RIGHT )
                        type = T_LEFT;
                    else if ( type == T_LEFT )
                        type = T_RIGHT;

                switch ( type ) {
                case T_RIGHT:
                case T_CENTER:
                {
                    // Look for the next tab (or EOL)
                    int c = chnum + 1;
                    int w = 0;
                    while ( c < str->length() - 1 && str->at( c ).c != '\t' && str->at( c ).c != '\n' )
                    {
                        KoTextStringChar & ch = str->at( c );
                        // Determine char width
                        // This must be done in the same way as in KoTextFormatter::format() or there can be different rounding errors.
                        if ( ch.isCustom() )
                            w += ch.customItem()->width;
                        else
                        {
                            KoTextFormat *charFormat = ch.format();
                            int ww = charFormat->charWidth( zh, false, &ch, this, c );
                            ww = KoTextZoomHandler::ptToLayoutUnitPt( ww );
                            w += ww;
                        }
                        ++c;
                    }

                    m_tabCache[chnum] = i;

                    if ( type == T_RIGHT )
                        return tab - w;
                    else // T_CENTER
                        return tab - w/2;
                }
                case T_DEC_PNT:
                {
                    // Look for the next tab (or EOL), and for alignChar
                    // Default to right-aligned if no decimal point found (behavior from msword)
                    int c = chnum + 1;
                    int w = 0;
                    while ( c < str->length()-1 && str->at( c ).c != '\t' && str->at( c ).c != '\n' )
                    {
                        KoTextStringChar & ch = str->at( c );
                        if ( ch.c == m_layout.tabList()[i].alignChar )
                        {
                            // Can't use ch.width yet, since the formatter hasn't run over those chars
                            int ww = ch.format()->charWidth( zh, false, &ch, this, c );
                            ww = KoTextZoomHandler::ptToLayoutUnitPt( ww );
                            if ( str->isRightToLeft() )
                            {
                                w = ww / 2; // center around the decimal point
                                ++c;
                                continue;
                            }
                            else
                            {
                                w += ww / 2; // center around the decimal point
                                break;
                            }
                        }

                        // Determine char width
                        if ( ch.isCustom() )
                            w += ch.customItem()->width;
                        else
                        {
                            int ww = ch.format()->charWidth( zh, false, &ch, this, c );
                            w += KoTextZoomHandler::ptToLayoutUnitPt( ww );
                        }

                        ++c;
                    }
                    m_tabCache[chnum] = i;
                    return tab - w;
                }
                default: // case T_LEFT:
                    m_tabCache[chnum] = i;
                    return tab;
                }
            }
            if ( str->isRightToLeft() )
                --i;
            else
                ++i;
        }
    }
    // No tab list, use tab-stop-width. qrichtext.cpp has the code :)
    return KoTextParag::nextTabDefault( chnum, x );
}

void KoTextParag::applyStyle( KoParagStyle *style )
{
    setParagLayout( style->paragLayout() );
    KoTextFormat *newFormat = &style->format();
    setFormat( 0, str->length(), newFormat );
    setFormat( newFormat );
}

void KoTextParag::setParagLayout( const KoParagLayout & layout, int flags, int marginIndex )
{
    //kDebug(32500) << "KoTextParag::setParagLayout flags=" << flags << endl;
    if ( flags & KoParagLayout::Alignment )
        setAlign( layout.alignment );
    if ( flags & KoParagLayout::Margins ) {
        if ( marginIndex == -1 )
            setMargins( layout.margins );
        else
            setMargin( (Q3StyleSheetItem::Margin)marginIndex, layout.margins[marginIndex] );
    }
    if ( flags & KoParagLayout::LineSpacing )
    {
        setLineSpacingType( layout.lineSpacingType );
        setLineSpacing( layout.lineSpacingValue() );
    }
    if ( flags & KoParagLayout::Borders )
    {
        setLeftBorder( layout.leftBorder );
        setRightBorder( layout.rightBorder );
        setTopBorder( layout.topBorder );
        setBottomBorder( layout.bottomBorder );
        setJoinBorder( layout.joinBorder );
    }
    if ( flags & KoParagLayout::BackgroundColor )
    {
        setBackgroundColor( layout.backgroundColor );
    }
    if ( flags & KoParagLayout::BulletNumber )
        setCounter( layout.counter );
    if ( flags & KoParagLayout::Tabulator )
        setTabList( layout.tabList() );
    if ( flags == KoParagLayout::All )
    {
        setDirection( static_cast<QChar::Direction>(layout.direction) );
        // Don't call applyStyle from here, it would overwrite any paragraph-specific settings
        setStyle( layout.style );
    }
}

void KoTextParag::setCustomItem( int index, KoTextCustomItem * custom, KoTextFormat * currentFormat )
{
    //kDebug(32500) << "KoTextParag::setCustomItem " << index << "  " << (void*)custom
    //               << "  currentFormat=" << (void*)currentFormat << endl;
    if ( currentFormat )
        setFormat( index, 1, currentFormat );
    at( index )->setCustomItem( custom );
    //addCustomItem();
    document()->registerCustomItem( custom, this );
    custom->recalc(); // calc value (e.g. for variables) and set initial size
    invalidate( 0 );
    setChanged( true );
}

void KoTextParag::removeCustomItem( int index )
{
    Q_ASSERT( at( index )->isCustom() );
    KoTextCustomItem * item = at( index )->customItem();
    at( index )->loseCustomItem();
    //KoTextParag::removeCustomItem();
    document()->unregisterCustomItem( item, this );
}


int KoTextParag::findCustomItem( const KoTextCustomItem * custom ) const
{
    int len = str->length();
    for ( int i = 0; i < len; ++i )
    {
        KoTextStringChar & ch = str->at(i);
        if ( ch.isCustom() && ch.customItem() == custom )
            return i;
    }
    kWarning() << "KoTextParag::findCustomItem custom item " << (void*)custom
              << " not found in paragraph " << paragId() << endl;
    return 0;
}

#ifndef NDEBUG
void KoTextParag::printRTDebug( int info )
{
    QString specialFlags;
    if ( str->needsSpellCheck() )
        specialFlags += " needsSpellCheck=true";
    if ( wasMovedDown() )
        specialFlags += " wasMovedDown=true";
    if ( partOfTableOfContents() )
        specialFlags += " part-of-TOC=true";
    kDebug(32500) << "Paragraph " << this << " (" << paragId() << ") [changed="
              << hasChanged() << ", valid=" << isValid()
              << specialFlags
              << "] ------------------ " << endl;
    if ( prev() && prev()->paragId() + 1 != paragId() )
        kWarning() << "  Previous paragraph " << prev() << " has ID " << prev()->paragId() << endl;
    if ( next() && next()->paragId() != paragId() + 1 )
        kWarning() << "  Next paragraph " << next() << " has ID " << next()->paragId() << endl;
    //if ( !next() )
    //    kDebug(32500) << "  next is 0L" << endl;
    kDebug(32500) << "  Style: " << style() << " " << ( style() ? style()->name().toLocal8Bit().data() : "NO STYLE" ) << endl;
    kDebug(32500) << "  Text: '" << str->toString() << "'" << endl;
    if ( info == 0 ) // paragraph info
    {
        if ( m_layout.counter )
        {
            m_layout.counter->printRTDebug( this );
        }
        static const char * const s_align[] = { "Auto", "Left", "Right", "ERROR", "HCenter", "ERR", "ERR", "ERR", "Justify", };
        static const char * const s_linespacing[] = { "Single", "1.5", "2", "custom", "atLeast", "Multiple", "Fixed" };
        static const char * const s_dir[] = { "DirL", "DirR", "DirEN", "DirES", "DirET", "DirAN", "DirCS", "DirB", "DirS", "DirWS", "DirON", "DirLRE", "DirLRO", "DirAL", "DirRLE", "DirRLO", "DirPDF", "DirNSM", "DirBN" };
        kDebug(32500) << "  align: " << s_align[alignment()] << "  resolveAlignment: " << s_align[resolveAlignment()]
                  << "  isRTL:" << str->isRightToLeft()
                  << "  dir: " << s_dir[direction()] << endl;
        QRect pixr = pixelRect( textDocument()->paintingZoomHandler() );
        kDebug(32500) << "  rect() : " << DEBUGRECT( rect() )
                  << "  pixelRect() : " << DEBUGRECT( pixr ) << endl;
        kDebug(32500) << "  topMargin()=" << topMargin() << " bottomMargin()=" << bottomMargin()
                  << " leftMargin()=" << leftMargin() << " firstLineMargin()=" << firstLineMargin()
                  << " rightMargin()=" << rightMargin() << endl;
        if ( kwLineSpacingType() != KoParagLayout::LS_SINGLE )
            kDebug(32500) << "  linespacing type=" << s_linespacing[ -kwLineSpacingType() ]
                           << " value=" << kwLineSpacing() << endl;
        const int pageBreaking = m_layout.pageBreaking;
        QStringList pageBreakingFlags;
        if ( pageBreaking & KoParagLayout::KeepLinesTogether )
            pageBreakingFlags.append( "KeepLinesTogether" );
        if ( pageBreaking & KoParagLayout::HardFrameBreakBefore )
            pageBreakingFlags.append( "HardFrameBreakBefore" );
        if ( pageBreaking & KoParagLayout::HardFrameBreakAfter )
            pageBreakingFlags.append( "HardFrameBreakAfter" );
        if ( pageBreaking & KoParagLayout::KeepWithPrevious )
            pageBreakingFlags.append( "KeepWithPrevious" );
        if ( pageBreaking & KoParagLayout::KeepWithNext )
            pageBreakingFlags.append( "KeepWithNext" );
        if ( !pageBreakingFlags.isEmpty() )
            kDebug(32500) << " page Breaking: " << pageBreakingFlags.join(",") << endl;

        static const char * const tabtype[] = { "T_LEFT", "T_CENTER", "T_RIGHT", "T_DEC_PNT", "error!!!" };
        KoTabulatorList tabList = m_layout.tabList();
        if ( tabList.isEmpty() ) {
            if ( str->toString().find( '\t' ) != -1 )
                kDebug(32500) << "Tab width: " << textDocument()->tabStopWidth() << endl;
        } else {
            KoTabulatorList::Iterator it = tabList.begin();
            for ( ; it != tabList.end() ; it++ )
                kDebug(32500) << "Tab type:" << tabtype[(*it).type] << " at: " << (*it).ptPos << endl;
        }
    } else if ( info == 1 ) // formatting info
    {
        kDebug(32500) << "  Paragraph format=" << paragFormat() << " " << paragFormat()->key()
                  << " fontsize:" << dynamic_cast<KoTextFormat *>(paragFormat())->pointSize() << endl;

        for ( int line = 0 ; line < lines(); ++ line ) {
            int y, h, baseLine;
            lineInfo( line, y, h, baseLine );
            int startOfLine;
            lineStartOfLine( line, &startOfLine );
            kDebug(32500) << "  Line " << line << " y=" << y << " height=" << h << " baseLine=" << baseLine << " startOfLine(index)=" << startOfLine << endl;
        }
        kDebug(32500) << endl;
        KoTextString * s = string();
        int lastX = 0; // pixels
        int lastW = 0; // pixels
        for ( int i = 0 ; i < s->length() ; ++i )
        {
            KoTextStringChar & ch = s->at(i);
            int pixelx =  textDocument()->formattingZoomHandler()->layoutUnitToPixelX( ch.x )
                          + ch.pixelxadj;
            if ( ch.lineStart )
                kDebug(32500) << "LINESTART" << endl;
            QString attrs = " ";
            if ( ch.whiteSpace )
                attrs += "whitespace ";
            if ( !ch.charStop )
                attrs += "notCharStop ";
            if ( ch.wordStop )
                attrs += "wordStop ";
            attrs.truncate( attrs.length() - 1 );

            kDebug(32500) << i << ": '" << QString(ch.c).rightJustified(2)
                           << "' (" << QString::number( ch.c.unicode() ).rightJustified(3) << ")"
                      << " x(LU)=" << ch.x
                      << " w(LU)=" << ch.width//s->width(i)
                      << " x(PIX)=" << pixelx
                      << " (xadj=" << + ch.pixelxadj << ")"
                      << " w(PIX)=" << ch.pixelwidth
                      << " height=" << ch.height()
                      << attrs
                //      << " format=" << ch.format()
                //      << " \"" << ch.format()->key() << "\" "
                //<< " fontsize:" << dynamic_cast<KoTextFormat *>(ch.format())->pointSize()
                      << endl;

	    // Check that the format is in the collection (i.e. its defaultFormat or in the dict)
	    if ( ch.format() != textDocument()->formatCollection()->defaultFormat() )
                Q_ASSERT( textDocument()->formatCollection()->dict()[ch.format()->key()] );

            if ( !str->isBidi() && !ch.lineStart )
                Q_ASSERT( lastX + lastW == pixelx ); // looks like some rounding problem with justified spaces
            lastX = pixelx;
            lastW = ch.pixelwidth;
            if ( ch.isCustom() )
            {
                KoTextCustomItem * item = ch.customItem();
                kDebug(32500) << " - custom item " << item
                          << " ownline=" << item->ownLine()
                          << " size=" << item->width << "x" << item->height
                          << " ascent=" << item->ascent()
                          << endl;
            }
        }
    }
}
#endif

void KoTextParag::drawFontEffects( QPainter * p, KoTextFormat *format, KoTextZoomHandler *zh, QFont font, const QColor & color, int startX, int baseLine, int bw, int lastY, int /*h*/, QChar firstChar )
{
    // This is about drawing underlines and strikeouts
    // So abort immediately if there's none to draw.
    if ( !format->isStrikedOrUnderlined() )
        return;
    //kDebug(32500) << "drawFontEffects wordByWord=" << format->wordByWord() <<
    //    " firstChar='" << QString(firstChar) << "'" << endl;
    // paintLines ensures that we're called word by word if wordByWord is true.
    if ( format->wordByWord() && firstChar.isSpace() )
        return;

    double dimd;
    int y;
    int offset = 0;
    if (format->vAlign() == KoTextFormat::AlignSubScript )
        offset = p->fontMetrics().height() / 6;
    else if (format->vAlign() == KoTextFormat::AlignSuperScript )
        offset = -p->fontMetrics().height() / 2;

    dimd = KoBorder::zoomWidthY( format->underLineWidth(), zh, 1 );
    if((format->vAlign() == KoTextFormat::AlignSuperScript) ||
	(format->vAlign() == KoTextFormat::AlignSubScript ) || (format->vAlign() == KoTextFormat::AlignCustom ))
	dimd*=format->relativeTextSize();
    y = lastY + baseLine + offset - ( (format->vAlign() == KoTextFormat::AlignCustom)?format->offsetFromBaseLine():0 );

    if ( format->doubleUnderline())
    {
        QColor col = format->textUnderlineColor().isValid() ? format->textUnderlineColor(): color ;
	int dim=static_cast<int>(0.75*dimd);
	dim=dim?dim:1; //width of line should be at least 1
        p->save();

        switch( format->underlineStyle())
        {
        case KoTextFormat::U_SOLID:
            p->setPen( QPen( col, dim, Qt::SolidLine ) );
            break;
        case KoTextFormat::U_DASH:
            p->setPen( QPen( col, dim, Qt::DashLine ) );
            break;
        case KoTextFormat::U_DOT:
            p->setPen( QPen( col, dim, Qt::DotLine ) );
            break;
        case KoTextFormat::U_DASH_DOT:
            p->setPen( QPen( col, dim, Qt::DashDotLine ) );
            break;
        case KoTextFormat::U_DASH_DOT_DOT:
            p->setPen( QPen( col, dim, Qt::DashDotDotLine ) );
            break;
        default:
            p->setPen( QPen( color, dim, Qt::SolidLine ) );
        }

        y += static_cast<int>(1.125*dimd); // slightly under the baseline if possible
        p->drawLine( startX, y, startX + bw, y );
        y += static_cast<int>(1.5*dimd);
        p->drawLine( startX, y, startX + bw, y );
        p->restore();
        if ( font.underline() ) { // can this happen?
            font.setUnderline( FALSE );
            p->setFont( font );
        }
    }
    else if ( format->underline() ||
                format->underlineType() == KoTextFormat::U_SIMPLE_BOLD)
    {

        QColor col = format->textUnderlineColor().isValid() ? format->textUnderlineColor(): color ;
        p->save();
	int dim=(format->underlineType() == KoTextFormat::U_SIMPLE_BOLD)?static_cast<int>(2*dimd):static_cast<int>(dimd);
	dim=dim?dim:1; //width of line should be at least 1
        y += static_cast<int>(1.875*dimd);

        switch( format->underlineStyle() )
        {
        case KoTextFormat::U_SOLID:
            p->setPen( QPen( col, dim, Qt::SolidLine ) );
            break;
        case KoTextFormat::U_DASH:
            p->setPen( QPen( col, dim, Qt::DashLine ) );
            break;
        case KoTextFormat::U_DOT:
            p->setPen( QPen( col, dim, Qt::DotLine ) );
            break;
        case KoTextFormat::U_DASH_DOT:
            p->setPen( QPen( col, dim, Qt::DashDotLine ) );
            break;
        case KoTextFormat::U_DASH_DOT_DOT:
            p->setPen( QPen( col, dim, Qt::DashDotDotLine ) );
            break;
        default:
            p->setPen( QPen( col, dim, Qt::SolidLine ) );
        }

        p->drawLine( startX, y, startX + bw, y );
        p->restore();
        font.setUnderline( FALSE );
        p->setFont( font );
    }
    else if ( format->waveUnderline() )
    {
	int dim=static_cast<int>(dimd);
	dim=dim?dim:1; //width of line should be at least 1
        y += dim;
        QColor col = format->textUnderlineColor().isValid() ? format->textUnderlineColor(): color ;
        p->save();
	int offset = 2 * dim;
	QPen pen(col, dim, Qt::SolidLine);
	pen.setCapStyle(Qt::RoundCap);
	p->setPen(pen);
	Q_ASSERT(offset);
	double anc=acos(1.0-2*(static_cast<double>(offset-(startX)%offset)/static_cast<double>(offset)))/3.1415*180;
	int pos=1;
	//set starting position
	if(2*((startX/offset)/2)==startX/offset)
	    pos*=-1;
	//draw first part of wave
	p->drawArc( (startX/offset)*offset, y, offset, offset, 0, -qRound(pos*anc*16) );
        //now the main part
	int zigzag_x = (startX/offset+1)*offset;
	for ( ; zigzag_x + offset <= bw+startX; zigzag_x += offset)
        {
	    p->drawArc( zigzag_x, y, offset, offset, 0, pos*180*16 );
	    pos*=-1;
        }
	//and here we finish
	anc=acos(1.0-2*(static_cast<double>((startX+bw)%offset)/static_cast<double>(offset)))/3.1415*180;
	p->drawArc( zigzag_x, y, offset, offset, 180*16, -qRound(pos*anc*16) );
	p->restore();
        font.setUnderline( FALSE );
        p->setFont( font );
    }

    dimd = KoBorder::zoomWidthY( static_cast<double>(format->pointSize())/18.0, zh, 1 );
    if((format->vAlign() == KoTextFormat::AlignSuperScript) ||
	(format->vAlign() == KoTextFormat::AlignSubScript ) || (format->vAlign() == KoTextFormat::AlignCustom ))
	dimd*=format->relativeTextSize();
    y = lastY + baseLine + offset - ( (format->vAlign() == KoTextFormat::AlignCustom)?format->offsetFromBaseLine():0 );

    if ( format->strikeOutType() == KoTextFormat::S_SIMPLE
         || format->strikeOutType() == KoTextFormat::S_SIMPLE_BOLD)
    {
        unsigned int dim = (format->strikeOutType() == KoTextFormat::S_SIMPLE_BOLD)? static_cast<int>(2*dimd) : static_cast<int>(dimd);
        p->save();

        switch( format->strikeOutStyle() )
        {
        case KoTextFormat::S_SOLID:
            p->setPen( QPen( color, dim, Qt::SolidLine ) );
            break;
        case KoTextFormat::S_DASH:
            p->setPen( QPen( color, dim, Qt::DashLine ) );
            break;
        case KoTextFormat::S_DOT:
            p->setPen( QPen( color, dim, Qt::DotLine ) );
            break;
        case KoTextFormat::S_DASH_DOT:
            p->setPen( QPen( color, dim, Qt::DashDotLine ) );
            break;
        case KoTextFormat::S_DASH_DOT_DOT:
            p->setPen( QPen( color, dim, Qt::DashDotDotLine ) );
            break;
        default:
            p->setPen( QPen( color, dim, Qt::SolidLine ) );
        }

        y -= static_cast<int>(5*dimd);
        p->drawLine( startX, y, startX + bw, y );
        p->restore();
        font.setStrikeOut( FALSE );
        p->setFont( font );
    }
    else if ( format->strikeOutType() == KoTextFormat::S_DOUBLE )
    {
        unsigned int dim = static_cast<int>(dimd);
        p->save();

        switch( format->strikeOutStyle() )
        {
        case KoTextFormat::S_SOLID:
            p->setPen( QPen( color, dim, Qt::SolidLine ) );
            break;
        case KoTextFormat::S_DASH:
            p->setPen( QPen( color, dim, Qt::DashLine ) );
            break;
        case KoTextFormat::S_DOT:
            p->setPen( QPen( color, dim, Qt::DotLine ) );
            break;
        case KoTextFormat::S_DASH_DOT:
            p->setPen( QPen( color, dim, Qt::DashDotLine ) );
            break;
        case KoTextFormat::S_DASH_DOT_DOT:
            p->setPen( QPen( color, dim, Qt::DashDotDotLine ) );
            break;
        default:
            p->setPen( QPen( color, dim, Qt::SolidLine ) );
        }

	y -= static_cast<int>(4*dimd);
        p->drawLine( startX, y, startX + bw, y);
	y -= static_cast<int>(2*dimd);
        p->drawLine( startX, y, startX + bw, y);
        p->restore();
        font.setStrikeOut( FALSE );
        p->setFont( font );
    }

}

// ### is this method correct for RightToLeft text?
QString KoTextParag::toString( int from, int length ) const
{
    QString str;
    if ( from == 0 && m_layout.counter && m_layout.counter->numbering() != KoParagCounter::NUM_FOOTNOTE )
        str += m_layout.counter->text( this ) + ' ';
    if ( length == -1 )
        length = this->length() - 1 /*trailing space*/ - from;
    for ( int i = from ; i < (length+from) ; ++i )
    {
        KoTextStringChar *ch = at( i );
        if ( ch->isCustom() )
        {
            KoVariable * var = dynamic_cast<KoVariable *>(ch->customItem());
            if ( var )
                str += var->text(true);
            else //frame inline
                str +=' ';
        }
        else
            str += ch->c;
    }
    return str;
}

void KoTextParag::loadOasisSpan( const QDomElement& parent, KoOasisContext& context, uint& pos )
{
    // Parse every child node of the parent
    // Can't use forEachElement here since we also care about text nodes
    QDomNode node;
    for ( node = parent.firstChild(); !node.isNull(); node = node.nextSibling() )
    {
        QDomElement ts = node.toElement();
        QString textData;
        const QString localName( ts.localName() );
        const bool isTextNS = ts.namespaceURI() == KoXmlNS::text;
        KoTextCustomItem* customItem = 0;

        // allow loadSpanTag to modify the stylestack
        context.styleStack().save();

        // Try to keep the order of the tag names by probability of happening
        if ( node.isText() )
        {
            textData = node.toText().data();
        }
        else if ( isTextNS && localName == "span" ) // text:span
        {
            context.styleStack().save();
            context.fillStyleStack( ts, KoXmlNS::text, "style-name", "text" );
            loadOasisSpan( ts, context, pos ); // recurse
            context.styleStack().restore();
        }
        else if ( isTextNS && localName == "s" ) // text:s
        {
            int howmany = 1;
            if (ts.hasAttributeNS( KoXmlNS::text, "c"))
                howmany = ts.attributeNS( KoXmlNS::text, "c", QString::null).toInt();

            textData.fill(32, howmany);
        }
        else if ( isTextNS && localName == "tab" ) // text:tab (it's tab-stop in OO-1.1 but tab in oasis)
        {
            textData = '\t';
        }
        else if ( isTextNS && localName == "line-break" ) // text:line-break
        {
            textData = '\n';
        }
        else if ( isTextNS && localName == "number" ) // text:number
        {
            // This is the number in front of a numbered paragraph,
            // written out to help export filters. We can ignore it.
        }
        else if ( node.isProcessingInstruction() )
        {
            QDomProcessingInstruction pi = node.toProcessingInstruction();
            if ( pi.target() == "opendocument" && pi.data().startsWith( "cursor-position" ) )
            {
                context.setCursorPosition( this, pos );
            }
        }
        else
        {
            bool handled = false;
            // Check if it's a variable
            KoVariable* var = context.variableCollection().loadOasisField( textDocument(), ts, context );
            if ( var )
            {
                textData = "#";     // field placeholder
                customItem = var;
                handled = true;
            }
            if ( !handled )
            {
                handled = textDocument()->loadSpanTag( ts, context,
                                                       this, pos,
                                                       textData, customItem );
                if ( !handled )
                {
                    kWarning(32500) << "Ignoring tag " << ts.tagName() << endl;
                    context.styleStack().restore();
                    continue;
                }
            }
        }

        const uint length = textData.length();
        if ( length )
        {
            insert( pos, textData );
            if ( customItem )
                setCustomItem( pos, customItem, 0 );
            KoTextFormat f;
            f.load( context );
            //kDebug(32500) << "loadOasisSpan: applying formatting from " << pos << " to " << pos+length << "\n   format=" << f.key() << endl;
            setFormat( pos, length, document()->formatCollection()->format( &f ), TRUE );
            pos += length;
        }
        context.styleStack().restore();
    }
}

KoParagLayout KoTextParag::loadParagLayout( KoOasisContext& context, KoStyleCollection *styleCollection, bool findStyle )
{
    KoParagLayout layout;

    // Only when loading paragraphs, not when loading styles
    if ( findStyle )
    {
        KoParagStyle *style;
        // Name of the style. If there is no style, then we do not supply
        // any default!
        QString styleName = context.styleStack().userStyleName( "paragraph" );
        if ( !styleName.isEmpty() )
        {
            style = styleCollection->findStyle( styleName );
            // When pasting the style names are random, the display names matter
            if (!style)
                style = styleCollection->findStyleByDisplayName( context.styleStack().userStyleDisplayName( "paragraph" ) );
            if (!style)
            {
                kError(32500) << "Cannot find style \"" << styleName << "\" - using Standard" << endl;
                style = styleCollection->findStyle( "Standard" );
            }
            //else kDebug() << "KoParagLayout::KoParagLayout setting style to " << style << " " << style->name() << endl;
        }
        else
        {
            kError(32500) << "No style name !? - using Standard" << endl;
            style = styleCollection->findStyle( "Standard" );
        }
        Q_ASSERT(style);
        layout.style = style;
    }

    KoParagLayout::loadOasisParagLayout( layout, context );

    return layout;
}

void KoTextParag::loadOasis( const QDomElement& parent, KoOasisContext& context, KoStyleCollection *styleCollection, uint& pos )
{
    // First load layout from style
    KoParagLayout paragLayout = loadParagLayout( context, styleCollection, true );
    setParagLayout( paragLayout );

    // Load paragraph format
    KoTextFormat defaultFormat;
    defaultFormat.load( context );
    setFormat( document()->formatCollection()->format( &defaultFormat ) );

    // Load text
    loadOasisSpan( parent, context, pos );

    // Apply default format to trailing space
    const int len = str->length();
    Q_ASSERT( len >= 1 );
    setFormat( len - 1, 1, paragFormat(), TRUE );

    setChanged( true );
    invalidate( 0 );
}

void KoTextParag::saveOasis( KoXmlWriter& writer, KoSavingContext& context,
                             int from /* default 0 */, int to /* usually length()-2 */,
                             bool /*saveAnchorsFramesets*/ /* default false */ ) const
{
    KoGenStyles& mainStyles = context.mainStyles();

    // Write paraglayout to styles (with parent == the parag's style)
    QString parentStyleName;
    if ( m_layout.style )
        parentStyleName = m_layout.style->name();

    KoGenStyle autoStyle( KoGenStyle::STYLE_AUTO, "paragraph", parentStyleName );
    paragFormat()->save( autoStyle, context );
    m_layout.saveOasis( autoStyle, context, false );

    // First paragraph is special, it includes page-layout info (for word-processing at least)
    if ( !prev() ) {
        if ( context.variableSettings() )
            autoStyle.addProperty( "style:page-number", context.variableSettings()->startingPageNumber() );
        // Well we support only one page layout, so the first parag always points to "Standard".
        autoStyle.addAttribute( "style:master-page-name", "Standard" );
    }


    QString autoParagStyleName = mainStyles.lookup( autoStyle, "P", KoGenStyles::ForceNumbering );

    KoParagCounter* paragCounter = m_layout.counter;
    // outline (text:h) assumes paragCounter != 0 (because depth is mandatory)
    bool outline = m_layout.style && m_layout.style->isOutline() && paragCounter;
    bool normalList = paragCounter && paragCounter->style() != KoParagCounter::STYLE_NONE && !outline;
    if ( normalList ) // non-heading list
    {
        writer.startElement( "text:numbered-paragraph" );
        writer.addAttribute( "text:level", (int)paragCounter->depth() + 1 );
        if ( paragCounter->restartCounter() )
            writer.addAttribute( "text:start-value", paragCounter->startNumber() );

        KoGenStyle listStyle( KoGenStyle::STYLE_AUTO_LIST /*, no family*/ );
        paragCounter->saveOasis( listStyle );

        QString autoListStyleName = mainStyles.lookup( listStyle, "L", KoGenStyles::ForceNumbering );
        writer.addAttribute( "text:style-name", autoListStyleName );

        QString textNumber = m_layout.counter->text( this );
        if ( !textNumber.isEmpty() )
        {
            // This is to help export filters
            writer.startElement( "text:number" );
            writer.addTextNode( textNumber );
            writer.endElement();
        }
    }
    else if ( outline ) // heading
    {
        writer.startElement( "text:h", false /*no indent inside this tag*/ );
        writer.addAttribute( "text:style-name", autoParagStyleName );
        writer.addAttribute( "text:outline-level", (int)paragCounter->depth() + 1 );
        if ( paragCounter->numbering() == KoParagCounter::NUM_NONE )
            writer.addAttribute( "text:is-list-header", "true" );

        QString textNumber = paragCounter->text( this );
        if ( !textNumber.isEmpty() )
        {
            // This is to help export filters
            writer.startElement( "text:number" );
            writer.addTextNode( textNumber );
            writer.endElement();
        }
    }

    if ( !outline ) // normal (non-numbered) paragraph, or normalList
    {
        writer.startElement( "text:p", false /*no indent inside this tag*/ );
        writer.addAttribute( "text:style-name", autoParagStyleName );
    }

    QString text = str->toString();
    Q_ASSERT( text.right(1)[0] == ' ' );

    const int cursorIndex = context.cursorTextParagraph() == this ? context.cursorTextIndex() : -1;

    //kDebug() << k_funcinfo << "'" << text << "' from=" << from << " to=" << to << " cursorIndex=" << cursorIndex << endl;

    // A helper method would need no less than 7 params...
#define WRITESPAN( next ) { \
        if ( curFormat == paragFormat() ) {                             \
            writer.addTextSpan( text.mid( startPos, next - startPos ), m_tabCache ); \
        } else {                                                        \
            KoGenStyle gs( KoGenStyle::STYLE_AUTO, "text" );            \
            curFormat->save( gs, context, paragFormat() );              \
            writer.startElement( "text:span" );                         \
            if ( !gs.isEmpty() ) {                                      \
                const QString autoStyleName = mainStyles.lookup( gs, "T" ); \
                writer.addAttribute( "text:style-name", autoStyleName );    \
            }                                                           \
            writer.addTextSpan( text.mid( startPos, next - startPos ), m_tabCache ); \
            writer.endElement();                                        \
        }                                                               \
    }
#define ISSTARTBOOKMARK( i ) bkStartIter != bookmarkStarts.end() && (*bkStartIter).pos == i
#define ISENDBOOKMARK( i ) bkEndIter != bookmarkEnds.end() && (*bkEndIter).pos == i
#define CHECKPOS( i ) \
        if ( cursorIndex == i ) { \
            writer.addProcessingInstruction( "opendocument cursor-position" ); \
        } \
        if ( ISSTARTBOOKMARK( i ) ) { \
            if ( (*bkStartIter).startEqualsEnd ) \
                writer.startElement( "text:bookmark" ); \
            else \
                writer.startElement( "text:bookmark-start" ); \
            writer.addAttribute( "text:name", (*bkStartIter).name ); \
            writer.endElement(); \
            ++bkStartIter; \
        } \
        if ( ISENDBOOKMARK( i ) ) { \
            writer.startElement( "text:bookmark-end" ); \
            writer.addAttribute( "text:name", (*bkEndIter).name ); \
            writer.endElement(); \
            ++bkEndIter; \
        }



    // Make (shallow) copy of bookmark list, since saving an inline frame might overwrite it
    // from the context while we're saving this paragraph.
    typedef KoSavingContext::BookmarkPositions BookmarkPositions;
    BookmarkPositions bookmarkStarts = context.bookmarkStarts();
    BookmarkPositions::const_iterator bkStartIter = bookmarkStarts.begin();
    while ( bkStartIter != bookmarkStarts.end() && (*bkStartIter).pos < from )
        ++bkStartIter;
    //int nextBookmarkStart = bkStartIter == bookmarkStarts.end() ? -1 : (*bkStartIter).pos;
    BookmarkPositions bookmarkEnds = context.bookmarkEnds();
    BookmarkPositions::const_iterator bkEndIter = bookmarkEnds.begin();
    while ( bkEndIter != bookmarkEnds.end() && (*bkEndIter).pos < from )
        ++bkEndIter;

    KoTextFormat *curFormat = 0;
    KoTextFormat *lastFormatRaw = 0; // this is for speeding up "removing misspelled" from each char
    KoTextFormat *lastFormatFixed = 0; // raw = as stored in the chars; fixed = after removing misspelled
    int startPos = from;
    for ( int i = from; i <= to; ++i ) {
        KoTextStringChar & ch = str->at(i);
        KoTextFormat * newFormat = static_cast<KoTextFormat *>( ch.format() );
        if ( newFormat->isMisspelled() ) {
            if ( newFormat == lastFormatRaw )
                newFormat = lastFormatFixed; // the fast way
            else
            {
                lastFormatRaw = newFormat;
                // Remove isMisspelled from format, to avoid useless derived styles
                // (which would be identical to their parent style)
                KoTextFormat tmpFormat( *newFormat );
                tmpFormat.setMisspelled( false );
                newFormat = formatCollection()->format( &tmpFormat );
                lastFormatFixed = newFormat;
            }
        }
        if ( !curFormat )
            curFormat = newFormat;
        if ( newFormat != curFormat  // Format changed, save previous one.
             || ch.isCustom() || cursorIndex == i || ISSTARTBOOKMARK( i ) || ISENDBOOKMARK( i ) )
        {
            WRITESPAN( i ) // write text up to i-1
            startPos = i;
            curFormat = newFormat;
        }
        CHECKPOS( i ) // do cursor position and bookmarks
        if ( ch.isCustom() ) {
            KoGenStyle gs( KoGenStyle::STYLE_AUTO, "text" );
            curFormat->save( gs, context, paragFormat() );
            writer.startElement( "text:span" );
            if ( !gs.isEmpty() ) {
                const QString autoStyleName = mainStyles.lookup( gs, "T" );
                writer.addAttribute( "text:style-name", autoStyleName );
            }
            KoTextCustomItem* customItem = ch.customItem();
            customItem->saveOasis( writer, context );
            writer.endElement();
            startPos = i + 1;
        }
    }

    //kDebug() << k_funcinfo << "startPos=" << startPos << " to=" << to << " curFormat=" << curFormat << endl;

    if ( to >= startPos ) { // Save last format
        WRITESPAN( to + 1 )
    }
    CHECKPOS( to + 1 ) // do cursor position and bookmarks

    writer.endElement(); // text:p or text:h
    if ( normalList )
        writer.endElement(); // text:numbered-paragraph (englobing a text:p)
}

void KoTextParag::applyListStyle( KoOasisContext& context, int restartNumbering, bool orderedList, bool heading, int level )
{
    //kDebug(32500) << k_funcinfo << "applyListStyle to parag " << this << " heading=" << heading << endl;
    delete m_layout.counter;
    m_layout.counter = new KoParagCounter;
    m_layout.counter->loadOasis( context, restartNumbering, orderedList, heading, level );
    // We emulate space-before with a left paragraph indent (#109223)
    const QDomElement listStyleProperties = context.listStyleStack().currentListStyleProperties();
    if ( listStyleProperties.hasAttributeNS( KoXmlNS::text, "space-before" ) )
    {
        double spaceBefore = KoUnit::parseValue( listStyleProperties.attributeNS( KoXmlNS::text, "space-before", QString::null ) );
        m_layout.margins[ Q3StyleSheetItem::MarginLeft ] += spaceBefore; // added to left-margin, see 15.12 in spec.
    }
    // need to call invalidateCounters() ? Not during the initial loading at least.
}

int KoTextParag::documentWidth() const
{
    return doc ? doc->width() : 0; //docRect.width();
}

//int KoTextParag::documentVisibleWidth() const
//{
//    return doc ? doc->visibleWidth() : 0; //docRect.width();
//}

int KoTextParag::documentX() const
{
    return doc ? doc->x() : 0; //docRect.x();
}

int KoTextParag::documentY() const
{
    return doc ? doc->y() : 0; //docRect.y();
}

void KoTextParag::fixParagWidth( bool viewFormattingChars )
{
    // Fixing the parag rect for the formatting chars (only CR here, KWord handles framebreak).
    if ( viewFormattingChars && lineStartList().count() == 1 ) // don't use lines() here, parag not formatted yet
    {
        KoTextFormat * lastFormat = at( length() - 1 )->format();
        setWidth( qMin( rect().width() + lastFormat->width('x'), doc->width() ) );
    }
    // Warning, if adding anything else here, adjust KWTextFrameSet::fixParagWidth
}

// Called by KoTextParag::drawParagString - all params are in pixel coordinates
void KoTextParag::drawFormattingChars( QPainter &painter, int start, int len,
                                       int lastY_pix, int baseLine_pix, int h_pix, // in pixels
                                       bool /*drawSelections*/,
                                       KoTextFormat * /*lastFormat*/, const Q3MemArray<int> &/*selectionStarts*/,
                                       const Q3MemArray<int> &/*selectionEnds*/, const QColorGroup & /*cg*/,
                                       bool rightToLeft, int /*line*/, KoTextZoomHandler* zh,
                                       int whichFormattingChars )
{
    if ( !whichFormattingChars )
        return;
    painter.save();
    //QPen pen( cg.color( QColorGroup::Highlight ) );
    QPen pen( KGlobalSettings::linkColor() ); // #101820
    painter.setPen( pen );
    //kDebug() << "KWTextParag::drawFormattingChars start=" << start << " len=" << len << " length=" << length() << endl;
    if ( start + len == length() && ( whichFormattingChars & FormattingEndParag ) )
    {
        // drawing the end of the parag
        KoTextStringChar &ch = str->at( length() - 1 );
        KoTextFormat* format = static_cast<KoTextFormat *>( ch.format() );
        int w = format->charWidth( zh, true, &ch, this, 'X' );
        int size = qMin( w, h_pix * 3 / 4 );
        // x,y is the bottom right corner of the
        //kDebug() << "startX=" << startX << " bw=" << bw << " w=" << w << endl;
        int x;
        if ( rightToLeft )
            x = zh->layoutUnitToPixelX( ch.x ) /*+ ch.pixelxadj*/ + ch.pixelwidth - 1;
        else
            x = zh->layoutUnitToPixelX( ch.x ) /*+ ch.pixelxadj*/ + w;
        int y = lastY_pix + baseLine_pix;
        //kDebug() << "KWTextParag::drawFormattingChars drawing CR at " << x << "," << y << endl;
        painter.drawLine( (int)(x - size * 0.2), y - size, (int)(x - size * 0.2), y );
        painter.drawLine( (int)(x - size * 0.5), y - size, (int)(x - size * 0.5), y );
        painter.drawLine( x, y, (int)(x - size * 0.7), y );
        painter.drawLine( x, y - size, (int)(x - size * 0.5), y - size);
        painter.drawArc( x - size, y - size, size, (int)(size / 2), -90*16, -180*16 );
#ifdef DEBUG_FORMATTING
        painter.setPen( Qt::blue );
        painter.drawRect( zh->layoutUnitToPixelX( ch.x ) /*+ ch.pixelxadj*/ - 1, lastY_pix, ch.pixelwidth, baseLine_pix );
        QPen pen( cg.color( QColorGroup::Highlight ) );
        painter.setPen( pen );
#endif
    }

    // Now draw spaces, tabs and newlines
    if ( (whichFormattingChars & FormattingSpace) ||
         (whichFormattingChars & FormattingTabs) ||
         (whichFormattingChars & FormattingBreak) )
    {
        int end = qMin( start + len, length() - 1 ); // don't look at the trailing space
        for ( int i = start ; i < end ; ++i )
        {
            KoTextStringChar &ch = str->at(i);
#ifdef DEBUG_FORMATTING
            painter.setPen( (i % 2)? Qt::red: Qt::green );
            painter.drawRect( zh->layoutUnitToPixelX( ch.x ) /*+ ch.pixelxadj*/ - 1, lastY_pix, ch.pixelwidth, baseLine_pix );
            QPen pen( cg.color( QColorGroup::Highlight ) );
            painter.setPen( pen );
#endif
            if ( ch.isCustom() )
                continue;
            if ( (ch.c == ' ' || ch.c.unicode() == 0x00a0U)
                 && (whichFormattingChars & FormattingSpace))
            {
                // Don't use ch.pixelwidth here. We want a square with
                // the same size for all spaces, even the justified ones
                int w = zh->layoutUnitToPixelX( ch.format()->width( ' ' ) );
                int height = zh->layoutUnitToPixelY( ch.ascent() );
                int size = qMax( 2, qMin( w/2, height/3 ) ); // Enfore that it's a square, and that it's visible
                int x = zh->layoutUnitToPixelX( ch.x ); // + ch.pixelxadj;
                QRect spcRect( x + (ch.pixelwidth - size) / 2, lastY_pix + baseLine_pix - (height - size) / 2, size, size );
                if ( ch.c == ' ' )
                    painter.drawRect( spcRect );
                else // nbsp
                    painter.fillRect( spcRect, pen.color() );
            }
            else if ( ch.c == '\t' && (whichFormattingChars & FormattingTabs) )
            {
                /*KoTextStringChar &nextch = str->at(i+1);
                  int nextx = (nextch.x > ch.x) ? nextch.x : rect().width();
                  //kDebug() << "tab x=" << ch.x << " nextch.x=" << nextch.x
                  //          << " nextx=" << nextx << " startX=" << startX << " bw=" << bw << endl;
                  int availWidth = nextx - ch.x - 1;
                  availWidth=zh->layoutUnitToPixelX(availWidth);*/

                int availWidth = ch.pixelwidth;

                KoTextFormat* format = ch.format();
                int x = zh->layoutUnitToPixelX( ch.x ) /*+ ch.pixelxadj*/ + availWidth / 2;
                int charWidth = format->screenFontMetrics( zh ).width( 'W' );
                int size = qMin( availWidth, charWidth ) / 2 ; // actually the half size
                int y = lastY_pix + baseLine_pix - zh->layoutUnitToPixelY( ch.ascent()/2 );
                int arrowsize = zh->zoomItYOld( 2 );
                painter.drawLine( x - size, y, x + size, y );
                if ( rightToLeft )
                {
                    painter.drawLine( x - size, y, x - size + arrowsize, y - arrowsize );
                    painter.drawLine( x - size, y, x - size + arrowsize, y + arrowsize );
                }
                else
                {
                    painter.drawLine( x + size, y, x + size - arrowsize, y - arrowsize );
                    painter.drawLine( x + size, y, x + size - arrowsize, y + arrowsize );
                }
            }
            else if ( ch.c == '\n' && (whichFormattingChars & FormattingBreak) )
            {
                // draw line break
                KoTextFormat* format = static_cast<KoTextFormat *>( ch.format() );
                int w = format->charWidth( zh, true, &ch, this, 'X' );
                int size = qMin( w, h_pix * 3 / 4 );
                int arrowsize = zh->zoomItYOld( 2 );
                // x,y is the bottom right corner of the reversed L
                //kDebug() << "startX=" << startX << " bw=" << bw << " w=" << w << endl;
                int y = lastY_pix + baseLine_pix - arrowsize;
                //kDebug() << "KWTextParag::drawFormattingChars drawing Line Break at " << x << "," << y << endl;
                if ( rightToLeft )
                {
                    int x = zh->layoutUnitToPixelX( ch.x ) /*+ ch.pixelxadj*/ + ch.pixelwidth - 1;
                    painter.drawLine( x - size, y - size, x - size, y );
                    painter.drawLine( x - size, y, (int)(x - size * 0.3), y );
                    // Now the arrow
                    painter.drawLine( (int)(x - size * 0.3), y, (int)(x - size * 0.3 - arrowsize), y - arrowsize );
                    painter.drawLine( (int)(x - size * 0.3), y, (int)(x - size * 0.3 - arrowsize), y + arrowsize );
                }
                else
                {
                    int x = zh->layoutUnitToPixelX( ch.x ) /*+ ch.pixelxadj*/ + w - 1;
                    painter.drawLine( x, y - size, x, y );
                    painter.drawLine( x, y, (int)(x - size * 0.7), y );
                    // Now the arrow
                    painter.drawLine( (int)(x - size * 0.7), y, (int)(x - size * 0.7 + arrowsize), y - arrowsize );
                    painter.drawLine( (int)(x - size * 0.7), y, (int)(x - size * 0.7 + arrowsize), y + arrowsize );
                }
            }
        }
        painter.restore();
    }
}

int KoTextParag::heightForLineSpacing( int startChar, int lastChar ) const
{
    int h = 0;
    int end = qMin( lastChar, length() - 1 ); // don't look at the trailing space
    for( int i = startChar; i <= end; ++i )
    {
        const KoTextStringChar &chr = str->at( i );
        if ( !chr.isCustom() )
            h = qMax( h, chr.format()->height() );
    }
    return h;
}
