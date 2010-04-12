/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_filter_configuration_test.h"
#include <QApplication>
#include <qtest_kde.h>
#include <kis_debug.h>
#include <KoID.h>
#include <kis_paint_device.h>


#include "../filter/kis_filter_configuration.h"
#include "../filter/kis_filter_registry.h"
#include "../filter/kis_filter.h"

void KisFilterConfigurationTest::testCreation()
{
    KisFilterConfiguration * kfc = new KisFilterConfiguration("test", 1);
    QVERIFY2(kfc != 0,  "Could not create test filter configuration");
    QCOMPARE(kfc->version(), 1);
    QCOMPARE(kfc->name(), QString("test"));

    delete kfc;
}

void KisFilterConfigurationTest::testRoundTrip()
{
    KisFilterConfiguration * kfc = new KisFilterConfiguration("test", 1);
    QCOMPARE(kfc->version(), 1);
    QCOMPARE(kfc->name(), QString("test"));
    QString s = kfc->toXML();
    delete kfc;
    kfc = new KisFilterConfiguration("test2", 2);
    kfc->fromXML(s);
    QCOMPARE(kfc->version(), 1);
    delete kfc;
}

void KisFilterConfigurationTest::testSetGetProperty()
{
    KisFilterConfiguration * kfc = new KisFilterConfiguration("test", 1);
    kfc->setProperty("value1", 10);
    kfc->setProperty("value2", "foo");
    QCOMPARE(kfc->getInt("value1"), 10);
    QCOMPARE(kfc->getString("value2"), QString("foo"));
    QString s = kfc->toXML();
    delete kfc;
    kfc = new KisFilterConfiguration("test2", 2);
    kfc->fromXML(s);
    QCOMPARE(kfc->getInt("value1"), 10);
    QCOMPARE(kfc->getString("value2"), QString("foo"));
    delete kfc;
}


QTEST_KDEMAIN(KisFilterConfigurationTest, NoGUI)
#include "kis_filter_configuration_test.moc"
