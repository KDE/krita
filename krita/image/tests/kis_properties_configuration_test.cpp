/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#include "kis_properties_configuration_test.h"


#include <qtest_kde.h>
#include "kis_properties_configuration.h"

KisPropertiesConfigurationTest::KisPropertiesConfigurationTest() :
        v1(10), v2("hello"), v3(1242.0), v4(true)
{
    QList<QPointF> pts; pts.push_back(QPointF(0.2, 0.3)); pts.push_back(QPointF(0.5, 0.7));
    v5.setPoints(pts);
}

void KisPropertiesConfigurationTest::testSerialization()
{
    KisPropertiesConfiguration* config = createConfig();
    QString xml = config->toXML();
    KisPropertiesConfiguration* decodedConfig = new KisPropertiesConfiguration();
    decodedConfig->fromXML(xml);
    testConfig(decodedConfig);
    delete config;
    delete decodedConfig;
}

void KisPropertiesConfigurationTest::testSetGet()
{
    KisPropertiesConfiguration* config = createConfig();
    testConfig(config);
    delete config;
}

void KisPropertiesConfigurationTest::testDefaultValues()
{
    KisPropertiesConfiguration* config = new KisPropertiesConfiguration();
    QVERIFY(config->getInt("bouh", v1) == v1);
    QVERIFY(config->getString("bouh", v2) == v2);
    QVERIFY(config->getDouble("bouh", v3) == v3);
    QVERIFY(config->getBool("bouh", v4) == v4);
    QVERIFY(config->getCubicCurve("bouh", v5) == v5);
    delete config;
}

KisPropertiesConfiguration* KisPropertiesConfigurationTest::createConfig()
{
    KisPropertiesConfiguration* config = new KisPropertiesConfiguration();
    config->setProperty("v1", v1);
    config->setProperty("v2", v2);
    config->setProperty("v3", v3);
    config->setProperty("v4", v4);
    config->setProperty("v5", qVariantFromValue(v5));
    return config;
}

void KisPropertiesConfigurationTest::testConfig(KisPropertiesConfiguration* config)
{
    QVERIFY(config->getInt("v1", 0) == v1);
    QVERIFY(config->getString("v2", "") == v2);
    QVERIFY(config->getDouble("v3", 0.0) == v3);
    QVERIFY(config->getBool("v4", !v4) == v4);
    QVERIFY(config->getCubicCurve("v5") == v5);
}

QTEST_KDEMAIN(KisPropertiesConfigurationTest, NoGUI)

#include "kis_properties_configuration_test.moc"
