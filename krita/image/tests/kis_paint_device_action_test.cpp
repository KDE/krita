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

#include "kis_paint_device.h"
#include "kis_paint_device_action.h"
#include "kis_paint_device_action_test.h"

#include <qtest_kde.h>

class TestAction : public KisPaintDeviceAction
{
public:

    void act(KisPaintDeviceSP, qint32, qint32) const
        {
        }

    QString name() const
        {
            return "TestAction";
        }
    QString description() const
        {
            return "TestAction description";
        }
};


void KisPaintDeviceActionTest::testCreation()
{
    TestAction test;
}


QTEST_KDEMAIN(KisPaintDeviceActionTest, GUI)
#include "kis_paint_device_action_test.moc"
