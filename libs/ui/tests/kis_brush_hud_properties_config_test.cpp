/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
