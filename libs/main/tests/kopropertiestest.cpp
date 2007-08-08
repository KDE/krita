/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <qtest_kde.h>
#include <KoProperties.h>
#include "kopropertiestest.h"

void KoPropertiesTest::testSerialization()
{
    QVERIFY( 1 == 0 );
}

void KoPropertiesTest::testProperties()
{
    KoProperties props;
    QVERIFY( props.isEmpty() );

    QString visible = "visible";
    QVERIFY( !props.value( visible ).isValid() );

    props.setProperty( "visible", "bla" );
    QVERIFY( props.value( "visible" ) == "bla");
    QVERIFY( props.stringProperty( "visible", "blabla" ) == "bla" );

    props.setProperty( "bool",  true );
    QVERIFY( props.boolProperty( "bool", false ) == true );
    props.setProperty( "bool",  false );
    QVERIFY( props.boolProperty( "bool", true ) == false );

    props.setProperty( "double",  1.0 );
    QVERIFY( props.doubleProperty( "double", 2.0 ) == 1.0 );
    props.setProperty( "double",  2.0 );
    QVERIFY( props.doubleProperty( "double", 1.0 ) == 2.0 );

    props.setProperty( "int",  1 );
    QVERIFY( props.intProperty( "int", 2 ) == 1 );
    props.setProperty( "int",  2 );
    QVERIFY( props.intProperty( "int", 1 ) == 2 );

    QVariant v;
    QVERIFY( props.property( "sdsadsakldjsajd", v ) == false );
    QVERIFY( !v.isValid() );
    QVERIFY( props.property( "visible", v ) == true );
    QVERIFY( v.isValid() );
    QVERIFY( v == "bla" );

    QVERIFY( !props.isEmpty() );
    QVERIFY( props.contains( "visible" ) );
    QVERIFY( !props.contains( "adsajkdsakj dsaieqwewqoie" ) );
    QVERIFY( props.contains( visible ) );

    int count = 0;
    QMapIterator<QString, QVariant> iter = props.propertyIterator();
    while ( iter.hasNext() ) {
        iter.next();
        count++;
    }
    QVERIFY( count == 4 );

}

bool checkProps( const KoProperties & props )
{
    return ( props.value( "bla" ) == 1 );
}

void KoPropertiesTest::testPassAround()
{
    KoProperties props;
    props.setProperty( "bla", 1 );
    QVERIFY( checkProps( props ) );

    KoProperties props2 = props;
    QVERIFY( checkProps( props2 ) );

    KoProperties props3( props );
    checkProps( props3 );
    props3.setProperty( "bla", 3 );
    QVERIFY( props3.value( "bla" ) == 3 );

    QVERIFY( checkProps( props ) );
    QVERIFY( checkProps( props2 ) );

}

QTEST_KDEMAIN(KoPropertiesTest, NoGUI)
#include "kopropertiestest.moc"


