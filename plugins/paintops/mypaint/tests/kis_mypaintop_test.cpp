/*
 *  SPDX-FileCopyrightText: 2020 Ashwin Dhakaita <ashwingpdhakaita@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <simpletest.h>
#include <QImageReader>
#include <QtTest/QtTest>
#include <qimage_based_test.h>

#include <kis_image.h>
#include <kis_node_facade.h>
#include <kis_group_layer.h>
#include <kis_paintop_preset.h>
#include <stroke_testing_utils.h>
#include <kis_paint_information.h>
#include <kis_random_accessor_ng.h>

#include "kis_mypaintop_test.h"
#include "MyPaintPaintOp.h"
#include "MyPaintSurface.h"
#include "MyPaintPaintOpSettings.h"

#include <qimage_test_util.h>

class KisMyPaintOpSettings;
KisMyPaintOpTest::KisMyPaintOpTest(): TestUtil::QImageBasedTest("MyPaintOp")
{

}

void KisMyPaintOpTest::testDab() {

    KisPaintDeviceSP dst = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    KisPainter painter(dst);

    mypaint_brush_new();

    QScopedPointer<KisMyPaintSurface> surface(new KisMyPaintSurface(&painter, dst));

    surface->draw_dab(surface->surface(), 250, 250, 100, 0, 0, 1, 1, 0.8, 1, 1, 90, 0, 0);

    QImage img = dst->convertToQImage(0, dst->exactBounds().x(), dst->exactBounds().y(), dst->exactBounds().width(), dst->exactBounds().height());
    QImage source(QString(FILES_DATA_DIR) + QDir::separator() + "draw_dab.png");

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, source, img)) {
        img.save("mypaint_test_draw_dab.png");
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }

}

void KisMyPaintOpTest::testGetColor() {

    KisPaintDeviceSP dst = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());

    QImage source(QString(FILES_DATA_DIR) + QDir::separator() + "draw_dab.png");
    dst->convertFromQImage(source, 0);

    KisPainter painter(dst);

    QScopedPointer<KisMyPaintSurface> surface(new KisMyPaintSurface(&painter, dst));

    surface->draw_dab(surface->surface(), 250, 250, 100, 0, 0, 1, 1, 0.8, 1, 1, 90, 0, 0);

    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 0.0f;

    surface->get_color(surface->surface(), 250, 250, 100, &r, &g, &b, &a);

    QVERIFY(qFuzzyCompare((float)qRound(r), 0.0L));
    QVERIFY(qFuzzyCompare((float)qRound(g), 0.0L));
    QVERIFY(qFuzzyCompare((float)qRound(b), 1.0L));
    QVERIFY(qFuzzyCompare((float)qRound(a), 1.0L));
}

void KisMyPaintOpTest::testLoading() {

    QScopedPointer<KisMyPaintPaintOpPreset> brush (new KisMyPaintPaintOpPreset(QString(FILES_DATA_DIR) + QDir::separator() + "basic.myb"));
    brush->load(0);
    QVERIFY(brush->valid());
}

SIMPLE_TEST_MAIN(KisMyPaintOpTest)
