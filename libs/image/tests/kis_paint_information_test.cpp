/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_paint_information_test.h"

#include <QTest>
#include <brushengine/kis_paint_information.h>
#include "kis_debug.h"


#include <QDomDocument>
#include <Eigen/Core>

void KisPaintInformationTest::testCreation()
{
    KisPaintInformation test;
}

void KisPaintInformationTest::testSerialisation()
{
    KisPaintInformation test(QPointF(double(rand()) / RAND_MAX, double(rand()) / RAND_MAX), double(rand()) / RAND_MAX, double(rand()) / RAND_MAX, double(rand()) / RAND_MAX, double(rand()) / RAND_MAX, double(rand()) / RAND_MAX, double(rand()) / RAND_MAX, double(rand()) / RAND_MAX, double(rand()) / RAND_MAX);

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
    QCOMPARE(test.rotation() , testUnS.rotation());
    QCOMPARE(test.tangentialPressure() , testUnS.tangentialPressure());
    /**
     * drawingAngle(), velocity() and distance() are calculated basing
     * on the KisDistanceInformation data and are not available without
     * it
     */
}

#include <boost/random/taus88.hpp>
#include <boost/random/uniform_smallint.hpp>

void KisPaintInformationTest::benchmarkTausRandomGeneration()
{
    boost::taus88 rnd;

    QBENCHMARK {
        // make a copy
        boost::taus88 rnd2(rnd);

        // use smallint shaper
        boost::uniform_smallint<int> smallint(0,10);

        // generate
        int value = smallint(rnd2);
        Q_UNUSED(value);
    }
}


QTEST_MAIN(KisPaintInformationTest)
