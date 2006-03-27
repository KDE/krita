/* This file is part of the KDE project
   Copyright (C) 2005 David Faure <faure@kde.org>

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
   Boston, MA 02110-1301, USA.
*/

// KoUserStyle/KoUserStyleCollection test

#include <kunittest/runner.h>
#include <kunittest/module.h>

#include <KoUserStyleCollection.h>
#include <KoUserStyle.h>
#include <kdebug.h>
#include <kglobal.h>

#include "KoUserStyleTester.h"
#include "KoUserStyleTester.moc"

KUNITTEST_MODULE(kunittest_KoUserStyleTester, "KoUserStyle Tester");
KUNITTEST_MODULE_REGISTER_TESTER(KoUserStyleTester);

#undef COMPARE
/// for source-compat with qttestlib: use COMPARE(x,y) if you plan to port to qttestlib later.
#define COMPARE CHECK

/// for source-compat with qttestlib: use VERIFY(x) if you plan to port to qttestlib later.
#undef VERIFY
#define VERIFY( x ) CHECK( x, true )

void KoUserStyleTester::testEmptyCollection()
{
    KoUserStyleCollection coll( "test" );
    VERIFY( coll.isEmpty() );
    COMPARE( coll.count(), 0 );
    VERIFY( coll.styleList().isEmpty() );
}

void KoUserStyleTester::testAddStyle()
{
    KoUserStyleCollection coll( "test" );

    KoUserStyle* style = new KoUserStyle( "test1" );
    COMPARE( style->name(), QString( "test1" ) );
    COMPARE( style->displayName(), QString( "test1" ) );
    const QString displayName = "A lovely name";
    style->setDisplayName( displayName );
    COMPARE( style->displayName(), displayName );

    KoUserStyle* ret = coll.addStyle( style );
    COMPARE( ret, style );

    KoUserStyle* style2 = new KoUserStyle( "test1" );
    COMPARE( style2->name(), QString( "test1" ) );
    style2->setDisplayName( displayName );
    ret = coll.addStyle( style2 );
    // here style2 got deleted.
    COMPARE( ret, style );

    VERIFY( !coll.isEmpty() );
    COMPARE( coll.count(), 1 );
    COMPARE( (int)coll.styleList().count(), 1 );

    // Add another style for good this time
    KoUserStyle* style3 = new KoUserStyle( "test3" );
    COMPARE( style3->name(), QString( "test3" ) );
    ret = coll.addStyle( style3 );

    QStringList displayNames = coll.displayNameList();
    COMPARE( (int)displayNames.count(), 2 );
    COMPARE( displayNames[0], displayName );
    COMPARE( displayNames[1], style3->name() );
}

void KoUserStyleTester::testFindStyle()
{
    KoUserStyleCollection coll( "test" );
    KoUserStyle* style = new KoUserStyle( "test1" );
    const QString displayName = "A lovely name";
    style->setDisplayName( displayName );
    coll.addStyle( style );

    // --- findStyle tests ---
    KoUserStyle* ret = coll.findStyle( "test1", QString::null );
    COMPARE( ret, style );

    ret = coll.findStyle( "foo", QString::null );
    COMPARE( ret, (KoUserStyle*)0 );

    ret = coll.findStyle( "foo", "test1" ); // fallback not used for style 'foo'
    COMPARE( ret, (KoUserStyle*)0 );

    ret = coll.findStyle( "test1", "test1" ); // fallback used for standard style test1
    COMPARE( ret, style );

    // --- findStyleByDisplayName tests ---
    ret = coll.findStyleByDisplayName( displayName );
    COMPARE( ret, style );

    ret = coll.findStyleByDisplayName( "foo" );
    COMPARE( ret, (KoUserStyle*)0 );

    // --- indexOf tests ---
    int pos = coll.indexOf( style );
    COMPARE( pos, 0 );

    KoUserStyle* style2 = new KoUserStyle( "test1" );
    pos = coll.indexOf( style2 );
    COMPARE( pos, -1 );
    delete style2;
}

void KoUserStyleTester::testRemoveStyle()
{
    KoUserStyleCollection coll( "test" );
    KoUserStyle* style = new KoUserStyle( "test1" );
    coll.addStyle( style );
    COMPARE( coll.count(), 1 );

    // Try removing an unrelated style (noop)
    KoUserStyle* style2 = new KoUserStyle( "test1" );
    coll.removeStyle( style2 );
    delete style2;
    COMPARE( coll.count(), 1 );

    coll.removeStyle( style );
    COMPARE( coll.count(), 0 );
}

void KoUserStyleTester::testReorder()
{
    KoUserStyleCollection coll( "test" );
    KoUserStyle* style = new KoUserStyle( "test1" );
    coll.addStyle( style );
    style = new KoUserStyle( "test2" );
    coll.addStyle( style );
    style = new KoUserStyle( "test3" );
    coll.addStyle( style );
    COMPARE( coll.count(), 3 );

    QStringList newOrder;
    newOrder << "test3";
    newOrder << "test2";
    newOrder << "test1";
    coll.updateStyleListOrder( newOrder );
    COMPARE( coll.count(), 3 );
    QStringList displayNames = coll.displayNameList();
    COMPARE( (int)displayNames.count(), 3 );
    COMPARE( displayNames.join(","), newOrder.join(",") );
}
