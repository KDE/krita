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

#include <QApplication>
#include <qtest_kde.h>
#include <kdebug.h>
#include <kunittest/runner.h>
#include <kunittest/module.h>

#include "kis_filter_configuration_tester.h"
#include "../kis_filter_configuration.h"

void KisFilterConfigurationTester::testCreation()
{
    KisFilterConfiguration * kfc = new KisFilterConfiguration("test", 1);
    QVERIFY2( kfc != 0,  "Could not create test filter configuration");
    QCOMPARE(kfc->version(), 1);
    QCOMPARE(kfc->name(), QString("test"));

    delete kfc;
}

void KisFilterConfigurationTester::testRoundTrip()
{
    KisFilterConfiguration * kfc = new KisFilterConfiguration("test", 1);
    QCOMPARE(kfc->version(), 1);
    QCOMPARE(kfc->name(), QString("test"));
    QString s = kfc->toLegacyXML();
    delete kfc;
    kfc = new KisFilterConfiguration("test2", 2);
    kfc->fromLegacyXML( s );
    QCOMPARE(kfc->version(), 1);
    QCOMPARE(kfc->name(), QString("test"));
    delete kfc;
}

void KisFilterConfigurationTester::testSetGetProperty()
{
}


QTEST_KDEMAIN(KisFilterConfigurationTester, NoGUI)
#include "kis_filter_configuration_tester.moc"
