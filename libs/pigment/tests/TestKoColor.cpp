/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "TestKoColor.h"

#include <QTest>

#include <QDomElement>

#include "KoColorModelStandardIds.h"

#include "KoColor.h"
#include "KoColorSpace.h"
#include "KoColorSpaceRegistry.h"
#include "DebugPigment.h"

#include "sdk/tests/testpigment.h"

bool nearEqualValue(int a, int b)
{
    return qAbs(a - b) <= 1;
}

void TestKoColor::testForModel(QString model)
{
    QColor qc(200, 125, 100);
    QList<KoID> depthIDs = KoColorSpaceRegistry::instance()->colorDepthList(model, KoColorSpaceRegistry::AllColorSpaces);
    Q_FOREACH (const KoID& depthId, depthIDs) {
        const KoColorSpace* cs = KoColorSpaceRegistry::instance()->colorSpace(model, depthId.id() , "");
        if (cs) {
            KoColor kc(cs);
            kc.fromQColor(qc);
            QDomDocument doc;
            QDomElement elt = doc.createElement("color");
            kc.toXML(doc, elt);
            doc.appendChild(elt);
            dbgPigment << doc.toString();
            KoColor kcu = KoColor::fromXML(elt.firstChildElement(), depthId.id());
            QVERIFY2(*(kc.colorSpace()) == *(kcu.colorSpace()),
                     QString("Not identical color space (colorModelId = %1 depthId = %2) != (colorModelId = %3 depthId = %4) ")
                     .arg(kc.colorSpace()->colorModelId().id())
                     .arg(kc.colorSpace()->colorDepthId().id())
                     .arg(kcu.colorSpace()->colorModelId().id())
                     .arg(kcu.colorSpace()->colorDepthId().id()).toLatin1());
            QVERIFY(cs->difference(kcu.data(), kc.data()) <= 1);
        }
    }

}

void TestKoColor::testSerialization()
{
    testForModel(RGBAColorModelID.id());
    testForModel(XYZAColorModelID.id());
    testForModel(LABAColorModelID.id());
    testForModel(CMYKAColorModelID.id());
    testForModel(GrayAColorModelID.id());
    // we cannot test ycbcr since we cannot ship profiles
    //testForModel(YCbCrAColorModelID.id());
}

void TestKoColor::testConversion()
{
    QColor c = Qt::red;
    const KoColorSpace *csOrig = KoColorSpaceRegistry::instance()->rgb8();
    const KoColorSpace *csDst = KoColorSpaceRegistry::instance()->lab16();

    KoColor kc(csOrig);
    kc.fromQColor(c);

    kc.convertTo(csDst);
}

void TestKoColor::testSimpleSerialization()
{
    QColor c = Qt::green;
    KoColor k;
    k.fromQColor(c);
    QString xml = k.toXML();
    KoColor k2 = KoColor::fromXML(xml);
    QVERIFY(k2.colorSpace() == k.colorSpace());
}

void TestKoColor::testComparison()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KoColor c1(Qt::white, cs);
    KoColor c2(Qt::white, cs);
    KoColor c3(Qt::black, cs);

    QVERIFY(c1 == c2);
    QVERIFY(c2 != c3);
}

void TestKoColor::testComparisonQVariant()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KoColor c1(Qt::white, cs);
    KoColor c2(Qt::white, cs);
    KoColor c3(Qt::black, cs);

    QVariant v1 = QVariant::fromValue(c1);
    QVariant v2 = QVariant::fromValue(c2);
    QVariant v3 = QVariant::fromValue(c3);

    QVERIFY(v1 == v2);
    QVERIFY(v2 != v3);
}

KISTEST_MAIN(TestKoColor)
