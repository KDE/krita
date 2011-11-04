/*
 *  Copyright (c) 2010 Boudewijn Rempt boud@valdyas.org
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

#include "kis_mypaint_surface_test.h"

#include <qtest_kde.h>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include <kis_paint_device.h>
#include <kis_painter.h>

#include "../mypaint_surface.h"


void KisMyPaintSurfaceTest::testCreation()
{
    KisPaintDeviceSP src = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb16());
    KisPaintDeviceSP dst = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb16());

    KisPainter painter(dst);
    MyPaintSurface surface(src, &painter);

    Q_UNUSED(surface);
}

void KisMyPaintSurfaceTest::testDrawDab()
{
    KisPaintDeviceSP src = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb16());
    KisPaintDeviceSP dst = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb16());

    KisPainter painter(dst);
    MyPaintSurface surface(src, &painter);

    surface.draw_dab(500.0, 500.0, 100.0, 0.0, 0.0, 1.0, 1.0, 1.0);
    QImage result = dst->convertToQImage(0);
    QImage source(QString(FILES_DATA_DIR) + QDir::separator() + "draw_dab.png");

    QVERIFY(result == source);
}

void KisMyPaintSurfaceTest::testGetColor()
{
    KisPaintDeviceSP src = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb16());
    KisPaintDeviceSP dst = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb16());
    QImage source(QString(FILES_DATA_DIR) + QDir::separator() + "draw_dab.png");
    src->convertFromQImage(source, 0);
    src->move(400, 400);
    KisPainter painter(dst);
    MyPaintSurface surface(src, &painter);

    float r = 0.0;
    float g = 0.0;
    float b = 0.0;
    float a = 0.0;

    surface.get_color(500.0, 500.0, 100.0, &r, &g, &b, &a);

    QVERIFY(qFuzzyCompare((float)qRound(r), 0.0L));
    QVERIFY(qFuzzyCompare((float)qRound(g), 0.0L));
    QVERIFY(qFuzzyCompare(b, 1.0L));
    QVERIFY(qFuzzyCompare(a, 1.0L));
}

QTEST_KDEMAIN(KisMyPaintSurfaceTest, GUI)
#include "kis_mypaint_surface_test.moc"
