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

#include <kdebug.h>
#include <kunittest/runner.h>
#include <kunittest/module.h>

#include "kis_filter_configuration_tester.h"
#include "../kis_filter_configuration.h"

using namespace KUnitTest;

KUNITTEST_MODULE(kunittest_kis_filter_configuration_tester, "KisFilterConfiguration Tester");
KUNITTEST_MODULE_REGISTER_TESTER(KisFilterConfigurationTester);

void KisFilterConfigurationTester::allTests()
{
    testCreation();
    testSetGetProperty();
    testRoundTrip();
}

void KisFilterConfigurationTester::testCreation()
{
    KisFilterConfiguration * kfc = new KisFilterConfiguration("test", 1);
    if ( kfc == 0 ) failure("Could not create test filter configuration");
    CHECK(kfc->version(), 1);
    CHECK(kfc->name(), QString("test"));

    delete kfc;
    success("testCreation success");
}

void KisFilterConfigurationTester::testRoundTrip()
{
    KisFilterConfiguration * kfc = new KisFilterConfiguration("test", 1);
    CHECK(kfc->version(), 1);
    CHECK(kfc->name(), QString("test"));
    QString s = kfc->toString();
    delete kfc;
    kfc = new KisFilterConfiguration(s);
    CHECK(kfc->version(), 1);
    CHECK(kfc->name(), QString("test"));
    delete kfc;
    success("testDeserializaton success");
}

void KisFilterConfigurationTester::testSetGetProperty()
{
}
