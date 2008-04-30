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

#include "kis_mask_generator_test.h"

#include <qtest_kde.h>
#include "kis_mask_generator.h"

#include <QDomDocument>

void KisMaskGeneratorTest::testCircleSerialisation()
{
    KisCircleMaskGenerator cmg(10.0 * rand() / RAND_MAX, 10.0 * rand() / RAND_MAX, rand() / RAND_MAX, rand() / RAND_MAX );
    QDomDocument doc = QDomDocument("cmg");
    QDomElement root = doc.createElement( "cmg" );
    doc.appendChild( root );
    cmg.toXML(doc, root );
    KisMaskGenerator* cmg2 = KisMaskGenerator::fromXML(root);
    for(int i = 0; i < 10; ++i)
    {
        for(int j = 0; j < 10; ++j)
        {
            QVERIFY( cmg.valueAt(i,j) == cmg2->valueAt(i, j ) );
        }
    }
    delete cmg2;
}

void KisMaskGeneratorTest::testSquareSerialisation()
{
    KisRectangleMaskGenerator cmg(10.0 * rand() / RAND_MAX, 10.0 * rand() / RAND_MAX, rand() / RAND_MAX, rand() / RAND_MAX );
    QDomDocument doc = QDomDocument("cmg");
    QDomElement root = doc.createElement( "cmg" );
    doc.appendChild( root );
    cmg.toXML(doc, root );
    KisMaskGenerator* cmg2 = KisMaskGenerator::fromXML(root);
    for(int i = 0; i < 10; ++i)
    {
        for(int j = 0; j < 10; ++j)
        {
            QVERIFY( cmg.valueAt(i,j) == cmg2->valueAt(i, j ) );
        }
    }
    delete cmg2;
}

QTEST_KDEMAIN(KisMaskGeneratorTest, GUI)
#include "kis_mask_generator_test.moc"
