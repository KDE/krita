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

#include "TestPAPageInsertCommand.h"

#include <PAMock.h>
#include "KoPAPageInsertCommand.h"
#include "KoPAPage.h"
#include "KoPAMasterPage.h"

#include <qtest_kde.h>

void TestPAPageInsertCommand::redoUndo()
{
    MockDocument doc;

    KoPAMasterPage * master1 = new KoPAMasterPage();
    doc.insertPage( master1, 0 );

    KoPAPage * page1 = new KoPAPage( master1 );
    doc.insertPage( page1, 0 );

    KoPAPage * p1 = dynamic_cast<KoPAPage *>( doc.pageByIndex( 0, false ) );
    KoPAMasterPage * m1 = dynamic_cast<KoPAMasterPage *>( doc.pageByIndex( 0, true ) );

    QVERIFY( p1 != 0 );
    QVERIFY( m1 != 0 );

    KoPAPage * p2 = new KoPAPage( m1 );
    KoPAPage * p3 = new KoPAPage( m1 );

    KoPAPageInsertCommand cmd( &doc, p2, p1 );
    KoPAPageInsertCommand cmd2( &doc, p3, 0 );

    cmd.redo();
    cmd2.redo();

    QList<KoPAPage *> pages;
    pages.append( p3 );
    pages.append( p1 );
    pages.append( p2 );

    QList<KoPAPage *> allPages = pages;

    for( int i = 0; i < pages.size(); ++i ) {
        QVERIFY( pages[i] == doc.pageByIndex( i, false ) );
    }

    cmd2.undo();
    pages.removeAll( p3 );
    for( int i = 0; i < pages.size(); ++i ) {
        QVERIFY( pages[i] == doc.pageByIndex( i, false ) );
    }

    cmd.undo();
    pages.removeAll( p2 );
    for( int i = 0; i < pages.size(); ++i ) {
        QVERIFY( pages[i] == doc.pageByIndex( i, false ) );
    }

    cmd.redo();
    cmd2.redo();
    for( int i = 0; i < allPages.size(); ++i ) {
        QVERIFY( allPages[i] == doc.pageByIndex( i, false ) );
    }
}

void TestPAPageInsertCommand::redoUndoMaster()
{
    MockDocument doc;

    KoPAMasterPage * master1 = new KoPAMasterPage();
    doc.insertPage( master1, 0 );

    KoPAMasterPage * m1 = dynamic_cast<KoPAMasterPage *>( doc.pageByIndex( 0, true ) );

    QVERIFY( m1 != 0 );

    KoPAMasterPage * m2 = new KoPAMasterPage();
    KoPAMasterPage * m3 = new KoPAMasterPage();

    KoPAPageInsertCommand cmd( &doc, m2, m1 );
    KoPAPageInsertCommand cmd2( &doc, m3, 0 );

    cmd.redo();
    cmd2.redo();

    QList<KoPAMasterPage *> masterPages;
    masterPages.append( m3 );
    masterPages.append( m1 );
    masterPages.append( m2 );

    QList<KoPAMasterPage *> allMasterPages = masterPages;

    for( int i = 0; i < masterPages.size(); ++i ) {
        QVERIFY( masterPages[i] == doc.pageByIndex( i, true ) );
    }

    cmd2.undo();
    masterPages.removeAll( m3 );
    for( int i = 0; i < masterPages.size(); ++i ) {
        QVERIFY( masterPages[i] == doc.pageByIndex( i, true ) );
    }

    cmd.undo();
    masterPages.removeAll( m2 );
    for( int i = 0; i < masterPages.size(); ++i ) {
        QVERIFY( masterPages[i] == doc.pageByIndex( i, true ) );
    }

    cmd.redo();
    cmd2.redo();
    for( int i = 0; i < allMasterPages.size(); ++i ) {
        QVERIFY( allMasterPages[i] == doc.pageByIndex( i, true ) );
    }
}

QTEST_KDEMAIN( TestPAPageInsertCommand, GUI )
#include "TestPAPageInsertCommand.moc"
