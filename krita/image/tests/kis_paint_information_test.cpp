/*
 *  Copyright (c) 2007 Boudewijn Rempt boud@valdyas.org
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
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

#include "kis_paint_information_test.h"

#include <qtest_kde.h>
#include "kis_paint_information.h"

#include <QDomDocument>

void KisPaintInformationTest::testCreation()
{
    KisPaintInformation test;
}

void KisPaintInformationTest::testSerialisation()
{
    KisPaintInformation test(QPointF( rand() / RAND_MAX, rand() / RAND_MAX), rand() / RAND_MAX, rand() / RAND_MAX, rand() / RAND_MAX, KisVector2D(rand() / RAND_MAX, rand() / RAND_MAX), rand() / RAND_MAX, rand() / RAND_MAX );
    
    QDomDocument doc = QDomDocument("pi");
    QDomElement root = doc.createElement( "pi" );
    doc.appendChild( root );
    test.toXML(doc, root );
    KisPaintInformation testUnS = KisPaintInformation::fromXML( root );
    QVERIFY( test.pos() == testUnS.pos() );
    QVERIFY( test.pressure() == testUnS.pressure() );
    QVERIFY( test.xTilt() == testUnS.xTilt() );
    QVERIFY( test.yTilt() == testUnS.yTilt() );
    QVERIFY( test.movement() == testUnS.movement() );
    QVERIFY( test.angle() == testUnS.angle() );
    QVERIFY( test.rotation() == testUnS.rotation() );
    QVERIFY( test.tangentialPressure() == testUnS.tangentialPressure() );
    
}


QTEST_KDEMAIN(KisPaintInformationTest, GUI)
#include "kis_paint_information_test.moc"
