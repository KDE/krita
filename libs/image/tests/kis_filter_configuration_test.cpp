/*
 *  SPDX-FileCopyrightText: 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_filter_configuration_test.h"
#include <QApplication>
#include <simpletest.h>
#include <kis_debug.h>
#include <KoID.h>
#include <kis_paint_device.h>


#include "../filter/kis_filter_configuration.h"
#include "../filter/kis_filter_registry.h"
#include "../filter/kis_filter.h"

#include <KisGlobalResourcesInterface.h>

void KisFilterConfigurationTest::testCreation()
{
    KisFilterConfigurationSP  kfc = new KisFilterConfiguration("test", 1, KisGlobalResourcesInterface::instance());
    QVERIFY2(kfc != 0,  "Could not create test filter configuration");
    QCOMPARE(kfc->version(), 1);
    QCOMPARE(kfc->name(), QString("test"));

}

void KisFilterConfigurationTest::testRoundTrip()
{
    KisFilterConfigurationSP  kfc = new KisFilterConfiguration("test", 1, KisGlobalResourcesInterface::instance());
    QCOMPARE(kfc->version(), 1);
    QCOMPARE(kfc->name(), QString("test"));
    QString s = kfc->toXML();

    kfc = new KisFilterConfiguration("test2", 2, KisGlobalResourcesInterface::instance());
    kfc->fromXML(s);
    QCOMPARE(kfc->version(), 1);
}

void KisFilterConfigurationTest::testSetGetProperty()
{
    KisFilterConfigurationSP  kfc = new KisFilterConfiguration("test", 1, KisGlobalResourcesInterface::instance());
    kfc->setProperty("value1", 10);
    kfc->setProperty("value2", "foo");
    QCOMPARE(kfc->getInt("value1"), 10);
    QCOMPARE(kfc->getString("value2"), QString("foo"));
    QString s = kfc->toXML();

    kfc = new KisFilterConfiguration("test2", 2, KisGlobalResourcesInterface::instance());
    kfc->fromXML(s);
    QCOMPARE(kfc->getInt("value1"), 10);
    QCOMPARE(kfc->getString("value2"), QString("foo"));
}


SIMPLE_TEST_MAIN(KisFilterConfigurationTest)
