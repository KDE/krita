/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "TestKoColor.h"

#include <simpletest.h>

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

void TestKoColor::testExistingSerializations()
{

    QString main;
    QDomDocument doc;

    QColor c;
    // Test sRGB.
    main = "<sRGB r='0' g='0' b='1' />";
    doc.setContent(main);
    KoColor sRGB = KoColor::fromXML(doc.documentElement(), Integer8BitsColorDepthID.id());
    sRGB.toQColor(&c);
    QString blue = "#0000FF";
    QVERIFY2(c == QColor(blue), QString("XML parser is not loading the sRGB properly: \nresult: %1 \nexpected: %2")
             .arg(c.name()).arg(blue).toLatin1());

    // Test wide gamut RGB -- We can only check that the values deserialize properly, which is fine in this case.
    QString Rec2020profile = KoColorSpaceRegistry::instance()->p2020G10Profile()->name();
    main = QString("<RGB r='3.0' g='0' b='1' space='%1'/>").arg(Rec2020profile);
    doc.setContent(main);
    KoColor rec2020color = KoColor::fromXML(doc.documentElement(), Float32BitsColorDepthID.id());

    QVector<float> rec2020ChannelValues(4);
    rec2020color.colorSpace()->normalisedChannelsValue(rec2020color.data(), rec2020ChannelValues);
    QVERIFY(qFuzzyCompare(rec2020ChannelValues[0], 3.0));
    QVERIFY(qFuzzyCompare(rec2020ChannelValues[1], 0.0));
    QVERIFY(qFuzzyCompare(rec2020ChannelValues[2], 1.0));
    QVERIFY(qFuzzyCompare(rec2020ChannelValues[3], 1.0));


    // Test cmyk, we can only check that the channels deserialize properly here.
    // NOTE: 32bit float gives wildly different values here, so I am unsure what is going on still...
    const KoColorSpace *cmykSpace = KoColorSpaceRegistry::instance()->colorSpace(CMYKAColorModelID.id(), Integer8BitsColorDepthID.id());
    main = QString("<CMYK c='0.2' m='0.5' y='1.0' k='0.0' space='%1'/>").arg(cmykSpace->profile()->name());
    doc.setContent(main);
    KoColor cmykColorU8 = KoColor::fromXML(doc.documentElement(), Integer8BitsColorDepthID.id());
    KoColor cmykColorU16 = KoColor::fromXML(doc.documentElement(), Integer16BitsColorDepthID.id());
    KoColor cmykColorF32 = KoColor::fromXML(doc.documentElement(), Float32BitsColorDepthID.id());

    QVector<QDomElement> elements;

    elements.append(doc.documentElement());

    doc.setContent(cmykColorU8.toXML());
    elements.append(doc.documentElement().firstChild().toElement());
    doc.setContent(cmykColorU16.toXML());
    elements.append(doc.documentElement().firstChild().toElement());
    doc.setContent(cmykColorF32.toXML());
    elements.append(doc.documentElement().firstChild().toElement());

    QStringList attributes;
    attributes << "c" << "m" << "y" << "k";

    for (QString attr : attributes) {
        double mainValue = elements.first().attribute(attr).toDouble();
        for (QDomElement el: elements) {
            double compare = el.attribute(attr).toDouble();
            QVERIFY2(fabs(mainValue - compare) < 0.01
                     , QString("XML CMYK parsing has too high of a difference when roundtripping channel %1: %2")
                     .arg(attr).arg(mainValue - compare).toLatin1());
        }
    }

    // CMYK has wildly different values in F32 than in U8. Avoid F32 CMYK!
    //cmykColorU8.convertTo(cmykColorF32.colorSpace());
    //qDebug() << ppVar(cmykColorU8);
    //qDebug() << cmykColorU8.toXML();
    //qDebug() << cmykColorF32.colorSpace()->difference(cmykColorF32.data(), cmykColorU8.data());

    // Test XYZ - check channels.
    const KoColorSpace *xyzSpace = KoColorSpaceRegistry::instance()->colorSpace(XYZAColorModelID.id(), Integer8BitsColorDepthID.id());
    main = QString("<XYZ x='0.0' y='0.0' z='1.0' space='%1'/>").arg(xyzSpace->profile()->name());
    doc.setContent(main);
    KoColor xyzColor = KoColor::fromXML(doc.documentElement(), Integer8BitsColorDepthID.id());
    quint8 *xyzData = xyzColor.data();
    QCOMPARE(xyzData[0], 0);
    QCOMPARE(xyzData[1], 0);
    QCOMPARE(xyzData[2], 255);


    // Test LAB
    // Lab has a different way of handling floating point from the rest of the colorspaces.
    const KoColorSpace *labSpace = KoColorSpaceRegistry::instance()->lab16();
    main = QString("<Lab space='%1' L='34.67' a='54.1289' b='-90.3359' />").arg(labSpace->profile()->name());
    doc.setContent(main);
    KoColor LABcolorU8 = KoColor::fromXML(doc.documentElement(), Integer8BitsColorDepthID.id());
    KoColor LABcolorU16 = KoColor::fromXML(doc.documentElement(), Integer16BitsColorDepthID.id());
    KoColor LABcolorF32 = KoColor::fromXML(doc.documentElement(), Float32BitsColorDepthID.id());

    // Check that there isn't too much of a discrepancy between the XML values of the different bitdepths.

    elements.clear();
    elements.append(doc.documentElement());

    doc.setContent(LABcolorU8.toXML());
    elements.append(doc.documentElement().firstChild().toElement());
    doc.setContent(LABcolorU16.toXML());
    elements.append(doc.documentElement().firstChild().toElement());
    doc.setContent(LABcolorF32.toXML());
    elements.append(doc.documentElement().firstChild().toElement());

    attributes.clear();
    attributes << "L" << "a" << "b";

    for (QString attr : attributes) {
        double mainValue = elements.first().attribute(attr).toDouble();
        for (QDomElement el: elements) {
            double compare = el.attribute(attr).toDouble();
            QVERIFY2(fabs(mainValue - compare) < 1.0
                    , QString("XML LAB parsing has too high of a difference when roundtripping channel %1: %2")
                     .arg(attr).arg(mainValue - compare).toLatin1());
        }
    }


    // The following is the known sRGB color that the test value matches with.
    // Let's make sure that all the labvalues roughly convert to this sRGB value.
    KoColor purpleCompare = KoColor(QColor("#442de9"), sRGB.colorSpace());

    LABcolorU8.convertTo(sRGB.colorSpace());
    QVERIFY2(sRGB.colorSpace()->difference(LABcolorU8.data(), purpleCompare.data()) <= 1
             , QString("LAB U8 has too high a difference to it's sRGB reference: %1")
             .arg(sRGB.colorSpace()->difference(LABcolorU8.data(), purpleCompare.data())).toLatin1());
    LABcolorU16.convertTo(sRGB.colorSpace());
    QVERIFY2(sRGB.colorSpace()->difference(LABcolorU16.data(), purpleCompare.data()) <= 1
             , QString("LAB U16 has too high a difference to it's sRGB reference: %1")
             .arg(sRGB.colorSpace()->difference(LABcolorU16.data(), purpleCompare.data())).toLatin1());
    LABcolorF32.convertTo(sRGB.colorSpace());
    QVERIFY2(sRGB.colorSpace()->difference(LABcolorF32.data(), purpleCompare.data()) <= 1
            , QString("LAB F32 has too high a difference to it's sRGB reference: %1")
            .arg(sRGB.colorSpace()->difference(LABcolorF32.data(), purpleCompare.data())).toLatin1());

    // Test Gray - check channels.
    const KoColorSpace *graySpace = KoColorSpaceRegistry::instance()->colorSpace(GrayAColorModelID.id(), Integer8BitsColorDepthID.id());
    main = QString("<Gray g='0.5' space='%1'/>").arg(graySpace->profile()->name());
    doc.setContent(main);
    KoColor grayColor = KoColor::fromXML(doc.documentElement(), Integer8BitsColorDepthID.id());
    quint8 *grayData = grayColor.data();
    QCOMPARE(grayData[0], 128);

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
    const KoColorSpace *sRGB = KoColorSpaceRegistry::instance()->rgb16(KoColorSpaceRegistry::instance()->p709SRGBProfile());
    KoColor c1 = KoColor(QColor("#ff0000"), sRGB);

    QVERIFY2(p1 == c1
             , QString("SVG11 parser is not loading the sRGB hex fallback: \nresult: %1 \nexpected: %2")
             .arg(KoColor::toQString(p1)).arg(KoColor::toQString(c1)).toLatin1());

    //2. testing case with fallback colorname and nonsense icc-color that we cannot parse

    KoColor p2 = KoColor::fromSVG11("#ff0000 silver icc-color(blah, 0.0, 1.0, 1.0, 0.0);", profileList);
    KoColor c2 = KoColor(QColor("silver"), sRGB);

    QVERIFY2(p2 == c2
             , QString("SVG11 parser is not loading the sRGB colorname fallback: \nresult: %1 \nexpected: %2")
             .arg(KoColor::toQString(p2)).arg(KoColor::toQString(c2)).toLatin1());

    //3. testing case with fallback color and useful icc-color

    const KoColorSpace *cmyk = KoColorSpaceRegistry::instance()->colorSpace(CMYKAColorModelID.id(), Integer8BitsColorDepthID.id());
    QString cmykName = "sillyCMYKName";
    profileList.insert(cmykName, cmyk->profile());

    KoColor p3 = KoColor::fromSVG11("#ff0000 silver icc-color("+cmykName+", 0.0, 0.0, 1.0, 1.0);", profileList);
    KoColor c3 = KoColor::fromXML("<color channeldepth='U16'><CMYK c='0.0' m='0.0' y='1.0' k='1.0' space='"+cmyk->profile()->name()+"'/></color>");

    QVERIFY2(p3 == c3
             , QString("SVG11 parsed cmyk incorrectly: \nresult: %1 \nexpected: %2")
             .arg(KoColor::toQString(p3)).arg(KoColor::toQString(c3)).toLatin1());
    //4. Roundtrip

    KoColor c4(KoColorSpaceRegistry::instance()->lab16());
    c4.fromQColor(QColor("#426471"));
    QString value = c4.toSVG11(&profileList);
    qDebug() << value;
    KoColor p4 = KoColor::fromSVG11(value, profileList);

    QVERIFY2(c4.colorSpace()->difference(p4.data(), c4.data()) < 1.0
             , QString("Difference between colors from serialization roundtrip above 1.0: %1")
             .arg(c4.colorSpace()->difference(p4.data(), c4.data())).toLatin1());

    //4.5 Check that the size stays the same even though we already added this profile to the stack before.
    int profileListSize = profileList.size();
    QString newColor = c4.toSVG11(&profileList);
    QCOMPARE(profileList.size(), profileListSize);

    //5. Testing rgb...

    KoColor p5 = KoColor::fromSVG11("#ff0000 rgb(100, 50, 50%)", profileList);
    KoColor c5 = KoColor(QColor(100, 50, 127), sRGB);

    QVERIFY2(p5 == c5, QString("the rgb() definition for SVG11 is not parsed correctly, \nresult: %1 \nexpected: %2")
             .arg(KoColor::toQString(p5)).arg(KoColor::toQString(c5)).toLatin1());

    //6. Testing special srgb definition... especially the part where it can be defined case-insensitive.

    KoColor p6 = KoColor::fromSVG11("#ff0000 icc-color(srgb, 1.0, 1.0, 0.0)", profileList);
    KoColor c6 = KoColor::fromXML("<color channeldepth='F32'><sRGB r='1.0' g='1.0' b='0.0'/></color>");

    QVERIFY2(p6 == c6
             , QString("sRGB parsing is different between SVG11 and XML: \nresult: %1 \nexpected: %2")
             .arg(KoColor::toQString(p6)).arg(KoColor::toQString(c6)).toLatin1());
    //7. Testing out-of-bounds values...

    KoColor p7 = KoColor::fromSVG11("#ff0000 icc-color(srgb, 2.0, 1.0, 0.0)", profileList);
    KoColor c7 = KoColor::fromXML("<color channeldepth='F32'><sRGB r='2.0' g='1.0' b='0.0'/></color>");

    QVERIFY2(p7 == c7, QString("Out of bounds RGB is parsing differently for XML and SVG11: \nresult: %1 \nexpected: %2")
             .arg(KoColor::toQString(p7)).arg(KoColor::toQString(c7)).toLatin1());

    //8. Check lab special case.
    KoColor p8 = KoColor::fromSVG11("#ff0000 icc-color(lab, 34.67, 54.1289, -90.3359)", profileList);
    QDomDocument doc;
    doc.setContent(QString("<Lab space='%1' L='34.67' a='54.1289' b='-90.3359' />").arg(c4.colorSpace()->profile()->name()));
    KoColor c8 = KoColor::fromXML(doc.documentElement(), "U16");

    QVERIFY2(p8 == c8, QString("Lab parsing is giving different values for XML and SVG11: \nresult: %1 \nexpected: %2")
                               .arg(KoColor::toQString(p8)).arg(KoColor::toQString(c8)).toLatin1());
    //9. Check xyz loading
    //We do not support XYZ because Inkscape decided that XYZ X and Z are 0-2, and I cannot figure out why.
    const KoColorSpace *xyzSpace = KoColorSpaceRegistry::instance()->colorSpace(XYZAColorModelID.id(), Integer16BitsColorDepthID.id());
    profileList.insert("XYZ", xyzSpace->profile());
    KoColor p9 = KoColor::fromSVG11("#0077FF icc-color(XYZ, 1.0, 0.0, 0.5)", profileList);
    KoColor c9 = KoColor(QColor("#0077FF"), sRGB);
    QVERIFY2(p9 == c9
             , QString("SVG11 parser is not loading the sRGB hex fallback for XYZ: \nresult: %1 \nexpected: %2")
             .arg(KoColor::toQString(p9)).arg(KoColor::toQString(c9)).toLatin1());

    //10. Check xyz saving
    KoColor p10 = KoColor(sRGB);
    QString c10 = "#0077FF";
    p10.fromQColor(QColor(c10));
    p10.convertTo(xyzSpace);

    QVERIFY2(p10.toSVG11(&profileList) != c10, QString("XYZ values are being saved: \nresult: %1 \nexpected: %2")
             .arg(KoColor::toQString(p10)).arg(c10).toLatin1());

    //11. Check gray loading.
    const KoColorSpace *gray = KoColorSpaceRegistry::instance()->graya8();
    profileList.insert("grayName", gray->profile());
    KoColor p11 = KoColor::fromSVG11("#ff0000 icc-color(grayName, 0.21);", profileList);
    KoColor c11 = KoColor::fromXML("<color channeldepth='F32'><Gray g='0.21' space='"+gray->profile()->name()+"'/></color>");

    QVERIFY2(p11 == c11
             , QString("SVG11 parsed gray incorrectly: \nresult: %1 \nexpected: %2")
             .arg(KoColor::toQString(p11)).arg(KoColor::toQString(c11)).toLatin1());

    //12. check sRGB is not saved.
    profileListSize = profileList.size();
    QString colorDef = c1.toSVG11(&profileList);
    QCOMPARE(profileList.size(), profileListSize);

}

KISTEST_MAIN(TestKoColor)
