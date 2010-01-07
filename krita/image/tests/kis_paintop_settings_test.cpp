/*
 *  Copyright (c) 2007 Boudewijn Rempt boud@valdyas.org
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

#include "kis_paintop_settings_test.h"

#include <qtest_kde.h>
#include "kis_paintop_settings.h"

void KisPaintopSettingsTest::testCreation()
{
//     KisPaintOpSettings test;
}

void KisPaintopSettingsTest::testClone()
{
    KisPaintOpSettings settings;
    settings.setProperty("paintop", "paintbrush");
    settings.setProperty("property1", 42);
    settings.setProperty("property2", "foo");
    settings.setProperty("property3", 3.1415);
    
    KisPaintOpSettingsSP settings2 = settings.clone();
    QVERIFY(settings2->getString("paintop") == "paintbrush");
    QVERIFY(settings2->getInt("property1", 0) == 42);
    QVERIFY(settings2->getString("property2", "") == "foo");
    QVERIFY(settings2->getDouble("property3", 0.0) == 3.1415);
}

QTEST_KDEMAIN(KisPaintopSettingsTest, GUI)
#include "kis_paintop_settings_test.moc"
