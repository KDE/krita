/****************************************************************************
** Implementation of the internal Qt classes dealing with rich text
**
** Created : 990101
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
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

#include "KoRichText.h"
#include "KoTextFormat.h"
#include "KoTextParag.h"

#include <q3paintdevicemetrics.h>
//#include "qdrawutil.h" // for KoTextHorizontalLine
//Added by qt3to4:
#include <Q3MemArray>
#include <Q3PtrList>

#include <stdlib.h>
#include "KoParagCounter.h"
#include "KoTextDocument.h"
#include <kdebug.h>
#include <kdeversion.h>
#include <kglobal.h>
#include <klocale.h>
//#include <private/qtextengine_p.h>

//#define DEBUG_COLLECTION
//#define DEBUG_TABLE_RENDERING

//static KoTextFormatCollection *qFormatCollection = 0;

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void KoTextDocCommandHistory::addCommand( KoTextDocCommand *cmd )
{
    if ( current < (int)history.count() - 1 ) {
	Q3PtrList<KoTextDocCommand> commands;
	commands.setAutoDelete( FALSE );

	for( int i = 0; i <= current; ++i ) {
	    commands.insert( i, history.at( 0 ) );
	    history.take( 0 );
	}

	commands.append( cmd );
	history.clear();
	history = commands;
	history.setAutoDelete( TRUE );
    } else {
	history.append( cmd );
    }

    if ( (int)history.count() > steps )
	history.removeFirst();
    else
	++current;
}

KoTextCursor *KoTextDocCommandHistory::undo( KoTextCursor *c )
{
    if ( current > -1 ) {
	KoTextCursor *c2 = history.at( current )->unexecute( c );
	--current;
	return c2;
    }
    return 0;
}

KoTextCursor *KoTextDocCommandHistory::redo( KoTextCursor *c )
{
    if ( current > -1 ) {
	if ( current < (int)history.count() - 1 ) {
	    ++current;
	    return history.at( current )->execute( c );
	}
    } else {
	if ( history.count() > 0 ) {
	    ++current;
	    return history.at( current )->execute( c );
	}
    }
    return 0;
}

bool KoTextDocCommandHistory::isUndoAvailable()
{
    return current > -1;
}

bool KoTextDocCommandHistory::isRedoAvailable()
{
   return current > -1 && current < (int)history.count() - 1 || current == -1 && history.count() > 0;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

KoTextDocDeleteCommand::KoTextDocDeleteCommand( KoTextDocument *d, int i, int idx, const Q3MemArray<KoTextStringChar> &str )
    : KoTextDocCommand( d ), id( i ), index( idx ), parag( 0 ), text( str )
{
    for ( int j = 0; j < (int)text.size(); ++j ) {
	if ( text[ j ].format() )
	    text[ j ].format()->addRef();
    }
}

/*KoTextDocDeleteCommand::KoTextDocDeleteCommand( KoTextParag *p, int idx, const QMemArray<KoTextStringChar> &str )
    : KoTextDocCommand( 0 ), id( -1 ), index( idx ), parag( p ), text( str )
{
    for ( int i = 0; i < (int)text.size(); ++i ) {
	if ( text[ i ].format() )
	    text[ i ].format()->addRef();
    }
}*/

KoTextDocDeleteCommand::~KoTextDocDeleteCommand()
{
    for ( int i = 0; i < (int)text.size(); ++i ) {
	if ( text[ i ].format() )
	    text[ i ].format()->removeRef();
    }
    text.resize( 0 );
}

KoTextCursor *KoTextDocDeleteCommand::execute( KoTextCursor *c )
{
    KoTextParag *s = doc ? doc->paragAt( id ) : parag;
    if ( !s ) {
	kWarning(32500) << "can't locate parag at " << id << ", last parag: " << doc->lastParag()->paragId() << endl;
	return 0;
    }

    cursor.setParag( s );
    cursor.setIndex( index );
    int len = text.size();
    if ( c )
	*c = cursor;
    if ( doc ) {
	doc->setSelectionStart( KoTextDocument::Temp, &cursor );
	for ( int i = 0; i < len; ++i )
	    cursor.gotoNextLetter();
	doc->setSelectionEnd( KoTextDocument::Temp, &cursor );
	doc->removeSelectedText( KoTextDocument::Temp, &cursor );
	if ( c )
	    *c = cursor;
    } else {
	s->remove( index, len );
    }

    return c;
}

KoTextCursor *KoTextDocDeleteCommand::unexecute( KoTextCursor *c )
{
    KoTextParag *s = doc ? doc->paragAt( id ) : parag;
    if ( !s ) {
	kWarning(32500) << "can't locate parag at " << id << ", last parag: " << doc->lastParag()->paragId() << endl;
	return 0;
    }

    cursor.setParag( s );
    cursor.setIndex( index );
    QString str = KoTextString::toString( text );
    cursor.insert( str, TRUE, &text );
    cursor.setParag( s );
    cursor.setIndex( index );
    if ( c ) {
	c->setParag( s );
	c->setIndex( index );
	for ( int i = 0; i < (int)text.size(); ++i )
	    c->gotoNextLetter();
    }

    s = cursor.parag();
    while ( s ) {
	s->format();
	s->setChanged( TRUE );
	if ( s == c->parag() )
	    break;
	s = s->next();
    }

    return &cursor;
}

KoTextDocFormatCommand::KoTextDocFormatCommand( KoTextDocument *d, int sid, int sidx, int eid, int eidx,
					const Q3MemArray<KoTextStringChar> &old, const KoTextFormat *f, int fl )
    : KoTextDocCommand( d ), startId( sid ), startIndex( sidx ), endId( eid ), endIndex( eidx ), oldFormats( old ), flags( fl )
{
    format = d->formatCollection()->format( f );
    for ( int j = 0; j < (int)oldFormats.size(); ++j ) {
	if ( oldFormats[ j ].format() )
	    oldFormats[ j ].format()->addRef();
    }
}

KoTextDocFormatCommand::~KoTextDocFormatCommand()
{
    format->removeRef();
    for ( int j = 0; j < (int)oldFormats.size(); ++j ) {
	if ( oldFormats[ j ].format() )
	    oldFormats[ j ].format()->removeRef();
    }
}

KoTextCursor *KoTextDocFormatCommand::execute( KoTextCursor *c )
{
    KoTextParag *sp = doc->paragAt( startId );
    KoTextParag *ep = doc->paragAt( endId );
    if ( !sp || !ep )
	return c;

    KoTextCursor start( doc );
    start.setParag( sp );
    start.setIndex( startIndex );
    KoTextCursor end( doc );
    end.setParag( ep );
    end.setIndex( endIndex );

    doc->setSelectionStart( KoTextDocument::Temp, &start );
    doc->setSelectionEnd( KoTextDocument::Temp, &end );
    doc->setFormat( KoTextDocument::Temp, format, flags );
    doc->removeSelection( KoTextDocument::Temp );
    if ( endIndex == ep->length() ) // ### Not in QRT - report sent. Description at http://bugs.kde.org/db/34/34556.html
        end.gotoLeft();
    *c = end;
    return c;
}

KoTextCursor *KoTextDocFormatCommand::unexecute( KoTextCursor *c )
{
    KoTextParag *sp = doc->paragAt( startId );
    KoTextParag *ep = doc->paragAt( endId );
    if ( !sp || !ep )
	return 0;

    int idx = startIndex;
    int fIndex = 0;
    if( !oldFormats.isEmpty()) // ## not in QRT. Not sure how it can happen.
    {
    for ( ;; ) {
	if ( oldFormats.at( fIndex ).c == '\n' ) {
	    if ( idx > 0 ) {
		if ( idx < sp->length() && fIndex > 0 )
		    sp->setFormat( idx, 1, oldFormats.at( fIndex - 1 ).format() );
		if ( sp == ep )
		    break;
		sp = sp->next();
		idx = 0;
	    }
	    fIndex++;
	}
	if ( oldFormats.at( fIndex ).format() )
	    sp->setFormat( idx, 1, oldFormats.at( fIndex ).format() );
	idx++;
	fIndex++;
	if ( fIndex >= (int)oldFormats.size() )
	    break;
	if ( idx >= sp->length() ) {
	    if ( sp == ep )
		break;
	    sp = sp->next();
	    idx = 0;
	}
    }
    }
    KoTextCursor end( doc );
    end.setParag( ep );
    end.setIndex( endIndex );
    if ( endIndex == ep->length() )
        end.gotoLeft();
    *c = end;
    return c;
}

KoTextAlignmentCommand::KoTextAlignmentCommand( KoTextDocument *d, int fParag, int lParag, int na, const Q3MemArray<int> &oa )
    : KoTextDocCommand( d ), firstParag( fParag ), lastParag( lParag ), newAlign( na ), oldAligns( oa )
{
}

KoTextCursor *KoTextAlignmentCommand::execute( KoTextCursor *c )
{
    KoTextParag *p = doc->paragAt( firstParag );
    if ( !p )
	return c;
    while ( p ) {
	p->setAlignment( newAlign );
	if ( p->paragId() == lastParag )
	    break;
	p = p->next();
    }
    return c;
}

KoTextCursor *KoTextAlignmentCommand::unexecute( KoTextCursor *c )
{
    KoTextParag *p = doc->paragAt( firstParag );
    if ( !p )
	return c;
    int i = 0;
    while ( p ) {
	if ( i < (int)oldAligns.size() )
	    p->setAlignment( oldAligns.at( i ) );
	if ( p->paragId() == lastParag )
	    break;
	p = p->next();
	++i;
    }
    return c;
}


// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

KoTextCursor::KoTextCursor( KoTextDocument *d )
    : doc( d )
{
    idx = 0;
    string = doc ? doc->firstParag() : 0;
    tmpIndex = -1;
}

KoTextCursor::KoTextCursor()
{
}

KoTextCursor::KoTextCursor( const KoTextCursor &c )
{
    doc = c.doc;
    idx = c.idx;
    string = c.string;
    tmpIndex = c.tmpIndex;
}

KoTextCursor &KoTextCursor::operator=( const KoTextCursor &c )
{
    doc = c.doc;
    idx = c.idx;
    string = c.string;
    tmpIndex = c.tmpIndex;

    return *this;
}

bool KoTextCursor::operator==( const KoTextCursor &c ) const
{
    return doc == c.doc && string == c.string && idx == c.idx;
}

void KoTextCursor::insert( const QString &str, bool checkNewLine, Q3MemArray<KoTextStringChar> *formatting )
{
    string->invalidate( idx );
    tmpIndex = -1;
    bool justInsert = TRUE;
    QString s( str );
#if defined(Q_WS_WIN)
    if ( checkNewLine )
	s = s.replace( QRegExp( "\\r" ), "" );
#endif
    if ( checkNewLine )
	justInsert = s.find( '\n' ) == -1;
    if ( justInsert ) {
	string->insert( idx, s );
	if ( formatting ) {
	    for ( int i = 0; i < (int)s.length(); ++i ) {
		if ( formatting->at( i ).format() ) {
		    formatting->at( i ).format()->addRef();
		    string->string()->setFormat( idx + i, formatting->at( i ).format(), TRUE );
		}
	    }
	}
	idx += s.length();
    } else {
	QStringList lst = QStringList::split( '\n', s, TRUE );
	QStringList::Iterator it = lst.begin();
	//int y = string->rect().y() + string->rect().height();
	int lastIndex = 0;
	KoTextFormat *lastFormat = 0;
	for ( ; it != lst.end(); ) {
	    if ( it != lst.begin() ) {
		splitAndInsertEmptyParag( FALSE, TRUE );
		//string->setEndState( -1 );
#if 0 // no!
		string->prev()->format( -1, FALSE );
#endif
		if ( lastFormat && formatting && string->prev() ) {
		    lastFormat->addRef();
		    string->prev()->string()->setFormat( string->prev()->length() - 1, lastFormat, TRUE );
		}
	    }
	    lastFormat = 0;
	    QString s = *it;
	    ++it;
	    if ( !s.isEmpty() )
		string->insert( idx, s );
            else
                string->invalidate( 0 );

	    if ( formatting ) {
		int len = s.length();
		for ( int i = 0; i < len; ++i ) {
		    if ( formatting->at( i + lastIndex ).format() ) {
			formatting->at( i + lastIndex ).format()->addRef();
			string->string()->setFormat( i + idx, formatting->at( i + lastIndex ).format(), TRUE );
		    }
		}
		if ( it != lst.end() )
		    lastFormat = formatting->at( len + lastIndex ).format();
		++len;
		lastIndex += len;
	    }

	    idx += s.length();
	}
#if 0  //// useless and wrong. We'll format things and move them down correctly in KoTextObject::insert().
	string->format( -1, FALSE );
	int dy = string->rect().y() + string->rect().height() - y;
#endif
	KoTextParag *p = string;
	p->setParagId( p->prev()->paragId() + 1 );
	p = p->next();
	while ( p ) {
	    p->setParagId( p->prev()->paragId() + 1 );
	    //p->move( dy );
	    p->invalidate( 0 );
	    p = p->next();
	}
    }

#if 0  //// useless and slow
    int h = string->rect().height();
    string->format( -1, TRUE );
#endif
    fixCursorPosition();
}

void KoTextCursor::gotoLeft()
{
    if ( string->string()->isRightToLeft() )
	gotoNextLetter();
    else
	gotoPreviousLetter();
}

void KoTextCursor::gotoPreviousLetter()
{
    tmpIndex = -1;

    if ( idx > 0 ) {
	idx = string->string()->previousCursorPosition( idx );
    } else if ( string->prev() ) {
	string = string->prev();
	while ( !string->isVisible() )
	    string = string->prev();
	idx = string->length() - 1;
    }
}

bool KoTextCursor::place( const QPoint &p, KoTextParag *s, bool link, int *customItemIndex )
{
    if ( customItemIndex )
        *customItemIndex = -1;
    QPoint pos( p );
    QRect r;
    if ( pos.y() < s->rect().y() )
	pos.setY( s->rect().y() );
    while ( s ) {
	r = s->rect();
	r.setWidth( doc ? doc->width() : QWIDGETSIZE_MAX );
	if ( !s->next() || ( pos.y() >= r.y() && pos.y() < s->next()->rect().y() ) )
	    break;
	s = s->next();
    }

    if ( !s )
	return FALSE;

    setParag( s, FALSE );
    int y = s->rect().y();
    int lines = s->lines();
    KoTextStringChar *chr = 0;
    int index = 0;
    int i = 0;
    int cy = 0;
    //int ch = 0;
    for ( ; i < lines; ++i ) {
	chr = s->lineStartOfLine( i, &index );
	cy = s->lineY( i );
	//ch = s->lineHeight( i );
	if ( !chr )
	    return FALSE;
	if ( i < lines - 1 && pos.y() >= y + cy && pos.y() <= y + s->lineY( i+1 ) )
	    break;
    }
    int nextLine;
    if ( i < lines - 1 )
	s->lineStartOfLine( i+1, &nextLine );
    else
	nextLine = s->length();
    i = index;
    int x = s->rect().x();
    if ( pos.x() < x )
	pos.setX( x + 1 );
    int cw;
    int curpos = s->length()-1;
    int dist = 10000000;
    while ( i < nextLine ) {
	chr = s->at(i);
	int cpos = x + chr->x;
	cw = chr->width; //s->string()->width( i );
	if ( chr->isCustom() ) {
             if ( pos.x() >= cpos && pos.x() <= cpos + cw &&
                  pos.y() >= y + cy && pos.y() <= y + cy + chr->height() ) {
                if ( customItemIndex )
                    *customItemIndex = i;
	    }
	}
        if( chr->rightToLeft )
            cpos += cw;
        int d = cpos - pos.x();
        bool dm = d < 0 ? !chr->rightToLeft : chr->rightToLeft;
        if ( (QABS( d ) < dist || (dist == d && dm == TRUE )) && string->string()->validCursorPosition( i ) ) {
            dist = QABS( d );
            if ( !link || pos.x() >= x + chr->x ) {
                curpos = i;
            }
        }
	i++;
    }
    setIndex( curpos, FALSE );

    return TRUE;
}

void KoTextCursor::gotoRight()
{
    if ( string->string()->isRightToLeft() )
	gotoPreviousLetter();
    else
	gotoNextLetter();
}

void KoTextCursor::gotoNextLetter()
{
    tmpIndex = -1;

    int len = string->length() - 1;
    if ( idx < len ) {
        idx = string->string()->nextCursorPosition( idx );
    } else if ( string->next() ) {
	string = string->next();
	while ( !string->isVisible() )
	    string = string->next();
	idx = 0;
    }
}

void KoTextCursor::gotoUp()
{
    int indexOfLineStart;
    int line;
    KoTextStringChar *c = string->lineStartOfChar( idx, &indexOfLineStart, &line );
    if ( !c )
	return;

    tmpIndex = qMax( tmpIndex, idx - indexOfLineStart );
    if ( indexOfLineStart == 0 ) {
	if ( !string->prev() ) {
            return;
	}
	string = string->prev();
	while ( !string->isVisible() )
	    string = string->prev();
	int lastLine = string->lines() - 1;
	if ( !string->lineStartOfLine( lastLine, &indexOfLineStart ) )
	    return;
	if ( indexOfLineStart + tmpIndex < string->length() )
	    idx = indexOfLineStart + tmpIndex;
	else
	    idx = string->length() - 1;
    } else {
	--line;
	int oldIndexOfLineStart = indexOfLineStart;
	if ( !string->lineStartOfLine( line, &indexOfLineStart ) )
	    return;
	if ( indexOfLineStart + tmpIndex < oldIndexOfLineStart )
	    idx = indexOfLineStart + tmpIndex;
	else
	    idx = oldIndexOfLineStart - 1;
    }
    fixCursorPosition();
}

void KoTextCursor::gotoDown()
{
    int indexOfLineStart;
    int line;
    KoTextStringChar *c = string->lineStartOfChar( idx, &indexOfLineStart, &line );
    if ( !c )
	return;

    tmpIndex = qMax( tmpIndex, idx - indexOfLineStart );
    if ( line == string->lines() - 1 ) {
	if ( !string->next() ) {
            return;
	}
	string = string->next();
	while ( !string->isVisible() )
	    string = string->next();
	if ( !string->lineStartOfLine( 0, &indexOfLineStart ) )
	    return;
	int end;
	if ( string->lines() == 1 )
	    end = string->length();
	else
	    string->lineStartOfLine( 1, &end );
	if ( indexOfLineStart + tmpIndex < end )
	    idx = indexOfLineStart + tmpIndex;
	else
	    idx = end - 1;
    } else {
	++line;
	int end;
	if ( line == string->lines() - 1 )
	    end = string->length();
	else
	    string->lineStartOfLine( line + 1, &end );
	if ( !string->lineStartOfLine( line, &indexOfLineStart ) )
	    return;
	if ( indexOfLineStart + tmpIndex < end )
	    idx = indexOfLineStart + tmpIndex;
	else
	    idx = end - 1;
    }
    fixCursorPosition();
}

void KoTextCursor::gotoLineEnd()
{
    tmpIndex = -1;
    int indexOfLineStart;
    int line;
    KoTextStringChar *c = string->lineStartOfChar( idx, &indexOfLineStart, &line );
    if ( !c )
	return;

    if ( line == string->lines() - 1 ) {
	idx = string->length() - 1;
    } else {
	c = string->lineStartOfLine( ++line, &indexOfLineStart );
	indexOfLineStart--;
	idx = indexOfLineStart;
    }
}

void KoTextCursor::gotoLineStart()
{
    tmpIndex = -1;
    int indexOfLineStart;
    int line;
    KoTextStringChar *c = string->lineStartOfChar( idx, &indexOfLineStart, &line );
    if ( !c )
	return;

    idx = indexOfLineStart;
}

void KoTextCursor::gotoHome()
{
    tmpIndex = -1;
    if ( doc )
	string = doc->firstParag();
    idx = 0;
}

void KoTextCursor::gotoEnd()
{
    // This can happen in a no-auto-resize frame with overflowing contents.
    // Don't prevent going to the end of the text, even if it's not visible.
    //if ( doc && !doc->lastParag()->isValid() )
    //{
//	kDebug(32500) << "Last parag, " << doc->lastParag()->paragId() << ", is invalid - aborting gotoEnd() !" << endl;
//	return;
//    }

    tmpIndex = -1;
    if ( doc )
	string = doc->lastParag();
    idx = string->length() - 1;
}

void KoTextCursor::gotoPageUp( int visibleHeight )
{
    tmpIndex = -1;
    KoTextParag *s = string;
    int h = visibleHeight;
    int y = s->rect().y();
    while ( s ) {
	if ( y - s->rect().y() >= h )
	    break;
	s = s->prev();
    }

    if ( !s && doc )
	s = doc->firstParag();

    string = s;
    idx = 0;
}

void KoTextCursor::gotoPageDown( int visibleHeight )
{
    tmpIndex = -1;
    KoTextParag *s = string;
    int h = visibleHeight;
    int y = s->rect().y();
    while ( s ) {
	if ( s->rect().y() - y >= h )
	    break;
	s = s->next();
    }

    if ( !s && doc ) {
	s = doc->lastParag();
	string = s;
	idx = string->length() - 1;
	return;
    }

    if ( !s->isValid() )
	return;

    string = s;
    idx = 0;
}

void KoTextCursor::gotoWordRight()
{
    if ( string->string()->isRightToLeft() )
	gotoPreviousWord();
    else
	gotoNextWord();
}

void KoTextCursor::gotoWordLeft()
{
    if ( string->string()->isRightToLeft() )
	gotoNextWord();
    else
	gotoPreviousWord();
}

void KoTextCursor::gotoPreviousWord()
{
    gotoPreviousLetter();
    tmpIndex = -1;
    KoTextString *s = string->string();
    bool allowSame = FALSE;
    if ( idx == ( (int)s->length()-1 ) )
        return;
    for ( int i = idx; i >= 0; --i ) {
	if ( s->at( i ).c.isSpace() || s->at( i ).c == '\t' || s->at( i ).c == '.' ||
	     s->at( i ).c == ',' || s->at( i ).c == ':' || s->at( i ).c == ';' ) {
	    if ( !allowSame )
		continue;
	    idx = i + 1;
	    return;
	}
	if ( !allowSame && !( s->at( i ).c.isSpace() || s->at( i ).c == '\t' || s->at( i ).c == '.' ||
			      s->at( i ).c == ',' || s->at( i ).c == ':' || s->at( i ).c == ';'  ) )
	    allowSame = TRUE;
    }
    idx = 0;
}

void KoTextCursor::gotoNextWord()
{
    tmpIndex = -1;
    KoTextString *s = string->string();
    bool allowSame = FALSE;
    for ( int i = idx; i < (int)s->length(); ++i ) {
	if ( ! ( s->at( i ).c.isSpace() || s->at( i ).c == '\t' || s->at( i ).c == '.' ||
	     s->at( i ).c == ',' || s->at( i ).c == ':' || s->at( i ).c == ';' ) ) {
	    if ( !allowSame )
		continue;
	    idx = i;
	    return;
	}
	if ( !allowSame && ( s->at( i ).c.isSpace() || s->at( i ).c == '\t' || s->at( i ).c == '.' ||
			      s->at( i ).c == ',' || s->at( i ).c == ':' || s->at( i ).c == ';'  ) )
	    allowSame = TRUE;
    }

    if ( idx < ((int)s->length()-1) ) {
        gotoLineEnd();
    } else if ( string->next() ) {
	string = string->next();
	while ( !string->isVisible() )
	    string = string->next();
	idx = 0;
    } else {
	gotoLineEnd();
    }
}

bool KoTextCursor::atParagStart() const
{
    return idx == 0;
}

bool KoTextCursor::atParagEnd() const
{
    return idx == string->length() - 1;
}

void KoTextCursor::splitAndInsertEmptyParag( bool ind, bool updateIds )
{
    if ( !doc )
	return;
    tmpIndex = -1;
    KoTextFormat *f = 0;
    if ( doc->useFormatCollection() ) {
	f = string->at( idx )->format();
	if ( idx == string->length() - 1 && idx > 0 )
	    f = string->at( idx - 1 )->format();
	if ( f->isMisspelled() ) {
            KoTextFormat fNoMisspelled( *f );
            fNoMisspelled.setMisspelled( false );
	    f = doc->formatCollection()->format( &fNoMisspelled );
	}
    }

    if ( atParagEnd() ) {
	KoTextParag *n = string->next();
	KoTextParag *s = doc->createParag( doc, string, n, updateIds );
	if ( f )
	    s->setFormat( 0, 1, f, TRUE );
	s->copyParagData( string );
#if 0
	if ( ind ) {
	    int oi, ni;
	    s->indent( &oi, &ni );
	    string = s;
	    idx = ni;
	} else
#endif
        {
	    string = s;
	    idx = 0;
	}
    } else if ( atParagStart() ) {
	KoTextParag *p = string->prev();
	KoTextParag *s = doc->createParag( doc, p, string, updateIds );
	if ( f )
	    s->setFormat( 0, 1, f, TRUE );
	s->copyParagData( string );
	if ( ind ) {
	    //s->indent();
	    s->format();
	    //indent();
	    string->format();
	}
    } else {
	QString str = string->string()->toString().mid( idx, 0xFFFFFF );
	KoTextParag *n = string->next();
	KoTextParag *s = doc->createParag( doc, string, n, updateIds );
	s->copyParagData( string );
	s->remove( 0, 1 );
	s->append( str, TRUE );
	for ( uint i = 0; i < str.length(); ++i ) {
            KoTextStringChar* tsc = string->at( idx + i );
	    s->setFormat( i, 1, tsc->format(), TRUE );
	    if ( tsc->isCustom() ) {
		KoTextCustomItem * item = tsc->customItem();
		s->at( i )->setCustomItem( item );
		tsc->loseCustomItem();
#if 0
		s->addCustomItem();
		string->removeCustomItem();
#endif
		doc->unregisterCustomItem( item, string );
		doc->registerCustomItem( item, s );
	    }
	}
	string->truncate( idx );
#if 0
	if ( ind ) {
	    int oi, ni;
	    s->indent( &oi, &ni );
	    string = s;
	    idx = ni;
	} else
#endif
        {
	    string = s;
	    idx = 0;
	}
    }
}

bool KoTextCursor::removePreviousChar()
{
    tmpIndex = -1;
    if ( !atParagStart() ) {
	string->remove( idx-1, 1 );
	idx--;
	// shouldn't be needed, just to make sure.
	fixCursorPosition();
	string->format( -1, TRUE );
	//else if ( string->document() && string->document()->parent() )
	//    string->document()->nextDoubleBuffered = TRUE;
	return FALSE;
    } else if ( string->prev() ) {
	string = string->prev();
	string->join( string->next() );
	string->invalidateCounters();
	return TRUE;
    }
    return FALSE;
}

bool KoTextCursor::remove()
{
    tmpIndex = -1;
    if ( !atParagEnd() ) {
	int next = string->string()->nextCursorPosition( idx );
	string->remove( idx, next-idx );
	string->format( -1, TRUE );
	//else if ( doc && doc->parent() )
	//    doc->nextDoubleBuffered = TRUE;
	return FALSE;
    } else if ( string->next() ) {
	if ( string->length() == 1 ) {
	    string->next()->setPrev( string->prev() );
	    if ( string->prev() )
		string->prev()->setNext( string->next() );
	    KoTextParag *p = string->next();
	    delete string;
	    string = p;
	    string->invalidate( 0 );
            //// kotext
            string->invalidateCounters();
            ////
	    KoTextParag *s = string;
	    while ( s ) {
		s->id = s->p ? s->p->id + 1 : 0;
		//s->state = -1;
		//s->needPreProcess = TRUE;
		s->changed = TRUE;
		s = s->n;
	    }
	    string->format();
	} else {
	    string->join( string->next() );
	}
	return TRUE;
    }
    return FALSE;
}

void KoTextCursor::killLine()
{
    if ( atParagEnd() )
	return;
    string->remove( idx, string->length() - idx - 1 );
    string->format( -1, TRUE );
    //else if ( doc && doc->parent() )
    //doc->nextDoubleBuffered = TRUE;
}

#if 0
void KoTextCursor::indent()
{
    int oi = 0, ni = 0;
    string->indent( &oi, &ni );
    if ( oi == ni )
	return;

    if ( idx >= oi )
	idx += ni - oi;
    else
	idx = ni;
}
#endif

void KoTextCursor::setDocument( KoTextDocument *d )
{
    doc = d;
    string = d->firstParag();
    idx = 0;
    tmpIndex = -1;
}


int KoTextCursor::x() const
{
    KoTextStringChar *c = string->at( idx );
    int curx = c->x;
    if ( c->rightToLeft )
        curx += c->width; //string->string()->width( idx );
    return curx;
}

int KoTextCursor::y() const
{
    int dummy, line;
    string->lineStartOfChar( idx, &dummy, &line );
    return string->lineY( line );
}


void KoTextCursor::fixCursorPosition()
{
    // searches for the closest valid cursor position
    if ( string->string()->validCursorPosition( idx ) )
 	return;

    int lineIdx;
    KoTextStringChar *start = string->lineStartOfChar( idx, &lineIdx, 0 );
    int x = string->string()->at( idx ).x;
    int diff = QABS(start->x - x);
    int best = lineIdx;

    KoTextStringChar *c = start;
    ++c;

    KoTextStringChar *end = &string->string()->at( string->length()-1 );
    while ( c <= end && !c->lineStart ) {
 	int xp = c->x;
 	if ( c->rightToLeft )
 	    xp += c->pixelwidth; //string->string()->width( lineIdx + (c-start) );
 	int ndiff = QABS(xp - x);
 	if ( ndiff < diff && string->string()->validCursorPosition(lineIdx + (c-start)) ) {
 	    diff = ndiff;
 	    best = lineIdx + (c-start);
 	}
 	++c;
    }
    idx = best;
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

KoTextString::KoTextString()
{
    bidiDirty = TRUE;
    bNeedsSpellCheck = true;
    bidi = FALSE;
    rightToLeft = FALSE;
    dir = QChar::DirON;
}

KoTextString::KoTextString( const KoTextString &s )
{
    bidiDirty = s.bidiDirty;
    bNeedsSpellCheck = s.bNeedsSpellCheck;
    bidi = s.bidi;
    rightToLeft = s.rightToLeft;
    dir = s.dir;
    data = s.data;
    data.detach();
    for ( int i = 0; i < (int)data.size(); ++i ) {
        KoTextFormat *f = data[i].format();
        if ( f )
            f->addRef();
    }
}

void KoTextString::insert( int index, const QString &s, KoTextFormat *f )
{
    int os = data.size();
    data.resize( data.size() + s.length() );
    if ( index < os ) {
	memmove( data.data() + index + s.length(), data.data() + index,
		 sizeof( KoTextStringChar ) * ( os - index ) );
    }
    for ( int i = 0; i < (int)s.length(); ++i ) {
	KoTextStringChar &ch = data[ (int)index + i ];
	ch.x = 0;
	ch.pixelxadj = 0;
	ch.pixelwidth = 0;
	ch.width = 0;
	ch.lineStart = 0;
	ch.d.format = 0;
	ch.type = KoTextStringChar::Regular;
	ch.rightToLeft = 0;
	ch.startOfRun = 0;
        ch.c = s[ i ];
#ifdef DEBUG_COLLECTION
	kDebug(32500) << "KoTextString::insert setting format " << f << " to character " << (int)index+i << endl;
#endif
	ch.setFormat( f );
    }
    bidiDirty = TRUE;
    bNeedsSpellCheck = true;
}

KoTextString::~KoTextString()
{
    clear();
}

void KoTextString::insert( int index, KoTextStringChar *c )
{
    int os = data.size();
    data.resize( data.size() + 1 );
    if ( index < os ) {
	memmove( data.data() + index + 1, data.data() + index,
		 sizeof( KoTextStringChar ) * ( os - index ) );
    }
    KoTextStringChar &ch = data[ (int)index ];
    ch.c = c->c;
    ch.x = 0;
    ch.pixelxadj = 0;
    ch.pixelwidth = 0;
    ch.width = 0;
    ch.lineStart = 0;
    ch.rightToLeft = 0;
    ch.d.format = 0;
    ch.type = KoTextStringChar::Regular;
    ch.setFormat( c->format() );
    bidiDirty = TRUE;
    bNeedsSpellCheck = true;
}

void KoTextString::truncate( int index )
{
    index = qMax( index, 0 );
    index = qMin( index, (int)data.size() - 1 );
    if ( index < (int)data.size() ) {
	for ( int i = index + 1; i < (int)data.size(); ++i ) {
	    KoTextStringChar &ch = data[ i ];
	    if ( ch.isCustom() ) {
		delete ch.customItem();
		if ( ch.d.custom->format )
		    ch.d.custom->format->removeRef();
		delete ch.d.custom;
		ch.d.custom = 0;
	    } else if ( ch.format() ) {
		ch.format()->removeRef();
	    }
	}
    }
    data.truncate( index );
    bidiDirty = TRUE;
    bNeedsSpellCheck = true;
}

void KoTextString::remove( int index, int len )
{
    for ( int i = index; i < (int)data.size() && i - index < len; ++i ) {
	KoTextStringChar &ch = data[ i ];
	if ( ch.isCustom() ) {
	    delete ch.customItem();
	    if ( ch.d.custom->format )
		ch.d.custom->format->removeRef();
            delete ch.d.custom;
	    ch.d.custom = 0;
	} else if ( ch.format() ) {
	    ch.format()->removeRef();
	}
    }
    memmove( data.data() + index, data.data() + index + len,
	     sizeof( KoTextStringChar ) * ( data.size() - index - len ) );
    data.resize( data.size() - len, Q3GArray::SpeedOptim );
    bidiDirty = TRUE;
    bNeedsSpellCheck = true;
}

void KoTextString::clear()
{
    for ( int i = 0; i < (int)data.count(); ++i ) {
	KoTextStringChar &ch = data[ i ];
	if ( ch.isCustom() ) {
            // Can't do that here, no access to the doc. See ~KoTextParag instead.
            // However clear() is also called by operator=, many times in kotextobject.cc...
            // Hopefully not with customitems in there...
            //if ( doc )
            //    doc->unregisterCustomItem( ch->customItem(), this );

	    delete ch.customItem();
	    if ( ch.d.custom->format )
		ch.d.custom->format->removeRef();
	    delete ch.d.custom;
	    ch.d.custom = 0;

	} else if ( ch.format() ) {
	    ch.format()->removeRef();
	}
    }
    data.resize( 0 );
}

void KoTextString::setFormat( int index, KoTextFormat *f, bool useCollection, bool setFormatAgain )
{
    KoTextStringChar &ch = data[ index ];
//    kDebug(32500) << "KoTextString::setFormat index=" << index << " f=" << f << endl;
    if ( useCollection && ch.format() )
    {
	//kDebug(32500) << "KoTextString::setFormat removing ref on old format " << ch.format() << endl;
	ch.format()->removeRef();
    }
    ch.setFormat( f, setFormatAgain );
}

void KoTextString::checkBidi() const
{
    KoTextString *that = (KoTextString *)this;
    that->bidiDirty = FALSE;
    int length = data.size();
    if ( !length ) {
        that->bidi = FALSE;
        that->rightToLeft = dir == QChar::DirR;
        return;
    }
    const KoTextStringChar *start = data.data();
    const KoTextStringChar *end = start + length;

#if 0  //KoText porting
    // determines the properties we need for layouting
    QTextLayout textLayout( toString() );

    //textEngine.direction = (QChar::Direction) dir; TODO
    QTextLine line = textLayout.createLine();
    Q_ASSERT(line.isValid()); // TODO error checking
    KoTextStringChar *ch = (KoTextStringChar *)end - 1;
    int pos = length-1;
    while ( ch >= start ) {
#if 0 // TODO RTL
        if ( item->position > pos ) {
            --item;
            Q_ASSERT( item >= &textEngine.items[0] );
            Q_ASSERT( item < &textEngine.items[textEngine.items.size()] );
            bidiLevel = item->analysis.bidiLevel;
            if ( bidiLevel )
                that->bidi = TRUE;
        }
#endif
        // TODO ch->softBreak = ca->softBreak;
        ch->whiteSpace = QChar( data[pos].c ).isSpace();
        ch->charStop = textLayout.isValidCursorPosition( pos );
        // TODO ch->wordStop = ca->wordStop;
        //ch->rightToLeft = (bidiLevel%2);
        --ch;
        //--ca;
        --pos;
    }

    if ( dir == QChar::DirR ) {
        that->bidi = TRUE;
        that->rightToLeft = TRUE;
    } else if ( dir == QChar::DirL ) {
        that->rightToLeft = FALSE;
    } else {
	that->rightToLeft = FALSE; // TODO RTL: (textEngine.direction == QChar::DirR);
    }
#endif
}

Q3MemArray<KoTextStringChar> KoTextString::subString( int start, int len ) const
{
    if ( len == 0xFFFFFF )
	len = data.size();
    Q3MemArray<KoTextStringChar> a;
    a.resize( len );
    for ( int i = 0; i < len; ++i ) {
	KoTextStringChar *c = &data[ i + start ];
	a[ i ].c = c->c;
	a[ i ].x = 0;
	a[ i ].pixelxadj = 0;
	a[ i ].pixelwidth = 0;
	a[ i ].width = 0;
	a[ i ].lineStart = 0;
	a[ i ].rightToLeft = 0;
	a[ i ].d.format = 0;
	a[ i ].type = KoTextStringChar::Regular;
	a[ i ].setFormat( c->format() );
	if ( c->format() )
	    c->format()->addRef();
    }
    return a;
}

QString KoTextString::mid( int start, int len ) const
{
    if ( len == 0xFFFFFF )
	len = data.size();
    QString res;
    res.setLength( len );
    for ( int i = 0; i < len; ++i ) {
	KoTextStringChar *c = &data[ i + start ];
	res[ i ] = c->c;
    }
    return res;
}

QString KoTextString::toString( const Q3MemArray<KoTextStringChar> &data )
{
    QString s;
    int l = data.size();
    s.setUnicode( 0, l );
    KoTextStringChar *c = data.data();
    QChar *uc = (QChar *)s.unicode();
    while ( l-- ) {
	*uc = c->c;
	uc++;
	c++;
    }

    return s;
}

QString KoTextString::toReverseString() const
{
    QString s;
    int l = length();
    s.setUnicode(0, l);
    KoTextStringChar *c = data.data() + (l-1);
    QChar *uc = (QChar *)s.unicode();
    while ( l-- ) {
	*uc = c->c;
	uc++;
	c--;
    }

    return s;
}

QString KoTextString::stringToSpellCheck()
{
    if ( !bNeedsSpellCheck )
        return QString::null;

    bNeedsSpellCheck = false;
    if ( length() <= 1 )
        return QString::null;

    QString str = toString();
    str.truncate( str.length() - 1 ); // remove trailing space
    return str;
}

int KoTextString::nextCursorPosition( int next )
{
    if ( bidiDirty )
        checkBidi();

    const KoTextStringChar *c = data.data();
    int len = length();

    if ( next < len - 1 ) {
        next++;
        while ( next < len - 1 && !c[next].charStop )
            next++;
    }
    return next;
}

int KoTextString::previousCursorPosition( int prev )
{
    if ( bidiDirty )
        checkBidi();

    const KoTextStringChar *c = data.data();

    if ( prev ) {
        prev--;
        while ( prev && !c[prev].charStop )
            prev--;
    }
    return prev;
}

bool KoTextString::validCursorPosition( int idx )
{
    if ( bidiDirty )
        checkBidi();

    return (at( idx ).charStop);
}

////

void KoTextStringChar::setFormat( KoTextFormat *f, bool setFormatAgain )
{
    if ( type == Regular ) {
	d.format = f;
    } else {
	if ( !d.custom ) {
	    d.custom = new CustomData;
	    d.custom->custom = 0;
	}
	d.custom->format = f;
        if ( d.custom->custom && setFormatAgain )
            d.custom->custom->setFormat( f );
    }
}

void KoTextStringChar::setCustomItem( KoTextCustomItem *i )
{
    if ( type == Regular ) {
	KoTextFormat *f = format();
	d.custom = new CustomData;
	d.custom->format = f;
	type = Custom;
    } else {
	delete d.custom->custom;
    }
    d.custom->custom = i;
}

void KoTextStringChar::loseCustomItem() // setRegular() might be a better name
{
    if ( isCustom() ) {
	KoTextFormat *f = d.custom->format;
	d.custom->custom = 0;
	delete d.custom;
	type = Regular;
	d.format = f;
    }
}

KoTextStringChar::~KoTextStringChar()
{
    if ( format() )
	format()->removeRef();
    switch ( type ) {
	case Custom:
	    delete d.custom; break;
	default:
	    break;
    }
}

int KoTextStringChar::height() const
{
    return !isCustom() ? format()->height() : ( customItem()->placement() == KoTextCustomItem::PlaceInline ? customItem()->height : 0 );
}

int KoTextStringChar::ascent() const
{
    return !isCustom() ? format()->ascent() : ( customItem()->placement() == KoTextCustomItem::PlaceInline ? customItem()->ascent() : 0 );
}

int KoTextStringChar::descent() const
{
    return !isCustom() ? format()->descent() : 0;
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

KoTextFormatterBase::KoTextFormatterBase()
    : wrapColumn( -1 ), //wrapEnabled( TRUE ),
      m_bViewFormattingChars( false ),
      biw( true /*default in kotext*/ )
{
}

#ifdef BIDI_DEBUG
#include <iostream>
#endif

#if 0
// collects one line of the paragraph and transforms it to visual order
KoTextParagLineStart *KoTextFormatterBase::bidiReorderLine( KoTextParag * /*parag*/, KoTextString *text, KoTextParagLineStart *line,
							KoTextStringChar *startChar, KoTextStringChar *lastChar, int align, int space )
{
    int start = (startChar - &text->at(0));
    int last = (lastChar - &text->at(0) );
    //kDebug(32500) << "doing BiDi reordering from " << start << " to " << last << "!" << endl;

    KoBidiControl *control = new KoBidiControl( line->context(), line->status );
    QString str;
    str.setUnicode( 0, last - start + 1 );
    // fill string with logically ordered chars.
    KoTextStringChar *ch = startChar;
    QChar *qch = (QChar *)str.unicode();
    while ( ch <= lastChar ) {
	*qch = ch->c;
	qch++;
	ch++;
    }
    int x = startChar->x;

    Q3PtrList<KoTextRun> *runs;
    runs = KoComplexText::bidiReorderLine(control, str, 0, last - start + 1,
					 (text->isRightToLeft() ? QChar::DirR : QChar::DirL) );

    // now construct the reordered string out of the runs...

    int numSpaces = 0;
    // set the correct alignment. This is a bit messy....
    if( align == Qt::AlignLeft ) {
	// align according to directionality of the paragraph...
	if ( text->isRightToLeft() )
	    align = Qt::AlignRight;
    }

    if ( align & Qt::AlignHCenter )
	x += space/2;
    else if ( align & Qt::AlignRight )
	x += space;
    else if ( align & Qt::AlignJustify ) {
	for ( int j = start; j < last; ++j ) {
	    if( isBreakable( text, j ) ) {
		numSpaces++;
	    }
	}
    }
    int toAdd = 0;
    bool first = TRUE;
    KoTextRun *r = runs->first();
    int xmax = -0xffffff;
    while ( r ) {
	if(r->level %2) {
	    // odd level, need to reverse the string
	    int pos = r->stop + start;
	    while(pos >= r->start + start) {
		KoTextStringChar *c = &text->at(pos);
		if( numSpaces && !first && isBreakable( text, pos ) ) {
		    int s = space / numSpaces;
		    toAdd += s;
		    space -= s;
		    numSpaces--;
		} else if ( first ) {
		    first = FALSE;
		    if ( c->c == ' ' )
			x -= c->format()->width( ' ' );
		}
		c->x = x + toAdd;
		c->rightToLeft = TRUE;
		c->startOfRun = FALSE;
		int ww = 0;
		if ( c->c.unicode() >= 32 || c->c == '\t' || c->c == '\n' || c->isCustom() ) {
		    ww = c->width;
		} else {
		    ww = c->format()->width( ' ' );
		}
		if ( xmax < x + toAdd + ww ) xmax = x + toAdd + ww;
		x += ww;
		pos--;
	    }
	} else {
	    int pos = r->start + start;
	    while(pos <= r->stop + start) {
		KoTextStringChar* c = &text->at(pos);
		if( numSpaces && !first && isBreakable( text, pos ) ) {
		    int s = space / numSpaces;
		    toAdd += s;
		    space -= s;
		    numSpaces--;
		} else if ( first ) {
		    first = FALSE;
		    if ( c->c == ' ' )
			x -= c->format()->width( ' ' );
		}
		c->x = x + toAdd;
		c->rightToLeft = FALSE;
		c->startOfRun = FALSE;
		int ww = 0;
		if ( c->c.unicode() >= 32 || c->c == '\t' || c->isCustom() ) {
		    ww = c->width;
		} else {
		    ww = c->format()->width( ' ' );
		}
		//kDebug(32500) << "setting char " << pos << " at pos " << x << endl;
		if ( xmax < x + toAdd + ww ) xmax = x + toAdd + ww;
		x += ww;
		pos++;
	    }
	}
	text->at( r->start + start ).startOfRun = TRUE;
	r = runs->next();
    }

    line->w = xmax + 10;
    KoTextParagLineStart *ls = new KoTextParagLineStart( control->context, control->status );
    delete control;
    delete runs;
    return ls;
}
#endif

bool KoTextFormatterBase::isStretchable( KoTextString *string, int pos ) const
{
    if ( string->at( pos ).c == QChar(160) ) //non-breaking space
	return true;
    KoTextStringChar& chr = string->at( pos );
    return chr.whiteSpace;
    //return isBreakable( string, pos );
}

bool KoTextFormatterBase::isBreakable( KoTextString *string, int pos ) const
{
    //if (string->at(pos).nobreak)
    //    return FALSE;
    return (pos < string->length()-1 && string->at(pos+1).softBreak);
}

void KoTextParag::insertLineStart( int index, KoTextParagLineStart *ls )
{
    // This tests if we break at the same character in more than one line,
    // i.e. there no space even for _one_ char in a given line.
    // However this shouldn't happen, KoTextFormatter prevents it, otherwise
    // we could loop forever (e.g. if one char is wider than the page...)
#ifndef NDEBUG
    QMap<int, KoTextParagLineStart*>::Iterator it;
    if ( ( it = lineStarts.find( index ) ) == lineStarts.end() ) {
	lineStarts.insert( index, ls );
    } else {
        kWarning(32500) << "insertLineStart: there's already a line for char index=" << index << endl;
	delete *it;
	lineStarts.remove( it );
	lineStarts.insert( index, ls );
    }
#else // non-debug code, take the fast route
    lineStarts.insert( index, ls );
#endif
}


/* Standard pagebreak algorithm using KoTextFlow::adjustFlow. Returns
 the shift of the paragraphs bottom line.
 */
int KoTextFormatterBase::formatVertically( KoTextDocument* doc, KoTextParag* parag )
{
    int oldHeight = parag->rect().height();
    QMap<int, KoTextParagLineStart*>& lineStarts = parag->lineStartList();
    QMap<int, KoTextParagLineStart*>::Iterator it = lineStarts.begin();
    int h = doc->addMargins() ? parag->topMargin() : 0;
    for ( ; it != lineStarts.end() ; ++it  ) {
	KoTextParagLineStart * ls = it.data();
	ls->y = h;
	KoTextStringChar *c = &parag->string()->at(it.key());
	if ( c && c->customItem() && c->customItem()->ownLine() ) {
	    int h = c->customItem()->height;
	    c->customItem()->pageBreak( parag->rect().y() + ls->y + ls->baseLine - h, doc->flow() );
	    int delta = c->customItem()->height - h;
	    ls->h += delta;
	    if ( delta )
		parag->setMovedDown( TRUE );
	} else {
	    int shift = doc->flow()->adjustFlow( parag->rect().y() + ls->y, ls->w, ls->h );
	    ls->y += shift;
	    if ( shift )
		parag->setMovedDown( TRUE );
	}
	h = ls->y + ls->h;
    }
    int m = parag->bottomMargin();
    if ( parag->next() && doc && !doc->addMargins() )
	m = qMax( m, parag->next()->topMargin() );
    //if ( parag->next() && parag->next()->isLineBreak() )
    //	m = 0;
    h += m;
    parag->setHeight( h );
    return h - oldHeight;
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

KoTextCustomItem::KoTextCustomItem( KoTextDocument *p )
      :  width(-1), height(0), parent(p), xpos(0), ypos(-1), parag(0)
{
    m_deleted = false; // added for kotext
}

KoTextCustomItem::~KoTextCustomItem()
{
}

KoTextFlow::KoTextFlow()
{
    w = 0;
    leftItems.setAutoDelete( FALSE );
    rightItems.setAutoDelete( FALSE );
}

KoTextFlow::~KoTextFlow()
{
}

void KoTextFlow::clear()
{
    leftItems.clear();
    rightItems.clear();
}

// Called by KoTextDocument::setWidth
void KoTextFlow::setWidth( int width )
{
    w = width;
}

void KoTextFlow::adjustMargins( int, int, int, int&, int&, int& pageWidth, KoTextParag* )
{
    pageWidth = w;
}


int KoTextFlow::adjustFlow( int /*y*/, int, int /*h*/ )
{
    return 0;
}

void KoTextFlow::unregisterFloatingItem( KoTextCustomItem* item )
{
    leftItems.removeRef( item );
    rightItems.removeRef( item );
}

void KoTextFlow::registerFloatingItem( KoTextCustomItem* item )
{
    if ( item->placement() == KoTextCustomItem::PlaceRight ) {
	if ( !rightItems.contains( item ) )
	    rightItems.append( item );
    } else if ( item->placement() == KoTextCustomItem::PlaceLeft &&
		!leftItems.contains( item ) ) {
	leftItems.append( item );
    }
}

int KoTextFlow::availableHeight() const
{
    return -1; // no limit
}

void KoTextFlow::drawFloatingItems( QPainter* p, int cx, int cy, int cw, int ch, const QColorGroup& cg, bool selected )
{
    KoTextCustomItem *item;
    for ( item = leftItems.first(); item; item = leftItems.next() ) {
	if ( item->x() == -1 || item->y() == -1 )
	    continue;
	item->draw( p, item->x(), item->y(), cx, cy, cw, ch, cg, selected );
    }

    for ( item = rightItems.first(); item; item = rightItems.next() ) {
	if ( item->x() == -1 || item->y() == -1 )
	    continue;
	item->draw( p, item->x(), item->y(), cx, cy, cw, ch, cg, selected );
    }
}

//void KoTextFlow::setPageSize( int ps ) { pagesize = ps; }
bool KoTextFlow::isEmpty() { return leftItems.isEmpty() && rightItems.isEmpty(); }
