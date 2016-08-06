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

#include "kis_brush_hud_properties_config_test.h"

#include <QTest>
#include "brushhud/kis_brush_hud_properties_config.h"
#include <QDomDocument>

#include "kis_debug.h"


void KisBrushHudPropertiesConfigTest::test()
{
    {
        KisBrushHudPropertiesConfig cfg;

        QList<QString> properties1;
        properties1 << "prop1";
        properties1 << "prop2";
        properties1 << "prop3";
        cfg.setSelectedProperties("brush1", properties1);

        QList<QString> properties2;
        properties2 << "prop4";
        properties2 << "prop5";
        properties2 << "prop6";
        cfg.setSelectedProperties("brush2", properties2);

        // {
        //     qDebug() << "===========================";
        //     QString str = cfg.testingGetDocument()->toString(4);
        //     printf(str.toLatin1());
        // }

        properties1.clear();
        properties1 << "prop7";
        properties1 << "prop8";
        cfg.setSelectedProperties("brush1", properties1);

        // {
        //     qDebug() << "===========================";
        //     QString str = cfg.testingGetDocument()->toString(4);
        //     printf(str.toLatin1());
        // }

        properties1.clear();
        properties1 = cfg.selectedProperties("brush1");

        QCOMPARE(properties1.size(), 2);
        QCOMPARE(properties1[0], QString("prop7"));
        QCOMPARE(properties1[1], QString("prop8"));
    }

    {
        KisBrushHudPropertiesConfig cfg;
        QList<QString> properties1;
        properties1 = cfg.selectedProperties("brush1");

        // qDebug() << "===========================";
        // qDebug() << ppVar(properties1);

        QCOMPARE(properties1.size(), 2);
        QCOMPARE(properties1[0], QString("prop7"));
        QCOMPARE(properties1[1], QString("prop8"));
    }
}

QTEST_MAIN(KisBrushHudPropertiesConfigTest)
