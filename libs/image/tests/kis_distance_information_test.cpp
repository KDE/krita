/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "kis_distance_information_test.h"

#include <simpletest.h>
#include <QDomDocument>
#include <QDomElement>
#include <QPointF>

#include "kis_algebra_2d.h"
#include "kis_distance_information.h"
#include "kis_spacing_information.h"
#include "kis_timing_information.h"
#include "kis_paint_information.h"

void KisDistanceInformationTest::testInitInfo()
{
    // Test equality checking operators.
    testInitInfoEquality();

    // Test XML cloning.
    testInitInfoXMLClone();
}

void KisDistanceInformationTest::testInterpolation()
{
    // Set up a scenario for interpolation.

    QPointF startPos;
    QPointF endPos(100.0, -50.0);
    qreal dist = KisAlgebra2D::norm(endPos - startPos);

    qreal startTime = 0.0;
    qreal endTime = 1000.0;
    qreal interval = endTime - startTime;

    KisPaintInformation p1(startPos, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, startTime, 0.0);
    KisPaintInformation p2(endPos, 1.0, 0.0, 0.0, 5.0, 0.0, 1.0, endTime, 0.0);

    // Test interpolation with various spacing settings.

    static const qreal interpTolerance = 0.000001;

    KisDistanceInformation dist1;
    dist1.updateSpacing(KisSpacingInformation(dist/10.0));
    dist1.updateTiming(KisTimingInformation());
    testInterpolationImpl(p1, p2, dist1, 1.0/10.0, false, false, interpTolerance);

    KisDistanceInformation dist2(interval/2.0, interval/3.0);
    dist2.updateSpacing(KisSpacingInformation(dist*2.0));
    dist2.updateTiming(KisTimingInformation(interval*1.5));
    testInterpolationImpl(p1, p2, dist2, -1.0, true, true, interpTolerance);

    KisDistanceInformation dist3(interval/1.5, interval*2.0);
    dist3.updateSpacing(KisSpacingInformation(dist*2.1));
    dist3.updateTiming(KisTimingInformation(interval*1.6));
    testInterpolationImpl(p1, p2, dist3, -1.0, true, false, interpTolerance);

    KisDistanceInformation dist4(interval*1.3, interval/2.4);
    dist4.updateSpacing(KisSpacingInformation(dist*1.7));
    dist4.updateTiming(KisTimingInformation(interval*1.9));
    testInterpolationImpl(p1, p2, dist4, -1.0, false, true, interpTolerance);

    KisDistanceInformation dist5(interval*1.1, interval*1.2);
    dist5.updateSpacing(KisSpacingInformation(dist/40.0));
    dist5.updateTiming(KisTimingInformation());
    testInterpolationImpl(p1, p2, dist5, 1.0/40.0, false, false, interpTolerance);

    KisDistanceInformation dist6;
    dist6.updateSpacing(KisSpacingInformation(false, 1.0));
    dist6.updateTiming(KisTimingInformation());
    testInterpolationImpl(p1, p2, dist6, -1.0, false, false, interpTolerance);

    KisDistanceInformation dist7;
    dist7.updateSpacing(KisSpacingInformation(false, 1.0));
    dist7.updateTiming(KisTimingInformation(interval/20.0));
    testInterpolationImpl(p1, p2, dist7, 1.0/20.0, false, false, interpTolerance);

    KisDistanceInformation dist8;
    dist8.updateSpacing(KisSpacingInformation(dist/10.0));
    dist8.updateTiming(KisTimingInformation(interval/15.0));
    testInterpolationImpl(p1, p2, dist8, 1.0/15.0, false, false, interpTolerance);

    KisDistanceInformation dist9;
    dist9.updateSpacing(KisSpacingInformation(dist/15.0));
    dist9.updateTiming(KisTimingInformation(interval/10.0));
    testInterpolationImpl(p1, p2, dist9, 1.0/15.0, false, false, interpTolerance);

    KisDistanceInformation dist10;
    dist10.updateSpacing(KisSpacingInformation(dist*2.0));
    dist10.updateTiming(KisTimingInformation(interval*1.5));
    testInterpolationImpl(p1, p2, dist10, -1.0, false, false, interpTolerance);

    KisDistanceInformation dist11;
    qreal a = 50.0;
    qreal b = 25.0;
    dist11.updateSpacing(KisSpacingInformation(QPointF(a * 2.0, b * 2.0), 0.0, false));
    dist11.updateTiming(KisTimingInformation());
    // Compute the expected interpolation factor; we are using anisotropic spacing here.
    qreal angle = KisAlgebra2D::directionBetweenPoints(startPos, endPos, 0.0);
    qreal cosTermSqrt = qCos(angle) / a;
    qreal sinTermSqrt = qSin(angle) / b;
    qreal spacingDist = 2.0 / qSqrt(cosTermSqrt * cosTermSqrt + sinTermSqrt * sinTermSqrt);
    qreal expectedInterp = spacingDist / dist;
    testInterpolationImpl(p1, p2, dist11, expectedInterp, false, false, interpTolerance);
}

void KisDistanceInformationTest::testInitInfoEquality() const
{
    KisDistanceInitInfo info1;
    KisDistanceInitInfo info2;
    QVERIFY(info1 == info2);
    QVERIFY(!(info1 != info2));

    KisDistanceInitInfo info3(0.1, 0.5, 4);
    KisDistanceInitInfo info4(0.1, 0.5, 4);
    QVERIFY(info3 == info4);
    QVERIFY(!(info3 != info4));

    KisDistanceInitInfo info5(QPointF(1.1, -10.7), 3.3, 7);
    KisDistanceInitInfo info6(QPointF(1.1, -10.7), 3.3, 7);
    QVERIFY(info5 == info6);
    QVERIFY(!(info5 != info6));

    KisDistanceInitInfo info7(QPointF(-12.3, 24.0), 5.0, 20.1, 35.7, 9);
    KisDistanceInitInfo info8(QPointF(-12.3, 24.0), 5.0, 20.1, 35.7, 9);
    QVERIFY(info7 == info8);
    QVERIFY(!(info7 != info8));

    QVERIFY(info1 != info3);
    QVERIFY(info1 != info5);
    QVERIFY(info1 != info7);
    QVERIFY(info3 != info5);
    QVERIFY(info3 != info7);
    QVERIFY(info5 != info7);
}

void KisDistanceInformationTest::testInitInfoXMLClone() const
{
    // Note: Numeric values used here must be values that get serialized to XML exactly (e.g. small
    // integers). Otherwise, roundoff error in serialization may cause a failure.

    KisDistanceInitInfo info1;
    QDomDocument doc;
    QDomElement elt1 = doc.createElement("Test1");
    info1.toXML(doc, elt1);
    KisDistanceInitInfo clone1 = KisDistanceInitInfo::fromXML(elt1);
    QVERIFY(clone1 == info1);

    KisDistanceInitInfo info2(40.0, 2.0, 3);
    QDomElement elt2 = doc.createElement("Test2");
    info2.toXML(doc, elt2);
    KisDistanceInitInfo clone2 = KisDistanceInitInfo::fromXML(elt2);
    QVERIFY(clone2 == info2);

    KisDistanceInitInfo info3(QPointF(-8.0, -5.0), 60.0, 3);
    QDomElement elt3 = doc.createElement("Test3");
    info3.toXML(doc, elt3);
    KisDistanceInitInfo clone3 = KisDistanceInitInfo::fromXML(elt3);
    QVERIFY(clone3 == info3);

    KisDistanceInitInfo info4(QPointF(0.0, 9.0), 6.0, 1.0, 3.0, 8);
    QDomElement elt4 = doc.createElement("Test4");
    info4.toXML(doc, elt4);
    KisDistanceInitInfo clone4 = KisDistanceInitInfo::fromXML(elt4);
    QVERIFY(clone4 == info4);
}

void KisDistanceInformationTest::testInterpolationImpl(const KisPaintInformation &p1,
                                                       const KisPaintInformation &p2,
                                                       KisDistanceInformation &dist,
                                                       qreal interpFactor,
                                                       bool needSpacingUpdate,
                                                       bool needTimingUpdate,
                                                       qreal interpTolerance) const
{
    qreal actualInterpFactor = dist.getNextPointPosition(p1.pos(), p2.pos(), p1.currentTime(),
                                                         p2.currentTime());
    QVERIFY(qAbs(interpFactor - actualInterpFactor) <= interpTolerance);
    QCOMPARE(dist.needsSpacingUpdate(), needSpacingUpdate);
    QCOMPARE(dist.needsTimingUpdate(), needTimingUpdate);
}

SIMPLE_TEST_MAIN(KisDistanceInformationTest)
