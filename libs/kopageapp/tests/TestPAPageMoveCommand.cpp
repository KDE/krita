/* This file is part of the KDE project
 * Copyright ( C ) 2007 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2009 Fredy Yanardi <fyanardi@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or ( at your option ) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "TestPAPageMoveCommand.h"

#include "KoPAPageMoveCommand.h"
#include "KoPAMasterPage.h"

#include <KDebug>

#include <qtest_kde.h>

void TestPAPageMoveCommand::initTestCase()
{
    for( int i = 0; i < 5; ++i )
    {
        m_pages.insert( i, new KoPAMasterPage() );
    }
    m_doc.insertPage( m_pages[0], -1 );
}

void TestPAPageMoveCommand::cleanupTestCase()
{
}

void TestPAPageMoveCommand::init()
{
    for ( int i = 1; i < 5; ++i )
    {
        m_doc.insertPage( m_pages[i], -1 );
    }
}

void TestPAPageMoveCommand::cleanup()
{
    for ( int i = 1; i < 5; ++i )
    {
        m_doc.takePage( m_pages[i] );
    }
}

void TestPAPageMoveCommand::redoUndoAfter()
{
    KoPAPageMoveCommand cmd1( &m_doc, m_pages[0], m_pages[1] );

    QList<KoPAMasterPage *> pages1;
    pages1.insert( 0, m_pages[1] );
    pages1.insert( 1, m_pages[0] );
    pages1.insert( 2, m_pages[2] );
    pages1.insert( 3, m_pages[3] );
    pages1.insert( 4, m_pages[4] );

    cmd1.redo();
    checkOrder( pages1 );

    KoPAPageMoveCommand cmd2( &m_doc, m_pages[1], m_pages[2] );

    QList<KoPAMasterPage *> pages2;
    pages2.insert( 0, m_pages[0] );
    pages2.insert( 1, m_pages[2] );
    pages2.insert( 2, m_pages[1] );
    pages2.insert( 3, m_pages[3] );
    pages2.insert( 4, m_pages[4] );

    cmd2.redo();
    checkOrder( pages2 );

    KoPAPageMoveCommand cmd3( &m_doc, m_pages[1], m_pages[4] );

    QList<KoPAMasterPage *> pages3;
    pages3.insert( 0, m_pages[0] );
    pages3.insert( 1, m_pages[2] );
    pages3.insert( 2, m_pages[3] );
    pages3.insert( 3, m_pages[4] );
    pages3.insert( 4, m_pages[1] );

    cmd3.redo();
    checkOrder( pages3 );

    // Moving multiple consecutive pages
    QList<KoPAPageBase *> multiPages1;
    multiPages1.insert( 0, m_pages[3] );
    multiPages1.insert( 1, m_pages[4] );
    QList<KoPAMasterPage *> pages4;
    pages4.insert( 0, m_pages[0] );
    pages4.insert( 1, m_pages[3] );
    pages4.insert( 2, m_pages[4] );
    pages4.insert( 3, m_pages[2] );
    pages4.insert( 4, m_pages[1] );

    KoPAPageMoveCommand cmd4( &m_doc, multiPages1, m_pages[0] );
    cmd4.redo();
    checkOrder( pages4 );

    // Moving multiple non-consecutive pages
    QList<KoPAPageBase *> multiPages2;
    multiPages2.insert( 0, m_pages[4] );
    multiPages2.insert( 1, m_pages[1] );
    QList<KoPAMasterPage *> pages5;
    pages5.insert( 0, m_pages[0] );
    pages5.insert( 1, m_pages[4] );
    pages5.insert( 2, m_pages[1] );
    pages5.insert( 3, m_pages[3] );
    pages5.insert( 4, m_pages[2] );

    KoPAPageMoveCommand cmd5( &m_doc, multiPages2, m_pages[0] );
    cmd5.redo();
    checkOrder( pages5 );

    cmd5.undo();
    checkOrder( pages4 );

    cmd4.undo();
    checkOrder( pages3 );

    cmd3.undo();
    checkOrder( pages2 );

    cmd2.undo();
    checkOrder( pages1 );

    cmd1.undo();
    checkOrder( m_pages );
}

void TestPAPageMoveCommand::redoUndoBefore()
{
    KoPAPageMoveCommand cmd1( &m_doc, m_pages[1], 0 );

    QList<KoPAMasterPage *> pages1;
    pages1.insert( 0, m_pages[1] );
    pages1.insert( 1, m_pages[0] );
    pages1.insert( 2, m_pages[2] );
    pages1.insert( 3, m_pages[3] );
    pages1.insert( 4, m_pages[4] );

    cmd1.redo();
    checkOrder( pages1 );

    KoPAPageMoveCommand cmd2( &m_doc, m_pages[2], m_pages[1] );

    QList<KoPAMasterPage *> pages2;
    pages2.insert( 0, m_pages[1] );
    pages2.insert( 1, m_pages[2] );
    pages2.insert( 2, m_pages[0] );
    pages2.insert( 3, m_pages[3] );
    pages2.insert( 4, m_pages[4] );

    cmd2.redo();
    checkOrder( pages2 );

    KoPAPageMoveCommand cmd3( &m_doc, m_pages[4], m_pages[1] );

    QList<KoPAMasterPage *> pages3;
    pages3.insert( 0, m_pages[1] );
    pages3.insert( 1, m_pages[4] );
    pages3.insert( 2, m_pages[2] );
    pages3.insert( 3, m_pages[0] );
    pages3.insert( 4, m_pages[3] );

    cmd3.redo();
    checkOrder( pages3 );

    // Moving multiple consecutive pages
    QList<KoPAPageBase *> multiPages1;
    multiPages1.insert( 0, m_pages[4] );
    multiPages1.insert( 1, m_pages[2] );
    QList<KoPAMasterPage *> pages4;
    pages4.insert( 0, m_pages[1] );
    pages4.insert( 1, m_pages[0] );
    pages4.insert( 2, m_pages[4] );
    pages4.insert( 3, m_pages[2] );
    pages4.insert( 4, m_pages[3] );

    KoPAPageMoveCommand cmd4( &m_doc, multiPages1, m_pages[0] );
    cmd4.redo();
    checkOrder( pages4 );

    // Moving multiple non-consecutive pages
    QList<KoPAPageBase *> multiPages2;
    multiPages2.insert( 0, m_pages[1] );
    multiPages2.insert( 1, m_pages[4] );
    QList<KoPAMasterPage *> pages5;
    pages5.insert( 0, m_pages[0] );
    pages5.insert( 1, m_pages[2] );
    pages5.insert( 2, m_pages[3] );
    pages5.insert( 3, m_pages[1] );
    pages5.insert( 4, m_pages[4] );

    KoPAPageMoveCommand cmd5( &m_doc, multiPages2, m_pages[3] );
    cmd5.redo();
    checkOrder( pages5 );

    cmd5.undo();
    checkOrder( pages4 );

    cmd4.undo();
    checkOrder( pages3 );

    cmd3.undo();
    checkOrder( pages2 );

    cmd2.undo();
    checkOrder( pages1 );

    cmd1.undo();
}

void TestPAPageMoveCommand::redoUndoStart()
{
    KoPAPageMoveCommand cmd1( &m_doc, m_pages[4], 0 );

    QList<KoPAMasterPage *> pages1;
    pages1.insert( 0, m_pages[4] );
    pages1.insert( 1, m_pages[0] );
    pages1.insert( 2, m_pages[1] );
    pages1.insert( 3, m_pages[2] );
    pages1.insert( 4, m_pages[3] );

    cmd1.redo();
    checkOrder( pages1 );

    KoPAPageMoveCommand cmd2( &m_doc, m_pages[1], 0 );

    QList<KoPAMasterPage *> pages2;
    pages2.insert( 0, m_pages[1] );
    pages2.insert( 1, m_pages[4] );
    pages2.insert( 2, m_pages[0] );
    pages2.insert( 3, m_pages[2] );
    pages2.insert( 4, m_pages[3] );

    cmd2.redo();
    checkOrder( pages2 );

    KoPAPageMoveCommand cmd3( &m_doc, m_pages[4], 0 );

    QList<KoPAMasterPage *> pages3;
    pages3.insert( 0, m_pages[4] );
    pages3.insert( 1, m_pages[1] );
    pages3.insert( 2, m_pages[0] );
    pages3.insert( 3, m_pages[2] );
    pages3.insert( 4, m_pages[3] );

    cmd3.redo();
    checkOrder( pages3 );

    // Moving multiple consecutive pages
    QList<KoPAPageBase *> multiPages1;
    multiPages1.insert( 0, m_pages[1] );
    multiPages1.insert( 1, m_pages[0] );
    QList<KoPAMasterPage *> pages4;
    pages4.insert( 0, m_pages[1] );
    pages4.insert( 1, m_pages[0] );
    pages4.insert( 2, m_pages[4] );
    pages4.insert( 3, m_pages[2] );
    pages4.insert( 4, m_pages[3] );

    KoPAPageMoveCommand cmd4( &m_doc, multiPages1, 0 );
    cmd4.redo();
    checkOrder( pages4 );

    // Moving multiple non-consecutive pages
    QList<KoPAPageBase *> multiPages2;
    multiPages2.insert( 0, m_pages[0] );
    multiPages2.insert( 1, m_pages[2] );
    QList<KoPAMasterPage *> pages5;
    pages5.insert( 0, m_pages[0] );
    pages5.insert( 1, m_pages[2] );
    pages5.insert( 2, m_pages[1] );
    pages5.insert( 3, m_pages[4] );
    pages5.insert( 4, m_pages[3] );

    KoPAPageMoveCommand cmd5( &m_doc, multiPages2, 0 );
    cmd5.redo();
    checkOrder( pages5 );

    cmd5.undo();
    checkOrder( pages4 );

    cmd4.undo();
    checkOrder( pages3 );

    cmd3.undo();
    checkOrder( pages2 );

    cmd2.undo();
    checkOrder( pages1 );

    cmd1.undo();
    checkOrder( m_pages );
}

void TestPAPageMoveCommand::redoUndoEnd()
{
    KoPAPageMoveCommand cmd1( &m_doc, m_pages[0], m_pages[4] );

    QList<KoPAMasterPage *> pages1;
    pages1.insert( 0, m_pages[1] );
    pages1.insert( 1, m_pages[2] );
    pages1.insert( 2, m_pages[3] );
    pages1.insert( 3, m_pages[4] );
    pages1.insert( 4, m_pages[0] );

    cmd1.redo();
    checkOrder( pages1 );

    KoPAPageMoveCommand cmd2( &m_doc, m_pages[3], m_pages[0] );

    QList<KoPAMasterPage *> pages2;
    pages2.insert( 0, m_pages[1] );
    pages2.insert( 1, m_pages[2] );
    pages2.insert( 2, m_pages[4] );
    pages2.insert( 3, m_pages[0] );
    pages2.insert( 4, m_pages[3] );

    cmd2.redo();
    checkOrder( pages2 );

    KoPAPageMoveCommand cmd3( &m_doc, m_pages[0], m_pages[3] );

    QList<KoPAMasterPage *> pages3;
    pages3.insert( 0, m_pages[1] );
    pages3.insert( 1, m_pages[2] );
    pages3.insert( 2, m_pages[4] );
    pages3.insert( 3, m_pages[3] );
    pages3.insert( 4, m_pages[0] );

    cmd3.redo();
    checkOrder( pages3 );

    // Moving multiple consecutive pages
    QList<KoPAPageBase *> multiPages1;
    multiPages1.insert( 0, m_pages[2] );
    multiPages1.insert( 1, m_pages[4] );
    QList<KoPAMasterPage *> pages4;
    pages4.insert( 0, m_pages[1] );
    pages4.insert( 1, m_pages[3] );
    pages4.insert( 2, m_pages[0] );
    pages4.insert( 3, m_pages[2] );
    pages4.insert( 4, m_pages[4] );

    KoPAPageMoveCommand cmd4( &m_doc, multiPages1, m_pages[0] );
    cmd4.redo();
    checkOrder( pages4 );

    // Moving multiple non-consecutive pages
    QList<KoPAPageBase *> multiPages2;
    multiPages2.insert( 0, m_pages[1] );
    multiPages2.insert( 1, m_pages[0] );
    QList<KoPAMasterPage *> pages5;
    pages5.insert( 0, m_pages[3] );
    pages5.insert( 1, m_pages[2] );
    pages5.insert( 2, m_pages[4] );
    pages5.insert( 3, m_pages[1] );
    pages5.insert( 4, m_pages[0] );

    KoPAPageMoveCommand cmd5( &m_doc, multiPages2, m_pages[4] );
    cmd5.redo();
    checkOrder( pages5 );

    cmd5.undo();
    checkOrder( pages4 );

    cmd4.undo();
    checkOrder( pages3 );

    cmd3.undo();
    checkOrder( pages2 );

    cmd2.undo();
    checkOrder( pages1 );

    cmd1.undo();
    checkOrder( m_pages );
}

// Moving multiple pages, where those pages are split into two sets, one set contains pages before "after"
// and the other set contains pages after "after"
void TestPAPageMoveCommand::redoUndoAfterInBetween()
{
    QList<KoPAPageBase *> multiPages;
    multiPages.insert( 0, m_pages[1] );
    multiPages.insert( 1, m_pages[3] );
    QList<KoPAMasterPage *> pages1;
    pages1.insert( 0, m_pages[0] );
    pages1.insert( 1, m_pages[2] );
    pages1.insert( 2, m_pages[1] );
    pages1.insert( 3, m_pages[3] );
    pages1.insert( 4, m_pages[4] );

    KoPAPageMoveCommand cmd( &m_doc, multiPages, m_pages[2] );
    cmd.redo();
    checkOrder( pages1 );

    cmd.undo();
    checkOrder( m_pages );
}

void TestPAPageMoveCommand::checkOrder( QList<KoPAMasterPage*> & pages )
{
    for( int i = 0; i < pages.size(); ++i ) {
        QVERIFY( pages[i] == m_doc.pageByIndex( i, true ) );
    }
}

QTEST_KDEMAIN( TestPAPageMoveCommand, GUI )

#include <TestPAPageMoveCommand.moc>

