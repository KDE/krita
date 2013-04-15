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
#include <Eigen/Array>

#include <kdebug.h>

void KisPaintInformationTest::testCreation()
{
    KisPaintInformation test;
}

void KisPaintInformationTest::testSerialisation()
{
    KisPaintInformation test(QPointF(double(rand()) / RAND_MAX, double(rand()) / RAND_MAX), double(rand()) / RAND_MAX, double(rand()) / RAND_MAX, double(rand()) / RAND_MAX, KisVector2D::Random(), double(rand()) / RAND_MAX, double(rand()) / RAND_MAX);

    QDomDocument doc = QDomDocument("pi");
    QDomElement root = doc.createElement("pi");
    doc.appendChild(root);
    test.toXML(doc, root);
    KisPaintInformation testUnS = KisPaintInformation::fromXML(root);
    QCOMPARE(test.pos().x() , testUnS.pos().x());
    QCOMPARE(test.pos().y() , testUnS.pos().y());
    QCOMPARE(test.pressure() , testUnS.pressure());
    QCOMPARE(test.xTilt() , testUnS.xTilt());
    QCOMPARE(test.yTilt() , testUnS.yTilt());
    QCOMPARE(test.movement().x() , testUnS.movement().x());
    QCOMPARE(test.movement().y() , testUnS.movement().y());
    QCOMPARE(test.angle() , testUnS.angle());
    QCOMPARE(test.rotation() , testUnS.rotation());
    QCOMPARE(test.tangentialPressure() , testUnS.tangentialPressure());
}


QTEST_KDEMAIN(KisPaintInformationTest, GUI)
#include "kis_paint_information_test.moc"
