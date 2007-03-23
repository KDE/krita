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

#include "TestPAPageDeleteCommand.h"

#include "PAMock.h"
#include "KoPAPageDeleteCommand.h"
#include "KoPAPage.h"
#include "KoPAMasterPage.h"

#include <qtest_kde.h>

void TestPAPageDeleteCommand::redoUndo()
{
    MockDocument doc;

    KoPAPage * p1 = dynamic_cast<KoPAPage *>( doc.pageByIndex( 0, false ) );
    KoPAMasterPage * m1 = dynamic_cast<KoPAMasterPage *>( doc.pageByIndex( 0, true ) );

    QVERIFY( p1 != 0 );
    QVERIFY( m1 != 0 );

    KoPAMasterPage * m2 = new KoPAMasterPage();
    KoPAMasterPage * m3 = new KoPAMasterPage();
    doc.insertPage( m2, 1 );
    doc.insertPage( m3, 2 );

    KoPAPageDeleteCommand cmd( &doc, m1 );
    KoPAPageDeleteCommand cmd2( &doc, m3 );


    QList<KoPAMasterPage *> masterPages;
    masterPages.append( m1 );
    masterPages.append( m2 );
    masterPages.append( m3 );

    QList<KoPAMasterPage *> allMasterPages = masterPages;

    for( int i = 0; i < masterPages.size(); ++i ) {
        QVERIFY( masterPages[i] == doc.pageByIndex( i, true ) );
    }

    cmd.redo();

    masterPages.removeAt( 0 );
    for( int i = 0; i < masterPages.size(); ++i ) {
        QVERIFY( masterPages[i] == doc.pageByIndex( i, true ) );
    }

    cmd2.redo();

    masterPages.removeAt( 1 );
    for( int i = 0; i < masterPages.size(); ++i ) {
        QVERIFY( masterPages[i] == doc.pageByIndex( i, true ) );
    }

    for( int i = 0; i < masterPages.size(); ++i ) {
        QVERIFY( masterPages[i] == doc.pageByIndex( i, true ) );
    }

    cmd2.undo();
    cmd.undo();

    for( int i = 0; i < allMasterPages.size(); ++i ) {
        QVERIFY( allMasterPages[i] == doc.pageByIndex( i, true ) );
    }
}

QTEST_KDEMAIN( TestPAPageDeleteCommand, GUI )
#include "TestPAPageDeleteCommand.moc"
