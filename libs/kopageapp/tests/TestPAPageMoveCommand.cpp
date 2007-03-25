/* This file is part of the KDE project
 * Copyright ( C ) 2007 Thorsten Zachmann <zachmann@kde.org>
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

#include <qtest_kde.h>

void TestPAPageMoveCommand::initTestCase()
{
    KoPAMasterPage *page = dynamic_cast<KoPAMasterPage *>( m_doc.pageByIndex( 0, true ) );
    QVERIFY( page != 0 );

    m_doc.takePage( page );

    m_pages.insert( 0, page );

    for( int i = 1; i < 5; ++i )
    {
        m_pages.insert( i, new KoPAMasterPage() );
    }
}

void TestPAPageMoveCommand::cleanupTestCase()
{
}

void TestPAPageMoveCommand::init()
{
    for( int i = 0; i < 5; ++i )
    {
        m_doc.insertPage( m_pages[i], i );
    }
}

void TestPAPageMoveCommand::cleanup()
{
    foreach( KoPAMasterPage * page, m_pages )
    {
        m_doc.takePage( page );
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

    cmd3.undo();
    checkOrder( pages2 );

    cmd2.undo();
    checkOrder( pages1 );

    cmd1.undo();
    checkOrder( m_pages );
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

    cmd3.undo();
    checkOrder( pages2 );

    cmd2.undo();
    checkOrder( pages1 );

    cmd1.undo();
    checkOrder( m_pages );
}

void TestPAPageMoveCommand::checkOrder( QList<KoPAMasterPage*> & pages )
{
    for( int i = 0; i < pages.size(); ++i ) {
        QVERIFY( pages[i] == m_doc.pageByIndex( i, true ) );
    }
}

QTEST_KDEMAIN( TestPAPageMoveCommand, GUI )
#include "TestPAPageMoveCommand.moc"
