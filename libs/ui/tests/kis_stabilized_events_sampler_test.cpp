/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_stabilized_events_sampler_test.h"

#include "kis_stabilized_events_sampler.h"
#include "kis_paint_information.h"

void KisStabilizedEventsSamplerTest::test()
{
    KisStabilizedEventsSampler sampler(20);

    KisPaintInformation pi1(QPoint(10,10));
    KisPaintInformation pi2(QPoint(20,20));

    sampler.addEvent(pi1);

    QTest::qSleep(50);

    sampler.addEvent(pi2);

    QTest::qSleep(70);

    KisStabilizedEventsSampler::iterator it;
    KisStabilizedEventsSampler::iterator end;
    std::tie(it, end) = sampler.range();


    int numTotal = 0;
    int num1 = 0;
    int num2 = 0;

    for (; it != end; ++it) {
        numTotal++;
        if (it->pos().x() == 10) {
            num1++;
        } else if (it->pos().x() == 20) {
            num2++;
        }

        qDebug() << ppVar(it->pos());
    }

    QVERIFY(numTotal >= 6);
    QVERIFY(num1 >= 3);
    QVERIFY(num2 >= 3);
}

QTEST_MAIN(KisStabilizedEventsSamplerTest)
