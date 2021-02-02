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
#include "KoColorProfile.h"
#include "KoColorSpaceRegistry.h"
#include "DebugPigment.h"
#include "kis_debug.h"

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

void TestKoColor::testSVGParsing()
{
    QHash <QString, const KoColorProfile *> profileList;

    //1. Testing case with fallback hexvalue and nonsense icc-color that we cannot parse

    KoColor p1 = KoColor::fromSVG11("#ff0000 icc-color(blah, 0.0, 1.0, 1.0, 0.0);", profileList);
    KoColor c1;
    c1.fromQColor(QColor("#ff0000"));

    //2. testing case with fallback colorname and nonsense icc-color that we cannot parse

    KoColor p2 = KoColor::fromSVG11("#ff0000 silver icc-color(blah, 0.0, 1.0, 1.0, 0.0);", profileList);
    KoColor c2;
    c2.fromQColor(QColor("silver"));

    //3. testing case with fallback color and useful icc-color

    const KoColorSpace *cmyk = KoColorSpaceRegistry::instance()->colorSpace(CMYKAColorModelID.id(), Integer8BitsColorDepthID.id());
    QString cmykName = "sillyCMYKName";
    profileList.insert(cmykName, cmyk->profile());

    KoColor p3 = KoColor::fromSVG11("#ff0000 silver icc-color("+cmykName+", 0.0, 0.0, 1.0, 1.0);", profileList);
    KoColor c3 = KoColor::fromXML("<color channeldepth='U16'><CMYK c='0.0' m='0.0' y='1.0' k='1.0' space='"+cmyk->name()+"'/></color>");

    //4. Roundtrip

    KoColor c4(KoColorSpaceRegistry::instance()->lab16());
    c4.fromQColor(QColor("#426471"));
    QString value = c4.toSVG11(&profileList);
    dbgPigment << value;
    KoColor p4 = KoColor::fromSVG11(value, profileList);

    //4.5 Check that the size stays the same even though we already added this profile to the stack before.
    int profileListSize = profileList.size();
    QString newColor = c4.toSVG11(&profileList);
    QCOMPARE(profileList.size(), profileListSize);

    //5. Testing rgb...

    KoColor p5 = KoColor::fromSVG11("#ff0000 rgb(100, 50, 50%)", profileList);
    KoColor c5;
    c5.fromQColor(QColor(100, 50, 127));

    //6. Testing special srgb definition... especially the part where it can be defined case-insensitive.

    KoColor p6 = KoColor::fromSVG11("#ff0000 icc-color(srgb, 1.0, 1.0, 0.0)", profileList);
    KoColor c6 = KoColor::fromXML("<color channeldepth='F32'><sRGB r='1.0' g='1.0' b='0.0'/></color>");

    //7. Testing out-of-bounds values...

    KoColor p7 = KoColor::fromSVG11("#ff0000 icc-color(srgb, 2.0, 1.0, 0.0)", profileList);
    KoColor c7 = KoColor::fromXML("<color channeldepth='F32'><sRGB r='2.0' g='1.0' b='0.0'/></color>");

    //8. test unique way of defining colors?
    // This is an improper way of using the api, but I guess someone might try?
    KoColor c8 = KoColor::fromXML("<RGB b=\"0.7\" space=\"sRGB-elle-V2-srgbtrc.icc\" r=\"1.0\" g=\"0.1\"/>");
    qDebug() << ppVar(c8);


    QVERIFY(p1 == c1);
    QVERIFY(p2 == c2);
    QVERIFY(p3 == c3);
    QVERIFY(p4 == c4);
    QVERIFY(p5 == c5);
    QVERIFY(p6 == c6);
    QVERIFY(p7 == c7);
}

KISTEST_MAIN(TestKoColor)
