/* This file is part of the KDE project
   Copyright (C) 2002-2006 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoTextIterator.h"
#include "KoTextParag.h"
#include "KoTextView.h"
#include <kfinddialog.h>
#include <kfind.h>
#include <kdebug.h>
#include <assert.h>
//Added by qt3to4:
#include <Q3ValueList>

//#define DEBUG_ITERATOR

/**
 * The search direction (forward or backward) is handled in a bit of a tricky way.
 * m_firstParag/m_firstIndex is where the search starts, whichever the direction
 * m_lastParag/m_lastIndex is where the search ends, whichever the direction
 * But the list of textobjects is as given (we assume document order).
 * So we go from the first to the last textobject, or from the last to the first textobject.
 */

void KoTextIterator::init( const Q3ValueList<KoTextObject *> & lstObjects, KoTextView* textView, int options )
{
    Q_ASSERT( !lstObjects.isEmpty() );

    m_lstObjects.clear();
    m_firstParag = 0;
    m_firstIndex = 0;
    m_options = options;

    // 'From Cursor' option
    if ( options & KFind::FromCursor )
    {
        if ( textView ) {
            m_firstParag = textView->cursor()->parag();
            m_firstIndex = textView->cursor()->index();
        } else {
            // !? FromCursor option can't work
            m_options &= ~KFind::FromCursor;
            kWarning(32500) << "FromCursor specified, but no textview?" << endl;
        }
    } // no else here !

    bool forw = ! ( options & KFind::FindBackwards );

    // 'Selected Text' option
    if ( textView && ( options & KFind::SelectedText ) )
    {
        KoTextObject* textObj = textView->textObject();
        KoTextCursor c1 = textObj->textDocument()->selectionStartCursor( KoTextDocument::Standard );
        KoTextCursor c2 = textObj->textDocument()->selectionEndCursor( KoTextDocument::Standard );
        if ( !m_firstParag ) // not from cursor
        {
            m_firstParag = forw ? c1.parag() : c2.parag();
            m_firstIndex = forw ? c1.index() : c2.index();
        }
        m_lastParag = forw ? c2.parag() : c1.parag();
        m_lastIndex = forw ? c2.index() : c1.index();
        // Find in the selection only -> only one textobject
        m_lstObjects.append( textObj );
        m_currentTextObj = m_lstObjects.begin();
    }
    else
    {
        // Not "selected text" -> loop through all textobjects
        m_lstObjects = lstObjects;
        if ( textView && (options & KFind::FromCursor) )
        {
            KoTextObject* initialFirst = m_lstObjects.first();
            // textView->textObject() should be first in m_lstObjects (last when going backwards) !
            // Let's ensure this is the case, but without changing the order of the objects.
            if ( forw ) {
                while( m_lstObjects.first() != textView->textObject() ) {
                    KoTextObject* textobj = m_lstObjects.front();
                    m_lstObjects.pop_front();
                    m_lstObjects.push_back( textobj );
                    if ( m_lstObjects.first() == initialFirst ) { // safety
                        kWarning(32500) << "Didn't manage to find " << textView->textObject() << " in the list of textobjects!!!" << endl;
                        break;
                    }
                }
            } else {
                while( m_lstObjects.last() != textView->textObject() ) {
                    KoTextObject* textobj = m_lstObjects.back();
                    m_lstObjects.pop_back();
                    m_lstObjects.push_front( textobj );
                    if ( m_lstObjects.first() == initialFirst ) { // safety
                        kWarning(32500) << "Didn't manage to find " << textView->textObject() << " in the list of textobjects!!!" << endl;
                        break;
                    }
                }
            }
        }

        KoTextParag* firstParag = m_lstObjects.first()->textDocument()->firstParag();
        int firstIndex = 0;
        KoTextParag* lastParag = m_lstObjects.last()->textDocument()->lastParag();
        int lastIndex = lastParag->length()-1;
        if ( !m_firstParag ) // only set this when not 'from cursor'.
        {
            m_firstParag = forw ? firstParag : lastParag;
            m_firstIndex = forw ? firstIndex : lastIndex;
        }
        // always set the ending point
        m_lastParag = forw ? lastParag : firstParag;
        m_lastIndex = forw ? lastIndex : firstIndex;
        m_currentTextObj = forw ? m_lstObjects.at(0) : m_lstObjects.fromLast();
    }

    assert( *m_currentTextObj ); // all branches set it
    assert( m_firstParag );
    assert( m_lastParag );
    Q_ASSERT( (*m_currentTextObj)->isVisible() );
    m_currentParag = m_firstParag;
#ifdef DEBUG_ITERATOR
    kDebug(32500) << "KoTextIterator::init from(" << *m_currentTextObj << "," << m_firstParag->paragId() << ") - to(" << (forw?m_lstObjects.last():m_lstObjects.first()) << "," << m_lastParag->paragId() << "), " << m_lstObjects.count() << " textObjects." << endl;
    Q3ValueList<KoTextObject *>::Iterator it = m_lstObjects.begin();
    for( ; it != m_lstObjects.end(); ++it )
        kDebug(32500) << (*it) << " " << (*it)->name() << endl;
#endif
    Q_ASSERT( (*m_currentTextObj)->textDocument() == m_currentParag->textDocument() );
    Q_ASSERT( (forw?m_lstObjects.last():m_lstObjects.first())->textDocument() == m_lastParag->textDocument() );

    connectTextObjects();
}

void KoTextIterator::restart()
{
    if( m_lstObjects.isEmpty() )
        return;
    m_currentParag = m_firstParag;
    bool forw = ! ( m_options & KFind::FindBackwards );
    Q_ASSERT( ! (m_options & KFind::FromCursor) ); // doesn't make much sense to keep it, right?
    if ( (m_options & KFind::FromCursor) || forw )
        m_currentTextObj = m_lstObjects.begin();
    else
        m_currentTextObj = m_lstObjects.fromLast();
    if ( !(*m_currentTextObj)->isVisible() )
        nextTextObject();
#ifdef DEBUG_ITERATOR
    if ( m_currentParag )
        kDebug(32500) << "KoTextIterator::restart from(" << *m_currentTextObj << "," << m_currentParag->paragId() << ") - to(" << (forw?m_lstObjects.last():m_lstObjects.first()) << "," << m_lastParag->paragId() << "), " << m_lstObjects.count() << " textObjects." << endl;
    else
        kDebug(32500) << "KoTextIterator::restart - nowhere to go!" << endl;
#endif
}

void KoTextIterator::connectTextObjects()
{
    Q3ValueList<KoTextObject *>::Iterator it = m_lstObjects.begin();
    for( ; it != m_lstObjects.end(); ++it ) {
        connect( (*it), SIGNAL( paragraphDeleted( KoTextParag* ) ),
                 this, SLOT( slotParagraphDeleted( KoTextParag* ) ) );
        connect( (*it), SIGNAL( paragraphModified( KoTextParag*, int, int, int ) ),
                 this, SLOT( slotParagraphModified( KoTextParag*, int, int, int ) ) );
        // We don't connect to destroyed(), because for undo/redo purposes,
        // we never really delete textdocuments nor textobjects.
        // So this is never called.
        // Instead the textobject is simply set to invisible, and this is handled by nextTextObject
    }
}

void KoTextIterator::slotParagraphModified( KoTextParag* parag, int modifyType, int pos, int length )
{
    if ( parag == m_currentParag )
        emit currentParagraphModified( modifyType, pos, length );
}

void KoTextIterator::slotParagraphDeleted( KoTextParag* parag )
{
#ifdef DEBUG_ITERATOR
    kDebug(32500) << "KoTextIterator::slotParagraphDeleted " << parag << " (" << parag->paragId() << ")" << endl;
#endif
    // Note that the direction doesn't matter here. A begin/end
    // at end of parag N or at beginning of parag N+1 is the same,
    // and m_firstIndex/m_lastIndex becomes irrelevant, anyway.
    if ( parag == m_lastParag )
    {
        if ( m_lastParag->prev() ) {
            m_lastParag = m_lastParag->prev();
            m_lastIndex = m_lastParag->length()-1;
        } else {
            m_lastParag = m_lastParag->next();
            m_lastIndex = 0;
        }
    }
    if ( parag == m_firstParag )
    {
        if ( m_firstParag->prev() ) {
            m_firstParag = m_firstParag->prev();
            m_firstIndex = m_firstParag->length()-1;
        } else {
            m_firstParag = m_firstParag->next();
            m_firstIndex = 0;
        }
    }
    if ( parag == m_currentParag )
    {
        operator++();
        emit currentParagraphDeleted();
    }
#ifdef DEBUG_ITERATOR
    if ( m_currentParag )
        kDebug(32500) << "KoTextIterator: firstParag:" << m_firstParag << " (" << m_firstParag->paragId() << ") -  lastParag:" << m_lastParag << " (" << m_lastParag->paragId() << ") m_currentParag:" << m_currentParag << " (" << m_currentParag->paragId() << ")" << endl;
#endif
}

// Go to next paragraph that we must iterate over
void KoTextIterator::operator++()
{
    if ( !m_currentParag ) {
        kDebug(32500) << k_funcinfo << " called past the end" << endl;
        return;
    }
    if ( m_currentParag == m_lastParag ) {
        m_currentParag = 0L;
#ifdef DEBUG_ITERATOR
        kDebug(32500) << "KoTextIterator++: done, after last parag " << m_lastParag << endl;
#endif
        return;
    }
    bool forw = ! ( m_options & KFind::FindBackwards );
    KoTextParag* parag = forw ? m_currentParag->next() : m_currentParag->prev();
    if ( parag )
    {
        m_currentParag = parag;
    }
    else
    {
        nextTextObject();
    }
#ifdef DEBUG_ITERATOR
    if ( m_currentParag )
        kDebug(32500) << "KoTextIterator++ (" << *m_currentTextObj << "," <<
            m_currentParag->paragId() << ")" << endl;
    else
        kDebug(32500) << "KoTextIterator++ (at end)" << endl;
#endif
}

void KoTextIterator::nextTextObject()
{
    bool forw = ! ( m_options & KFind::FindBackwards );
    do {
        if ( forw ) {
            ++m_currentTextObj;
            if ( m_currentTextObj == m_lstObjects.end() )
                m_currentParag = 0L; // done
            else
                m_currentParag = (*m_currentTextObj)->textDocument()->firstParag();
        } else {
            if ( m_currentTextObj == m_lstObjects.begin() )
                m_currentParag = 0L; // done
            else
            {
                --m_currentTextObj;
                m_currentParag = (*m_currentTextObj)->textDocument()->lastParag();
            }
        }
    }
    // loop in case this new textobject is not visible
    while ( m_currentParag && !(*m_currentTextObj)->isVisible() );
#ifdef DEBUG_ITERATOR
    if ( m_currentParag )
        kDebug(32500) << k_funcinfo << " m_currentTextObj=" << (*m_currentTextObj) << endl;
#endif
}

bool KoTextIterator::atEnd() const
{
    // operator++ sets m_currentParag to 0 when it's done
    return m_currentParag == 0L;
}

int KoTextIterator::currentStartIndex() const
{
    return currentTextAndIndex().first;
}

QString KoTextIterator::currentText() const
{
    return currentTextAndIndex().second;
}

QPair<int, QString> KoTextIterator::currentTextAndIndex() const
{
    Q_ASSERT( m_currentParag );
    Q_ASSERT( m_currentParag->string() );
    QString str = m_currentParag->string()->toString();
    str.truncate( str.length() - 1 ); // remove trailing space
    bool forw = ! ( m_options & KFind::FindBackwards );
    if ( m_currentParag == m_firstParag )
    {
        if ( m_firstParag == m_lastParag ) // special case, needs truncating at both ends
            return forw ? qMakePair( m_firstIndex, str.mid( m_firstIndex, m_lastIndex - m_firstIndex ) )
                : qMakePair( m_lastIndex, str.mid( m_lastIndex, m_firstIndex - m_lastIndex ) );
        else
            return forw ? qMakePair( m_firstIndex, str.mid( m_firstIndex ) )
                        : qMakePair( 0, str.left( m_firstIndex ) );
    }
    if ( m_currentParag == m_lastParag )
    {
        return forw ? qMakePair( 0, str.left( m_lastIndex ) )
                    : qMakePair( m_lastIndex, str.mid( m_lastIndex ) );
    }
    // Not the first parag, nor the last, so we return it all
    return qMakePair( 0, str );
}

bool KoTextIterator::hasText() const
{
    // Same logic as currentTextAndIndex, but w/o calling it, to avoid all the string copying
    bool forw = ! ( m_options & KFind::FindBackwards );
    int strLength = m_currentParag->string()->length() - 1;
    if ( m_currentParag == m_firstParag )
    {
        if ( m_firstParag == m_lastParag )
            return m_firstIndex < m_lastIndex;
        else
            return forw ? m_firstIndex < strLength
                        : m_firstIndex > 0;
    }
    if ( m_currentParag == m_lastParag )
        return forw ? m_lastIndex > 0
                    : m_lastIndex < strLength;
    return strLength > 0;
}

void KoTextIterator::setOptions( int options )
{
    if ( m_options != options )
    {
        bool wasBack = (m_options & KFind::FindBackwards);
        bool isBack = (options & KFind::FindBackwards);
        if ( wasBack != isBack )
        {
            qSwap( m_firstParag, m_lastParag );
            qSwap( m_firstIndex, m_lastIndex );
            if ( m_currentParag == 0 ) // done? -> reinit
            {
#ifdef DEBUG_ITERATOR
                kDebug(32500) << k_funcinfo << "was done -> reinit" << endl;
#endif
                restart();
            }
        }
        bool wasFromCursor = (m_options & KFind::FromCursor);
        bool isFromCursor = (options & KFind::FromCursor);
        // We can only handle the case where fromcursor got removed.
        // If it got added, then we need a textview to take the cursor position from...
        if ( wasFromCursor && !isFromCursor )
        {
            // We also can't handle the "selected text" option here
            // It's very hard to have a cursor that's not at the beginning
            // or end of the selection, anyway.
            if ( ! (options & KFind::SelectedText ) )
            {
                // Set m_firstParag/m_firstIndex to the beginning of the first object
                // (end of last object when going backwards)
                KoTextParag* firstParag = m_lstObjects.first()->textDocument()->firstParag();
                int firstIndex = 0;
                KoTextParag* lastParag = m_lstObjects.last()->textDocument()->lastParag();
                int lastIndex = lastParag->length()-1;
                m_firstParag = (!isBack) ? firstParag : lastParag;
                m_firstIndex = (!isBack) ? firstIndex : lastIndex;
#ifdef DEBUG_ITERATOR
                kDebug(32500) << "setOptions: FromCursor removed. New m_firstParag=" << m_firstParag << " (" << m_firstParag->paragId() << ") isBack=" << isBack << endl;
#endif
            }
        }
        m_options = options;
    }
}

#include "KoTextIterator.moc"
