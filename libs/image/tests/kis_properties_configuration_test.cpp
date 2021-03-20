/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_properties_configuration_test.h"


#include <simpletest.h>
#include "kis_properties_configuration.h"

KisPropertiesConfigurationTest::KisPropertiesConfigurationTest() :
        v1(10), v2("hello"), v3(1242.0), v4(true)
{
    QList<QPointF> pts; pts.push_back(QPointF(0.2, 0.3)); pts.push_back(QPointF(0.5, 0.7));
    v5.setPoints(pts);
}

void KisPropertiesConfigurationTest::testSerialization()
{
    KisPropertiesConfigurationSP config = createConfig();
    QString xml = config->toXML();
    KisPropertiesConfigurationSP decodedConfig = new KisPropertiesConfiguration();
    decodedConfig->fromXML(xml);
    testConfig(decodedConfig);

}

void KisPropertiesConfigurationTest::testSetGet()
{
    KisPropertiesConfigurationSP config = createConfig();
    testConfig(config);
}

void KisPropertiesConfigurationTest::testDefaultValues()
{
    KisPropertiesConfigurationSP config = new KisPropertiesConfiguration();
    QVERIFY(config->getInt("bouh", v1) == v1);
    QVERIFY(config->getString("bouh", v2) == v2);
    QVERIFY(config->getDouble("bouh", v3) == v3);
    QVERIFY(config->getBool("bouh", v4) == v4);
    QVERIFY(config->getCubicCurve("bouh", v5) == v5);
}

KisPropertiesConfigurationSP KisPropertiesConfigurationTest::createConfig()
{
    KisPropertiesConfigurationSP config = new KisPropertiesConfiguration();
    config->setProperty("v1", v1);
    config->setProperty("v2", v2);
    config->setProperty("v3", v3);
    config->setProperty("v4", v4);
    config->setProperty("v5", QVariant::fromValue(v5));
    return config;
}

void KisPropertiesConfigurationTest::testConfig(KisPropertiesConfigurationSP config)
{
    QVERIFY(config->getInt("v1", 0) == v1);
    QVERIFY(config->getString("v2", QString()) == v2);
    QVERIFY(config->getDouble("v3", 0.0) == v3);
    QVERIFY(config->getBool("v4", !v4) == v4);
    QVERIFY(config->getCubicCurve("v5") == v5);
}

void KisPropertiesConfigurationTest::testNotSavedValues()
{
    KisPropertiesConfigurationSP config = createConfig();
    config->setPropertyNotSaved("v3");
    testConfig(config);
    QString s = config->toXML();

    config = new KisPropertiesConfiguration();
    config->fromXML(s);
    QVERIFY(config->getInt("v1", 0) == v1);
    QVERIFY(config->getString("v2", QString()) == v2);
    QVERIFY(config->hasProperty("v3") == false);
    QVERIFY(config->getBool("v4", !v4) == v4);
    QVERIFY(config->getCubicCurve("v5") == v5);

}

void KisPropertiesConfigurationTest::testCopy()
{
    KisPropertiesConfigurationSP p1 = createConfig();
    p1->setProperty("v6", "bla");
    p1->setPropertyNotSaved("v6");
    KisPropertiesConfiguration config = KisPropertiesConfiguration(*p1.data());
    QVERIFY(config.getInt("v1", 0) == v1);
    QVERIFY(config.getString("v2", QString()) == v2);
    QVERIFY(config.getDouble("v3", 0.0) == v3);
    QVERIFY(config.getBool("v4", !v4) == v4);
    QVERIFY(config.getCubicCurve("v5") == v5);
    QVERIFY(config.hasProperty("v6") == true); // copying works!

    p1->setProperty("testBool1", true);
    p1->setProperty("testBool2", false);

    QString string = p1->toXML();

    KisPropertiesConfiguration p2;
    p2.fromXML(string);
    QVERIFY(p2.getInt("v1", 0) == v1);
    QVERIFY(p2.getString("v2", QString()) == v2);
    QVERIFY(p2.getDouble("v3", 0.0) == v3);
    QVERIFY(p2.getBool("v4", !v4) == v4);
    QVERIFY(p2.hasProperty("v6") == false); // round-tripping -- no
    QCOMPARE(p2.getBool("testBool1", false), true);
    QCOMPARE(p2.getBool("testBool2", true), false);
}

void KisPropertiesConfigurationTest::testGetColor()
{
    KisPropertiesConfiguration pc;
    KoColor kc = KoColor(QColor(Qt::red), KoColorSpaceRegistry::instance()->rgb8());
    QVariant c = QVariant::fromValue<KoColor>(kc);
    pc.setProperty("colorAsKoColor", c);
    pc.setProperty("colorAsQColor", QColor(Qt::red));
    pc.setProperty("colorAsString", "#FF0000");
    pc.setProperty("colorAsXML", "<!DOCTYPE color><color><RGB space=\"sRGB-elle-V2-g10.icc\" g=\"0\" b=\"0\" r=\"1\"/></color>");

    kc = pc.getColor("colorAsKoColor");
    QVERIFY(kc.toQColor() == QColor(Qt::red));
    kc = pc.getColor("colorAsQColor");
    QVERIFY(kc.toQColor() == QColor(Qt::red));
    kc = pc.getColor("colorAsString");
    QVERIFY(kc.toQColor() == QColor(Qt::red));
    kc = pc.getColor("colorAsXML");
    QVERIFY(kc.toQColor() == QColor(Qt::red));
}

void roundTripStringList(const QStringList &refList)
{
    KisPropertiesConfiguration config1;
    config1.setProperty("testProp", refList);

    const QString xmlData = config1.toXML();

    KisPropertiesConfiguration config2;
    config2.fromXML(xmlData);

    const QStringList results = config2.getStringList("testProp");

    QCOMPARE(results, refList);
}

void KisPropertiesConfigurationTest::testLists()
{
    const QString str1("str1 str2\\ str3/ str4; str5% str6& str7;; str8\\; str9]]> str10");
    const QString str2("str1 str2\\ str3/ str4; str5% str6& str7;; str8\\; str9]]> str10;");

    {
        const QStringList refList({str1, str1});
        roundTripStringList(refList);
    }

    {
        const QStringList refList({str2, str2});
        roundTripStringList(refList);
    }

    {
        const QStringList refList({"", str2, str2});
        roundTripStringList(refList);
    }

    {
        const QStringList refList({str2, str2, ""});
        roundTripStringList(refList);
    }

    {
        const QStringList refList({str2, str2, ";"});
        roundTripStringList(refList);
    }

    {
        const QStringList refList({";", str2, str2});
        roundTripStringList(refList);
    }
}

SIMPLE_TEST_MAIN(KisPropertiesConfigurationTest)

